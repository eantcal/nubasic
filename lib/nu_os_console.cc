//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_os_console.h"
#include <cstdio>
#include <functional>

/* -------------------------------------------------------------------------- */

#ifdef _WIN32


/* -------------------------------------------------------------------------- */

#include <conio.h>
#include <windows.h>


/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

static std::string _input_str(int n, std::function<int()> _getch_f)
{
    if (n <= 1) {
        n = 1;
    }

    std::string ret;

    for (int i = 0; i < n; ++i) {
        int c = _getch_f();

        if (c == 0x03 /* ETX */) {
            break;
        }

        ret += (c & 0xff);
    }

    return ret;
}


/* -------------------------------------------------------------------------- */

void _os_init()
{
    // do nothing
}


/* -------------------------------------------------------------------------- */

void _os_config_term(bool echo_mode)
{
    // do nothing
}


/* -------------------------------------------------------------------------- */

void _os_cls()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coordScreen = { 0, 0 };
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    // Get the number of character cells in the current buffer.
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill the entire screen with blanks.
    if (!FillConsoleOutputCharacter(hConsole, // Handle to console screen buffer
            (TCHAR)' ', // Character to write to the buffer
            dwConSize, // Number of cells to write
            coordScreen, // Coordinates of first cell
            &cCharsWritten)) // Receive number of characters written
    {
        return;
    }

    // Get the current text attribute.
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    // Set the buffer's attributes accordingly.
    if (!FillConsoleOutputAttribute(hConsole, // Handle to console screen buffer
            csbi.wAttributes, // Character attributes to use
            dwConSize, // Number of cells to set attribute
            coordScreen, // Coordinates of first cell
            &cCharsWritten)) // Receive number of characters written
    {
        return;
    }

    // Put the cursor at its home coordinates.
    SetConsoleCursorPosition(hConsole, coordScreen);
}


/* -------------------------------------------------------------------------- */

void _os_locate(int y, int x)
{
    COORD c = { short((x - 1) & 0xffff), short((y - 1) & 0xffff) };
    ::SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}


/* -------------------------------------------------------------------------- */

std::string _os_input_str(int n)
{
    // Implements INPUT$(x) semantic
    return _input_str(n, _getch);
}


/* -------------------------------------------------------------------------- */

// Implements INPUT semantic
std::string _os_input(FILE* finput_ptr)
{
    std::string s;
    char c;

    do {
        c = getc(finput_ptr);

        if (finput_ptr != stdin) {
            if (feof(finput_ptr) || ferror(finput_ptr))
                return s;
        }

        if (c != '\n') {
            s += c;
        }

    } 
    while (c != '\n');

    return s;
}


/* -------------------------------------------------------------------------- */

int _os_kbhit() 
{ 
    return _kbhit() ? _getch() : 0; 
}


/* -------------------------------------------------------------------------- */

static struct _cursor_info_t {
    _cursor_info_t() {
        hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleCursorInfo(hConsoleOutput, &structCursorInfo);
    }

    ~_cursor_info_t() {
        SetConsoleCursorInfo(hConsoleOutput, &structCursorInfo);
    }

    void set_cur_state(bool on) {
        CONSOLE_CURSOR_INFO structCursorInfo_ = { 0 };
        structCursorInfo_.bVisible = on ? TRUE : FALSE;
        structCursorInfo_.dwSize = on ? structCursorInfo.dwSize : 1;
        SetConsoleCursorInfo(hConsoleOutput, &structCursorInfo_);
    }

    CONSOLE_CURSOR_INFO structCursorInfo;
    HANDLE hConsoleOutput;

} _cur_info;


/* -------------------------------------------------------------------------- */

void _os_cursor_visible(bool on) 
{ 
    _cur_info.set_cur_state(on); 
}


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#else /*--------------- LINUX / MAC ------------------------------------------*/
/* -------------------------------------------------------------------------- */


#include "nu_terminal.h"

#include <fcntl.h>
#include <stdio.h>
#ifdef __linux__
#include <stdio_ext.h>
#endif
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

namespace nu {


/* -------------------------------------------------------------------------- */

void _os_config_term(bool echo_mode)
{
    struct termios tty_set;

    tcgetattr(STDIN_FILENO, &tty_set); // grab old terminal I/O settings

    tty_set.c_cc[VMIN] = 1;
    tty_set.c_cc[VTIME] = 0;

    if (echo_mode) {
        tty_set.c_lflag |= ECHO | ICANON;
    } else {
        tty_set.c_lflag &= ~ECHO;
        tty_set.c_lflag &= ~ICANON;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &tty_set);

    tty_set.c_cc[VMIN] = 1;
    tty_set.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &tty_set);
}


/* -------------------------------------------------------------------------- */

class save_tty_settings_t {
private:
    int _old_flags = 0;
    struct termios _tty_set;

public:
    save_tty_settings_t() {
        _old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        tcgetattr(STDIN_FILENO, &_tty_set); // grab old terminal I/O settings
        _tty_set.c_lflag &= ~ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &_tty_set);
    }

    ~save_tty_settings_t() { 
        fcntl(STDIN_FILENO, F_SETFL, _old_flags); 
    }
};


/* -------------------------------------------------------------------------- */

void _os_init() 
{ 
    static save_tty_settings_t _init_terminal; 
}


/* -------------------------------------------------------------------------- */

void _os_cls()
{
    int res = system("clear");
    (void)res;
}


/* -------------------------------------------------------------------------- */

void _os_locate(int y, int x) 
{ 
    printf("%c[%d;%df", 0x1B, y, x); 
}


/* -------------------------------------------------------------------------- */

void _os_cursor_visible(bool on) 
{ 
    printf("%c[?25%c", 0x1B, !on ? 'l' : 'h'); 
}


/* -------------------------------------------------------------------------- */

// Implements INPUT$(x) semantic
std::string _os_input_str(int n)
{
    std::string ret;
    std::string line;

    while (n > 0) {
        terminal_input_t ti(true);
        terminal_t terminal(ti);
        terminal.set_max_line_length(n);
        terminal.register_eol_ch(nu::terminal_input_t::CR);
        terminal.register_eol_ch(nu::terminal_input_t::LF);
        terminal.set_flags(terminal_t::ECHO_DIS);
        terminal.get_line(line, true /* return if len(line) == n */);

        if (line.empty()) {
            line = "\n";
        }

        ret += line;
        n -= line.size();
        line.clear();
    }

    ret += line;

    return ret;
}


/* -------------------------------------------------------------------------- */

// Implements INPUT semantic
std::string _os_input(FILE* finput_ptr)
{
    if (finput_ptr == stdin) {
        std::string line;

        struct _term {
            terminal_input_t ti;
            terminal_t terminal;

            _term()
                : ti(true)
                , terminal(ti)
            {
                terminal.register_eol_ch(nu::terminal_input_t::CR);
                terminal.register_eol_ch(nu::terminal_input_t::LF);
                terminal.set_insert_enabled(true);
            }
        };

        static _term term;

        term.terminal.get_line(line);
        printf("\n");
        fflush(stdout);

        return line;
    }

    std::string s;
    char c;

    do {
        c = fgetc(finput_ptr);

        if (feof(finput_ptr) || ferror(finput_ptr))
            return s;

        if (c != '\n')
            s += c;
    } 
    while (c != '\n');

    return s;
}


/* -------------------------------------------------------------------------- */

int _os_kbhit()
{
    terminal_input_t ti(true);
    int c = 0;

    while (ti.keybhit()) {
        c = fgetc(stdin);
    }

    return c;
}

/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif
