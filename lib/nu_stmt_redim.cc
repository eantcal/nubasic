//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_redim.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_redim_t::run(rt_prog_ctx_t& ctx)
{
    const auto rt_error_if
        = [&](bool cond, rt_error_code_t::value_t err, const std::string desc) {
              rt_error_code_t::get_instance().throw_if(
                  cond, ctx.runtime_pc.get_line(), err, desc);
          };

    for (const auto& [name, value] : _vars) {
        const auto vsize = int(value.second->eval(ctx).to_int());

        rt_error_if(vsize <= 0, rt_error_code_t::value_t::E_INV_VECT_SIZE,
            "ReDim '" + name + "'");

        var_scope_t::handle_t scope
            = ctx.proc_scope.get(ctx.proc_scope.get_type(name));

        rt_error_if(!scope->is_defined(name), rt_error_code_t::value_t::E_VAR_UNDEF,
            "ReDim '" + name + "'");

        const auto vtype = value.first;
        const auto vtype_code = variable_t::type_by_name(vtype); // TODO Struct

        const std::string init_val = 
            vtype_code == variable_t::type_t::STRING ? "" : "0";

        switch (vtype_code) {
        case variable_t::type_t::STRING:
        case variable_t::type_t::DOUBLE:
        case variable_t::type_t::INTEGER:
        case variable_t::type_t::BOOLEAN:
        case variable_t::type_t::BYTEVECTOR:
            scope->define(
                name, var_value_t(variant_t(init_val, vtype_code, vsize),
                          VAR_ACCESS_RW));
            break;

        case variable_t::type_t::UNDEFINED:
        case variable_t::type_t::ANY:
        case variable_t::type_t::OBJECT:
        default:
            rt_error_if(
                true, rt_error_code_t::value_t::E_INV_IDENTIF, "ReDim '" + name + "'");
            break;
        }
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
