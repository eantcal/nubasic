//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_read.h"
#include "nu_os_console.h"
#include "nu_rt_prog_ctx.h"
#include <cstdio>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_read_t::run(rt_prog_ctx_t& ctx)
{
    rt_error_code_t::get_instance().throw_if(
        ctx.read_data_store_index >= ctx.read_data_store.size(),
        ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_VAL_OUT_OF_RANGE,
        "Read");

    for (auto const& variable : _vars) {
        auto index = variable.second;
        auto name = variable.first;

        const auto & data_value = ctx.read_data_store[ctx.read_data_store_index++];

        bool is_vector = index != nullptr;

        if (is_vector) {
            var_scope_t::handle_t scope
                = ctx.proc_scope.get(ctx.proc_scope.get_type(name));

            rt_error_code_t::get_instance().throw_if(!scope->is_defined(name),
                ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_VAR_UNDEF,
                "'" + name + "'");

            size_t idx = index->eval(ctx).to_int();

            auto& v = (*scope)[name];
            variant_t var = v.first;
            const bool const_var = (v.second & VAR_ACCESS_RO) == VAR_ACCESS_RO;

            rt_error_code_t::get_instance().throw_if(const_var,
                ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_CANNOT_MOD_CONST,
                "'" + name + "'");

            rt_error_code_t::get_instance().throw_if(idx >= var.vector_size(),
                ctx.runtime_pc.get_line(),
                rt_error_code_t::value_t::E_VEC_IDX_OUT_OF_RANGE, "'" + name + "'");

            variant_t::type_t t = var.get_type();

            if (t == variant_t::type_t::UNDEFINED)
                t = variable_t::type_by_name(name);

            switch (t) {
            case variant_t::type_t::UNDEFINED:
            case variant_t::type_t::ANY:
            case variant_t::type_t::OBJECT:
            case variant_t::type_t::STRUCT:
                rt_error_code_t::get_instance().throw_if(true,
                    ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_TYPE_ILLEGAL,
                    "'" + name + "'");
                break;

            case variant_t::type_t::STRING:
                var.set_str(data_value.to_str(), idx);
                break;

            case variant_t::type_t::FLOAT:
                var.set_real(data_value.to_real(), idx);
                break;

            case variant_t::type_t::DOUBLE:
                var.set_double(data_value.to_double(), idx);
                break;

            case variant_t::type_t::INTEGER:
                var.set_int(data_value.to_int(), idx);
                break;

            case variant_t::type_t::BYTEVECTOR:
                var.set_bvect(data_value.to_int(), idx);
                break;

            case variant_t::type_t::BOOLEAN:
                var.set_bool(data_value.to_bool(), idx);
                break;

            case variant_t::type_t::LONG64:
                var.set_long64(data_value.to_long64(), idx);
                break;
            }

            scope->define(name, var_value_t(var, VAR_ACCESS_RW));
        } 
        else {
            var_scope_t::handle_t scope
                = ctx.proc_scope.get(ctx.proc_scope.get_type(name));

            auto& v = (*scope)[name];
            const bool const_var = (v.second & VAR_ACCESS_RO) == VAR_ACCESS_RO;

            rt_error_code_t::get_instance().throw_if(const_var,
                ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_CANNOT_MOD_CONST,
                "'" + name + "'");

            variant_t var = v.first;
            variant_t::type_t t = var.get_type();

            if (t == variant_t::type_t::UNDEFINED)
                t = variable_t::type_by_name(name);

            switch (t) {
            case variable_t::type_t::STRING:
                scope->define(name, var_value_t(data_value.to_str(), VAR_ACCESS_RW));
                break;

            case variable_t::type_t::DOUBLE:
                scope->define(name, var_value_t(data_value.to_double(), VAR_ACCESS_RW));
                break;

            case variable_t::type_t::FLOAT:
                scope->define(name, var_value_t(data_value.to_real(), VAR_ACCESS_RW));
                break;

            case variable_t::type_t::LONG64:
                scope->define(name, var_value_t(data_value.to_long64(), VAR_ACCESS_RW));
                break;

            case variable_t::type_t::BOOLEAN:
                scope->define(name, var_value_t(data_value.to_bool(), VAR_ACCESS_RW));
                break;

            case variable_t::type_t::INTEGER:
            default:
                scope->define(name, var_value_t(data_value.to_int(), VAR_ACCESS_RW));
                break;

            case variant_t::type_t::UNDEFINED:
            case variant_t::type_t::STRUCT:
                rt_error_code_t::get_instance().throw_if(true,
                    ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_TYPE_ILLEGAL,
                    "'" + name + "'");
                break;
            }
        }
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
