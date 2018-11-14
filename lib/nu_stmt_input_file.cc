//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_input_file.h"
#include "nu_os_console.h"
#include "nu_rt_prog_ctx.h"
#include <cstdio>


/* -------------------------------------------------------------------------- */

namespace nu {

void stmt_input_file_t::run(rt_prog_ctx_t& ctx)
{
    FILE* s_in = ctx.get_stdin_ptr();

    if (_fd > 0) {
        s_in = ctx.file_tbl.resolve_fd(_fd);

        if (!s_in) {
            ctx.set_errno(EBADF);
            ctx.go_to_next();
            return;
        }
    }

    int ret = 0;

    for (auto const& variable : _vars) {
        auto vtype = variable_t::type_by_name(variable.first);

        std::string svalue;
        int ivalue = 0;
        long long llvalue = 0;
        float fvalue = 0.0F;
        double dvalue = 0.0L;

        switch (vtype) {
        case nu::variable_t::type_t::INTEGER:
            ret = fscanf(s_in, "%i", &ivalue);
            break;

        case nu::variable_t::type_t::LONG64:
            ret = fscanf(s_in, "%lli", &llvalue);
            break;

        case nu::variable_t::type_t::FLOAT:
            ret = fscanf(s_in, "%f", &fvalue);
            break;

        case nu::variable_t::type_t::DOUBLE:
            ret = fscanf(s_in, "%lf", &dvalue);
            break;

        case nu::variable_t::type_t::STRING:
        case nu::variable_t::type_t::BOOLEAN:
            svalue = _os_input(s_in);
            break;

        case nu::variable_t::type_t::UNDEFINED:
        case nu::variable_t::type_t::STRUCT:
        default:
            rt_error_code_t::get_instance().throw_if(true,
                ctx.runtime_pc.get_line(), rt_error_code_t::E_TYPE_ILLEGAL,
                "input#");
            break;
        }

        auto index = variable.second;
        auto name = variable.first;

        bool is_vector = index != nullptr;

        if (is_vector) {
            rt_error_code_t::get_instance().throw_if(true,
                ctx.runtime_pc.get_line(), rt_error_code_t::E_VAR_UNDEF,
                "'" + name + "'");

            size_t idx = index->eval(ctx).to_int();
            auto& v = (*(ctx.proc_scope.get()))[name];
            const bool const_var = (v.second & VAR_ACCESS_RO) == VAR_ACCESS_RO;

            variant_t var = v.first;

            rt_error_code_t::get_instance().throw_if(const_var,
                ctx.runtime_pc.get_line(), rt_error_code_t::E_CANNOT_MOD_CONST,
                "'" + name + "'");

            rt_error_code_t::get_instance().throw_if(idx >= var.vector_size(),
                ctx.runtime_pc.get_line(),
                rt_error_code_t::E_VEC_IDX_OUT_OF_RANGE, "'" + name + "'");

            switch (vtype) {
            case nu::variable_t::type_t::INTEGER:
                var.set_int(ivalue, idx);
                break;

            case nu::variable_t::type_t::LONG64:
                var.set_long64(llvalue, idx);
                break;

            case nu::variable_t::type_t::DOUBLE:
                var.set_double(dvalue, idx);
                break;

            case nu::variable_t::type_t::FLOAT:
                var.set_real(fvalue, idx);
                break;

            case nu::variable_t::type_t::STRING:
                var.set_str(svalue, idx);
                break;

            case nu::variable_t::type_t::BOOLEAN:
                var.set_bool(strcasecmp(svalue.c_str(), "false") != 0
                        && strcasecmp(svalue.c_str(), "0") != 0,
                    idx);
                break;

            case nu::variable_t::type_t::BYTEVECTOR:
                var.set_bvect(ivalue, idx);
                break;

            case nu::variable_t::type_t::UNDEFINED:
                break;

            case nu::variable_t::type_t::STRUCT:
            case nu::variable_t::type_t::OBJECT:
            case nu::variable_t::type_t::ANY:
                rt_error_code_t::get_instance().throw_if(true,
                    ctx.runtime_pc.get_line(), rt_error_code_t::E_TYPE_ILLEGAL,
                    "'" + name + "'");
                break;
            }

            ctx.proc_scope.get()->define(name, var_value_t(var, VAR_ACCESS_RW));
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
            variant_t::type_t vtype = var.get_type();

            if (vtype == variant_t::type_t::UNDEFINED)
                vtype = variable_t::type_by_name(name);

            switch (vtype) {
            case variable_t::type_t::STRING:
                (ctx.proc_scope.get())
                    ->define(name, var_value_t(svalue, VAR_ACCESS_RW));
                break;

            case variable_t::type_t::DOUBLE:
                try {
                    (ctx.proc_scope.get())
                        ->define(name, var_value_t(dvalue, VAR_ACCESS_RW));
                } 
                catch (...) {
                    (ctx.proc_scope.get())
                        ->define(name, var_value_t(double(0), VAR_ACCESS_RW));
                }

                break;

            case variable_t::type_t::FLOAT:
                try {
                    (ctx.proc_scope.get())
                        ->define(name, var_value_t(fvalue, VAR_ACCESS_RW));
                } 
                catch (...) {
                    (ctx.proc_scope.get())
                        ->define(name, var_value_t(float(0), VAR_ACCESS_RW));
                }

                break;

            case variable_t::type_t::BOOLEAN:
                try {
                    (ctx.proc_scope.get())
                        ->define(
                            name,
                            var_value_t(strcasecmp(svalue.c_str(), "false") != 0
                                    && strcasecmp(svalue.c_str(), "0") != 0,
                                VAR_ACCESS_RW));
                } 
                catch (...) {
                    (ctx.proc_scope.get())
                        ->define(name, var_value_t(false, VAR_ACCESS_RW));
                }

                break;

            case variable_t::type_t::LONG64:
                try {
                    (ctx.proc_scope.get())
                        ->define(name, var_value_t(llvalue, VAR_ACCESS_RW));
                } 
                catch (...) {
                    (ctx.proc_scope.get())
                        ->define(
                            name, var_value_t((long long)0, VAR_ACCESS_RW));
                }

                break;

            case nu::variable_t::type_t::STRUCT:
            case nu::variable_t::type_t::OBJECT:
            case nu::variable_t::type_t::ANY:
                rt_error_code_t::get_instance().throw_if(true,
                    ctx.runtime_pc.get_line(), rt_error_code_t::E_TYPE_ILLEGAL,
                    "'" + name + "'");
                break;

            case variable_t::type_t::INTEGER:
            default:
                try {
                    (ctx.proc_scope.get())
                        ->define(name, var_value_t(ivalue, VAR_ACCESS_RW));
                } 
                catch (...) {
                    (ctx.proc_scope.get())
                        ->define(name, var_value_t(0, VAR_ACCESS_RW));
                }

                break;
            }
        }
    }

    ctx.set_errno(ret != 0 ? errno : 0);

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
