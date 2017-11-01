//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_expr_subscrop.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

variant_t expr_subscrop_t::eval(rt_prog_ctx_t& ctx) const
{
    var_scope_t::handle_t scope
        = ctx.proc_scope.get(ctx.proc_scope.get_type(_name));

    if (!scope->is_defined(_name)) {
        throw exception_t(
            std::string("Error: \"" + _name + "\" undefined symbol"));
    }

    const variant_t& var_value = (*scope)[_name].first;
    (void)var_value; // TODO

    // TODO apply to string!
    // if (var_value.is_vector())
    //    return var_value[_var[0]->eval(ctx).to_int()];

    throw exception_t(std::string("Cannot evaluate \"" + _name + "\""));
}


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */
