//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "nu_expr_function.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

bool expr_function_t::empty() const noexcept { return false; }


/* -------------------------------------------------------------------------- */

variant_t expr_function_t::eval(rt_prog_ctx_t& ctx) const
{
    if (!global_function_tbl_t::get_instance().is_defined(_name)) {

        var_scope_t::handle_t scope;
        variant_t* var = nullptr;

        if (scope == nullptr) {
            scope = ctx.proc_scope.get(ctx.proc_scope.get_type(_name));
        }

        if (!var && scope->is_defined(_name)) {
            var = &(((*scope)[_name]).first);
        }

        if (!var) {
            throw exception_t(
                std::string("Error: \"" + _name + "\" undefined symbol"));
        }

        return (*var)[_var[0]->eval(ctx).to_int()];
    }

    return global_function_tbl_t::get_instance()[_name](ctx, _name, _var);
}


/* -------------------------------------------------------------------------- */

bool expr_new_t::empty() const noexcept { return false; }


/* -------------------------------------------------------------------------- */

variant_t expr_new_t::eval(rt_prog_ctx_t& ctx) const
{
    const auto& sprototypes = ctx.struct_prototypes.data;
    const auto prototype_it = sprototypes.find(_class_name);

    auto& errors = rt_error_code_t::get_instance();
    const auto line = ctx.runtime_pc.get_line();

    errors.throw_if(
        prototype_it == sprototypes.end() || !ctx.is_class_type(_class_name),
        line, rt_error_code_t::value_t::E_CLASS_UNDEF, _class_name);

    variant_t value
        = variant_t::make_object_instance(prototype_it->second.second);

    const std::string ctor_key = _class_name + ".new";
    const auto ctor_it = ctx.proc_prototypes.data.find(ctor_key);

    if (ctor_it != ctx.proc_prototypes.data.end()) {
        errors.throw_if(!ctx.is_class_member_access_allowed(ctor_key), line,
            rt_error_code_t::value_t::E_MEMBER_ACCESS, ctor_key);

        ctx.program().run_method(ctor_key, _args, std::string(), value);
    } else {
        errors.throw_if(!_args.empty(), line,
            rt_error_code_t::value_t::E_SUB_UNDEF, ctor_key);
    }

    return value;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
