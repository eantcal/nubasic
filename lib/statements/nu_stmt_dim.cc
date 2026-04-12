//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_dim.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"

#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_dim_t::run(rt_prog_ctx_t& ctx)
{
    auto rt_error_if = [&](bool cond, rt_error_code_t::value_t err,
                           const std::string& desc) {
        rt_error_code_t::get_instance().throw_if(
            cond, ctx.runtime_pc.get_line(), err, desc);
    };

    for (const auto& v : _vars) {
        const auto vsize = int(v.second.second->eval(ctx).to_int());
        const std::string& name = v.first;

        auto scope_type = ctx.proc_scope.get_type(name);

        switch (scope_type) {
        case proc_scope_t::type_t::GLOBAL:
            rt_error_if(true, rt_error_code_t::value_t::E_VAR_REDEF,
                "Global '" + name + "'");
            break;

        case proc_scope_t::type_t::LOCAL:
            rt_error_if(true, rt_error_code_t::value_t::E_VAR_REDEF,
                "Local '" + name + "'");
            break;

        case proc_scope_t::type_t::UNDEF:
        default:
            break;
        }

        rt_error_if(vsize < 0, rt_error_code_t::value_t::E_INV_VECT_SIZE,
            "Dim: '" + name + "'");

        var_scope_t::handle_t scope
            = ctx.proc_scope.get(ctx.proc_scope.get_type(name));

        auto vtype = v.second.first;
        auto vtype_code = variable_t::type_by_typename(vtype);

        std::string init_val;

        switch (vtype_code) {
        case variable_t::type_t::STRING:
        case variable_t::type_t::ANY:
            init_val = "";
            scope->define(name,
                var_value_t(
                    variant_t(init_val, vtype_code, vsize), VAR_ACCESS_RW));
            break;

        case variable_t::type_t::DOUBLE:
        case variable_t::type_t::INTEGER:
        case variable_t::type_t::BOOLEAN:
        case variable_t::type_t::BYTEVECTOR:
            init_val = "0";
            scope->define(name,
                var_value_t(
                    variant_t(init_val, vtype_code, vsize), VAR_ACCESS_RW));
            break;

        case variable_t::type_t::STRUCT:
        case variable_t::type_t::OBJECT:
        case variable_t::type_t::UNDEFINED: {
            auto& sprototypes = ctx.struct_prototypes.data;
            auto it = sprototypes.find(vtype);
            auto ctor_it = _ctor_args.find(name);
            const bool is_class_type = ctx.is_class_type(vtype);

            rt_error_if(it == sprototypes.end(),
                rt_error_code_t::value_t::E_STRUCT_UNDEF,
                "Dim: '" + name + "'");

            variant_t value;

            if (is_class_type) {
                value = ctor_it != _ctor_args.end()
                    ? variant_t::make_object_instance(it->second.second, vsize)
                    : variant_t::make_nothing(vtype, vsize);
            } else {
                value = it->second.second; // struct prototype

                if (vsize > 0)
                    value.resize(vsize);
            }

            scope->define(name, var_value_t(value, VAR_ACCESS_RW));

            // Invoke parameterized constructor Sub New(...) if defined and args
            // present
            {
                if (ctor_it != _ctor_args.end()) {
                    const std::string ctor_key = vtype + ".new";
                    auto proto_it = ctx.proc_prototypes.data.find(ctor_key);
                    if (proto_it != ctx.proc_prototypes.data.end()) {
                        const auto& prototype = proto_it->second.second;

                        rt_error_if(prototype.parameters.size()
                                != ctor_it->second.size(),
                            rt_error_code_t::value_t::E_WRG_NUM_ARGS, ctor_key);

                        // Evaluate constructor arguments
                        std::vector<variant_t> values;
                        values.reserve(ctor_it->second.size());
                        for (const auto& arg : ctor_it->second) {
                            values.emplace_back(arg.first ? arg.first->eval(ctx)
                                                          : variant_t(""));
                        }

                        // ByRef writeback frame: Me → object variable name
                        std::vector<rt_prog_ctx_t::byref_entry_t> frame;
                        frame.emplace_back("me", name);
                        ctx.byref_writeback_stack.push_back(std::move(frame));

                        // Jump to constructor; return address = this Dim
                        // statement
                        ctx.set_return_line(std::make_pair(
                            ctx.runtime_pc.get_line(), get_stmt_id()));
                        ctx.go_to(proto_it->second.first);
                        ctx.proc_scope.enter_scope(ctor_key, false);

                        // Inject Me and formal parameters into the callee scope
                        auto callee_scope = ctx.proc_scope.get();
                        callee_scope->define("me",
                            var_value_t((*scope)[name].first, VAR_ACCESS_RW));

                        auto arg_it = prototype.parameters.begin();
                        for (const auto& val : values) {
                            callee_scope->define(arg_it->var_name,
                                var_value_t(val, VAR_ACCESS_RW));
                            ++arg_it;
                        }

                        // Do NOT call go_to_next(): the constructor handles
                        // flow
                        return;
                    }
                }
            }

            break;
        }

        default:
            rt_error_if(true, rt_error_code_t::value_t::E_INV_VECT_SIZE,
                "Dim: '" + name + "'");
        }
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
