//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_read.h"
#include "nu_error_codes.h"
#include "nu_os_console.h"
#include "nu_rt_prog_ctx.h"

#include <cstdio>
#include <cstdlib>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_read_t::run(rt_prog_ctx_t& ctx)
{
    auto rt_error_if
        = [&](bool cond, rt_error_code_t::value_t err, const std::string desc) {
              rt_error_code_t::get_instance().throw_if(
                  cond, ctx.runtime_pc.get_line(), err, desc);
          };

    rt_error_if(_args.size() != 1, rt_error_code_t::E_WRG_NUM_ARGS, "Read");

    auto size_expr = _args.begin()->first->eval(ctx);

    rt_error_if(!variable_t::is_number(size_expr.get_type()),
        rt_error_code_t::E_INVALID_ARGS, "Read");

    auto size = _args.begin()->first->eval(ctx).to_int();

    rt_error_if(size < 0, rt_error_code_t::E_TYPE_ILLEGAL,
        "Read (size=" + to_string(size) + ")");

    if (size == 0) {
        // nothing to do
        return;
    }

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

    auto const& variable = _var;

    auto index = variable.second;
    auto name = variable.first;
    bool is_vector = index != nullptr;


    std::string svalue;
    int ivalue = 0;
    long long llvalue = 0;
    float fvalue(0);
    double dvalue(0);

    std::vector<byte_t> buf(size);

    var_scope_t::handle_t scope
        = ctx.proc_scope.get(ctx.proc_scope.get_type(name));

    auto& v = (*scope)[name];
    variant_t var = v.first;
    const bool const_var = (v.second & VAR_ACCESS_RO) == VAR_ACCESS_RO;

    rt_error_if(const_var, rt_error_code_t::E_CANNOT_MOD_CONST, "Read " + name);

    auto vtype = var.get_type();

    if (vtype == variant_t::type_t::UNDEFINED)
        vtype = variable_t::type_by_name(name);

    switch (vtype) {
    case nu::variable_t::type_t::INTEGER:
        rt_error_if(size != sizeof(integer_t), rt_error_code_t::E_TYPE_ILLEGAL,
            "Read (size=" + to_string(size) + ")");

        ret = fscanf(s_in, "%i", &ivalue);
        break;


    case nu::variable_t::type_t::LONG64:
        rt_error_if(size != sizeof(long64_t), rt_error_code_t::E_TYPE_ILLEGAL,
            "Read (size=" + to_string(size) + ")");

        ret = fscanf(s_in, "%lli", &llvalue);
        break;

    case nu::variable_t::type_t::FLOAT:
        rt_error_if(size != sizeof(real_t), rt_error_code_t::E_TYPE_ILLEGAL,
            "Read (size=" + to_string(size) + ")");

        ret = fscanf(s_in, "%f", &fvalue);
        break;

    case nu::variable_t::type_t::DOUBLE:
        rt_error_if(size != sizeof(double_t), rt_error_code_t::E_TYPE_ILLEGAL,
            "Read (size=" + to_string(size) + ")");

        ret = fscanf(s_in, "%lf", &dvalue);
        break;

    case nu::variable_t::type_t::STRING:
    case nu::variable_t::type_t::BOOLEAN:
    case nu::variable_t::type_t::BYTEVECTOR:
        size = static_cast<nu::integer_t> ( ::fread(buf.data(), 1, size, s_in) );
        ret = size >= 0;

        if (vtype == nu::variable_t::type_t::STRING
            || vtype == nu::variable_t::type_t::BOOLEAN) 
        {
            svalue = std::string(buf.begin(), buf.end());
        }

        break;

    case nu::variable_t::type_t::UNDEFINED:
    case nu::variable_t::type_t::STRUCT:
    case nu::variable_t::type_t::ANY:
    case nu::variable_t::type_t::OBJECT:
    default:
        rt_error_if(true, rt_error_code_t::E_TYPE_ILLEGAL, "Read");
        break;
    }


    if (is_vector) {
        rt_error_if(!scope->is_defined(name), rt_error_code_t::E_VAR_UNDEF,
            "Read '" + name + "'");

        size_t idx = index->eval(ctx).to_int();

        if (var.get_type() == variant_t::type_t::BYTEVECTOR) {
            rt_error_if(
                true, rt_error_code_t::E_TYPE_ILLEGAL, "Read '" + name + "'");
        }

        else {
            // check size
            rt_error_if(idx >= var.vector_size(),
                rt_error_code_t::E_INV_VECT_SIZE,
                "Read '" + name + "(" + nu::to_string(idx) + ")'");

            switch (vtype) {
            case nu::variable_t::type_t::INTEGER:
                var.set_str(nu::to_string(ivalue), idx);
                break;

            case nu::variable_t::type_t::LONG64:
                var.set_str(nu::to_string(llvalue), idx);
                break;

            case nu::variable_t::type_t::FLOAT:
                var.set_real(fvalue, idx);
                break;

            case nu::variable_t::type_t::DOUBLE:
                var.set_double(dvalue, idx);
                break;

            case nu::variable_t::type_t::STRING:
                var.set_str(svalue, idx);
                break;

            case nu::variable_t::type_t::BOOLEAN:
                var.set_bool(strcasecmp(svalue.c_str(), "false") != 0
                        && strcasecmp(svalue.c_str(), "0") != 0,
                    idx);
                break;

            case nu::variable_t::type_t::UNDEFINED:
            case nu::variable_t::type_t::BYTEVECTOR:
            case nu::variable_t::type_t::STRUCT:
            case nu::variable_t::type_t::ANY:
            case nu::variable_t::type_t::OBJECT:
                break;
            }
        }

        scope->define(name, var_value_t(var, VAR_ACCESS_RW));
    } 
    else {
        switch (vtype) {
        case variable_t::type_t::STRING:
            scope->define(name, var_value_t(svalue, VAR_ACCESS_RW));
            break;

        case variable_t::type_t::FLOAT:
            try {
                scope->define(name, var_value_t(fvalue, VAR_ACCESS_RW));
            } 
            catch (...) {
                scope->define(name, var_value_t(float(0), VAR_ACCESS_RW));
            }

            break;

        case variable_t::type_t::DOUBLE:
            try {
                scope->define(name, var_value_t(dvalue, VAR_ACCESS_RW));
            } 
            catch (...) {
                scope->define(name, var_value_t(double(0), VAR_ACCESS_RW));
            }

            break;


        case variable_t::type_t::LONG64:
            try {
                scope->define(name, var_value_t(llvalue, VAR_ACCESS_RW));
            } 
            catch (...) {
                scope->define(name, var_value_t((long long)0, VAR_ACCESS_RW));
            }

            break;


        case variable_t::type_t::BOOLEAN:
            try {
                scope->define(
                    name, var_value_t(strcasecmp(svalue.c_str(), "false") != 0
                                  && strcasecmp(svalue.c_str(), "0") != 0,
                              VAR_ACCESS_RW));
            } 
            catch (...) {
                scope->define(name, var_value_t(false, VAR_ACCESS_RW));
            }

            break;

        case variable_t::type_t::BYTEVECTOR:
            if (buf.size() > 0) {
                scope->define(name, var_value_t(buf, VAR_ACCESS_RW));
            }

            break;

        case nu::variable_t::type_t::UNDEFINED:
        case nu::variable_t::type_t::STRUCT:
        case nu::variable_t::type_t::ANY:
        case nu::variable_t::type_t::OBJECT:
            rt_error_if(true, rt_error_code_t::E_TYPE_ILLEGAL, "Read");
            break;

        case variable_t::type_t::INTEGER:
        default:
            try {
                scope->define(name, var_value_t(ivalue, VAR_ACCESS_RW));
            } 
            catch (...) {
                scope->define(name, var_value_t(0, VAR_ACCESS_RW));
            }

            break;
        }
    }

    ctx.set_errno(ret != 0 ? errno : 0);
    ctx.go_to_next();
}

/* -------------------------------------------------------------------------- */

} // namespace nu
