//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_basic_defs.h"
#include "nu_builtin_function_support.h"
#include "nu_builtin_registry.h"

namespace nu {
namespace {

    class runtime_builtin_module_t final : public builtin_module_t {
    public:
        const std::string& name() const noexcept override
        {
            static const std::string module_name = "runtime";
            return module_name;
        }

        const builtin_export_list_t& exports() const noexcept override
        {
            static const builtin_export_list_t module_exports
                = { "eval", "conv", "restore", "sizeof@", "sizeof" };
            return module_exports;
        }

        void register_functions(global_function_tbl_t& fmap) const override
        {
            fmap["eval"] = evaluate_expression;
            fmap["__eval_export"] = evaluate_and_export_result;
            fmap[NU_BASIC_OP_INC] = process_operator;
            fmap[NU_BASIC_OP_DEC] = process_operator;
            fmap["conv"] = conv_functor;
            fmap["restore"] = restore_functor;

            auto functor_sizeof_bv
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      std::vector<variant_t> vargs;
                      get_functor_vargs(ctx, name, args,
                          { variant_t::type_t::UNDEFINED }, vargs);

                      return variant_t(integer_t(vargs[0].vector_size()));
                  };
            fmap["sizeof@"] = functor_sizeof_bv;

            auto functor_sizeof
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      std::vector<variant_t> vargs;
                      get_functor_vargs(ctx, name, args,
                          { variant_t::type_t::UNDEFINED }, vargs);

                      return variant_t(integer_t(vargs[0].vector_size()));
                  };
            fmap["sizeof"] = functor_sizeof;
        }
    };

} // namespace

const builtin_module_t& get_runtime_builtin_module()
{
    static runtime_builtin_module_t module;
    return module;
}

} // namespace nu
