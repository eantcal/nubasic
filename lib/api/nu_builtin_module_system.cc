//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_about.h"
#include "nu_builtin_function_support.h"
#include "nu_builtin_registry.h"
#include "nu_os_console.h"
#include "nu_os_std.h"

#include <cstdlib>

namespace nu {
namespace {

    class system_builtin_module_t final : public builtin_module_t {
    public:
        const std::string& name() const noexcept override
        {
            static const std::string module_name = "system";
            return module_name;
        }

        const builtin_export_list_t& exports() const noexcept override
        {
            static const builtin_export_list_t module_exports
                = { "inkey$", "inkey", "getvkey", "pwd$", "getenv$", "getenv",
                      "setenv", "unsetenv", "erase", "rmdir", "mkdir", "pi",
                      "getplatid", "getapppath", "ver$", "time", "systime$",
                      "systime", "getdatetime", "quit" };
            return module_exports;
        }

        void register_functions(global_function_tbl_t& fmap) const override
        {
            auto functor_inkeys = [](rt_prog_ctx_t&, const std::string& name,
                                      const func_args_t& args) {
                check_arg_num(args, 0, name);

                const int ch = _os_kbhit();
                std::string s;
                if (ch) {
                    s.push_back(ch);
                }

                return variant_t(s, variant_t::type_t::STRING);
            };
            fmap["inkey$"] = functor_inkeys;
            fmap["inkey"] = functor_inkeys;

            auto functor_getvkey = [](rt_prog_ctx_t&, const std::string& name,
                                       const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(_os_get_vkey());
            };
            fmap["getvkey"] = functor_getvkey;

            auto functor_pwd = [](rt_prog_ctx_t&, const std::string& name,
                                   const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(
                    _os_get_working_dir(), variant_t::type_t::STRING);
            };
            fmap["pwd$"] = functor_pwd;

            struct _getenv_str {
                std::string operator()(const std::string& x) noexcept
                {
                    std::string ret;
                    char* var = ::getenv(x.c_str());
                    if (var) {
                        ret = var;
                    }

                    return ret;
                }
            };
            fmap["getenv$"] = functor_string_string<_getenv_str>;
            fmap["getenv"] = functor_string_string<_getenv_str>;

            struct _setenv_str {
                int operator()(
                    const std::string& var, const std::string& val) noexcept
                {
                    return _os_setenv(var.c_str(), val.c_str());
                }
            };
            fmap["setenv"] = functor_int_string_string<_setenv_str>;

            struct _unsetenv_str {
                int operator()(const std::string& var) noexcept
                {
                    return _os_unsetenv(var.c_str());
                }
            };
            fmap["unsetenv"] = functor_int_string<_unsetenv_str>;

            struct _erase_file {
                int operator()(const std::string& var) noexcept
                {
                    return _os_erase_file(var.c_str());
                }
            };
            fmap["erase"] = functor_int_string<_erase_file>;

            struct _erase_dir {
                int operator()(const std::string& var) noexcept
                {
                    return _os_erase_dir(var.c_str());
                }
            };
            fmap["rmdir"] = functor_int_string<_erase_dir>;

            struct _make_dir {
                int operator()(const std::string& var) noexcept
                {
                    return _os_make_dir(var.c_str());
                }
            };
            fmap["mkdir"] = functor_int_string<_make_dir>;

            auto functor_pi = [](rt_prog_ctx_t&, const std::string& name,
                                  const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(3.1415926535897F);
            };
            fmap["pi"] = functor_pi;

            auto functor_plat_id = [](rt_prog_ctx_t&, const std::string& name,
                                       const func_args_t& args) {
                check_arg_num(args, 0, name);
#ifdef WIN32
                return variant_t(1);
#else
                return variant_t(2);
#endif
            };
            fmap["getplatid"] = functor_plat_id;

            auto functor_get_app_path
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      return variant_t(_os_get_app_path());
                  };
            fmap["getapppath"] = functor_get_app_path;

            auto functor_ver = [](rt_prog_ctx_t&, const std::string& name,
                                   const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(about::version);
            };
            fmap["ver$"] = functor_ver;

            auto functor_get_time = [](rt_prog_ctx_t&, const std::string& name,
                                        const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(integer_t(nu::_os_get_time()));
            };
            fmap["time"] = functor_get_time;

            auto functor_sys_time = [](rt_prog_ctx_t&, const std::string& name,
                                        const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(_os_get_systime());
            };
            fmap["systime$"] = functor_sys_time;
            fmap["systime"] = functor_sys_time;

            auto functor_getdatetime
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      return _os_get_datetime_struct();
                  };
            fmap["getdatetime"] = functor_getdatetime;

            struct _exit_program {
                int operator()(int retcode) noexcept
                {
                    exit(retcode);
                    return 0;
                }
            };
            fmap["quit"] = functor_int_int<_exit_program>;
        }
    };

} // namespace

const builtin_module_t& get_system_builtin_module()
{
    static system_builtin_module_t module;
    return module;
}

} // namespace nu
