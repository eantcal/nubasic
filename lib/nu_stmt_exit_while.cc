//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_exit_while.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_exit_while_t::stmt_exit_while_t(prog_ctx_t& ctx)
    : stmt_t(ctx)
{
    ctx.while_metadata.compile_exit_point(ctx.compiletime_pc);
}


/* -------------------------------------------------------------------------- */

void stmt_exit_while_t::run(rt_prog_ctx_t& ctx)
{
    auto handle = ctx.while_metadata.exit_find(ctx.runtime_pc);

    if (!handle)
        rt_error_code_t::get_instance().throw_if(true,
            ctx.runtime_pc.get_line(), rt_error_code_t::E_EXIT_WHILE_OUT, "");

    handle->flag.set(instrblock_t::EXIT, true);

    if (handle->pc_end_stmt.get_line() < 1)
        rt_error_code_t::get_instance().throw_if(true,
            ctx.runtime_pc.get_line(), rt_error_code_t::E_NO_MATCH_WEND,
            "Exit While");

    ctx.go_to(handle->pc_end_stmt);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
