//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_mybase_call.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"
#include "nu_var_scope.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_mybase_call_t::run(rt_prog_ctx_t& ctx)
{
    const auto rt_error_if = [&](bool cond, rt_error_code_t::value_t err,
                                 const std::string& desc) {
        rt_error_code_t::get_instance().throw_if(
            cond, ctx.runtime_pc.get_line(), err, desc);
    };

    // The mangled base-class method name was resolved at parse time.
    const std::string mangled = _base_class + "." + _method_name;

    rt_error_if(!ctx.is_class_member_access_allowed(mangled),
        rt_error_code_t::value_t::E_MEMBER_ACCESS, mangled);

    // Retrieve Me from the current (callee) scope.
    auto callee_scope = ctx.proc_scope.get();
    rt_error_if(!callee_scope || !callee_scope->is_defined("me"),
        rt_error_code_t::value_t::E_VAR_UNDEF,
        "Me (MyBase call outside method)");

    variant_t me_value = (*callee_scope)["me"].first;

    // Locate the base-class method prototype.
    auto proto_it = ctx.proc_prototypes.data.find(mangled);
    rt_error_if(proto_it == ctx.proc_prototypes.data.end(),
        rt_error_code_t::value_t::E_SUB_UNDEF, mangled);

    const auto& prototype = proto_it->second.second;
    rt_error_if(prototype.parameters.size() != _args.size(),
        rt_error_code_t::value_t::E_WRG_NUM_ARGS, mangled);

    // Evaluate arguments.
    std::vector<variant_t> values;
    values.reserve(_args.size());
    for (const auto& arg : _args)
        values.emplace_back(arg.first ? arg.first->eval(ctx) : variant_t(""));

    // Build ByRef writeback frame.
    // "me" is written back to the callee scope (our caller injected it there).
    {
        std::vector<rt_prog_ctx_t::byref_entry_t> frame;
        auto parg = prototype.parameters.begin();
        auto aarg = _args.begin();
        for (; parg != prototype.parameters.end(); ++parg, ++aarg) {
            if (parg->by_ref && aarg->first) {
                const std::string caller_var = aarg->first->name();
                if (!caller_var.empty())
                    frame.emplace_back(parg->var_name, caller_var);
            }
        }
        // Write Me back to the current scope's "me" variable on return.
        frame.emplace_back("me", "me");
        ctx.byref_writeback_stack.push_back(std::move(frame));
    }

    // Jump to the base-class method body.
    ctx.set_return_line(
        std::make_pair(ctx.runtime_pc.get_line(), get_stmt_id()));
    ctx.go_to(proto_it->second.first);
    ctx.proc_scope.enter_scope(mangled, false);

    // Inject Me into the new scope.
    auto new_scope = ctx.proc_scope.get();
    new_scope->define("me", var_value_t(me_value, VAR_ACCESS_RW));

    // Bind formal parameters.
    if (!values.empty()) {
        auto arg_it = prototype.parameters.begin();
        for (const auto& val : values) {
            new_scope->define(
                arg_it->var_name, var_value_t(val, VAR_ACCESS_RW));
            ++arg_it;
        }
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
