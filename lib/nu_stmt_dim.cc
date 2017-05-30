//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_dim.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_dim_t::run(rt_prog_ctx_t& ctx)
{
    auto rt_error_if = [&](
        bool cond, rt_error_code_t::value_t err, const std::string& desc) {
        rt_error_code_t::get_instance().throw_if(
            cond, ctx.runtime_pc.get_line(), err, desc);
    };

    for (const auto& v : _vars) {
        int vsize = v.second.second->eval(ctx).to_int();
        const std::string& name = v.first;

        auto scope_type = ctx.proc_scope.get_type(name);

        switch (scope_type) {
        case proc_scope_t::type_t::GLOBAL:
            rt_error_if(
                true, rt_error_code_t::E_VAR_REDEF, "Global '" + name + "'");
            break;

        case proc_scope_t::type_t::LOCAL:
            rt_error_if(
                true, rt_error_code_t::E_VAR_REDEF, "Local '" + name + "'");
            break;

        case proc_scope_t::type_t::UNDEF:
        default:
            break;
        }

        rt_error_if(
            vsize < 0, rt_error_code_t::E_INV_VECT_SIZE, "Dim: '" + name + "'");

        var_scope_t::handle_t scope
            = ctx.proc_scope.get(ctx.proc_scope.get_type(name));

        auto vtype = v.second.first;
        auto vtype_code = variable_t::type_by_typename(vtype);

        std::string init_val;

        switch (vtype_code) {
        case variable_t::type_t::STRING:
        case variable_t::type_t::ANY:
            init_val = "";

        case variable_t::type_t::FLOAT:
        case variable_t::type_t::DOUBLE:
        case variable_t::type_t::INTEGER:
        case variable_t::type_t::LONG64:
        case variable_t::type_t::BOOLEAN:
        case variable_t::type_t::BYTEVECTOR:
            init_val = "0";
            scope->define(
                name, var_value_t(variant_t(init_val, vtype_code, vsize),
                          VAR_ACCESS_RW));
            break;

        case variable_t::type_t::STRUCT:
        case variable_t::type_t::UNDEFINED: {
            auto& sprototypes = ctx.struct_prototypes.data;
            auto it = sprototypes.find(vtype);

            rt_error_if(it == sprototypes.end(),
                rt_error_code_t::E_STRUCT_UNDEF, "Dim: '" + name + "'");

            auto value = it->second.second; // struct prototype

            if (vsize > 0)
                value.resize(vsize);

            scope->define(name, var_value_t(value, VAR_ACCESS_RW));
            break;
        }

        default:
            rt_error_if(
                true, rt_error_code_t::E_INV_VECT_SIZE, "Dim: '" + name + "'");
        }
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
