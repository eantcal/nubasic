//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_block.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_block_t& stmt_block_t::operator+=(stmt_t::handle_t stmt)
{
    _stmts.push_back(stmt);
    return *this;
}


/* -------------------------------------------------------------------------- */

bool stmt_block_t::run_pos(rt_prog_ctx_t& ctx, int pos)
{
    auto stmt_it = _stmts.begin() + pos;

    prog_pointer_t::stmt_number_t stmt_num = pos;

    bool break_cond = false;

    while (stmt_it != _stmts.end()) {
        ctx.runtime_pc.set_stmt_pos(stmt_num);
        ctx.go_to_next();
        (*stmt_it)->run(ctx);

        // end request must be served asap
        if (ctx.flag[rt_prog_ctx_t::FLG_END_REQUEST]) {
            break_cond = true;
            break;
        }

        if (ctx.flag[rt_prog_ctx_t::FLG_RETURN_REQUEST]) {
            break_cond = true;
            break;
        }

        // jump request
        if (ctx.flag[rt_prog_ctx_t::FLG_JUMP_REQUEST]) {
            // Inner jump to stmt of processing block
            const auto& rt_line = ctx.runtime_pc.get_line();

            if (rt_line && rt_line == ctx.goingto_pc.get_line()) {
                stmt_num = ctx.goingto_pc.get_stmt_pos();
                stmt_it = _stmts.begin() + stmt_num;
                continue;
            } else {
                // exit this block
                break_cond = true;
                break;
            }
        }

        ++stmt_it;
        ++stmt_num;
    }

    return break_cond;
}


/* -------------------------------------------------------------------------- */

int stmt_block_t::find_stmt_pos(int stmtid) const noexcept
{
    int stmt_pos = -1;

    auto stmt_it = _stmts.cbegin();
    prog_pointer_t::stmt_number_t stmt_num = 0;

    while (stmt_it != _stmts.cend()) {
        if (*stmt_it && (*stmt_it)->get_stmt_id() == stmtid) {
            stmt_pos = stmt_num;
            break;
        }

        ++stmt_it;
        ++stmt_num;
    }

    return stmt_pos;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
