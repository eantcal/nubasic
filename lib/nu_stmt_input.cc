//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_input.h"
#include "nu_os_console.h"
#include "nu_rt_prog_ctx.h"
#include <cstdio>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_input_t::run(rt_prog_ctx_t& ctx)
{
    FILE* sout = ctx.get_stdout_ptr();

    fprintf(sout, "%s", _input_str.c_str());

    for (auto const& variable : _vars) {
        std::string value = _os_input(ctx.get_stdin_ptr());

        while (!value.empty() && (value.c_str()[value.size() - 1] == '\n')) {
            value.erase(value.end() - 1);
        }

        auto index = variable.second;
        auto name = variable.first;

        bool is_vector = index != nullptr;

        if (is_vector) {
            var_scope_t::handle_t scope
                = ctx.proc_scope.get(ctx.proc_scope.get_type(name));

            rt_error_code_t::get_instance().throw_if(!scope->is_defined(name),
                ctx.runtime_pc.get_line(), rt_error_code_t::E_VAR_UNDEF,
                "'" + name + "'");

            size_t idx = index->eval(ctx).to_int();

            auto& v = (*scope)[name];
            variant_t var = v.first;
            const bool const_var = (v.second & VAR_ACCESS_RO) == VAR_ACCESS_RO;

            rt_error_code_t::get_instance().throw_if(const_var,
                ctx.runtime_pc.get_line(), rt_error_code_t::E_CANNOT_MOD_CONST,
                "'" + name + "'");

            rt_error_code_t::get_instance().throw_if(idx >= var.vector_size(),
                ctx.runtime_pc.get_line(),
                rt_error_code_t::E_VEC_IDX_OUT_OF_RANGE, "'" + name + "'");

            variant_t::type_t t = var.get_type();

            if (t == variant_t::type_t::UNDEFINED)
                t = variable_t::type_by_name(name);

            switch (t) {
            case variant_t::type_t::UNDEFINED:
            case variant_t::type_t::ANY:
            case variant_t::type_t::OBJECT:
            case variant_t::type_t::STRUCT:
                rt_error_code_t::get_instance().throw_if(true,
                    ctx.runtime_pc.get_line(), rt_error_code_t::E_TYPE_ILLEGAL,
                    "'" + name + "'");
                break;

            case variant_t::type_t::STRING:
                var.set_str(value, idx);
                break;

            case variant_t::type_t::FLOAT:
                var.set_real(nu::stof(value), idx);
                break;

            case variant_t::type_t::DOUBLE:
                var.set_double(nu::stod(value), idx);
                break;

            case variant_t::type_t::INTEGER:
                var.set_int(nu::stoi(value), idx);
                break;

            case variant_t::type_t::BYTEVECTOR:
                var.set_bvect(nu::stoi(value), idx);
                break;

            case variant_t::type_t::BOOLEAN:
                var.set_bool(strcasecmp(value.c_str(), "false") != 0
                        && strcasecmp(value.c_str(), "0") != 0,
                    idx);
                break;

            case variant_t::type_t::LONG64:
                var.set_long64(nu::stoll(value), idx);
                break;
            }

            scope->define(name, var_value_t(var, VAR_ACCESS_RW));
        } 
        else {
            var_scope_t::handle_t scope
                = ctx.proc_scope.get(ctx.proc_scope.get_type(name));

            auto& v = (*scope)[name];
            const bool const_var = (v.second & VAR_ACCESS_RO) == VAR_ACCESS_RO;

            rt_error_code_t::get_instance().throw_if(const_var,
                ctx.runtime_pc.get_line(), rt_error_code_t::E_CANNOT_MOD_CONST,
                "'" + name + "'");

            variant_t var = v.first;
            variant_t::type_t t = var.get_type();

            if (t == variant_t::type_t::UNDEFINED)
                t = variable_t::type_by_name(name);

            switch (t) {
            case variable_t::type_t::STRING:
                scope->define(name, var_value_t(value, VAR_ACCESS_RW));
                break;

            case variable_t::type_t::DOUBLE:
                try {
                    scope->define(
                        name, var_value_t(nu::stod(value), VAR_ACCESS_RW));
                } 
                catch (...) {
                    scope->define(name, var_value_t(double(0), VAR_ACCESS_RW));
                }

                break;

            case variable_t::type_t::FLOAT:
                try {
                    scope->define(
                        name, var_value_t(nu::stof(value), VAR_ACCESS_RW));
                } 
                catch (...) {
                    scope->define(name, var_value_t(float(0), VAR_ACCESS_RW));
                }

                break;

            case variable_t::type_t::LONG64:
                try {
                    scope->define(
                        name, var_value_t(nu::stoll(value), VAR_ACCESS_RW));
                } 
                catch (...) {
                    scope->define(name, var_value_t(0, VAR_ACCESS_RW));
                }

                break;

            case variable_t::type_t::BOOLEAN:
                try {
                    scope->define(
                        name,
                        var_value_t(strcasecmp(value.c_str(), "false") != 0
                                && strcasecmp(value.c_str(), "0") != 0,
                            VAR_ACCESS_RW));
                } 
                catch (...) {
                    scope->define(name, var_value_t(false, VAR_ACCESS_RW));
                }

                break;

            case variable_t::type_t::INTEGER:
            default:
                try {
                    scope->define(
                        name, var_value_t(nu::stoi(value), VAR_ACCESS_RW));
                } 
                catch (...) {
                    scope->define(name, var_value_t(0, VAR_ACCESS_RW));
                }

                break;

            case variant_t::type_t::UNDEFINED:
            case variant_t::type_t::STRUCT:
                rt_error_code_t::get_instance().throw_if(true,
                    ctx.runtime_pc.get_line(), rt_error_code_t::E_TYPE_ILLEGAL,
                    "'" + name + "'");
                break;
            }
        }
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
