//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "nu_expr_function.h"


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

        if (scope == nullptr)
            scope = ctx.proc_scope.get(ctx.proc_scope.get_type(_name));

        if (!var && scope->is_defined(_name)) {
            var = &(((*scope)[_name]).first);
        }

        if (!var)
            throw exception_t(
                std::string("Error: \"" + _name + "\" undefined symbol"));

        return (*var)[_var[0]->eval(ctx).to_int()];
    }

    return global_function_tbl_t::get_instance()[_name](ctx, _name, _var);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
