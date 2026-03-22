//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#define NOMINMAX
#include "ConsoleWindow.h"
#include <algorithm>
#include <mutex>

/* -------------------------------------------------------------------------- */

#define TIMER_CURSOR_BLINK 1
// Posted from any thread: run scrollbar + invalidate on the window thread.
#define WM_CONSOLE_REFRESH (WM_USER + 55)
static const wchar_t* WINDOW_CLASS_NAME = L"nuBasicConsoleWindow";

/* -------------------------------------------------------------------------- */
/* console_config_t helpers (implementations live here to avoid a separate TU)
 */
/* -------------------------------------------------------------------------- */

void console_config_t::calculate_window_size(HDC hdc)
{
    HFONT hFont = create_console_font();
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

    TEXTMETRIC tm;
    GetTextMetrics(hdc, &tm);

    char_width = tm.tmAveCharWidth;
    char_height = tm.tmHeight;

    window_width = cols * char_width + margin_left + margin_right;
    window_height = rows * char_height + margin_top + margin_bottom;

    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

/* -------------------------------------------------------------------------- */

HFONT console_config_t::create_console_font() const
{
    return CreateFontW(font_size, 0, 0, 0, font_weight, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, font_name.c_str());
}

/* -------------------------------------------------------------------------- */
/* console_window_t                                                           */
/* -------------------------------------------------------------------------- */

console_window_t::console_window_t(HINSTANCE hInstance)
    : _hinstance(hInstance)
    , _hwnd(nullptr)
    , _hfont(nullptr)
    , _cursor_blink_state(true)
    , _cursor_force_visible(true)
    , _mem_dc(nullptr)
    , _mem_bitmap(nullptr)
    , _backbuffer_width(0)
    , _backbuffer_height(0)
    , _input_mode(false)
    , _line_ready(false)
{
    _buffer = std::make_unique<console_buffer_t>(_config);
}

/* -------------------------------------------------------------------------- */

console_window_t::~console_window_t()
{
    stop_cursor_blink();

    // Avoid deadlock if another thread still holds the surface lock.
    if (_surface_mutex.try_lock()) {
        _surface_client_locked = false;

        if (_hfont)
            DeleteObject(_hfont);
        _hfont = nullptr;
        if (_mem_bitmap)
            DeleteObject(_mem_bitmap);
        _mem_bitmap = nullptr;
        if (_mem_dc)
            DeleteDC(_mem_dc);
        _mem_dc = nullptr;

        _surface_mutex.unlock();
    }
}

/* -------------------------------------------------------------------------- */

bool console_window_t::create(const wchar_t* title)
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = window_proc;
    wc.hInstance = _hinstance;
    wc.hCursor = LoadCursor(nullptr, IDC_IBEAM);
    wc.hbrBackground = CreateSolidBrush(_config.background_color);
    wc.lpszClassName = WINDOW_CLASS_NAME;

    static bool class_registered = false;
    if (!class_registered) {
        if (!RegisterClassExW(&wc))
            return false;
        class_registered = true;
    }

    // Measure character cell size
    HDC hdc = GetDC(nullptr);
    _config.calculate_window_size(hdc);
    ReleaseDC(nullptr, hdc);

    _hfont = _config.create_console_font();

    RECT rect = { 0, 0, _config.window_width, _config.window_height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_VSCROLL, FALSE);

    _hwnd = CreateWindowExW(0, WINDOW_CLASS_NAME, title,
        WS_OVERLAPPEDWINDOW | WS_VSCROLL, CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr,
        _hinstance, this);

    if (!_hwnd)
        return false;

    update_scrollbar();

    if (_config.cursor_blink)
        start_cursor_blink();

    return true;
}

/* -------------------------------------------------------------------------- */

void console_window_t::show(int nCmdShow)
{
    if (_hwnd) {
        ShowWindow(_hwnd, nCmdShow);
        UpdateWindow(_hwnd);
    }
}

/* -------------------------------------------------------------------------- */

void console_window_t::set_config(const console_config_t& config)
{
    _config = config;
    _buffer = std::make_unique<console_buffer_t>(config);

    if (_hfont)
        DeleteObject(_hfont);
    _hfont = _config.create_console_font();

    if (_hwnd)
        InvalidateRect(_hwnd, nullptr, TRUE);
}

/* -------------------------------------------------------------------------- */

void console_window_t::set_cursor_visible(bool visible)
{
    _cursor_force_visible = visible;
    if (_hwnd)
        InvalidateRect(_hwnd, nullptr, FALSE);
}

/* -------------------------------------------------------------------------- */

void console_window_t::set_mouse_text_selection_enabled(bool enabled)
{
    _mouse_text_selection_enabled = enabled;
    if (!enabled && _selecting && _hwnd) {
        _selecting = false;
        ReleaseCapture();
    }
    if (_hwnd)
        InvalidateRect(_hwnd, nullptr, FALSE);
}

/* -------------------------------------------------------------------------- */

void console_window_t::refresh()
{
    if (!_hwnd)
        return;
    // Never call SetScrollInfo / synchronous paint from the interpreter
    // thread: marshal to the thread that owns the HWND.
    if (!PostMessageW(_hwnd, WM_CONSOLE_REFRESH, 0, 0))
        InvalidateRect(_hwnd, nullptr, FALSE);
}

/* -------------------------------------------------------------------------- */

int console_window_t::run()
{
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

/* -------------------------------------------------------------------------- */

LRESULT CALLBACK console_window_t::window_proc(
    HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    console_window_t* window = nullptr;

    if (msg == WM_CREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        window = (console_window_t*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)window);
    } else {
        window = (console_window_t*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (window) {
        switch (msg) {
        case WM_PAINT:
            window->on_paint();
            return 0;

        case WM_ERASEBKGND:
            return 1;

        case WM_SIZE:
            window->on_size(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_APP:
            // Requested by read_line() (worker thread) to grab keyboard focus
            // so that subsequent keystrokes are routed here.
            SetFocus(hwnd);
            return 0;

        case WM_CONSOLE_REFRESH:
            window->update_scrollbar();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;

        case WM_LBUTTONDOWN:
            window->on_lbutton_down(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_LBUTTONDBLCLK:
            window->on_lbutton_dblclk(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_MOUSEMOVE:
            if (wParam & MK_LBUTTON)
                window->on_mouse_move(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_LBUTTONUP:
            window->on_lbutton_up(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_RBUTTONUP:
            window->on_rbutton_up(LOWORD(lParam), HIWORD(lParam));
            return 0;

        case WM_KEYDOWN:
            window->on_key_down(wParam);
            return 0;

        case WM_CHAR:
            window->on_char(wParam);
            return 0;

        case WM_MOUSEWHEEL:
            window->on_mouse_wheel(GET_WHEEL_DELTA_WPARAM(wParam));
            return 0;

        case WM_VSCROLL:
            window->on_vscroll(wParam);
            return 0;

        case WM_TIMER:
            if (wParam == TIMER_CURSOR_BLINK)
                window->on_cursor_timer();
            return 0;

        case WM_CLOSE: {
            LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
            if (!(style & WS_CHILD)) {
                ShowWindow(hwnd, SW_HIDE);
                if (window->_close_callback)
                    window->_close_callback();
                return 0;
            }
        } break;

        case WM_DESTROY:
            window->on_destroy();
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_paint()
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(_hwnd, &ps);

    // Always use the actual client area size rather than the cached
    // _config.window_width/height, which may lag the first WM_SIZE.
    RECT client_rc;
    GetClientRect(_hwnd, &client_rc);
    int w = client_rc.right;
    int h = client_rc.bottom;
    if (w <= 0 || h <= 0) {
        EndPaint(_hwnd, &ps);
        return;
    }
    _config.window_width = w;
    _config.window_height = h;

    // If the interpreter thread holds the surface lock (GDI drawing), do not
    // block the UI thread here: Detach / SetParent / MoveWindow can trigger
    // WM_PAINT synchronously; waiting would freeze the detached window until
    // the next BASIC statement boundary.
    {
        std::unique_lock<std::recursive_mutex> surf_lock(
            _surface_mutex, std::defer_lock);
        if (surf_lock.try_lock()) {
            if (!_mem_dc || _backbuffer_width != w || _backbuffer_height != h) {
                HDC new_dc = CreateCompatibleDC(hdc);
                HBITMAP new_bmp = CreateCompatibleBitmap(hdc, w, h);
                SelectObject(new_dc, new_bmp);

                HBRUSH bg = CreateSolidBrush(_config.background_color);
                RECT rc = { 0, 0, w, h };
                FillRect(new_dc, &rc, bg);
                DeleteObject(bg);

                if (_mem_dc) {
                    BitBlt(new_dc, 0, 0, std::min(w, _backbuffer_width),
                        std::min(h, _backbuffer_height), _mem_dc, 0, 0,
                        SRCCOPY);
                    DeleteDC(_mem_dc);
                    DeleteObject(_mem_bitmap);
                }

                _mem_dc = new_dc;
                _mem_bitmap = new_bmp;
                _backbuffer_width = w;
                _backbuffer_height = h;
            }

            if (_mem_dc)
                BitBlt(hdc, 0, 0, w, h, _mem_dc, 0, 0, SRCCOPY);
        } else {
            HBRUSH bg = CreateSolidBrush(_config.background_color);
            FillRect(hdc, &client_rc, bg);
            DeleteObject(bg);
        }
    }

    render_text(hdc);

    if (_cursor_force_visible && _cursor_blink_state)
        render_cursor(hdc);

    EndPaint(_hwnd, &ps);
}

/* -------------------------------------------------------------------------- */

void console_window_t::render_console(HDC hdc)
{
    render_text(hdc);

    if (_cursor_force_visible && _cursor_blink_state)
        render_cursor(hdc);
}

/* -------------------------------------------------------------------------- */
// Viewport model:
//
//   screen_top    = first row of the current active screen in the buffer
//   scroll_offset = how many rows above screen_top the view is shifted
//                   (0 = viewing current screen, n > 0 = n rows above it)
//
//   first_visible_row = screen_top - scroll_offset   (clamped to >= 0)
//   row y on screen   = buffer row  first_visible_row + y
/* -------------------------------------------------------------------------- */

void console_window_t::render_text(HDC hdc)
{
    HFONT hOldFont = (HFONT)SelectObject(hdc, _hfont);
    SetBkMode(hdc, OPAQUE);

    int screen_top = _buffer->get_screen_top();
    int scroll_offset = _buffer->get_scroll_offset();
    int first_row = std::max(0, screen_top - scroll_offset);

    // Normalise selection so r0/c0 is always <= r1/c1.
    int sr0 = _sel_r0, sc0 = _sel_c0;
    int sr1 = _sel_r1, sc1 = _sel_c1;
    if (sr0 > sr1 || (sr0 == sr1 && sc0 > sc1)) {
        std::swap(sr0, sr1);
        std::swap(sc0, sc1);
    }
    bool has_sel = (sr0 != sr1 || sc0 != sc1);

    auto in_sel = [&](int row, int col) -> bool {
        if (!has_sel)
            return false;
        if (row < sr0 || row > sr1)
            return false;
        if (row == sr0 && col < sc0)
            return false;
        if (row == sr1 && col >= sc1)
            return false;
        return true;
    };

    for (int y = 0; y < _config.rows; ++y) {
        int buffer_y = first_row + y;
        if (buffer_y >= _buffer->get_total_rows())
            break;

        std::wstring line = _buffer->get_line(buffer_y);
        int line_len = std::min((int)line.size(), _config.cols);
        int y_pos = _config.margin_top + y * _config.char_height;

        if (!has_sel) {
            // Skip blank lines — nothing to draw.
            size_t first_ch = line.find_first_not_of(L' ');
            if (first_ch == std::wstring::npos)
                continue;
            size_t last_ch = line.find_last_not_of(L' ');
            std::wstring seg = line.substr(first_ch, last_ch - first_ch + 1);
            int x_pos
                = _config.margin_left + (int)first_ch * _config.char_width;
            SetTextColor(hdc, _config.text_color);
            SetBkColor(hdc, _config.background_color);
            // Use ETO_OPAQUE with a rect spanning the full line width so
            // that leading/trailing spaces are filled with the background
            // colour.  This prevents stale pixel content (e.g. from a
            // previous LOCATE + PRINT) from showing through the transparent
            // space regions.
            RECT line_rc = { _config.margin_left, y_pos,
                _config.margin_left + _config.cols * _config.char_width,
                y_pos + _config.char_height };
            ExtTextOutW(hdc, x_pos, y_pos, ETO_OPAQUE, &line_rc, seg.c_str(),
                (int)seg.size(), nullptr);
        } else {
            // Selection path: render in runs of same colour.
            int x = 0;
            while (x < line_len) {
                bool sel = in_sel(buffer_y, x);
                int run_start = x;
                while (x < line_len && in_sel(buffer_y, x) == sel)
                    ++x;
                std::wstring run = line.substr(run_start, x - run_start);
                bool all_spaces
                    = (run.find_first_not_of(L' ') == std::wstring::npos);
                // Only render if selected (highlight) or has non-space chars.
                if (!sel && all_spaces)
                    continue;
                if (sel) {
                    SetTextColor(hdc, _config.background_color);
                    SetBkColor(hdc, _config.text_color);
                } else {
                    SetTextColor(hdc, _config.text_color);
                    SetBkColor(hdc, _config.background_color);
                }
                int x_pos
                    = _config.margin_left + run_start * _config.char_width;
                TextOutW(hdc, x_pos, y_pos, run.c_str(), (int)run.size());
            }
        }
    }

    SelectObject(hdc, hOldFont);
}

/* -------------------------------------------------------------------------- */

void console_window_t::render_cursor(HDC hdc)
{
    int cursor_x, cursor_y;
    _buffer->get_cursor_pos(cursor_x, cursor_y);

    int screen_top = _buffer->get_screen_top();
    int scroll_offset = _buffer->get_scroll_offset();
    int first_row = std::max(0, screen_top - scroll_offset);

    int viewport_y = cursor_y - first_row;

    if (viewport_y >= 0 && viewport_y < _config.rows) {
        int x = _config.margin_left + cursor_x * _config.char_width;
        int y = _config.margin_top + viewport_y * _config.char_height;

        HBRUSH hBrush = CreateSolidBrush(_config.cursor_color);
        RECT cursor_rect
            = { x, y, x + _config.cursor_width, y + _config.char_height };
        FillRect(hdc, &cursor_rect, hBrush);
        DeleteObject(hBrush);
    }
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_size(int width, int height)
{
    if (width <= 0 || height <= 0)
        return;

    _config.window_width = width;
    _config.window_height = height;

    // Recalculate how many text rows and columns fit the new pixel size.
    // The buffer keeps its own fixed row count; we only adjust the render loop.
    if (_config.char_height > 0) {
        int visible = (height - _config.margin_top - _config.margin_bottom)
            / _config.char_height;
        if (visible > 0)
            _config.rows = visible;
    }
    if (_config.char_width > 0) {
        int visible = (width - _config.margin_left - _config.margin_right)
            / _config.char_width;
        if (visible > 0) {
            _config.cols = visible;
            // Grow the buffer rows to match the new column count.
            if (visible > _buffer->get_cols())
                _buffer->resize_cols(visible);
        }
    }

    update_scrollbar();
    InvalidateRect(_hwnd, nullptr, FALSE);
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_key_down(WPARAM key)
{
    // Map Windows VK_* codes to nuBASIC vk_* codes (from nu_os_std.h):
    //   vk_PageUp=6, vk_PageDown=7, vk_End=8, vk_Home=9
    //   vk_Left=10,  vk_Up=11,      vk_Right=12, vk_Down=13
    //   vk_Insert=14, vk_Delete=15
    // Escape (VK_ESCAPE) → vk_Escape=4
    // When not in line-input mode, queue the vk code so GetVKey() can see it.
    static const struct {
        WPARAM vk;
        int nu_vk;
    } vkey_map[] = {
        { VK_ESCAPE, 4 },
        { VK_PRIOR, 6 }, // PageUp
        { VK_NEXT, 7 }, // PageDown
        { VK_END, 8 },
        { VK_HOME, 9 },
        { VK_LEFT, 10 },
        { VK_UP, 11 },
        { VK_RIGHT, 12 },
        { VK_DOWN, 13 },
        { VK_INSERT, 14 },
        { VK_DELETE, 15 },
    };

    for (auto& e : vkey_map) {
        if (key == e.vk) {
            // Queue for GetVKey() only when not waiting for a full line.
            if (!_input_mode.load()) {
                std::lock_guard<std::mutex> lock(_input_mutex);
                _vkey_queue.push_back(e.nu_vk);
            }
            break;
        }
    }

    // Scrollback shortcuts (Ctrl+Home / Ctrl+End) — display only, no queueing.
    switch (key) {
    case VK_UP:
        if (!_input_mode.load())
            break; // arrow already queued above
        _buffer->scroll_up(1);
        update_scrollbar();
        InvalidateRect(_hwnd, nullptr, FALSE);
        break;

    case VK_DOWN:
        if (!_input_mode.load())
            break;
        _buffer->scroll_down(1);
        update_scrollbar();
        InvalidateRect(_hwnd, nullptr, FALSE);
        break;

    case VK_PRIOR:
        if (!_input_mode.load())
            break;
        _buffer->scroll_up(_config.rows);
        update_scrollbar();
        InvalidateRect(_hwnd, nullptr, FALSE);
        break;

    case VK_NEXT:
        if (!_input_mode.load())
            break;
        _buffer->scroll_down(_config.rows);
        update_scrollbar();
        InvalidateRect(_hwnd, nullptr, FALSE);
        break;

    case VK_HOME:
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            _buffer->set_scroll_offset(_buffer->get_max_scroll_offset());
            update_scrollbar();
            InvalidateRect(_hwnd, nullptr, FALSE);
        }
        break;

    case VK_END:
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            _buffer->set_scroll_offset(0);
            update_scrollbar();
            InvalidateRect(_hwnd, nullptr, FALSE);
        }
        break;

    case VK_INSERT:
        if (GetKeyState(VK_CONTROL) & 0x8000)
            copy_selection_to_clipboard();
        break;

    case 0x41: // 'A' key
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            // Ctrl+A: select all visible content
            _sel_r0 = 0;
            _sel_c0 = 0;
            _sel_r1 = _buffer->get_screen_top() + _config.rows - 1;
            _sel_c1 = _buffer->get_cols();
            InvalidateRect(_hwnd, nullptr, FALSE);
        }
        break;
    }
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_char(WPARAM ch)
{
    wchar_t wch = (wchar_t)ch;

    // Ctrl+C (ETX = 0x03)
    if (wch == L'\x03') {
        // If there is an active selection, copy it to clipboard first.
        bool has_sel = (_sel_r0 != _sel_r1 || _sel_c0 != _sel_c1);
        if (has_sel)
            copy_selection_to_clipboard();

        // Cancel any pending INPUT/read_line.
        if (_input_mode.load()) {
            _input_line.clear();
            _line_ready.store(true);
        }
        // Notify the application (sets interpreter break event).
        if (_ctrlc_callback)
            _ctrlc_callback();

        InvalidateRect(_hwnd, nullptr, FALSE);
        return;
    }

    if (_input_mode.load()) {
        // ---- Line-input mode (read_line is waiting) ----
        if (wch == L'\r') {
            // Enter: commit the line
            _buffer->put_char(L'\n');
            _line_ready.store(true);
        } else if (wch == L'\b') {
            // Backspace: erase last character from the editing buffer and
            // screen
            if (!_input_line.empty()) {
                _input_line.pop_back();
                _buffer->put_char(L'\b');
            }
        } else if (wch >= L' ') {
            // Printable character: echo, accumulate, and clear any selection.
            _sel_r0 = _sel_r1 = 0;
            _sel_c0 = _sel_c1 = 0;
            _input_line += wch;
            _buffer->put_char(wch);
        }
    } else {
        // ---- INKEY$ / INPUT$(n) mode — queue without echo ----
        std::lock_guard<std::mutex> lock(_input_mutex);
        _input_queue.push_back(wch);
    }

    InvalidateRect(_hwnd, nullptr, FALSE);
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_mouse_wheel(int delta)
{
    int lines = delta / WHEEL_DELTA;
    if (lines > 0)
        _buffer->scroll_up(lines);
    else
        _buffer->scroll_down(-lines);

    update_scrollbar();
    InvalidateRect(_hwnd, nullptr, FALSE);
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_vscroll(WPARAM wParam)
{
    int action = LOWORD(wParam);

    switch (action) {
    case SB_LINEUP:
        _buffer->scroll_up(1);
        break;

    case SB_LINEDOWN:
        _buffer->scroll_down(1);
        break;

    case SB_PAGEUP:
        _buffer->scroll_up(_config.rows);
        break;

    case SB_PAGEDOWN:
        _buffer->scroll_down(_config.rows);
        break;

    case SB_THUMBTRACK:
    case SB_THUMBPOSITION: {
        // nPos is the desired first_visible_row (== screen_top - scroll_offset)
        int pos = HIWORD(wParam);
        int screen_top = _buffer->get_screen_top();
        int new_offset = screen_top - pos;
        _buffer->set_scroll_offset(std::max(0, new_offset));
        break;
    }

    case SB_TOP:
        _buffer->set_scroll_offset(_buffer->get_max_scroll_offset());
        break;

    case SB_BOTTOM:
        _buffer->set_scroll_offset(0);
        break;
    }

    update_scrollbar();
    InvalidateRect(_hwnd, nullptr, FALSE);
}

/* -------------------------------------------------------------------------- */

void console_window_t::update_scrollbar()
{
    if (!_hwnd)
        return;

    int screen_top = _buffer->get_screen_top();
    int scroll_offset = _buffer->get_scroll_offset();

    // Total content range: rows 0 .. screen_top + rows - 1
    // nPos = first visible row = screen_top - scroll_offset
    int first_visible = std::max(0, screen_top - scroll_offset);

    SCROLLINFO si = {};
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_DISABLENOSCROLL;
    si.nMin = 0;
    si.nMax = screen_top + _config.rows - 1;
    si.nPage = (UINT)_config.rows;
    si.nPos = first_visible;

    SetScrollInfo(_hwnd, SB_VERT, &si, TRUE);
}

/* -------------------------------------------------------------------------- */

HDC console_window_t::get_offscreen_dc()
{
    if (!_hwnd)
        return nullptr;

    _surface_mutex.lock();

    if (!_mem_dc) {
        HDC hdc = GetDC(_hwnd);
        if (!hdc) {
            _surface_mutex.unlock();
            return nullptr;
        }

        int w = _config.window_width > 0 ? _config.window_width : 640;
        int h = _config.window_height > 0 ? _config.window_height : 400;

        _mem_dc = CreateCompatibleDC(hdc);
        _mem_bitmap = CreateCompatibleBitmap(hdc, w, h);
        if (!_mem_dc || !_mem_bitmap) {
            if (_mem_bitmap)
                DeleteObject(_mem_bitmap);
            if (_mem_dc)
                DeleteDC(_mem_dc);
            _mem_dc = nullptr;
            _mem_bitmap = nullptr;
            ReleaseDC(_hwnd, hdc);
            _surface_mutex.unlock();
            return nullptr;
        }
        SelectObject(_mem_dc, _mem_bitmap);
        _backbuffer_width = w;
        _backbuffer_height = h;

        HBRUSH bg = CreateSolidBrush(_config.background_color);
        RECT rc = { 0, 0, w, h };
        FillRect(_mem_dc, &rc, bg);
        DeleteObject(bg);

        ReleaseDC(_hwnd, hdc);
    }
    _surface_client_locked = true;
    return _mem_dc;
}

/* -------------------------------------------------------------------------- */

void console_window_t::release_offscreen_dc()
{
    if (_surface_client_locked) {
        _surface_client_locked = false;
        _surface_mutex.unlock();
    }
    refresh();
}

/* -------------------------------------------------------------------------- */

void console_window_t::clear_backbuffer()
{
    if (!_hwnd)
        return;
    std::lock_guard<std::recursive_mutex> surf_lock(_surface_mutex);
    if (!_mem_dc)
        return;
    HBRUSH bg = CreateSolidBrush(_config.background_color);
    RECT rc = { 0, 0, _backbuffer_width, _backbuffer_height };
    FillRect(_mem_dc, &rc, bg);
    DeleteObject(bg);
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_destroy()
{
    LONG_PTR style = GetWindowLongPtr(_hwnd, GWL_STYLE);
    if (!(style & WS_CHILD))
        PostQuitMessage(0);
}

/* -------------------------------------------------------------------------- */
// read_line()
//
// Blocks until the user presses Enter.  Rather than using a condition variable
// (which would deadlock because the message loop lives on this same thread),
// we pump Windows messages ourselves via MsgWaitForMultipleObjects +
// PeekMessage. on_char() sets _line_ready = true when it sees '\r'.
/* -------------------------------------------------------------------------- */

void console_window_t::cancel_input() { _line_ready.store(true); }

/* -------------------------------------------------------------------------- */

std::wstring console_window_t::read_line()
{
    _input_line.clear();
    _line_ready.store(false);
    _input_mode.store(true);

    if (_hwnd)
        PostMessage(_hwnd, WM_APP, 0, 0);

    // Keyboard input is delivered to the thread that owns the HWND (the one
    // that called CreateWindow).  Standalone nuBasicCLI runs the interpreter on
    // that same thread, so we must pump messages here or the UI freezes.
    // When embedded in the IDE, the console HWND is owned by the IDE thread
    // while INPUT may run on a worker — then we only poll _line_ready.
    const DWORD owner_tid
        = _hwnd ? GetWindowThreadProcessId(_hwnd, nullptr) : 0;
    const DWORD self_tid = GetCurrentThreadId();
    const bool pump_here = (_hwnd && owner_tid == self_tid);

    while (!_line_ready.load(std::memory_order_acquire)) {
        if (pump_here) {
            MSG msg;
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    PostQuitMessage(static_cast<int>(msg.wParam));
                    _line_ready.store(true, std::memory_order_release);
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            if (_line_ready.load(std::memory_order_acquire))
                break;
            MsgWaitForMultipleObjects(
                0, nullptr, FALSE, 50, QS_ALLEVENTS | QS_ALLPOSTMESSAGE);
        } else {
            Sleep(10);
        }
    }

    _input_mode.store(false);
    _line_ready.store(false);
    return _input_line;
}

/* -------------------------------------------------------------------------- */

std::wstring console_window_t::get_key()
{
    std::lock_guard<std::mutex> lock(_input_mutex);
    if (_input_queue.empty())
        return L"";
    wchar_t ch = _input_queue.front();
    _input_queue.pop_front();
    return std::wstring(1, ch);
}

/* -------------------------------------------------------------------------- */

bool console_window_t::is_key_available() const
{
    std::lock_guard<std::mutex> lock(_input_mutex);
    return !_input_queue.empty();
}

/* -------------------------------------------------------------------------- */

int console_window_t::get_vkey()
{
    std::lock_guard<std::mutex> lock(_input_mutex);
    // Virtual-key queue (arrows, etc.) has priority over character queue.
    if (!_vkey_queue.empty()) {
        int code = _vkey_queue.front();
        _vkey_queue.pop_front();
        return code;
    }
    if (!_input_queue.empty()) {
        wchar_t ch = _input_queue.front();
        _input_queue.pop_front();
        return (int)ch;
    }
    return 0;
}

/* -------------------------------------------------------------------------- */

bool console_window_t::vkey_available() const
{
    std::lock_guard<std::mutex> lock(_input_mutex);
    return !_vkey_queue.empty() || !_input_queue.empty();
}

/* -------------------------------------------------------------------------- */

void console_window_t::start_cursor_blink()
{
    SetTimer(_hwnd, TIMER_CURSOR_BLINK, _config.cursor_blink_rate, nullptr);
}

/* -------------------------------------------------------------------------- */

void console_window_t::stop_cursor_blink()
{
    if (_hwnd)
        KillTimer(_hwnd, TIMER_CURSOR_BLINK);
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_cursor_timer()
{
    _cursor_blink_state = !_cursor_blink_state;
    InvalidateRect(_hwnd, nullptr, FALSE);
}

/* -------------------------------------------------------------------------- */

void console_window_t::mouse_to_buffer(int mx, int my, int& row, int& col) const
{
    int screen_top = _buffer->get_screen_top();
    int scroll_offset = _buffer->get_scroll_offset();
    int first_row = std::max(0, screen_top - scroll_offset);

    int ch = std::max(1, _config.char_height);
    int cw = std::max(1, _config.char_width);
    int viewport_y = (my - _config.margin_top) / ch;
    int viewport_x = (mx - _config.margin_left) / cw;

    row = first_row + std::max(0, viewport_y);
    col = std::max(0, viewport_x);
    row = std::min(row, _buffer->get_total_rows() - 1);
    col = std::min(col, _buffer->get_cols());
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_lbutton_down(int x, int y)
{
    SetFocus(_hwnd);
    if (!_mouse_text_selection_enabled)
        return;
    int row, col;
    mouse_to_buffer(x, y, row, col);
    _sel_r0 = _sel_r1 = row;
    _sel_c0 = _sel_c1 = col;
    _selecting = true;
    SetCapture(_hwnd);
    InvalidateRect(_hwnd, nullptr, FALSE);
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_lbutton_dblclk(int x, int y)
{
    SetFocus(_hwnd);
    if (!_mouse_text_selection_enabled)
        return;
    int row, col;
    mouse_to_buffer(x, y, row, col);

    std::wstring line = _buffer->get_line(row);
    int len = (int)line.size();

    // If not on a word character, treat like a single click.
    auto is_word_char = [](wchar_t c) { return iswalnum(c) || c == L'_'; };

    if (col >= len || !is_word_char(line[col])) {
        _sel_r0 = _sel_r1 = row;
        _sel_c0 = _sel_c1 = col;
        InvalidateRect(_hwnd, nullptr, FALSE);
        return;
    }

    // Expand leftward to find the start of the word.
    int word_start = col;
    while (word_start > 0 && is_word_char(line[word_start - 1]))
        --word_start;

    // Expand rightward to find the end of the word.
    int word_end = col;
    while (word_end < len && is_word_char(line[word_end]))
        ++word_end;

    _sel_r0 = _sel_r1 = row;
    _sel_c0 = word_start;
    _sel_c1 = word_end;
    InvalidateRect(_hwnd, nullptr, FALSE);
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_mouse_move(int x, int y)
{
    if (!_mouse_text_selection_enabled || !_selecting)
        return;
    int row, col;
    mouse_to_buffer(x, y, row, col);
    _sel_r1 = row;
    _sel_c1 = col;
    InvalidateRect(_hwnd, nullptr, FALSE);
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_lbutton_up(int x, int y)
{
    if (_mouse_text_selection_enabled && _selecting) {
        _selecting = false;
        ReleaseCapture();
        int row, col;
        mouse_to_buffer(x, y, row, col);
        _sel_r1 = row;
        _sel_c1 = col;
        InvalidateRect(_hwnd, nullptr, FALSE);
    }
}

/* -------------------------------------------------------------------------- */

void console_window_t::on_rbutton_up(int x, int y)
{
    if (!_mouse_text_selection_enabled)
        return;
    POINT pt = { x, y };
    ClientToScreen(_hwnd, &pt);

    HMENU hMenu = CreatePopupMenu();
    bool has_sel = (_sel_r0 != _sel_r1 || _sel_c0 != _sel_c1);
    AppendMenuW(hMenu, MF_STRING | (has_sel ? 0 : MF_GRAYED), 1, L"Copy");
    AppendMenuW(hMenu, MF_STRING, 2, L"Select All");

    int cmd = TrackPopupMenu(
        hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, _hwnd, nullptr);
    DestroyMenu(hMenu);

    if (cmd == 1)
        copy_selection_to_clipboard();
    else if (cmd == 2) {
        _sel_r0 = 0;
        _sel_c0 = 0;
        _sel_r1 = _buffer->get_screen_top() + _config.rows - 1;
        _sel_c1 = _buffer->get_cols();
        InvalidateRect(_hwnd, nullptr, FALSE);
    }
}

/* -------------------------------------------------------------------------- */

void console_window_t::copy_selection_to_clipboard()
{
    int r0 = _sel_r0, c0 = _sel_c0;
    int r1 = _sel_r1, c1 = _sel_c1;
    if (r0 > r1 || (r0 == r1 && c0 > c1)) {
        std::swap(r0, r1);
        std::swap(c0, c1);
    }
    if (r0 == r1 && c0 == c1)
        return; // nothing selected

    std::wstring text;
    for (int row = r0; row <= r1; ++row) {
        std::wstring line = _buffer->get_line(row);
        int start = (row == r0) ? c0 : 0;
        int end = (row == r1) ? c1 : (int)line.size();
        end = std::min(end, (int)line.size());
        if (start < end) {
            std::wstring seg = line.substr(start, end - start);
            // Trim trailing spaces from this segment.
            size_t last = seg.find_last_not_of(L' ');
            if (last != std::wstring::npos)
                seg = seg.substr(0, last + 1);
            else
                seg.clear();
            text += seg;
        }
        if (row < r1)
            text += L"\r\n";
    }

    if (text.empty())
        return;

    if (!OpenClipboard(_hwnd))
        return;
    EmptyClipboard();
    size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (hMem) {
        wchar_t* ptr = (wchar_t*)GlobalLock(hMem);
        if (ptr) {
            wmemcpy(ptr, text.c_str(), text.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        } else {
            GlobalFree(hMem);
        }
    }
    CloseClipboard();
}

/* -------------------------------------------------------------------------- */
