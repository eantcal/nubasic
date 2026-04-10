//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_end_select.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_end_select_t::stmt_end_select_t(prog_ctx_t& ctx)
    : stmt_t(ctx)
{
    syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
        "'End Select' must be the first statement of the code line");

    auto& sc_meta = ctx.select_case_metadata;

    syntax_error_if(
        sc_meta.pc_stack.empty(), "'End Select': no matching 'Select Case'");

    const auto select_pc = sc_meta.pc_stack.top();

    sc_meta.data[select_pc].pc_end_select_stmt = ctx.compiletime_pc;
    sc_meta.block_to_select_line_tbl[ctx.compiletime_pc] = select_pc;
    sc_meta.pc_stack.pop();
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_end_select_t::get_cl() const noexcept
{
    return stmt_cl_t::SELECT_CASE_END;
}


/* -------------------------------------------------------------------------- */

void stmt_end_select_t::run(rt_prog_ctx_t& ctx)
{
    auto& sc_meta = ctx.select_case_metadata;
    const auto tbl_it = sc_meta.block_to_select_line_tbl.find(ctx.runtime_pc);

    if (tbl_it != sc_meta.block_to_select_line_tbl.end()) {
        const auto select_pc = tbl_it->second;
        ctx.select_case_matched.erase(select_pc);
        ctx.select_case_values.erase(select_pc);
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
