//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_endfunction.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_endfunction_t::stmt_endfunction_t(prog_ctx_t& ctx)
    : stmt_endsub_t(ctx)
{
}


/* -------------------------------------------------------------------------- */

void stmt_endfunction_t::run(rt_prog_ctx_t& ctx)
{
    const auto handle = ctx.procedure_metadata.end_find(ctx.runtime_pc);

    if (!handle || handle->identifier.empty()) {
        rt_error_code_t::get_instance().throw_if(true,
            ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_NO_MATCH_FUNC, "");
    }

    if (!handle->flag[instrblock_t::EXIT]) {
        ctx.flag.set(rt_prog_ctx_t::FLG_RETURN_REQUEST, true);

        // Retrieve name of this this function
        const std::string& identifier = handle->identifier;

        const auto scope_type = ctx.proc_scope.get_type(identifier);

        // The return-value (same function name) must be defined
        if (scope_type != proc_scope_t::type_t::LOCAL)
            rt_error_code_t::get_instance().throw_if(true,
                ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_NO_RET_VAL,
                " '" + identifier + "' not defined. ");

        const bool expected_retval = ctx.proc_scope.is_func_call(identifier);

        if (expected_retval) {
            // Get return-value
            const variant_t value = (*(ctx.proc_scope.get()))[identifier].first;

            // Insert the return value in the context
            ctx.function_retval_tbl[identifier].emplace_back(value);
        }

        // Clean up any FOR-loop dynamic data
        const auto scope_name = ctx.proc_scope.get_scope_id();
        ctx.for_loop_tbl.cleanup_data(scope_name);

        // Leave the function scope
        ctx.proc_scope.exit_scope();
    } 
    else {
        // Sub completed, go to next line
        ctx.go_to_next();
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
