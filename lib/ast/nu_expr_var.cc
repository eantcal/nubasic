//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_expr_var.h"
#include "nu_error_codes.h"
#include "nu_variable.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

variant_t expr_var_t::eval(rt_prog_ctx_t& ctx) const
{
    var_scope_t::handle_t scope;

    if (scope == nullptr) {
        scope = ctx.proc_scope.get(ctx.proc_scope.get_type(_name));
    }

    if (scope->is_defined(_name)) {
        if (scope == nullptr) {
            scope = ctx.proc_scope.get(ctx.proc_scope.get_type(_name));
        }

        return (*scope)[_name].first;
    }

    // Modern mode: every variable must be declared via Dim before use.
    // Exception: function return-value variable (BASIC "FuncName = value"
    // pattern).
    {
        // scope_id has form "funcname[N]" (or "Class.func[N]" for methods).
        // Strip "[N]" to get the base name used in _rec_tbl, then strip the
        // class prefix to get the short name used inside the body.
        const std::string& scope_id = ctx.proc_scope.get_scope_id();
        const auto bracket = scope_id.find('[');
        const std::string base_scope = (bracket != std::string::npos)
            ? scope_id.substr(0, bracket)
            : scope_id;
        const auto dot = base_scope.rfind('.');
        const std::string short_func_name = (dot != std::string::npos)
            ? base_scope.substr(dot + 1)
            : base_scope;
        const bool is_func_retval = !base_scope.empty()
            && ctx.proc_scope.is_func_call(base_scope)
            && _name == short_func_name;

        rt_error_code_t::get_instance().throw_if(
            ctx.get_syntax_mode() == prog_ctx_t::syntax_mode_t::MODERN
                && !is_func_retval,
            ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_VAR_UNDEF,
            "variable '" + _name
                + "' is not declared (Syntax Modern requires Dim)");
    }

    auto vtype = variable_t::type_by_name(_name);
    variant_t value("", vtype, 0);
    scope->define(_name, var_value_t(value, VAR_ACCESS_RW));

    return (*scope)[_name].first;
}

/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
