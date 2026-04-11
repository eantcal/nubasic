//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_method_call.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"
#include "nu_var_scope.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_method_call_t::run(rt_prog_ctx_t& ctx)
{
    const auto rt_error_if = [&](bool cond, rt_error_code_t::value_t err,
                                 const std::string& desc) {
        rt_error_code_t::get_instance().throw_if(
            cond, ctx.runtime_pc.get_line(), err, desc);
    };

    // Check for static method call: ClassName.Method(args)
    // A static call is recognized when _obj_name is a known class prototype
    // (not a variable in scope) and the mangled name is in
    // class_static_methods.
    {
        const std::string static_key = _obj_name + "." + _method_name;
        if (ctx.class_static_methods.count(static_key) > 0) {
            auto proto_it = ctx.proc_prototypes.data.find(static_key);
            rt_error_if(proto_it == ctx.proc_prototypes.data.end(),
                rt_error_code_t::value_t::E_SUB_UNDEF, static_key);

            const auto& prototype = proto_it->second.second;
            rt_error_if(prototype.parameters.size() != _args.size(),
                rt_error_code_t::value_t::E_WRG_NUM_ARGS, static_key);

            // Evaluate arguments
            std::vector<variant_t> values;
            values.reserve(_args.size());
            for (const auto& arg : _args)
                values.emplace_back(
                    arg.first ? arg.first->eval(ctx) : variant_t(""));

            // Build ByRef writeback frame (no "me" entry — no instance)
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
                ctx.byref_writeback_stack.push_back(std::move(frame));
            }

            ctx.set_return_line(
                std::make_pair(ctx.runtime_pc.get_line(), get_stmt_id()));
            ctx.go_to(proto_it->second.first);
            ctx.proc_scope.enter_scope(static_key, false);

            // Bind formal parameters (no "Me" injection for static methods)
            auto callee_scope = ctx.proc_scope.get();
            auto arg_it = prototype.parameters.begin();
            for (const auto& val : values) {
                callee_scope->define(
                    arg_it->var_name, var_value_t(val, VAR_ACCESS_RW));
                ++arg_it;
            }
            return;
        }
    }

    // Resolve the object variable to get its class name
    variant_t* obj_ptr = nullptr;
    var_scope_t::handle_t obj_scope;

    if (_obj_name.find('.') != std::string::npos) {
        // Nested qualified name: e.g., "container.item"
        obj_ptr = ctx.get_struct_member_value(_obj_name, obj_scope);
    } else {
        auto scope_type = ctx.proc_scope.get_type(_obj_name);
        obj_scope = ctx.proc_scope.get(scope_type);
        rt_error_if(!obj_scope || !obj_scope->is_defined(_obj_name),
            rt_error_code_t::value_t::E_VAR_UNDEF, _obj_name);
        obj_ptr = &(*obj_scope)[_obj_name].first;
    }

    rt_error_if(!obj_ptr, rt_error_code_t::value_t::E_VAR_UNDEF, _obj_name);
    rt_error_if(!obj_ptr->is_struct(),
        rt_error_code_t::value_t::E_TYPE_MISMATCH,
        _obj_name + " is not a class instance");

    // Mangle to "ClassName.MethodName"
    const std::string class_name = obj_ptr->struct_type_name();
    const std::string mangled = class_name + "." + _method_name;

    // Walk the inheritance chain to find the method.
    // Override semantics are natural: the derived class method is found first.
    auto i = ctx.proc_prototypes.data.cend();
    {
        std::string cls = class_name;
        while (!cls.empty()) {
            const std::string key = cls + "." + _method_name;

            // Private ancestor methods are not callable by external callers.
            if (cls != class_name) {
                auto vis_it = ctx.class_member_visibility.find(key);
                if (vis_it != ctx.class_member_visibility.end()
                    && !vis_it->second) {
                    auto base_it = ctx.class_bases.find(cls);
                    if (base_it == ctx.class_bases.end())
                        break;
                    cls = base_it->second;
                    continue;
                }
            }

            auto it = ctx.proc_prototypes.data.find(key);
            if (it != ctx.proc_prototypes.data.end()) {
                i = it;
                break;
            }
            auto base_it = ctx.class_bases.find(cls);
            if (base_it == ctx.class_bases.end())
                break;
            cls = base_it->second;
        }
    }
    rt_error_if(i == ctx.proc_prototypes.data.cend(),
        rt_error_code_t::value_t::E_SUB_UNDEF, mangled);

    // Check access (public/private) for the resolved method
    {
        auto vis_it = ctx.class_member_visibility.find(i->first);
        if (vis_it != ctx.class_member_visibility.end() && !vis_it->second) {
            const std::string scope_id = ctx.proc_scope.get_scope_id();
            const bool same_class = !scope_id.empty()
                && scope_id.size() > class_name.size()
                && scope_id.substr(0, class_name.size() + 1)
                    == class_name + ".";
            rt_error_if(!same_class, rt_error_code_t::value_t::E_MEMBER_ACCESS,
                i->first);
        }
    }

    const auto& prototype = i->second.second;
    rt_error_if(prototype.parameters.size() != _args.size(),
        rt_error_code_t::value_t::E_WRG_NUM_ARGS, i->first);

    // Evaluate arguments
    std::vector<variant_t> values;
    values.reserve(_args.size());
    for (const auto& arg : _args) {
        const variant_t val = arg.first ? arg.first->eval(ctx) : variant_t("");
        values.emplace_back(val);
    }

    // Build ByRef writeback frame.
    // The "me" entry ensures the (possibly modified) Me is copied back
    // to the original object variable when the method returns.
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

        // Always write Me back to the source object on return
        frame.emplace_back("me", _obj_name);

        ctx.byref_writeback_stack.push_back(std::move(frame));
    }

    // Jump to method body; use the resolved method's key for scope naming
    ctx.set_return_line(
        std::make_pair(ctx.runtime_pc.get_line(), get_stmt_id()));
    ctx.go_to(i->second.first);
    ctx.proc_scope.enter_scope(i->first, false);

    // Inject "Me" as a copy of the object into the callee scope
    auto callee_scope = ctx.proc_scope.get();
    callee_scope->define("me", var_value_t(*obj_ptr, VAR_ACCESS_RW));

    // Bind formal parameters
    if (!values.empty()) {
        auto arg_it = prototype.parameters.begin();
        for (const auto& val : values) {
            callee_scope->define(
                arg_it->var_name, var_value_t(val, VAR_ACCESS_RW));
            ++arg_it;
        }
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
