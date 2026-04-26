//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_os_console.h"
#include "nu_signal_handling.h"
#include "nu_unicode.h"
#include <cstdio>
#include <iostream>
#include <string>

/* -------------------------------------------------------------------------- */

#ifdef _WIN32


/* -------------------------------------------------------------------------- */

#include <conio.h>
#include <windows.h>

#include <locale.h>

#include "nu_winconsole_api.h"

/* -------------------------------------------------------------------------- */

namespace nu {

namespace {

    class input_break_listener_t : public signal_handler_t {
    public:
        input_break_listener_t()
        {
            signal_mgr_t::instance().register_handler(event_t::BREAK, this);
        }

        ~input_break_listener_t() override
        {
            signal_mgr_t::instance().unregister_handler(event_t::BREAK, this);
        }

        bool notify(const event_t& ev) override
        {
            _interrupted = ev == event_t::BREAK;
            return true;
        }

        bool interrupted() const noexcept { return _interrupted; }

    private:
        volatile bool _interrupted = false;
    };

    static void append_input_char(std::string& line, int ch)
    {
        line += static_cast<char>(ch & 0xff);
    }

    static void erase_last_input_char(std::string& line)
    {
        if (!line.empty())
            line.pop_back();
    }

    static os_input_result_t read_stream_line(FILE* finput_ptr)
    {
        os_input_result_t result;
        char c = 0;

        do {
            c = getc(finput_ptr);

            if (feof(finput_ptr) || ferror(finput_ptr))
                return result;

            if (c != '\n')
                result.text += c;

        } while (c != '\n');

        return result;
    }

} // namespace

/* -------------------------------------------------------------------------- */

// Screen mode:
//   0 = text/hybrid — I/O via real Windows console (headless-safe, for tests)
//   1 = GDI console  — I/O and graphics through the custom GDI window (default)
static int g_screen_mode = 1;

void _os_set_screen_mode(int mode) { g_screen_mode = mode; }
int _os_get_screen_mode() { return g_screen_mode; }


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
        // Text mode: use standard printf so output works on any handle type
        // (real console, ConPTY pipe, redirect to file, etc.).
        const std::string utf8 = nu::u16_to_utf8(output);
        printf("%s", utf8.c_str());
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

std::string _os_input_str(int n) { return _os_input_str_interruptible(n).text; }


/* -------------------------------------------------------------------------- */

os_input_result_t _os_input_str_interruptible(int n)
{
    if (g_screen_mode == 0) {
        input_break_listener_t break_listener;
        os_input_result_t result;
        result.text.reserve(n);
        const HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        const DWORD file_type = hIn && hIn != INVALID_HANDLE_VALUE
            ? GetFileType(hIn)
            : FILE_TYPE_UNKNOWN;

        if (file_type == FILE_TYPE_PIPE) {
            for (int i = 0; i < n && !break_listener.interrupted();) {
                DWORD available = 0;
                if (!PeekNamedPipe(
                        hIn, nullptr, 0, nullptr, &available, nullptr)) {
                    return result;
                }

                if (available == 0) {
                    Sleep(10);
                    continue;
                }

                char ch = 0;
                DWORD read = 0;
                if (!ReadFile(hIn, &ch, 1, &read, nullptr) || read == 0)
                    return result;

                if (ch == 0x03)
                    return { result.text, true };

                result.text += ch;
                ++i;
            }

            return break_listener.interrupted()
                ? os_input_result_t{ result.text, true }
                : result;
        }

        if (file_type == FILE_TYPE_DISK) {
            for (int i = 0; i < n && !break_listener.interrupted();) {
                const int ch = fgetc(stdin);
                if (ch == EOF)
                    return result;

                if (ch == 0x03)
                    return { result.text, true };

                result.text += static_cast<char>(ch & 0xff);
                ++i;
            }

            return break_listener.interrupted()
                ? os_input_result_t{ result.text, true }
                : result;
        }

        for (int i = 0; i < n && !break_listener.interrupted(); ++i) {
            const int ch = _getch();
            if (ch == 0x03)
                return { result.text, true };
            if (ch > 0 && ch < 256)
                result.text += static_cast<char>(ch & 0xff);
        }
        return result;
    }

    input_break_listener_t break_listener;
    os_input_result_t result;
    result.text.reserve(n);
    for (int i = 0; i < n && !break_listener.interrupted(); ++i) {
        while (!nu_winconsole_key_available()) {
            if (break_listener.interrupted())
                return { result.text, true };
            nu_winconsole_process_messages();
            Sleep(10);
        }
        int key = nu_winconsole_get_key();
        if (key == 0x03)
            return { result.text, true };
        if (key > 0 && key < 256) {
            result.text += (char)(key & 0xff);
        }
    }
    return break_listener.interrupted() ? os_input_result_t{ result.text, true }
                                        : result;
}


/* -------------------------------------------------------------------------- */

// Implements INPUT semantic
std::string _os_input(FILE* finput_ptr)
{
    return _os_input_interruptible(finput_ptr).text;
}


/* -------------------------------------------------------------------------- */

os_input_result_t _os_input_interruptible(FILE* finput_ptr)
{
    if (finput_ptr != stdin)
        return read_stream_line(finput_ptr);

    input_break_listener_t break_listener;

    if (g_screen_mode == 0) {
        const HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
        const DWORD file_type = hIn && hIn != INVALID_HANDLE_VALUE
            ? GetFileType(hIn)
            : FILE_TYPE_UNKNOWN;

        if (file_type == FILE_TYPE_PIPE) {
            os_input_result_t result;

            while (!break_listener.interrupted()) {
                DWORD available = 0;
                if (!PeekNamedPipe(
                        hIn, nullptr, 0, nullptr, &available, nullptr)) {
                    return result;
                }

                if (available == 0) {
                    Sleep(10);
                    continue;
                }

                char ch = 0;
                DWORD read = 0;
                if (!ReadFile(hIn, &ch, 1, &read, nullptr) || read == 0)
                    return result;

                if (ch == 0x03)
                    return { result.text, true };

                if (ch == '\n')
                    return result;

                if (ch != '\r')
                    result.text += ch;
            }

            return { result.text, true };
        }

        if (file_type == FILE_TYPE_CHAR) {
            os_input_result_t result;

            while (!break_listener.interrupted()) {
                if (!_kbhit()) {
                    Sleep(10);
                    continue;
                }

                const int ch = _getch();
                if (ch == 0x03)
                    return { result.text, true };

                if (ch == '\r') {
                    printf("\n");
                    fflush(stdout);
                    return result;
                }

                if (ch == '\b') {
                    if (!result.text.empty()) {
                        erase_last_input_char(result.text);
                        printf("\b \b");
                        fflush(stdout);
                    }
                    continue;
                }

                if (ch >= 0x20 && ch < 0x100) {
                    append_input_char(result.text, ch);
                    printf("%c", ch & 0xff);
                    fflush(stdout);
                }
            }

            return { result.text, true };
        }

        if (file_type == FILE_TYPE_DISK) {
            os_input_result_t result;

            while (!break_listener.interrupted()) {
                const int ch = fgetc(stdin);
                if (ch == EOF)
                    return result;

                if (ch == 0x03)
                    return { result.text, true };

                if (ch == '\n')
                    return result;

                if (ch != '\r')
                    result.text += static_cast<char>(ch & 0xff);
            }

            return { result.text, true };
        }
    } else {
        os_input_result_t result;

        while (!break_listener.interrupted()) {
            while (!nu_winconsole_key_available()) {
                if (break_listener.interrupted())
                    return { result.text, true };

                nu_winconsole_process_messages();
                Sleep(10);
            }

            const int key = nu_winconsole_get_key();
            if (key == 0x03)
                return { result.text, true };

            if (key == '\r' || key == '\n') {
                nu_winconsole_write("\n");
                return result;
            }

            if (key == '\b') {
                if (!result.text.empty()) {
                    erase_last_input_char(result.text);
                    nu_winconsole_write("\b \b");
                }
                continue;
            }

            if (key >= 0x20 && key < 0x100) {
                append_input_char(result.text, key);
                char text[2] = { static_cast<char>(key & 0xff), '\0' };
                nu_winconsole_write(text);
            }
        }

        return { result.text, true };
    }

    return read_stream_line(finput_ptr);
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

#else /*--------------- LINUX / MAC \                                                                             \
         ------------------------------------------*/
/* -------------------------------------------------------------------------- */


#include "nu_terminal.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/select.h>
#ifdef __linux__
#include <stdio_ext.h>
#endif
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

namespace nu {

namespace {

    class input_break_listener_t : public signal_handler_t {
    public:
        input_break_listener_t()
        {
            signal_mgr_t::instance().register_handler(event_t::BREAK, this);
        }

        ~input_break_listener_t() override
        {
            signal_mgr_t::instance().unregister_handler(event_t::BREAK, this);
        }

        bool notify(const event_t& ev) override
        {
            _interrupted = ev == event_t::BREAK;
            return true;
        }

        bool interrupted() const noexcept { return _interrupted; }

    private:
        volatile bool _interrupted = false;
    };

    static void append_input_char(std::string& line, int ch)
    {
        line += static_cast<char>(ch & 0xff);
    }

    static void erase_last_input_char(std::string& line)
    {
        if (!line.empty())
            line.pop_back();
    }

    static os_input_result_t read_stream_line(FILE* finput_ptr)
    {
        os_input_result_t result;
        char c = 0;

        do {
            c = fgetc(finput_ptr);

            if (feof(finput_ptr) || ferror(finput_ptr))
                return result;

            if (c != '\n')
                result.text += c;
        } while (c != '\n');

        return result;
    }

    static bool poll_input_ready(int fd, int timeout_ms)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        const int ret = ::select(fd + 1, &fds, nullptr, nullptr, &tv);
        return ret > 0 && FD_ISSET(fd, &fds);
    }

} // namespace


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
std::string _os_input_str(int n) { return _os_input_str_interruptible(n).text; }


/* -------------------------------------------------------------------------- */

os_input_result_t _os_input_str_interruptible(int n)
{
    input_break_listener_t break_listener;
    os_input_result_t result;
    result.text.reserve(n > 0 ? n : 0);

    while (n > 0 && !break_listener.interrupted()) {
        if (!poll_input_ready(STDIN_FILENO, 50))
            continue;

        char ch = 0;
        const auto read_count = ::read(STDIN_FILENO, &ch, sizeof(ch));
        if (read_count <= 0)
            return result;

        if (ch == 0x03)
            return { result.text, true };

        result.text += ch;
        --n;
    }

    return break_listener.interrupted() ? os_input_result_t{ result.text, true }
                                        : result;
}


/* -------------------------------------------------------------------------- */

// Implements INPUT semantic
std::string _os_input(FILE* finput_ptr)
{
    return _os_input_interruptible(finput_ptr).text;
}


/* -------------------------------------------------------------------------- */

os_input_result_t _os_input_interruptible(FILE* finput_ptr)
{
    if (finput_ptr != stdin)
        return read_stream_line(finput_ptr);

    input_break_listener_t break_listener;
    os_input_result_t result;
    const int fd = fileno(stdin);

    while (!break_listener.interrupted()) {
        if (!poll_input_ready(fd, 50))
            continue;

        char ch = 0;
        const auto read_count = ::read(fd, &ch, sizeof(ch));
        if (read_count <= 0)
            return result;

        if (ch == '\n' || ch == '\r') {
            printf("\n");
            fflush(stdout);
            return result;
        }

        if (ch == 0x03) {
            return { result.text, true };
        }

        if (ch == terminal_input_t::CTRL_D) {
            return result;
        }

        if (ch == terminal_input_t::BACKSPACE
            || ch == terminal_input_t::CTRL_H) {
            if (!result.text.empty()) {
                erase_last_input_char(result.text);
                printf("\b \b");
                fflush(stdout);
            }
            continue;
        }

        if (ch >= 0x20 && ch < 0x7f) {
            append_input_char(result.text, ch);
            printf("%c", ch);
            fflush(stdout);
        }
    }

    return { result.text, true };
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
int _os_get_screen_mode() { return g_screen_mode; }

/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif
