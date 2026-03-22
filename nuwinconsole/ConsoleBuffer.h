//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#ifndef __CONSOLE_BUFFER_H__
#define __CONSOLE_BUFFER_H__

#include "ConsoleConfig.h"
#include <mutex>
#include <string>
#include <vector>

/* -------------------------------------------------------------------------- */

class console_buffer_t {
public:
    console_buffer_t(const console_config_t& config);
    ~console_buffer_t() = default;

    // Non-copyable
    console_buffer_t(const console_buffer_t&) = delete;
    console_buffer_t& operator=(const console_buffer_t&) = delete;

    /* --------------------------------------------------------------------------
     */
    /* Output Operations */

    // Write a character at current cursor position
    void put_char(wchar_t ch);

    // Write a string
    void write(const std::wstring& text);
    void write(const std::string& text); // Convert from UTF-8

    // Newline
    void new_line();

    // Clear screen (resets buffer and cursor to origin)
    void clear();

    /* --------------------------------------------------------------------------
     */
    /* Cursor Operations */

    // Set cursor position (0-based buffer-absolute coordinates)
    void set_cursor_pos(int x, int y);
    void get_cursor_pos(int& x, int& y) const;

    // Position cursor at screen-relative coordinates (0-based).
    // screen_row 0 = top of current visible screen (_screen_top).
    // Also resets scroll offset so the current screen is visible.
    void locate_screen(int screen_col, int screen_row);

    // Move cursor
    void move_cursor(int dx, int dy);
    void cursor_home(); // (0, 0)
    void cursor_to_col(int col); // Beginning of current line

    /* --------------------------------------------------------------------------
     */
    /* Scrolling (viewport scroll — does not move the cursor) */

    void scroll_up(int lines = 1);
    void scroll_down(int lines = 1);
    void set_scroll_offset(int offset);
    int get_scroll_offset() const { return _scroll_offset; }

    // Maximum scroll offset == _screen_top (scroll all the way back to row 0)
    int get_max_scroll_offset() const;

    /* --------------------------------------------------------------------------
     */
    /* Buffer / Viewport Information */

    // First row of the current visible screen in the buffer
    int get_screen_top() const { return _screen_top; }

    // Get character at specific position
    wchar_t get_char(int x, int y) const;

    // Get line content (for rendering); y is buffer-absolute
    std::wstring get_line(int y) const;

    // Dimensions
    int get_cols() const { return _cols; }
    int get_rows() const { return _rows; }
    int get_total_rows() const { return _total_rows; }

    // Grow the column count of all rows (only expands, never shrinks)
    void resize_cols(int new_cols);

    /* --------------------------------------------------------------------------
     */
    /* Thread Safety */

    std::mutex& get_mutex() { return _mutex; }

private:
    // Configuration
    int _cols;
    int _rows;
    int _total_rows; // visible rows + scrollback rows
    int _tab_width;
    bool _auto_scroll;

    // Buffer: _buffer[physical_row][col]
    std::vector<std::vector<wchar_t>> _buffer;

    // Cursor — buffer-absolute coordinates
    int _cursor_x;
    int _cursor_y;

    // Top row of the current visible screen (advances when output scrolls past
    // the bottom of the screen — analogous to the Windows console screen buffer
    // window origin).
    int _screen_top;

    // Viewport scroll offset.
    //   0          = viewing the current screen  (_screen_top ..
    //   _screen_top+rows-1) n > 0      = viewing n rows above the current
    //   screen
    int _scroll_offset;

    // Circular buffer head
    int _buffer_start;

    mutable std::mutex _mutex;

    /* Helper functions */
    void
    scroll_buffer(); // Rotate circular buffer (called when cursor_y overflows)
    int buffer_index(int y) const; // Logical Y → physical buffer index
    void
    advance_screen_top(); // Called after cursor_y moves past bottom of screen
};

/* -------------------------------------------------------------------------- */

#endif // __CONSOLE_BUFFER_H__
