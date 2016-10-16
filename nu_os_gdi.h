/*
*  This file is part of nuBASIC
*
*  nuBASIC is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  nuBASIC is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with nuBASIC; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
*
*  Author: Antonino Calderone <acaldmail@gmail.com>
*
*/


/* -------------------------------------------------------------------------- */

#if (!defined(__NU_OS_GDI_H__) && !defined(TINY_NUBASIC_VER))
#define __NU_OS_GDI_H__


/* -------------------------------------------------------------------------- */

#include "nu_variant.h"
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class rt_prog_ctx_t;

using gdi_iarg_t = unsigned int;
using gdi_iargs_t = std::vector<gdi_iarg_t>;

using gdi_varg_t = variant_t;
using gdi_vargs_t = std::vector<gdi_varg_t>;


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

int _os_get_mouse_x() noexcept;
int _os_get_mouse_y() noexcept;
int _os_get_mouse_btn() noexcept;

int _os_msg_box(const std::string& title, const std::string& message, int flg);
int _os_play_sound(const std::string& filename, int flg);
int _os_move_window(int x, int y, int dx, int dy);
int _os_get_pixel(int x, int y);

int _os_get_window_x() noexcept;
int _os_get_window_y() noexcept;
int _os_get_window_dx() noexcept;
int _os_get_window_dy() noexcept;


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_OS_GDI_H__