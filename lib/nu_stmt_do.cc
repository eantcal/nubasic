//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_do.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"
#include "nu_stmt_empty.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_do_t::stmt_do_t(prog_ctx_t& ctx)
    : stmt_t(ctx)
{
    syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
        "Do-statement must be first line statement");

    ctx.do_loop_while_metadata.compile_begin(ctx.compiletime_pc);
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_do_t::get_cl() const noexcept
{
    return stmt_cl_t::DO_BEGIN;
}


/* -------------------------------------------------------------------------- */

void stmt_do_t::run(rt_prog_ctx_t& ctx)
{
    //  Do
    //   ...
    //  Loop While <cond>
    auto& dowctxs = ctx.do_loop_while_metadata;
    auto handle = dowctxs.begin_find(ctx.runtime_pc);

    if (!handle)
        rt_error_code_t::get_instance().throw_if(true,
            ctx.runtime_pc.get_line(), rt_error_code_t::E_INTERNAL,
            "Do... Loop While");

    handle->flag.set(instrblock_t::EXIT, false);
    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
