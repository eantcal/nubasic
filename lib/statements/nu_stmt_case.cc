//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_case.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

bool case_condition_t::matches(const variant_t& sv, rt_prog_ctx_t& ctx) const
{
    switch (kind) {
    case kind_t::EQUAL: {
        auto v = expr1->eval(ctx);
        return static_cast<bool>(sv == v);
    }
    case kind_t::RANGE: {
        auto lo = expr1->eval(ctx);
        auto hi = expr2->eval(ctx);
        return static_cast<bool>(sv >= lo) && static_cast<bool>(sv <= hi);
    }
    case kind_t::IS_COMPARISON: {
        auto v = expr1->eval(ctx);
        if (op == ">")
            return static_cast<bool>(sv > v);
        if (op == ">=")
            return static_cast<bool>(sv >= v);
        if (op == "<")
            return static_cast<bool>(sv < v);
        if (op == "<=")
            return static_cast<bool>(sv <= v);
        if (op == "=" || op == "==")
            return static_cast<bool>(sv == v);
        if (op == "<>")
            return static_cast<bool>(sv != v);
        return false;
    }
    }
    return false;
}


/* -------------------------------------------------------------------------- */

// CASE <condition-list>
stmt_case_t::stmt_case_t(prog_ctx_t& ctx, conditions_t conditions)
    : stmt_t(ctx)
    , _conditions(std::move(conditions))
    , _is_else(false)
{
    syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
        "'Case' must be the first statement of the code line");

    auto& sc_meta = ctx.select_case_metadata;

    syntax_error_if(
        sc_meta.pc_stack.empty(), "'Case': no matching 'Select Case'");

    const auto select_pc = sc_meta.pc_stack.top();
    sc_meta.data[select_pc].case_list.push_back(ctx.compiletime_pc);
    sc_meta.block_to_select_line_tbl[ctx.compiletime_pc] = select_pc;
}


/* -------------------------------------------------------------------------- */

// CASE ELSE
stmt_case_t::stmt_case_t(prog_ctx_t& ctx)
    : stmt_t(ctx)
    , _is_else(true)
{
    syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
        "'Case Else' must be the first statement of the code line");

    auto& sc_meta = ctx.select_case_metadata;

    syntax_error_if(
        sc_meta.pc_stack.empty(), "'Case Else': no matching 'Select Case'");

    const auto select_pc = sc_meta.pc_stack.top();
    auto& block = sc_meta.data[select_pc];

    syntax_error_if(block.has_case_else,
        "'Case Else' already defined for this 'Select Case'");

    block.case_list.push_back(ctx.compiletime_pc);
    block.has_case_else = true;
    block.pc_case_else = ctx.compiletime_pc;
    sc_meta.block_to_select_line_tbl[ctx.compiletime_pc] = select_pc;
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_case_t::get_cl() const noexcept
{
    return stmt_cl_t::CASE_OF;
}


/* -------------------------------------------------------------------------- */

void stmt_case_t::run(rt_prog_ctx_t& ctx)
{
    auto& sc_meta = ctx.select_case_metadata;

    const auto tbl_it = sc_meta.block_to_select_line_tbl.find(ctx.runtime_pc);

    rt_error_code_t::get_instance().throw_if(
        tbl_it == sc_meta.block_to_select_line_tbl.end(),
        ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_INTERNAL,
        "Case");

    const auto select_pc = tbl_it->second;

    const auto data_it = sc_meta.data.find(select_pc);

    rt_error_code_t::get_instance().throw_if(data_it == sc_meta.data.end(),
        ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_INTERNAL,
        "Case");

    auto& block = data_it->second;

    // If a previous CASE already matched, skip to End Select
    const auto matched_it = ctx.select_case_matched.find(select_pc);
    if (matched_it != ctx.select_case_matched.end() && matched_it->second) {
        ctx.go_to(block.pc_end_select_stmt);
        return;
    }

    // Evaluate whether this CASE matches
    bool matches = false;

    if (_is_else) {
        matches = true;
    } else {
        const auto val_it = ctx.select_case_values.find(select_pc);
        if (val_it != ctx.select_case_values.end()) {
            const auto& sv = val_it->second;
            for (const auto& cond : _conditions) {
                if (cond.matches(sv, ctx)) {
                    matches = true;
                    break;
                }
            }
        }
    }

    if (matches) {
        ctx.select_case_matched[select_pc] = true;
        ctx.go_to_next();
    } else {
        // Jump to the next CASE clause, or to End Select if there is none
        const auto& case_list = block.case_list;
        auto it = case_list.begin();

        for (; it != case_list.end(); ++it) {
            if (it->get_line() == ctx.runtime_pc.get_line()) {
                ++it;
                break;
            }
        }

        if (it == case_list.end()) {
            ctx.go_to(block.pc_end_select_stmt);
        } else {
            ctx.go_to(*it);
        }
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
