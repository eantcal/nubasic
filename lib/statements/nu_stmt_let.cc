//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_let.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_let_t::run(rt_prog_ctx_t& ctx)
{
    variant_t val = _arg->eval(ctx);

    size_t idx = 0;

    var_scope_t::handle_t scope;
    variant_t* var = nullptr;

    if (_struct_member) {
        if (_vect_idx) {
            idx = _vect_idx->eval(ctx).to_int();
        }

        std::string err_msg;
        var = ctx.get_struct_member_value(_variable, scope, idx, &err_msg);

        rt_error_code_t::get_instance().throw_if(!err_msg.empty(),
            ctx.runtime_pc.get_line(),
            rt_error_code_t::value_t::E_MEMBER_ACCESS, err_msg);

        rt_error_code_t::get_instance().throw_if(!var,
            ctx.runtime_pc.get_line(),
            rt_error_code_t::value_t::E_TYPE_MISMATCH, "'" + _variable + "'");
    }

    // Determine whether this assignment is the BASIC function-return-value
    // pattern: inside "Function Foo" the body writes "Foo = value".
    // scope_id has form "funcname[N]" (or "Class.func[N]" for methods).
    // Strip "[N]" to get the base name, then strip the class prefix to get
    // the short name the programmer uses inside the body.
    // Class method functions are entered with fncall=false, so we also check
    // function_tbl which records all Function definitions (including methods).
    bool is_func_retval = false;
    {
        const std::string& scope_id = ctx.proc_scope.get_scope_id();
        const auto bracket = scope_id.find('[');
        const std::string base_scope = (bracket != std::string::npos)
            ? scope_id.substr(0, bracket)
            : scope_id;
        const auto dot = base_scope.rfind('.');
        const std::string short_func_name = (dot != std::string::npos)
            ? base_scope.substr(dot + 1)
            : base_scope;
        is_func_retval = !base_scope.empty()
            && (ctx.proc_scope.is_func_call(base_scope)
                || ctx.function_tbl.count(base_scope) > 0)
            && _variable == short_func_name;
    }

    if (!var) {
        if (scope == nullptr) {
            scope = ctx.proc_scope.get(ctx.proc_scope.get_type(_variable));
        }

        // Modern mode: require explicit Dim declaration before first use.
        // Exception: the BASIC function return-value pattern ("FuncName =
        // value").
        rt_error_code_t::get_instance().throw_if(!scope->is_defined(_variable)
                && ctx.get_syntax_mode() == prog_ctx_t::syntax_mode_t::MODERN
                && !is_func_retval,
            ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_VAR_UNDEF,
            "variable '" + _variable
                + "' is not declared (Syntax Modern requires Dim)");

        auto& v = (*scope)[_variable];

        const bool const_var = (v.second & VAR_ACCESS_RO) == VAR_ACCESS_RO;

        if (const_var) {
            rt_error_code_t::get_instance().throw_if(true,
                ctx.runtime_pc.get_line(),
                rt_error_code_t::value_t::E_CANNOT_MOD_CONST,
                "'" + _variable + "'");
        }

        var = &(v.first);
    }

    variable_t::type_t vart = var->get_type();

    if (var->is_class_type()) {
        rt_error_code_t::get_instance().throw_if(!val.is_class_type(),
            ctx.runtime_pc.get_line(),
            rt_error_code_t::value_t::E_TYPE_MISMATCH, "'" + _variable + "'");

        const std::string target_type = var->declared_class_type().empty()
            ? var->struct_type_name()
            : var->declared_class_type();

        rt_error_code_t::get_instance().throw_if(
            !ctx.is_class_assignable(target_type, val.struct_type_name()),
            ctx.runtime_pc.get_line(),
            rt_error_code_t::value_t::E_TYPE_MISMATCH, "'" + _variable + "'");

        *var = val;
        var->set_declared_class_type(target_type);
        ctx.go_to_next();
        return;
    }

    if (vart == variable_t::type_t::ANY || vart == variable_t::type_t::OBJECT) {
        *var = val;
        ctx.go_to_next();
        return;
    }

    if (vart == variable_t::type_t::UNDEFINED) {
        // Modern mode: implicit suffix-based typing is not allowed.
        // Exception: function return value assignment — the suffix is part of
        // the function name, not an implicit variable type declaration.
        rt_error_code_t::get_instance().throw_if(
            ctx.get_syntax_mode() == prog_ctx_t::syntax_mode_t::MODERN
                && variable_t::has_type_suffix(_variable) && !is_func_retval,
            ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_VAR_UNDEF,
            "'" + _variable
                + "': implicit suffix type not allowed in Syntax Modern (use "
                  "explicit As Type)");
        vart = variable_t::type_by_name(_variable);
    }

    bool is_vector = var->is_vector();

    if (_element_vect_idx != nullptr) {
        is_vector = true;
        _vect_idx = _element_vect_idx;
    }

    if (is_vector) {
        if (_vect_idx) {
            size_t idx = _vect_idx->eval(ctx).to_int();

            rt_error_code_t::get_instance().throw_if(idx >= var->vector_size(),
                ctx.runtime_pc.get_line(),
                rt_error_code_t::value_t::E_VEC_IDX_OUT_OF_RANGE,
                "'" + _variable + "(" + nu::to_string(idx) + ")'");

            if (val.is_struct()) {
                rt_error_code_t::get_instance().throw_if(
                    var->struct_type_name() != val.struct_type_name(),
                    ctx.runtime_pc.get_line(),
                    rt_error_code_t::value_t::E_TYPE_MISMATCH,
                    "'" + _variable + "(" + nu::to_string(idx) + ")'");

                var->set_struct_value(val, idx);
            } else {
                _assign<size_t>(ctx, *var, val, vart, idx);
            }
        } else {
            // destination vector size will be set to the source size

            rt_error_code_t::get_instance().throw_if(
                var->get_type() != val.get_type(), ctx.runtime_pc.get_line(),
                rt_error_code_t::value_t::E_TYPE_MISMATCH,
                "'" + _variable + "'");

            scope->define(_variable, var_value_t(val, VAR_ACCESS_RW));
        }

    } else {
        if (val.is_struct()) {
            var->set_struct_value(val, 0);
        } else {
            _assign<>(ctx, *var, val, vart);
        }
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
