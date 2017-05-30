//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_endif.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_endif_t::stmt_cl_t stmt_endif_t::get_cl() const noexcept
{
    return stmt_cl_t::IF_BLOCK_END;
}


/* -------------------------------------------------------------------------- */

stmt_endif_t::stmt_endif_t(prog_ctx_t& ctx)
    : stmt_t(ctx)
{
    auto& ifctxs = ctx.if_metadata;

    syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
        "'End If' statement must be a first statement of the line");

    syntax_error_if(
        ifctxs.pc_stack.empty(), "'End If': no any if-statement matching");

    auto if_pc = ifctxs.pc_stack.top();
    ifctxs.data[if_pc].pc_endif_stmt = ctx.compiletime_pc;
    ifctxs.block_to_if_line_tbl[ctx.compiletime_pc] = if_pc;
    ifctxs.pc_stack.pop();
}


/* -------------------------------------------------------------------------- */

void stmt_endif_t::run(rt_prog_ctx_t& ctx)
{
    auto& ifctxs = ctx.if_metadata;
    auto ifstmt_pc = ifctxs.block_to_if_line_tbl[ctx.runtime_pc];
    const auto& metadata_it = ifctxs.data.find(ifstmt_pc);

    rt_error_code_t::get_instance().throw_if(metadata_it == ifctxs.data.end(),
        ctx.runtime_pc.get_line(), rt_error_code_t::E_INTERNAL, "If");

    metadata_it->second.condition = false;

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
