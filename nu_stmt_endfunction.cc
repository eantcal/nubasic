/*
*  This file is part of nuBASIC
*
*  nuBASIC is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  nuBASIC is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with nuBASIC; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
*
*  Author: Antonino Calderone <acaldmail@gmail.com>
*
*/


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

    auto handle = ctx.procedure_metadata.end_find(ctx.runtime_pc);

    if (!handle || handle->identifier.empty())
        rt_error_code_t::get_instance().throw_if(true,
            ctx.runtime_pc.get_line(), rt_error_code_t::E_NO_MATCH_FUNC, "");


    if (!handle->flag[instrblock_t::EXIT]) {
        ctx.flag.set(rt_prog_ctx_t::FLG_RETURN_REQUEST, true);

        // Retrieve name of this this function
        const std::string& identifier = handle->identifier;

        auto scope_type = ctx.proc_scope.get_type(identifier);

        // The return-value (same function name) must be defined
        if (scope_type != proc_scope_t::type_t::LOCAL)
            rt_error_code_t::get_instance().throw_if(true,
                ctx.runtime_pc.get_line(), rt_error_code_t::E_NO_RET_VAL,
                " '" + identifier + "' not defined. ");

        bool expected_retval = ctx.proc_scope.is_func_call(identifier);

        if (expected_retval) {
            // Get return-value
            variant_t value = (*(ctx.proc_scope.get()))[identifier].first;

            // Insert the return value in the context
            ctx.function_retval_tbl[identifier].push_back(value);
        }

        // Clean up any FOR-loop dynamic data
        auto scope_name = ctx.proc_scope.get_scope_id();
        ctx.for_loop_tbl.cleanup_data(scope_name);


        // Leave the function scope
        ctx.proc_scope.exit_scope();
    } else {
        // Sub completed, go to next line
        ctx.go_to_next();
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu