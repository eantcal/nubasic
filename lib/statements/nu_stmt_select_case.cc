//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_select_case.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_select_case_t::stmt_select_case_t(
    prog_ctx_t& ctx, expr_any_t::handle_t expression)
    : stmt_t(ctx)
    , _expression(expression)
{
    auto& sc_meta = ctx.select_case_metadata;
    sc_meta.data[ctx.compiletime_pc].pc_select_stmt = ctx.compiletime_pc;
    sc_meta.pc_stack.push(ctx.compiletime_pc);
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_select_case_t::get_cl() const noexcept
{
    return stmt_cl_t::SELECT_CASE_BEGIN;
}


/* -------------------------------------------------------------------------- */

void stmt_select_case_t::run(rt_prog_ctx_t& ctx)
{
    // runtime_pc still points at the SELECT CASE line here
    auto& sc_meta = ctx.select_case_metadata;
    const auto it = sc_meta.data.find(ctx.runtime_pc);

    if (it != sc_meta.data.end()) {
        ctx.select_case_values[ctx.runtime_pc] = _expression->eval(ctx);
        ctx.select_case_matched[ctx.runtime_pc] = false;
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
