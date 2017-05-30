//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_expr_unary_op.h"
#include "nu_global_function_tbl.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

expr_unary_op_t::expr_unary_op_t(
    const std::string& op_name, expr_any_t::handle_t var)
    : _op_name(op_name)
    , _var(var)
{
}


/* -------------------------------------------------------------------------- */

bool expr_unary_op_t::empty() const noexcept { return false; }


/* -------------------------------------------------------------------------- */

variant_t expr_unary_op_t::eval(rt_prog_ctx_t& ctx) const
{

    if (!global_function_tbl_t::get_instance().is_defined(_op_name)) {
        throw exception_t(
            std::string("Error: \"" + _op_name + "\" undefined symbol"));
    }

    func_args_t args;
    args.push_back(_var);

    return global_function_tbl_t::get_instance()[_op_name](ctx, _op_name, args);
}


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */
