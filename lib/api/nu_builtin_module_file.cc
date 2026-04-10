//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_builtin_function_support.h"
#include "nu_builtin_registry.h"

#include <cerrno>
#include <cstdio>
#include <cstring>

namespace nu {
namespace {

    class file_builtin_module_t final : public builtin_module_t {
    public:
        const std::string& name() const noexcept override
        {
            static const std::string module_name = "file";
            return module_name;
        }

        const builtin_export_list_t& exports() const noexcept override
        {
            static const builtin_export_list_t module_exports
                = { "errno", "errno$", "eof", "ferror", "ftell", "fsize" };
            return module_exports;
        }

        void register_functions(global_function_tbl_t& fmap) const override
        {
            auto functor_errno = [](rt_prog_ctx_t& ctx, const std::string& name,
                                     const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(ctx.get_errno());
            };
            fmap["errno"] = functor_errno;

            auto functor_errno_str
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      std::vector<variant_t> vargs;
                      get_functor_vargs(ctx, name, args,
                          { variant_t::type_t::INTEGER }, vargs);
                      return variant_t(strerror(int(vargs[0].to_int())));
                  };
            fmap["errno$"] = functor_errno_str;

            auto functor_eof = [](rt_prog_ctx_t& ctx, const std::string& name,
                                   const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::INTEGER }, vargs);

                auto file = ctx.file_tbl.resolve_fd(int(vargs[0].to_int()));
                if (!file) {
                    ctx.set_errno(EINVAL);
                    return variant_t(-1);
                }

                return variant_t(feof(file));
            };
            fmap["eof"] = functor_eof;

            auto functor_ferror = [](rt_prog_ctx_t& ctx,
                                      const std::string& name,
                                      const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::INTEGER }, vargs);

                auto file = ctx.file_tbl.resolve_fd(int(vargs[0].to_int()));
                if (!file) {
                    ctx.set_errno(EINVAL);
                    return variant_t(-1);
                }

                return variant_t(ferror(file));
            };
            fmap["ferror"] = functor_ferror;

            auto functor_ftell = [](rt_prog_ctx_t& ctx, const std::string& name,
                                     const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::INTEGER }, vargs);

                auto file = ctx.file_tbl.resolve_fd(int(vargs[0].to_int()));
                if (!file) {
                    ctx.set_errno(EINVAL);
                    return variant_t(-1);
                }

                return variant_t(int(ftell(file)));
            };
            fmap["ftell"] = functor_ftell;

            auto functor_fsize = [](rt_prog_ctx_t& ctx, const std::string& name,
                                     const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::INTEGER }, vargs);

                auto file = ctx.file_tbl.resolve_fd(int(vargs[0].to_int()));
                if (!file) {
                    ctx.set_errno(EINVAL);
                    return variant_t(-1);
                }

                const int cur_pos = ftell(file);
                if (cur_pos < 0 || fseek(file, 0, SEEK_END) < 0) {
                    ctx.set_errno(EINVAL);
                    return variant_t(-1);
                }

                const auto size = ftell(file);
                if (fseek(file, cur_pos, SEEK_SET) < 0) {
                    ctx.set_errno(EINVAL);
                    return variant_t(-1);
                }

                return variant_t(integer_t(size));
            };
            fmap["fsize"] = functor_fsize;
        }
    };

} // namespace

const builtin_module_t& get_file_builtin_module()
{
    static file_builtin_module_t module;
    return module;
}

} // namespace nu
