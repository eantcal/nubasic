//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#pragma once

#ifndef TINY_NUBASIC_VER


/* -------------------------------------------------------------------------- */

#include "nu_variant.h"
#include <vector>


#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
// clang-format on
#endif


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class rt_prog_ctx_t;

using gdi_iarg_t = unsigned int;
using gdi_iargs_t = std::vector<gdi_iarg_t>;

using gdi_varg_t = variant_t;
using gdi_vargs_t = std::vector<gdi_varg_t>;


/* -------------------------------------------------------------------------- */

#if 0
struct image_t : public object_t {
    Gdiplus::Image * _image = nullptr;

    image_t(const std::string& filename) {
        WCHAR wsfname[2048] = { 0 };
        mbstowcs(wsfname, filename.c_str(), sizeof(wsfname) / sizeof(WCHAR));
        _image = new Gdiplus::Image(wsfname);
    }

    virtual ~image_t() { 
        delete _image;
    }
};
#endif


/* -------------------------------------------------------------------------- */

struct os_textout_t {
    int operator()(rt_prog_ctx_t& ctx, gdi_vargs_t args);
};


/* -------------------------------------------------------------------------- */

struct os_plotimage_t {
    int operator()(rt_prog_ctx_t& ctx, gdi_vargs_t args);
};


/* -------------------------------------------------------------------------- */

struct os_set_pixel_t {
    int operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args);
};


/* -------------------------------------------------------------------------- */

struct os_line_t {
    int operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args);
};


/* -------------------------------------------------------------------------- */

struct os_rect_t {
    int operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args);
};


/* -------------------------------------------------------------------------- */

struct os_fillrect_t {
    int operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args);
};


/* -------------------------------------------------------------------------- */

struct os_ellipse_t {
    int operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args);
};


/* -------------------------------------------------------------------------- */

struct os_fillellipse_t {
    int operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args);
};


/* -------------------------------------------------------------------------- */

int _os_get_screen_width() noexcept;
int _os_get_screen_height() noexcept;

//! Returns a Mouse struct with all pointer state at once.
//! Fields: x (pixels from left), y (pixels from top),
//!         btn (0=none, 1=left, 2=middle, 4=right).
//! Replaces the individual GetMouseX/Y/Btn functions (deprecated v2.0).
variant_t _os_get_mouse_state();

int _os_msg_box(const std::string& title, const std::string& message, int flg);
int _os_play_sound(const std::string& filename, int flg);
int _os_move_window(int x, int y, int dx, int dy);
int _os_get_pixel(int x, int y);

int _os_get_window_x() noexcept;
int _os_get_window_y() noexcept;
int _os_get_window_dx() noexcept;
int _os_get_window_dy() noexcept;

int _os_set_topmost() noexcept;


/* -------------------------------------------------------------------------- */

// GDI Console window management (Windows only)
#ifdef _WIN32
void set_gdi_target_window(HWND hwnd);
HWND get_gdi_target_window();
#endif


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // !TINY_NUBASIC_VER
