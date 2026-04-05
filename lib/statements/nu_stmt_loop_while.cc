//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_loop_while.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_loop_while_t::stmt_loop_while_t(prog_ctx_t& ctx, expr_any_t::handle_t cond)
    : stmt_t(ctx)
    , _condition(cond)
{
    syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
        "Loop-While must be first line statement");

    auto handle = ctx.do_loop_while_metadata.end_find(ctx.compiletime_pc);

    if (!handle) {
        handle = ctx.do_loop_while_metadata.compile_end(ctx.compiletime_pc);
    }

    if (handle) {
        handle->pc_end_stmt = ctx.compiletime_pc;
    }
}


/* -------------------------------------------------------------------------- */

void stmt_loop_while_t::run(rt_prog_ctx_t& ctx)
{
    auto handle = ctx.do_loop_while_metadata.end_find(ctx.runtime_pc);

    if (!handle) {
        rt_error_code_t::get_instance().throw_if(true,
            ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_NO_MATCH_DO,
            "Loop While");
    }

    // Loop condition check
    if (!handle->flag[instrblock_t::EXIT] && _condition->eval(ctx)) {
        // jump to DO statement
        ctx.go_to(handle->pc_begin_stmt);
    } 
    else {
        // LOOP completed, go to next line
        ctx.go_to_next();
    }
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_loop_while_t::get_cl() const noexcept
{
    return stmt_cl_t::DO_END;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
