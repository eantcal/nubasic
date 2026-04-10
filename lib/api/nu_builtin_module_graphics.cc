//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_builtin_function_support.h"
#include "nu_builtin_registry.h"

#ifndef TINY_NUBASIC_VER
#include "nu_os_gdi.h"
#include "nu_playsnd.h"
#endif

namespace nu {
namespace {

    class graphics_builtin_module_t final : public builtin_module_t {
    public:
        const std::string& name() const noexcept override
        {
            static const std::string module_name = "graphics";
            return module_name;
        }

        const builtin_export_list_t& exports() const noexcept override
        {
            static const builtin_export_list_t module_exports
                = { "getswidth", "getsheight", "getwindowx", "getwindowy",
                      "getwindowdx", "getwindowdy", "getmouse", "settopmost",
                      "msgbox", "playsound", "movewindow", "getpixel", "rgb" };
            return module_exports;
        }

        void register_functions(global_function_tbl_t& fmap) const override
        {
#ifndef TINY_NUBASIC_VER
            auto functor_getswidth = [](rt_prog_ctx_t&, const std::string& name,
                                         const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(_os_get_screen_width());
            };
            fmap["getswidth"] = functor_getswidth;

            auto functor_getsheight
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      return variant_t(_os_get_screen_height());
                  };
            fmap["getsheight"] = functor_getsheight;

            auto functor_getwindowx
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      return variant_t(_os_get_window_x());
                  };
            fmap["getwindowx"] = functor_getwindowx;

            auto functor_getwindowy
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      return variant_t(_os_get_window_y());
                  };
            fmap["getwindowy"] = functor_getwindowy;

            auto functor_getwindowdx
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      return variant_t(_os_get_window_dx());
                  };
            fmap["getwindowdx"] = functor_getwindowdx;

            auto functor_getwindowdy
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      return variant_t(integer_t(_os_get_window_dy()));
                  };
            fmap["getwindowdy"] = functor_getwindowdy;

            auto functor_getmouse = [](rt_prog_ctx_t&, const std::string& name,
                                        const func_args_t& args) {
                check_arg_num(args, 0, name);
                return _os_get_mouse_state();
            };
            fmap["getmouse"] = functor_getmouse;

            auto functor_set_topmost
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      return variant_t(integer_t(_os_set_topmost()));
                  };
            fmap["settopmost"] = functor_set_topmost;

            struct _msg_box {
                int operator()(const std::string& title,
                    const std::string& message) noexcept
                {
                    return _os_msg_box(title, message, 0);
                }
            };
            fmap["msgbox"] = functor_int_string_string<_msg_box>;

            struct _play_sound {
                int operator()(const std::string& filename, int flg) noexcept
                {
                    return _os_play_sound(filename, flg);
                }
            };
            fmap["playsound"] = functor_int_string_int<_play_sound>;

            struct _move_window {
                int operator()(int x, int y, int dx, int dy) noexcept
                {
                    return _os_move_window(x, y, dx, dy);
                }
            };
            fmap["movewindow"] = functor_int_int_int_int_int<_move_window>;

            struct _get_pixel {
                int operator()(int x, int y) noexcept
                {
                    return _os_get_pixel(x, y);
                }
            };
            fmap["getpixel"] = functor_int_int_int<_get_pixel>;
#endif

            struct _rgb {
                int operator()(int r, int g, int b) noexcept
                {
                    return (
                        (int)(((uint8_t)(r) | ((uint16_t)((uint8_t)(g)) << 8))
                            | (((uint32_t)(uint8_t)(b)) << 16)));
                }
            };
            fmap["rgb"] = functor_int_int_int_int<_rgb>;
        }
    };

} // namespace

const builtin_module_t& get_graphics_builtin_module()
{
    static graphics_builtin_module_t module;
    return module;
}

} // namespace nu
