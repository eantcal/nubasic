//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_let.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_let_t::run(rt_prog_ctx_t& ctx)
{
    variant_t val = _arg->eval(ctx);

    size_t idx = 0;

    var_scope_t::handle_t scope;
    variant_t* var = nullptr;

    if (_struct_member) {
        if (_vect_idx) {
            idx = _vect_idx->eval(ctx).to_int();
        }

        var = ctx.get_struct_member_value(_variable, scope, idx);

        rt_error_code_t::get_instance().throw_if(!var,
            ctx.runtime_pc.get_line(), rt_error_code_t::E_TYPE_MISMATCH,
            "'" + _variable + "'");
    }

    if (!var) {
        if (scope == nullptr) {
            scope = ctx.proc_scope.get(ctx.proc_scope.get_type(_variable));
        }

        auto& v = (*scope)[_variable];

        const bool const_var = (v.second & VAR_ACCESS_RO) == VAR_ACCESS_RO;

        if (const_var) {
            rt_error_code_t::get_instance().throw_if(true,
                ctx.runtime_pc.get_line(), rt_error_code_t::E_CANNOT_MOD_CONST,
                "'" + _variable + "'");
        }

        var = &(v.first);
    }

    variable_t::type_t vart = var->get_type();

    if (vart == variable_t::type_t::ANY || vart == variable_t::type_t::OBJECT) {
        *var = val;
        ctx.go_to_next();
        return;
    }

    if (vart == variable_t::type_t::UNDEFINED) {
        vart = variable_t::type_by_name(_variable);
    }

    bool is_vector = var->is_vector();

    if (_element_vect_idx != nullptr) {
        is_vector = true;
        _vect_idx = _element_vect_idx;
    }

    if (is_vector) {
        if (_vect_idx) {
            size_t idx = _vect_idx->eval(ctx).to_int();

            rt_error_code_t::get_instance().throw_if(idx >= var->vector_size(),
                ctx.runtime_pc.get_line(),
                rt_error_code_t::E_VEC_IDX_OUT_OF_RANGE,
                "'" + _variable + "(" + nu::to_string(idx) + ")'");

            if (val.is_struct()) {
                rt_error_code_t::get_instance().throw_if(
                    var->struct_type_name() != val.struct_type_name(),
                    ctx.runtime_pc.get_line(), rt_error_code_t::E_TYPE_MISMATCH,
                    "'" + _variable + "(" + nu::to_string(idx) + ")'");

                var->set_struct_value(val, idx);
            } 
            else {
                _assign<size_t>(ctx, *var, val, vart, idx);
            }
        } 
        else {
            // destination vector size will be set to the source size

            rt_error_code_t::get_instance().throw_if(
                var->get_type() != val.get_type(),
                ctx.runtime_pc.get_line(), rt_error_code_t::E_TYPE_MISMATCH,
                "'" + _variable + "'");

            scope->define(_variable, var_value_t(val, VAR_ACCESS_RW));
        }

    } 
    else {
        if (val.is_struct()) {
            var->set_struct_value(val, 0);
        } 
        else {
            _assign<>(ctx, *var, val, vart);
        }
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
