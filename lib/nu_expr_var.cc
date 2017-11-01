//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_expr_var.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

variant_t expr_var_t::eval(rt_prog_ctx_t& ctx) const
{
    var_scope_t::handle_t scope;

    if (scope == nullptr) {
        scope = ctx.proc_scope.get(ctx.proc_scope.get_type(_name));
    }

    if (scope->is_defined(_name)) {
        if (scope == nullptr) {
            scope = ctx.proc_scope.get(ctx.proc_scope.get_type(_name));
        }

        return (*scope)[_name].first;
    }

    auto vtype = variable_t::type_by_name(_name);
    variant_t value("", vtype, 0);
    scope->define(_name, var_value_t(value, VAR_ACCESS_RW));

    return (*scope)[_name].first;
}

/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */
