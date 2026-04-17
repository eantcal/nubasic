//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_endsub.h"
#include "nu_error_codes.h"
#include "nu_prog_ctx.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

// Apply ByRef writebacks: called after exit_scope() so the current scope is
// the caller's.  Qualified names (e.g. "obj.field") are resolved via
// get_struct_member_value; simple names go straight to the scope map.
static void apply_byref_writebacks(rt_prog_ctx_t& ctx,
    const std::vector<rt_prog_ctx_t::byref_entry_t>& entries,
    const std::vector<variant_t>& values)
{
    for (size_t i = 0; i < entries.size(); ++i) {
        const std::string& caller_var = entries[i].second;
        const variant_t& val = values[i];

        auto dot = caller_var.find('.');
        if (dot != std::string::npos) {
            // Qualified struct member — resolve into the correct scope
            const std::string root = caller_var.substr(0, dot);
            var_scope_t::handle_t scope
                = ctx.proc_scope.get(ctx.proc_scope.get_type(root));
            variant_t* target = ctx.get_struct_member_value(caller_var, scope);
            if (target)
                *target = val;
        } else {
            auto scope_type = ctx.proc_scope.get_type(caller_var);
            if (scope_type == proc_scope_t::type_t::UNDEF)
                scope_type = proc_scope_t::type_t::GLOBAL;
            auto scope = ctx.proc_scope.get(scope_type);
            if (scope && scope->is_defined(caller_var))
                (*scope)[caller_var].first = val;
        }
    }
}


/* -------------------------------------------------------------------------- */

stmt_endsub_t::stmt_endsub_t(prog_ctx_t& ctx)
    : stmt_t(ctx)
{
    syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
        "END procedure must be a first line-statement");

    auto handle = ctx.procedure_metadata.end_find(ctx.compiletime_pc);

    if (!handle) {
        handle = ctx.procedure_metadata.compile_end(ctx.compiletime_pc);
    }

    if (handle) {
        handle->pc_end_stmt = ctx.compiletime_pc;
    }
}


/* -------------------------------------------------------------------------- */

void stmt_endsub_t::run(rt_prog_ctx_t& ctx)
{
    auto handle = ctx.procedure_metadata.end_find(ctx.runtime_pc);

    if (!handle) {
        rt_error_code_t::get_instance().throw_if(true,
            ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_NO_MATCH_SUB,
            "");
    }

    if (!handle->flag[instrblock_t::EXIT]) {
        ctx.flag.set(rt_prog_ctx_t::FLG_RETURN_REQUEST, true);

        // Collect ByRef values from callee scope BEFORE exiting it
        std::vector<rt_prog_ctx_t::byref_entry_t> byref_entries;
        std::vector<variant_t> byref_values;
        if (!ctx.byref_writeback_stack.empty()) {
            byref_entries = std::move(ctx.byref_writeback_stack.back());
            ctx.byref_writeback_stack.pop_back();
            auto callee_scope = ctx.proc_scope.get();
            for (const auto& e : byref_entries) {
                if (callee_scope && callee_scope->is_defined(e.first))
                    byref_values.push_back((*callee_scope)[e.first].first);
                else
                    byref_values.push_back(variant_t());
            }
        }

        // Clean up any FOR-loop dynamic data
        const auto& scope_name = ctx.proc_scope.get_scope_id();
        ctx.for_loop_tbl.cleanup_data(scope_name);

        if (ctx.debug_mode && !ctx.call_stack.empty())
            ctx.call_stack.pop_back();

        ctx.proc_scope.exit_scope();

        // Write collected ByRef values back to caller scope
        if (!byref_entries.empty())
            apply_byref_writebacks(ctx, byref_entries, byref_values);
    } else {
        // SUB completed, go to next line
        ctx.go_to_next();
    }
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_endsub_t::get_cl() const noexcept
{
    return stmt_cl_t::SUB_END;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
