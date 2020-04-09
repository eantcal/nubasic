//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_for_to_step.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_for_to_step_t::stmt_cl_t stmt_for_to_step_t::get_cl() const noexcept
{
    return stmt_cl_t::FOR_BEGIN;
}


/* -------------------------------------------------------------------------- */

void stmt_for_to_step_t::run(rt_prog_ctx_t& ctx)
{
    variant_t val = _from_arg->eval(ctx);

    auto scope_id = ctx.proc_scope.get_scope_id();
    auto scope_type = ctx.proc_scope.get_type(_variable);

    std::string counter = _variable;

    if (scope_type != proc_scope_t::type_t::GLOBAL && !scope_id.empty()) {
        counter = scope_id + "::" + _variable;
    }

    auto vartype = variable_t::type_by_name(_variable);

    auto& forctx = ctx.for_loop_tbl[counter];

    // first execution
    if (forctx.flag[for_loop_ctx_t::FLG_FIRST_EXEC]) {
        var_scope_t::handle_t scope = ctx.proc_scope.get(scope_type);

        if (scope->is_defined(_variable)) {
            const auto type = (*scope)[_variable].first.get_type();

            if (type != variable_t::type_t::UNDEFINED) {
                vartype = type;
            }
        }

        switch (vartype) {
        case variable_t::type_t::STRING:
        case variable_t::type_t::BOOLEAN:
            rt_error_code_t::get_instance().throw_if(true,
                ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_TYPE_ILLEGAL,
                "For " + _variable);
            break;

        case variable_t::type_t::FLOAT:
            scope->define(_variable, var_value_t(val.to_real(), VAR_ACCESS_RW));
            break;

        case variable_t::type_t::DOUBLE:
            scope->define(
                _variable, var_value_t(val.to_double(), VAR_ACCESS_RW));
            break;

        case variable_t::type_t::LONG64:
            scope->define(
                _variable, var_value_t(val.to_long64(), VAR_ACCESS_RW));
            break;

        case variable_t::type_t::INTEGER:
        default:
            scope->define(_variable, var_value_t(val.to_int(), VAR_ACCESS_RW));
            break;
        }


        forctx.pc_for_stmt = ctx.runtime_pc;
        forctx.step = _step->eval(ctx);
        forctx.end_counter = _to_arg->eval(ctx);

        auto check_type = [&](variant_t::type_t t) {
            switch (t) {
            case variant_t::type_t::INTEGER:
            case variant_t::type_t::LONG64:
                break;

            case variant_t::type_t::FLOAT:
            case variant_t::type_t::DOUBLE:
                if (vartype != variant_t::type_t::FLOAT && 
                    vartype != variant_t::type_t::DOUBLE) 
                {
                    rt_error_code_t::get_instance().throw_if(
                       true,
                       ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_TYPE_ILLEGAL,
                       "For " + _variable);
                }
                break;

            case variant_t::type_t::STRING:
            case variant_t::type_t::UNDEFINED:
            default:
                rt_error_code_t::get_instance().throw_if(true,
                    ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_TYPE_ILLEGAL,
                    "For " + _variable);
                break;
            }
        };

        check_type(forctx.step.get_type());
        check_type(forctx.end_counter.get_type());

        ctx.go_to_next();
        forctx.flag.set(for_loop_ctx_t::FLG_FIRST_EXEC, false);

        auto handle = ctx.for_loop_metadata.begin_find(ctx.runtime_pc);

        if (handle) {
            handle->flag.set(instrblock_t::EXIT, false);
        }
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
