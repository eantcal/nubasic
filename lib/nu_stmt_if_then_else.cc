//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_if_then_else.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_if_then_else_t::stmt_if_then_else_t(prog_ctx_t& ctx,
    expr_any_t::handle_t condition, handle_t then_stmt, handle_t else_stmt)
    : stmt_t(ctx)
    , _condition(condition)
    , _then_stmt(then_stmt)
    , _else_stmt(else_stmt)
{
    if (!_else_stmt) {
        _else_stmt = std::make_shared<stmt_empty_t>(ctx);
    }

    if (then_stmt->get_cl() == stmt_cl_t::EMPTY) {
        auto& ifctxs = ctx.if_metadata;
        ifctxs.data[ctx.compiletime_pc].pc_if_stmt = ctx.compiletime_pc;
        ifctxs.pc_stack.push(ctx.compiletime_pc);
    }

    auto cl = then_stmt->get_cl();

    syntax_error_if(cl == stmt_cl_t::DO_BEGIN || cl == stmt_cl_t::DO_BEGIN
        || cl == stmt_cl_t::DO_END || cl == stmt_cl_t::DO_END
        || cl == stmt_cl_t::WHILE_BEGIN || cl == stmt_cl_t::WHILE_BEGIN
        || cl == stmt_cl_t::WHILE_END || cl == stmt_cl_t::WHILE_END
        || cl == stmt_cl_t::SUB_BEGIN || cl == stmt_cl_t::SUB_BEGIN
        || cl == stmt_cl_t::SUB_END || cl == stmt_cl_t::SUB_END,
        "This construct is not allowed");
}


/* -------------------------------------------------------------------------- */

stmt_if_then_else_t::stmt_cl_t stmt_if_then_else_t::get_cl() const noexcept
{
    return stmt_cl_t::IF_BLOCK_BEGIN;
}


/* -------------------------------------------------------------------------- */

void stmt_if_then_else_t::run(rt_prog_ctx_t& ctx)
{
    ctx.go_to_next();

    auto& ifctxs = ctx.if_metadata;
    const auto& metadata_it = ifctxs.data.find(ctx.runtime_pc);

    if (_then_stmt->get_cl() == stmt_cl_t::EMPTY
        && metadata_it != ifctxs.data.end())
    {
        if (static_cast<bool>(_condition->eval(ctx)) == false) {
            auto& metadata = metadata_it->second;
            metadata.condition = false;

            ctx.go_to(metadata.else_list.empty() ? metadata.pc_endif_stmt
                : *metadata.else_list.begin());
        }
        else {
            metadata_it->second.condition = true;

            ctx.go_to_next();
        }
    }
    else {
        if (static_cast<bool>(_condition->eval(ctx)) == true) {
            _then_stmt->run(ctx);
        }
        else {
            _else_stmt->run(ctx);
        }
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
