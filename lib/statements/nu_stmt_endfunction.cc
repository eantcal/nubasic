//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_endfunction.h"
#include "nu_error_codes.h"
#include "nu_prog_ctx.h"
#include "nu_rt_prog_ctx.h"
#include "nu_var_scope.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_endfunction_t::stmt_endfunction_t(prog_ctx_t& ctx)
    : stmt_endsub_t(ctx)
{
}


/* -------------------------------------------------------------------------- */

void stmt_endfunction_t::run(rt_prog_ctx_t& ctx)
{
    const auto handle = ctx.procedure_metadata.end_find(ctx.runtime_pc);

    if (!handle || handle->identifier.empty()) {
        rt_error_code_t::get_instance().throw_if(true,
            ctx.runtime_pc.get_line(),
            rt_error_code_t::value_t::E_NO_MATCH_FUNC, "");
    }

    if (!handle->flag[instrblock_t::EXIT]) {
        ctx.flag.set(rt_prog_ctx_t::FLG_RETURN_REQUEST, true);

        // Retrieve name of this function
        const std::string& identifier = handle->identifier;

        // For class methods the identifier is mangled as
        // "ClassName.MethodName". Inside the body the programmer writes
        // "MethodName = value", so the return variable in the local scope uses
        // the short (unmangled) name.
        const auto dot = identifier.rfind('.');
        const std::string short_name = (dot != std::string::npos)
            ? identifier.substr(dot + 1)
            : identifier;

        auto scope_type = ctx.proc_scope.get_type(identifier);
        std::string retval_var = identifier;
        if (scope_type != proc_scope_t::type_t::LOCAL
            && short_name != identifier) {
            const auto short_type = ctx.proc_scope.get_type(short_name);
            if (short_type == proc_scope_t::type_t::LOCAL) {
                retval_var = short_name;
                scope_type = short_type;
            }
        }

        // The return-value (same function name) must be defined
        if (scope_type != proc_scope_t::type_t::LOCAL)
            rt_error_code_t::get_instance().throw_if(true,
                ctx.runtime_pc.get_line(),
                rt_error_code_t::value_t::E_NO_RET_VAL,
                " '" + identifier + "' not defined. ");

        const bool expected_retval = ctx.proc_scope.is_func_call(identifier);

        if (expected_retval) {
            // Get return-value (use short/unmangled name if needed)
            const variant_t value = (*(ctx.proc_scope.get()))[retval_var].first;

            // Insert the return value in the context under the full mangled
            // name
            ctx.function_retval_tbl[identifier].emplace_back(value);
        }

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
        const auto scope_name = ctx.proc_scope.get_scope_id();
        ctx.for_loop_tbl.cleanup_data(scope_name);

        if (ctx.debug_mode && !ctx.call_stack.empty())
            ctx.call_stack.pop_back();

        // Leave the function scope
        ctx.proc_scope.exit_scope();

        // Write collected ByRef values back to caller scope
        if (!byref_entries.empty()) {
            // Reuse the helper defined in nu_stmt_endsub.cc (via forward decl)
            // To avoid linking issues, inline the writeback here
            for (size_t i = 0; i < byref_entries.size(); ++i) {
                const std::string& caller_var = byref_entries[i].second;
                const variant_t& val = byref_values[i];
                auto dot = caller_var.find('.');
                if (dot != std::string::npos) {
                    const std::string root = caller_var.substr(0, dot);
                    var_scope_t::handle_t scope
                        = ctx.proc_scope.get(ctx.proc_scope.get_type(root));
                    variant_t* target
                        = ctx.get_struct_member_value(caller_var, scope);
                    if (target)
                        *target = val;
                } else {
                    auto stype = ctx.proc_scope.get_type(caller_var);
                    if (stype == proc_scope_t::type_t::UNDEF)
                        stype = proc_scope_t::type_t::GLOBAL;
                    auto scope = ctx.proc_scope.get(stype);
                    if (scope && scope->is_defined(caller_var))
                        (*scope)[caller_var].first = val;
                }
            }
        }
    } else {
        // Sub completed, go to next line
        ctx.go_to_next();
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
