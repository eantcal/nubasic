//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_os_console.h"
#include "nu_unicode.h"
#include <cstdio>
#include <iostream>
#include <string>

/* -------------------------------------------------------------------------- */

#ifdef _WIN32


/* -------------------------------------------------------------------------- */

#include <windows.h>
#include <conio.h>

#include <locale.h>

#include "nu_winconsole_api.h"

/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

// Screen mode:
//   0 = text/hybrid — I/O via real Windows console (headless-safe, for tests)
//   1 = GDI console  — I/O and graphics through the custom GDI window (default)
static int g_screen_mode = 1;

void _os_set_screen_mode(int mode) { g_screen_mode = mode; }
int  _os_get_screen_mode()         { return g_screen_mode; }


/* -------------------------------------------------------------------------- */

void _os_init()
{
    // do nothing
    //_setmode(_fileno(stderr), _O_U16TEXT);
    setlocale(LC_CTYPE, ".932");
}


/* -------------------------------------------------------------------------- */

void _os_u16write(const std::u16string& output)
{
    if (g_screen_mode == 0) {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD written = 0;
        WriteConsoleW(hOut, output.c_str(),
            static_cast<DWORD>(output.size()), &written, nullptr);
    } else {
        nu_winconsole_write_w((const wchar_t*)output.c_str());
    }
}


/* -------------------------------------------------------------------------- */

void _os_config_term(bool echo_mode)
{
    // do nothing
}


/* -------------------------------------------------------------------------- */

void _os_cls()
{
    if (g_screen_mode == 0) {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
            DWORD cells = csbi.dwSize.X * csbi.dwSize.Y;
            DWORD written = 0;
            COORD origin = { 0, 0 };
            FillConsoleOutputCharacterA(hOut, ' ', cells, origin, &written);
            FillConsoleOutputAttribute(
                hOut, csbi.wAttributes, cells, origin, &written);
            SetConsoleCursorPosition(hOut, origin);
        }
    } else {
        nu_winconsole_cls();
    }
}


/* -------------------------------------------------------------------------- */

void _os_locate(int y, int x)
{
    if (g_screen_mode == 0) {
        COORD pos = { static_cast<SHORT>(x - 1), static_cast<SHORT>(y - 1) };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
    } else {
        nu_winconsole_locate(x - 1, y - 1);
    }
}


/* -------------------------------------------------------------------------- */

std::string _os_input_str(int n)
{
    if (g_screen_mode == 0) {
        std::string result;
        result.reserve(n);
        for (int i = 0; i < n; ++i) {
            int ch = _getch();
            if (ch == 0x03) break; // ETX / Ctrl+C
            if (ch > 0 && ch < 256)
                result += static_cast<char>(ch & 0xff);
        }
        return result;
    }

    std::string result;
    for (int i = 0; i < n; ++i) {
        while (!nu_winconsole_key_available()) {
            nu_winconsole_process_messages();
            Sleep(10);
        }
        int key = nu_winconsole_get_key();
        if (key == 0x03)
            break; // ETX
        if (key > 0 && key < 256) {
            result += (char)(key & 0xff);
        }
    }
    return result;
}


/* -------------------------------------------------------------------------- */

// Implements INPUT semantic
std::string _os_input(FILE* finput_ptr)
{
    if (finput_ptr == stdin) {
        if (g_screen_mode == 0) {
            char buffer[4096];
            if (fgets(buffer, sizeof(buffer), stdin)) {
                std::string s(buffer);
                if (!s.empty() && s.back() == '\n')
                    s.pop_back();
                return s;
            }
            return "";
        }
        char buffer[4096];
        int len = nu_winconsole_read_line(buffer, sizeof(buffer));
        if (len > 0)
            return std::string(buffer);
        return "";
    }

    std::string s;
    char c;

    do {
        c = getc(finput_ptr);

        if (feof(finput_ptr) || ferror(finput_ptr))
            return s;

        if (c != '\n')
            s += c;

    } while (c != '\n');

    return s;
}


/* -------------------------------------------------------------------------- */

int _os_kbhit()
{
    if (g_screen_mode == 0)
        return _kbhit() ? _getch() : 0;
    return nu_winconsole_key_available() ? nu_winconsole_get_key() : 0;
}


/* -------------------------------------------------------------------------- */

void _os_cursor_visible(bool on)
{
    if (g_screen_mode == 0) {
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO ci = { 25, on ? TRUE : FALSE };
        SetConsoleCursorInfo(hOut, &ci);
    } else {
        nu_winconsole_cursor_visible(on ? 1 : 0);
    }
}


/* -------------------------------------------------------------------------- */

void _os_screenlock()
{
    if (g_screen_mode != 0)
        nu_winconsole_screenlock();
}


/* -------------------------------------------------------------------------- */

void _os_screenunlock()
{
    if (g_screen_mode != 0)
        nu_winconsole_screenunlock();
}


/* -------------------------------------------------------------------------- */

void _os_refresh()
{
    if (g_screen_mode != 0)
        nu_winconsole_refresh();
}


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#else /*--------------- LINUX / MAC                                            \
         ------------------------------------------*/
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
    save_tty_settings_t()
    {
        _old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        tcgetattr(STDIN_FILENO, &_tty_set); // grab old terminal I/O settings
        _tty_set.c_lflag &= ~ICANON;
        tcsetattr(STDIN_FILENO, TCSANOW, &_tty_set);
    }

    ~save_tty_settings_t() { fcntl(STDIN_FILENO, F_SETFL, _old_flags); }
};


/* -------------------------------------------------------------------------- */

void _os_init()
{
    static save_tty_settings_t _init_terminal;
    setlocale(LC_CTYPE, "");
}


/* -------------------------------------------------------------------------- */

void _os_u16write(const std::u16string& output)
{
    std::string bs = nu::u16_to_utf8(output);
    printf("%s", bs.c_str());
}


/* -------------------------------------------------------------------------- */

void _os_cls()
{
    int res = system("clear");
    (void)res;
}


/* -------------------------------------------------------------------------- */

void _os_locate(int y, int x) { printf("%c[%d;%df", 0x1B, y, x); }


/* -------------------------------------------------------------------------- */

void _os_cursor_visible(bool on) { printf("%c[?25%c", 0x1B, !on ? 'l' : 'h'); }


/* -------------------------------------------------------------------------- */

// Implements INPUT$(x) semantic
std::string _os_input_str(int n)
{
    std::string ret;
    std::string line;

    while (n > 0) {
        terminal_input_t ti;
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
                : ti()
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
    } while (c != '\n');

    return s;
}


/* -------------------------------------------------------------------------- */

int _os_kbhit()
{
    terminal_input_t ti;
    int c = 0;

    while (ti.keybhit()) {
        c = fgetc(stdin);
    }

    return c;
}

/* -------------------------------------------------------------------------- */

// No GDI back-buffer on non-Windows: these are intentional no-ops.
void _os_screenlock() {}
void _os_screenunlock() {}
void _os_refresh() {}

// Screen mode is a Windows-only concept; on Linux/macOS always text mode.
static int g_screen_mode = 0;
void _os_set_screen_mode(int mode) { g_screen_mode = mode; }
int  _os_get_screen_mode()         { return g_screen_mode; }

/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif
