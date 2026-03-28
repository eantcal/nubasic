//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#ifndef __NU_WINCONSOLE_API_H__
#define __NU_WINCONSOLE_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */
/* Initialization and Lifecycle */

// Initialize the GDI console (must be called from WinMain thread)
int nu_winconsole_init(void* hInstance, int nCmdShow);

// Check if console is initialized
int nu_winconsole_is_active();

// Shutdown the console
void nu_winconsole_shutdown();

// Process Windows messages (call periodically or in message loop)
int nu_winconsole_process_messages();

/* -------------------------------------------------------------------------- */
/* Output Operations (printf-like) */

// Write a UTF-8 string to the console
void nu_winconsole_write(const char* text);

// Write a wide string (UTF-16) to the console
void nu_winconsole_write_w(const wchar_t* text);

// Printf-style formatted output
void nu_winconsole_printf(const char* format, ...);

// Wide printf-style formatted output
void nu_winconsole_wprintf(const wchar_t* format, ...);

/* -------------------------------------------------------------------------- */
/* Input Operations */

// Read a line of input (blocking, like fgets or INPUT in BASIC)
// Returns number of characters read, or -1 on error
// buffer: destination buffer
// size: buffer size in characters
int nu_winconsole_read_line(char* buffer, int size);

// Read a line of input (blocking, wide char version)
int nu_winconsole_read_line_w(wchar_t* buffer, int size);

// Get a single key press (non-blocking, like INKEY$ in BASIC)
// Returns key code, or 0 if no key available
int nu_winconsole_get_key();

// Check if a key is available (for INKEY$)
int nu_winconsole_key_available();

// Get a virtual-key code (non-blocking, for GetVKey() in BASIC)
// Special keys (arrows, Home, End, etc.) are returned as nuBASIC vk_* values.
// Regular character keys are returned as their Unicode code point.
// Returns 0 if no key is available.
int nu_winconsole_get_vkey();

// Check if any key (including virtual keys) is available for GetVKey()
int nu_winconsole_vkey_available();

/* -------------------------------------------------------------------------- */
/* Console Control */

// Clear the console screen
void nu_winconsole_cls();

// Set cursor position (0-based)
void nu_winconsole_locate(int x, int y);

// Show/hide cursor
void nu_winconsole_cursor_visible(int visible);

// Refresh/update the display
void nu_winconsole_refresh();

// Screen lock / unlock: suppress or re-enable the automatic per-primitive
// refresh.  While locked, GDI drawing accumulates in the back-buffer without
// updating the screen.  nu_winconsole_screenunlock() forces one immediate
// repaint so the whole frame appears at once.
void nu_winconsole_screenlock();
void nu_winconsole_screenunlock();

// Cancel any pending blocking input (e.g. read_line)
void nu_winconsole_cancel_input();

// Enable/disable mouse-driven text selection and the right-click copy menu.
// The Windows IDE turns this off while a program runs with the console
// embedded.
void nu_winconsole_set_mouse_text_selection_enabled(int enabled);

/* -------------------------------------------------------------------------- */
/* Graphics Support (for GDI functions) */

// Get the console window handle (for GDI operations)
void* nu_winconsole_get_hwnd();

// Get a device context for GDI drawing
void* nu_winconsole_get_hdc();

/* -------------------------------------------------------------------------- */
/* Close Callback */

// When enabled (non-zero), closing the top-level console window destroys it
// and posts WM_QUIT so the message loop exits cleanly.  Enable for CLI/
// standalone mode; leave disabled (default) when embedded in the IDE so that
// closing the detached console does not terminate the host application.
void nu_winconsole_set_exit_on_close(int enabled);

// Register a callback invoked when the user closes a detached (top-level)
// console window.  Pass NULL to unregister.
void nu_winconsole_set_close_callback(void (*fn)());

// Register a callback invoked when the user presses Ctrl+C in the console.
// Use this to dispatch a BREAK signal to the interpreter.
void nu_winconsole_set_ctrlc_callback(void (*fn)());

// Optional: after Ctrl+C clears the current read_line buffer (cmd-style, no
// selection), this runs so the CLI can print the prompt again. Pass NULL to
// clear.
void nu_winconsole_set_readline_cancel_hook(void (*fn)());

// Release a device context obtained with nu_winconsole_get_hdc
void nu_winconsole_release_hdc(void* hdc);

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif // __NU_WINCONSOLE_API_H__
