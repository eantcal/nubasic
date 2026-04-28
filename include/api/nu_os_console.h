//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */
#pragma once

/* -------------------------------------------------------------------------- */

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

struct os_input_result_t {
    std::string text;
    bool interrupted = false;
};

void _os_init();
void _os_locate(int y, int x);
void _os_cls();
std::string _os_input_str(int n);
os_input_result_t _os_input_str_interruptible(int n);
std::string _os_input(FILE* finput_ptr);
os_input_result_t _os_input_interruptible(FILE* finput_ptr);
int _os_kbhit();
void _os_cursor_visible(bool on);
void _os_config_term(bool on);
void _os_u16write(const std::u16string& output);

// Screen lock / unlock / explicit refresh (no-ops on non-GDI platforms).
void _os_screenlock();
void _os_screenunlock();
void _os_refresh();

// Screen mode:
//   0 = text mode:    I/O via real console, GDI ops are no-ops (headless-safe)
//   1 = GDI console:  I/O and graphics through the custom GDI window (default)
void _os_set_screen_mode(int mode);
int _os_get_screen_mode();

// Hybrid stdio keeps text I/O on stdin/stdout while allowing GDI primitives
// to draw to a custom graphics window. This is used by editor debuggers.
void _os_set_hybrid_stdio(int enabled);
int _os_get_hybrid_stdio();

/* -------------------------------------------------------------------------- */
} // namespace nu


/* -------------------------------------------------------------------------- */
