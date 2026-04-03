//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#ifndef __CONSOLE_WINDOW_H__
#define __CONSOLE_WINDOW_H__

#include "ConsoleBuffer.h"
#include "ConsoleConfig.h"
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <windows.h>

/* -------------------------------------------------------------------------- */

class console_window_t {
public:
    explicit console_window_t(HINSTANCE hInstance);
    ~console_window_t();

    // Non-copyable
    console_window_t(const console_window_t&) = delete;
    console_window_t& operator=(const console_window_t&) = delete;

    /* Initialization */
    bool create(const wchar_t* title = L"nuBASIC Console");
    void show(int nCmdShow = SW_SHOW);

    /* Configuration */
    void set_config(const console_config_t& config);
    const console_config_t& get_config() const { return _config; }

    /* Buffer access */
    console_buffer_t& get_buffer() { return *_buffer; }

    /* Window handle */
    HWND get_hwnd() const { return _hwnd; }

    /* Cursor visibility */
    void set_cursor_visible(bool visible);

    /* Refresh display.
     * force=false (default): coalesced — posts at most one WM_CONSOLE_REFRESH.
     * force=true           : always invalidates immediately (used by CLS,
     *                        explicit nu_winconsole_refresh, GDI unlock, etc.)
     */
    void refresh(bool force = false);

    /* Back-buffer DC for GDI drawing (graphics persist across repaints).
     * Acquires an internal lock; must call release_offscreen_dc() when done
     * (each get_offscreen_dc pairs with exactly one release_offscreen_dc). */
    HDC get_offscreen_dc();

    void release_offscreen_dc();

    /* Clear the back-buffer (called by CLS) */
    void clear_backbuffer();

    /* Screen lock / unlock: suppress or re-enable the automatic refresh that
     * fires after every GDI drawing primitive (LINE, RECT, …).  While locked,
     * drawing commands accumulate in the back-buffer without updating the
     * screen.  unlock_rendering() re-enables and forces one immediate repaint,
     * so the whole frame appears at once with no intermediate flicker.
     * refresh() can be called explicitly at any time to blit the current
     * back-buffer to screen, even while the lock is active. */
    void lock_rendering();
    void unlock_rendering();

    /* Cancel any pending read_line (unblocks the waiting thread) */
    void cancel_input();

    /* Restore text mode after a graphics program finishes:
     * clears the graphics layer and re-enables the text overlay. */
    void restore_text_mode();

    /* Returns true if the console is currently in graphics mode. */
    bool is_graphics_mode() const noexcept { return _graphics_mode.load(); }

    /* CLI-only: overlay "Press any key to continue..." on the graphics layer
     * at the bottom of the screen so the user knows how to dismiss it.
     * Draws directly onto _gfx_dc so the prompt appears on top of any GDI
     * content already there.  Follows this call with a key-wait loop. */
    void show_graphics_end_prompt();

    /* Input Operations */

    // Blocking line input. May be called from a worker thread: WM_CHAR is
    // handled on the window's thread; this side waits on _line_ready.
    std::wstring read_line();

    // Non-blocking key fetch for INKEY$ / INPUT$(n)
    std::wstring get_key();
    bool is_key_available() const;

    // Non-blocking virtual-key fetch for GetVKey()
    // Returns the next vk_* code (special keys first, then char keys), or 0.
    int get_vkey();
    bool vkey_available() const;

    /* Message loop (optional — call run() if you want a dedicated loop) */
    int run();

    /* When true (CLI/standalone mode): WM_CLOSE on the top-level window
     * destroys it (triggering WM_DESTROY → PostQuitMessage).  When false (IDE
     * embedded): WM_CLOSE merely hides the window and fires the close callback.
     */
    void set_exit_on_close(bool v) { _exit_on_close = v; }

    /* Close callback: called when the user closes a top-level (detached) window
     */
    void set_close_callback(void (*fn)()) { _close_callback = fn; }

    /* Ctrl+C callback: called when the user presses Ctrl+C in the window */
    void set_ctrlc_callback(void (*fn)()) { _ctrlc_callback = fn; }

    /* After Ctrl+C abandons the current read_line edit (cmd-style), redraw the
     * prompt. Used by nuBasicCLI; leave unset in the IDE. */
    void set_readline_cancel_hook(void (*fn)()) { _readline_cancel_hook = fn; }

    /* When false, mouse drag / double-click selection and the R-button menu
     * for copy/select-all are ignored (IDE embedded console while RUN). */
    void set_mouse_text_selection_enabled(bool enabled);

private:
    static LRESULT CALLBACK window_proc(
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    void on_paint();
    void on_size(int width, int height);
    void on_key_down(WPARAM key, LPARAM lParam);
    void on_char(WPARAM ch);
    void on_mouse_wheel(int delta);
    void on_vscroll(WPARAM wParam);
    void on_destroy();
    void on_lbutton_down(int x, int y);
    void on_lbutton_dblclk(int x, int y);
    void on_mouse_move(int x, int y);
    void on_lbutton_up(int x, int y);
    void on_rbutton_up(int x, int y);

    void mouse_to_buffer(int mx, int my, int& row, int& col) const;
    void copy_selection_to_clipboard();
    void paste_from_clipboard_line_input();

    void update_scrollbar();
    void render_console(HDC hdc);
    void render_cursor(HDC hdc);
    void render_text(HDC hdc);

    void start_cursor_blink();
    void stop_cursor_blink();
    void on_cursor_timer();

private:
    HINSTANCE _hinstance;
    HWND _hwnd;

    console_config_t _config;
    std::unique_ptr<console_buffer_t> _buffer;

    HFONT _hfont;
    bool _cursor_blink_state; // current blink phase (used for painting)
    bool _cursor_force_visible; // set_cursor_visible() override

    // Two-layer back-buffer:
    //   _gfx_dc   — persistent graphics layer; GDI primitives (Line, Rect, …)
    //               draw here.  Never cleared by text rendering.
    //   _mem_dc   — composite layer; on every WM_PAINT: _gfx_dc is blitted onto
    //               _mem_dc first, then render_text() overlays text on top.
    //               This DC is never handed to external callers.
    //
    // Concurrent access: _surface_mutex protects both DCs.  get_offscreen_dc()
    // acquires the lock and sets _surface_client_locked; on_paint() uses
    // try_lock so it never blocks the UI thread.
    std::recursive_mutex _surface_mutex;
    bool _surface_client_locked = false; // get_offscreen_dc holds the lock
    HDC _gfx_dc; // graphics layer (source for GDI primitives)
    HBITMAP _gfx_bitmap;
    HDC _mem_dc; // composite layer (blitted to screen)
    HBITMAP _mem_bitmap;
    int _backbuffer_width;
    int _backbuffer_height;

    // When true, release_offscreen_dc() skips the automatic refresh so that
    // multiple drawing commands accumulate in the back-buffer without causing
    // per-primitive screen updates (SCREENLOCK / SCREENUNLOCK).
    std::atomic<bool> _render_locked{ false };

    // Graphics mode: true after the first GDI primitive is drawn into _gfx_dc.
    // While true, render_text() / render_cursor() are suppressed so that the
    // graphics layer is fully visible.  Resets to false on clear_backbuffer()
    // (CLS), which signals a return to text mode.
    std::atomic<bool> _graphics_mode{ false };

    // Input handling
    // _input_queue is accessed from both threads → protected by _input_mutex.
    // _input_mode and _line_ready are written on one thread and read on
    // another, so they must be atomic.
    mutable std::mutex _input_mutex;
    std::deque<wchar_t> _input_queue; // character keys for INKEY$ / INPUT$(n)
    std::deque<int> _vkey_queue; // virtual-key codes for GetVKey()

    std::wstring _input_line; // assembled by on_char() on UI thread
    std::atomic<bool> _input_mode; // true while read_line() is waiting
    std::atomic<bool> _line_ready; // set by on_char() when Enter pressed

    // Command history for read_line (up/down arrow navigation).
    // All access is on the UI thread (on_char / on_key_down).
    static const int HISTORY_MAX = 200;
    std::deque<std::wstring> _history;
    int _history_idx = -1; // -1 = not browsing
    std::wstring _history_save; // saved current draft while browsing

    bool _exit_on_close = false;
    void (*_close_callback)() = nullptr;
    void (*_ctrlc_callback)() = nullptr;
    void (*_readline_cancel_hook)() = nullptr;

    // Refresh coalescing: at most one WM_CONSOLE_REFRESH in the queue.
    std::atomic<bool> _refresh_pending{ false };

    // Mouse text selection (buffer-absolute coordinates)
    int _sel_r0 = 0, _sel_c0 = 0; // anchor (where drag started)
    int _sel_r1 = 0, _sel_c1 = 0; // cursor (current drag end)
    bool _selecting = false;
    bool _mouse_text_selection_enabled = true;
    // If WM_KEYDOWN handled Ctrl+V paste, ignore the following WM_CHAR (0x16).
    bool _skip_next_ctrl_v_char = false;
};

/* -------------------------------------------------------------------------- */

#endif // __CONSOLE_WINDOW_H__
