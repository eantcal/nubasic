//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_winconsole_api.h"
#include "ConsoleWindow.h"
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <windows.h>

/* -------------------------------------------------------------------------- */

static console_window_t* g_console = nullptr;
static bool g_running = false;

/* -------------------------------------------------------------------------- */

static std::wstring utf8_to_wstring(const char* utf8)
{
    if (!utf8 || !*utf8)
        return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, nullptr, 0);
    if (n <= 0)
        return L"";
    std::wstring ws(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, &ws[0], n);
    if (!ws.empty() && ws.back() == L'\0')
        ws.pop_back();
    return ws;
}

/* -------------------------------------------------------------------------- */

static std::string wstring_to_utf8(const wchar_t* ws)
{
    if (!ws || !*ws)
        return "";
    int n
        = WideCharToMultiByte(CP_UTF8, 0, ws, -1, nullptr, 0, nullptr, nullptr);
    if (n <= 0)
        return "";
    std::string s(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws, -1, &s[0], n, nullptr, nullptr);
    if (!s.empty() && s.back() == '\0')
        s.pop_back();
    return s;
}

/* -------------------------------------------------------------------------- */

int nu_winconsole_init(void* hInstance, int nCmdShow)
{
    if (g_console)
        return 1; // Already initialized

    try {
        g_console = new console_window_t((HINSTANCE)hInstance);

        console_config_t config;
        config.cols = 80;
        config.rows = 25;
        config.scrollback_rows = 10000;
        config.font_name = L"Consolas";
        config.font_size = 16;
        config.text_color = RGB(192, 192, 192);
        config.background_color = RGB(0, 0, 0);
        config.cursor_color = RGB(255, 255, 255);
        config.cursor_blink = true;
        config.tab_width = 8;
        config.auto_scroll = true;

        g_console->set_config(config);

        if (!g_console->create(L"nuBASIC CLI")) {
            delete g_console;
            g_console = nullptr;
            return 0;
        }

        g_console->show(nCmdShow);
        g_console->refresh(true); // force first paint
        g_running = true;
        return 1;
    } catch (...) {
        delete g_console;
        g_console = nullptr;
        return 0;
    }
}

/* -------------------------------------------------------------------------- */

int nu_winconsole_is_active() { return g_console ? 1 : 0; }

/* -------------------------------------------------------------------------- */

void nu_winconsole_shutdown()
{
    g_running = false;
    delete g_console;
    g_console = nullptr;
}

/* -------------------------------------------------------------------------- */

int nu_winconsole_process_messages()
{
    if (!g_console)
        return 0;

    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            g_running = false;
            return 0;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return g_running ? 1 : 0;
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_write(const char* text)
{
    if (!g_console || !text)
        return;
    g_console->get_buffer().write(utf8_to_wstring(text));
    g_console->refresh();
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_write_w(const wchar_t* text)
{
    if (!g_console || !text)
        return;
    g_console->get_buffer().write(std::wstring(text));
    g_console->refresh();
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_printf(const char* format, ...)
{
    if (!g_console || !format)
        return;
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    nu_winconsole_write(buffer);
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_wprintf(const wchar_t* format, ...)
{
    if (!g_console || !format)
        return;
    va_list args;
    va_start(args, format);
    wchar_t buffer[4096];
    vswprintf(buffer, sizeof(buffer) / sizeof(wchar_t), format, args);
    va_end(args);
    nu_winconsole_write_w(buffer);
}

/* -------------------------------------------------------------------------- */

int nu_winconsole_read_line(char* buffer, int size)
{
    if (!g_console || !buffer || size <= 0)
        return -1;

    // read_line() pumps the message loop internally — no deadlock
    std::wstring line = g_console->read_line();
    std::string utf8 = wstring_to_utf8(line.c_str());

    int len = (int)utf8.length();
    if (len >= size)
        len = size - 1;
    memcpy(buffer, utf8.c_str(), len);
    buffer[len] = '\0';
    return len;
}

/* -------------------------------------------------------------------------- */

int nu_winconsole_read_line_w(wchar_t* buffer, int size)
{
    if (!g_console || !buffer || size <= 0)
        return -1;

    std::wstring line = g_console->read_line();

    int len = (int)line.length();
    if (len >= size)
        len = size - 1;
    wcsncpy(buffer, line.c_str(), len);
    buffer[len] = L'\0';
    return len;
}

/* -------------------------------------------------------------------------- */

int nu_winconsole_get_key()
{
    if (!g_console)
        return 0;
    std::wstring key = g_console->get_key();
    return key.empty() ? 0 : (int)key[0];
}

/* -------------------------------------------------------------------------- */

int nu_winconsole_get_vkey()
{
    if (!g_console)
        return 0;
    return g_console->get_vkey();
}

/* -------------------------------------------------------------------------- */

int nu_winconsole_vkey_available()
{
    if (!g_console)
        return 0;
    return g_console->vkey_available() ? 1 : 0;
}

/* -------------------------------------------------------------------------- */

int nu_winconsole_key_available()
{
    if (!g_console)
        return 0;
    return g_console->is_key_available() ? 1 : 0;
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_cls()
{
    if (!g_console)
        return;
    g_console->get_buffer().clear();
    g_console->clear_backbuffer();
    g_console->refresh(true); // force immediate visual clear
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_locate(int x, int y)
{
    if (!g_console)
        return;
    // x, y are 0-based screen coordinates (passed from _os_locate which
    // already subtracts 1 from the 1-based BASIC coordinates).
    g_console->get_buffer().locate_screen(x, y);
    g_console->refresh();
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_cursor_visible(int visible)
{
    if (!g_console)
        return;
    g_console->set_cursor_visible(visible != 0);
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_refresh()
{
    if (!g_console)
        return;
    g_console->refresh(true); // explicit user call — force immediate
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_screenlock()
{
    if (g_console)
        g_console->lock_rendering();
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_screenunlock()
{
    if (g_console)
        g_console->unlock_rendering();
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_cancel_input()
{
    if (!g_console)
        return;
    g_console->cancel_input();
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_set_mouse_text_selection_enabled(int enabled)
{
    if (!g_console)
        return;
    g_console->set_mouse_text_selection_enabled(enabled != 0);
}

/* -------------------------------------------------------------------------- */

void* nu_winconsole_get_hwnd()
{
    return g_console ? (void*)g_console->get_hwnd() : nullptr;
}

/* -------------------------------------------------------------------------- */

void* nu_winconsole_get_hdc()
{
    if (!g_console)
        return nullptr;
    return (void*)g_console->get_offscreen_dc();
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_release_hdc(void* hdc)
{
    (void)hdc;
    if (g_console)
        g_console->release_offscreen_dc();
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_set_exit_on_close(int enabled)
{
    if (g_console)
        g_console->set_exit_on_close(enabled != 0);
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_set_close_callback(void (*fn)())
{
    if (g_console)
        g_console->set_close_callback(fn);
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_set_ctrlc_callback(void (*fn)())
{
    if (g_console)
        g_console->set_ctrlc_callback(fn);
}

/* -------------------------------------------------------------------------- */

void nu_winconsole_set_readline_cancel_hook(void (*fn)())
{
    if (g_console)
        g_console->set_readline_cancel_hook(fn);
}

/* -------------------------------------------------------------------------- */
