//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_wend.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_wend_t::stmt_wend_t(prog_ctx_t& ctx)
    : stmt_t(ctx)
{
    syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
        "END WHILE or WEND must be first line statement");

    auto handle = ctx.while_metadata.end_find(ctx.compiletime_pc);

    if (!handle)
        handle = ctx.while_metadata.compile_end(ctx.compiletime_pc);

    if (handle)
        handle->pc_end_stmt = ctx.compiletime_pc;
}


/* -------------------------------------------------------------------------- */

void stmt_wend_t::run(rt_prog_ctx_t& ctx)
{
    auto handle = ctx.while_metadata.end_find(ctx.runtime_pc);

    rt_error_code_t::get_instance().throw_if(!handle, ctx.runtime_pc.get_line(),
        rt_error_code_t::E_NO_MATCH_WHILE, "Wend / While End");

    // Loop condition check
    if (!handle->flag[instrblock_t::EXIT]) {
        // jump to while statement
        ctx.go_to(handle->pc_begin_stmt);
    }

    else {
        // LOOP completed, go to next line
        ctx.go_to_next();
    }
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_wend_t::get_cl() const noexcept
{
    return stmt_cl_t::WHILE_END;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
