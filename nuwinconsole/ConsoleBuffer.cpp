//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#define NOMINMAX
#include "ConsoleBuffer.h"
#include <algorithm>
#include <windows.h>

/* -------------------------------------------------------------------------- */

static std::wstring utf8_to_wstring(const std::string& str)
{
    if (str.empty())
        return std::wstring();
    int size_needed = MultiByteToWideChar(
        CP_UTF8, 0, str.c_str(), (int)str.size(), nullptr, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(
        CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

/* -------------------------------------------------------------------------- */

console_buffer_t::console_buffer_t(const console_config_t& config)
    : _cols(config.cols)
    , _rows(config.rows)
    , _total_rows(config.rows + config.scrollback_rows)
    , _tab_width(config.tab_width)
    , _auto_scroll(config.auto_scroll)
    , _cursor_x(0)
    , _cursor_y(0)
    , _screen_top(0)
    , _scroll_offset(0)
    , _buffer_start(0)
{
    _buffer.resize(_total_rows);
    for (auto& row : _buffer)
        row.resize(_cols, L' ');
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::advance_screen_top()
{
    // Called (without acquiring the lock) when cursor_y has moved past the
    // bottom of the current screen window.  _screen_top follows the cursor
    // so that the current screen always ends at cursor_y.
    if (_cursor_y >= _screen_top + _rows)
        _screen_top = _cursor_y - _rows + 1;

    if (_auto_scroll)
        _scroll_offset = 0;
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::put_char(wchar_t ch)
{
    std::lock_guard<std::mutex> lock(_mutex);

    switch (ch) {
    case L'\r':
        _cursor_x = 0;
        return;

    case L'\n':
        _cursor_x = 0;
        ++_cursor_y;
        if (_cursor_y >= _total_rows) {
            scroll_buffer();
            _cursor_y = _total_rows - 1;
            // Keep _screen_top valid after buffer rotation
            if (_screen_top > 0)
                --_screen_top;
        }
        advance_screen_top();
        return;

    case L'\t': {
        int spaces = _tab_width - (_cursor_x % _tab_width);
        for (int i = 0; i < spaces && _cursor_x < _cols; ++i) {
            _buffer[buffer_index(_cursor_y)][_cursor_x] = L' ';
            ++_cursor_x;
        }
        return;
    }

    case L'\b':
        if (_cursor_x > 0) {
            --_cursor_x;
            _buffer[buffer_index(_cursor_y)][_cursor_x] = L' ';
        }
        return;
    }

    // Normal printable character
    if (_cursor_x >= _cols) {
        // Wrap to next line
        _cursor_x = 0;
        ++_cursor_y;
        if (_cursor_y >= _total_rows) {
            scroll_buffer();
            _cursor_y = _total_rows - 1;
            if (_screen_top > 0)
                --_screen_top;
        }
        advance_screen_top();
    }

    _buffer[buffer_index(_cursor_y)][_cursor_x] = ch;
    ++_cursor_x;
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::write(const std::wstring& text)
{
    for (wchar_t ch : text)
        put_char(ch);
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::write(const std::string& text)
{
    write(utf8_to_wstring(text));
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::new_line()
{
    std::lock_guard<std::mutex> lock(_mutex);

    _cursor_x = 0;
    ++_cursor_y;

    if (_cursor_y >= _total_rows) {
        scroll_buffer();
        _cursor_y = _total_rows - 1;
        if (_screen_top > 0)
            --_screen_top;
    }
    advance_screen_top();
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::clear()
{
    std::lock_guard<std::mutex> lock(_mutex);

    for (auto& row : _buffer)
        std::fill(row.begin(), row.end(), L' ');

    _cursor_x = 0;
    _cursor_y = 0;
    _screen_top = 0;
    _scroll_offset = 0;
    _buffer_start = 0;
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::set_cursor_pos(int x, int y)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _cursor_x = std::max(0, std::min(x, _cols - 1));
    _cursor_y = std::max(0, std::min(y, _total_rows - 1));
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::locate_screen(int screen_col, int screen_row)
{
    std::lock_guard<std::mutex> lock(_mutex);

    int new_y = _screen_top + screen_row;
    new_y = std::max(0, std::min(new_y, _total_rows - 1));
    int new_x = std::max(0, std::min(screen_col, _cols - 1));

    _cursor_x = new_x;
    _cursor_y = new_y;
    _scroll_offset = 0; // Snap viewport to current screen
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::get_cursor_pos(int& x, int& y) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    x = _cursor_x;
    y = _cursor_y;
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::move_cursor(int dx, int dy)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _cursor_x = std::max(0, std::min(_cursor_x + dx, _cols - 1));
    _cursor_y = std::max(0, std::min(_cursor_y + dy, _total_rows - 1));
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::cursor_home()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _cursor_x = 0;
    _cursor_y = _screen_top; // Top-left of current screen
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::cursor_to_col(int col)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _cursor_x = std::max(0, std::min(col, _cols - 1));
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::scroll_up(int lines)
{
    std::lock_guard<std::mutex> lock(_mutex);
    int max_offset = _screen_top; // Cannot scroll back past the very first row
    _scroll_offset = std::min(_scroll_offset + lines, max_offset);
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::scroll_down(int lines)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _scroll_offset = std::max(_scroll_offset - lines, 0);
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::set_scroll_offset(int offset)
{
    std::lock_guard<std::mutex> lock(_mutex);
    int max_offset = _screen_top;
    _scroll_offset = std::max(0, std::min(offset, max_offset));
}

/* -------------------------------------------------------------------------- */

int console_buffer_t::get_max_scroll_offset() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _screen_top;
}

/* -------------------------------------------------------------------------- */

wchar_t console_buffer_t::get_char(int x, int y) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (x < 0 || x >= _cols || y < 0 || y >= _total_rows)
        return L' ';
    return _buffer[buffer_index(y)][x];
}

/* -------------------------------------------------------------------------- */

std::wstring console_buffer_t::get_line(int y) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (y < 0 || y >= _total_rows)
        return std::wstring(_cols, L' ');
    return std::wstring(
        _buffer[buffer_index(y)].begin(), _buffer[buffer_index(y)].end());
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::scroll_buffer()
{
    // Rotate circular buffer (assumes lock already held).
    // The oldest row is recycled and cleared at the new logical end.
    _buffer_start = (_buffer_start + 1) % _total_rows;
    int last_line = (_buffer_start + _total_rows - 1) % _total_rows;
    std::fill(_buffer[last_line].begin(), _buffer[last_line].end(), L' ');
}

/* -------------------------------------------------------------------------- */

int console_buffer_t::buffer_index(int y) const
{
    return (_buffer_start + y) % _total_rows;
}

/* -------------------------------------------------------------------------- */

void console_buffer_t::resize_cols(int new_cols)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (new_cols <= _cols)
        return; // only grow
    for (auto& row : _buffer)
        row.resize(new_cols, L' ');
    _cols = new_cols;
}

/* -------------------------------------------------------------------------- */
