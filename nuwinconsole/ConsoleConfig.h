//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#ifndef __CONSOLE_CONFIG_H__
#define __CONSOLE_CONFIG_H__

#include <string>
#include <windows.h>

/* -------------------------------------------------------------------------- */

struct console_config_t {
    // Buffer dimensions
    int cols = 80; // Columns (width)
    int rows = 25; // Visible rows
    int scrollback_rows = 1000; // Total buffer rows (for scrolling)

    // Font settings
    std::wstring font_name = L"Consolas";
    int font_size = 16; // Font height in pixels
    int font_weight = FW_NORMAL; // FW_NORMAL, FW_BOLD, etc.

    // Colors
    COLORREF text_color = RGB(192, 192, 192); // Light gray
    COLORREF background_color = RGB(0, 0, 0); // Black
    COLORREF cursor_color = RGB(255, 255, 255); // White

    // Cursor behavior
    bool cursor_blink = true;
    int cursor_blink_rate = 500; // Milliseconds
    int cursor_width = 2; // Pixels

    // Tab handling
    int tab_width = 8; // Spaces per tab

    // Auto scroll
    bool auto_scroll = true; // Auto scroll when writing beyond bottom

    // Window dimensions (calculated)
    int char_width = 0; // Character width (calculated from font)
    int char_height = 0; // Character height (calculated from font)

    // Window
    int window_width = 0; // Window width (calculated)
    int window_height = 0; // Window height (calculated)

    // Margins
    int margin_left = 2;
    int margin_top = 2;
    int margin_right = 2;
    int margin_bottom = 2;

    /* Calculate window size based on font */
    void calculate_window_size(HDC hdc);

    /* Create Windows font */
    HFONT create_console_font() const;
};

/* -------------------------------------------------------------------------- */

#endif // __CONSOLE_CONFIG_H__
