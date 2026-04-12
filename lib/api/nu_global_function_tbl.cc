//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_basic_defs.h"
#include "nu_builtin_function_support.h"
#include "nu_builtin_registry.h"
#include "nu_eval_expr.h"
#include "nu_expr_var.h"

#include <cassert>

/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

global_function_tbl_t* global_function_tbl_t::_instance = nullptr;
global_operator_tbl_t* global_operator_tbl_t::_instance = nullptr;


/* -------------------------------------------------------------------------- */

template <typename T>
std::vector<T> conv(const std::vector<T>& f, const std::vector<T>& g)
{
    const int nf = int(f.size());
    const int ng = int(g.size());
    const int n = nf + ng - 1;

    std::vector<T> out(n, T());

    for (auto i = 0; i < n; ++i) {
        const int jmn = (i >= ng - 1) ? i - (ng - 1) : 0;
        const int jmx = (i < nf - 1) ? i : nf - 1;

        for (auto j = jmn; j <= jmx; ++j) {
            out[i] += (f[j] * g[i - j]);
        }
    }

    return out;
}


/* -------------------------------------------------------------------------- */

variant_t conv_functor(
    rt_prog_ctx_t& ctx, const std::string& name, const nu::func_args_t& args)
{
    (void)name;
    const auto args_num = args.size();

    rt_error_code_t::get_instance().throw_if(args_num != 4 && args_num != 2, 0,
        rt_error_code_t::value_t::E_INVALID_ARGS, "");

    const auto variant_v1 = args[0]->eval(ctx);
    const auto variant_v2 = args[1]->eval(ctx);

    const auto actual_v1_size = variant_v1.vector_size();
    const auto actual_v2_size = variant_v2.vector_size();

    const size_t size_v1
        = args_num == 4 ? size_t(args[2]->eval(ctx).to_int()) : actual_v1_size;

    const size_t size_v2
        = args_num == 4 ? size_t(args[3]->eval(ctx).to_int()) : actual_v2_size;

    rt_error_code_t::get_instance().throw_if(
        size_v1 > actual_v1_size || size_v1 < 1, 0,
        rt_error_code_t::value_t::E_INV_VECT_SIZE, args[0]->name());

    rt_error_code_t::get_instance().throw_if(
        size_v2 > actual_v2_size || size_v2 < 1, 0,
        rt_error_code_t::value_t::E_INV_VECT_SIZE, args[1]->name());

    std::vector<double> v1(size_v1);
    std::vector<double> v2(size_v2);

    bool ok = variant_v1.copy_vector_content(v1);

    rt_error_code_t::get_instance().throw_if(
        !ok, 0, rt_error_code_t::value_t::E_INV_VECT_SIZE, args[0]->name());

    ok = variant_v2.copy_vector_content(v2);

    rt_error_code_t::get_instance().throw_if(
        !ok, 0, rt_error_code_t::value_t::E_INV_VECT_SIZE, args[1]->name());

    auto vr = conv(v1, v2);

    const nu::variant_t result(std::move(vr));

    return result;
}


/* -------------------------------------------------------------------------- */

variant_t restore_functor(
    rt_prog_ctx_t& ctx, const std::string& name, const nu::func_args_t& args)
{
    const auto args_num = args.size();

    rt_error_code_t::get_instance().throw_if(
        args_num > 1, 0, rt_error_code_t::value_t::E_INVALID_ARGS, "");

    nu::variant_t v(nu::integer_t(0));

    if (args_num > 0) {
        v = args[0]->eval(ctx);
    }

    const auto index = static_cast<int>(v.to_int());

    const nu::integer_t old_val = ctx.read_data_store_index;

    if (index < 0) {
        ctx.read_data_store.clear();
        ctx.read_data_store_index = 0;
        nu::variant_t result(0);
        return result;
    }

    rt_error_code_t::get_instance().throw_if(
        index < 0 || index >= int(ctx.read_data_store.size()), 0,
        rt_error_code_t::value_t::E_VAL_OUT_OF_RANGE, name);

    ctx.read_data_store_index = index;
    const nu::variant_t result(old_val);

    return result;
}


/* -------------------------------------------------------------------------- */

variant_t evaluate_expression(
    rt_prog_ctx_t& ctx, const std::string& name, const nu::func_args_t& args)
{
    std::vector<variant_t::type_t> check_vect = { variant_t::type_t::STRING };
    std::vector<variant_t> vargs;
    get_functor_vargs(ctx, name, args, check_vect, vargs);

    const std::string& arg = vargs[0].to_str();
    return nu::eval_expr(ctx, arg);
}


/* -------------------------------------------------------------------------- */

variant_t evaluate_and_export_result(
    rt_prog_ctx_t& ctx, const std::string& name, const nu::func_args_t& args)
{
    ctx.exported_result = evaluate_expression(ctx, name, args);
    return ctx.exported_result;
}


/* -------------------------------------------------------------------------- */

static variant_t* resolve_mutable_unary_target(rt_prog_ctx_t& ctx,
    const expr_any_t::handle_t& expr, var_scope_t::handle_t& scope)
{
    rt_error_code_t::get_instance().throw_if(
        !expr, 0, rt_error_code_t::value_t::E_INVALID_ARGS, "");

    const std::string target_name = expr->name();

    rt_error_code_t::get_instance().throw_if(
        target_name.empty(), 0, rt_error_code_t::value_t::E_INVALID_ARGS, "");

    if (target_name.find('.') != std::string::npos) {
        const std::string root = target_name.substr(0, target_name.find('.'));
        scope = ctx.proc_scope.get(ctx.proc_scope.get_type(root));

        auto* value = ctx.get_struct_member_value(target_name, scope);

        rt_error_code_t::get_instance().throw_if(
            !value, 0, rt_error_code_t::value_t::E_INV_IDENTIF, target_name);

        return value;
    }

    auto scope_type = ctx.proc_scope.get_type(target_name);

    if (scope_type == proc_scope_t::type_t::UNDEF) {
        scope_type = proc_scope_t::type_t::GLOBAL;
    }

    scope = ctx.proc_scope.get(scope_type);

    rt_error_code_t::get_instance().throw_if(
        !scope || !scope->is_defined(target_name), 0,
        rt_error_code_t::value_t::E_INV_IDENTIF, target_name);

    auto& entry = (*scope)[target_name];
    const bool const_var = (entry.second & VAR_ACCESS_RO) == VAR_ACCESS_RO;

    rt_error_code_t::get_instance().throw_if(const_var, 0,
        rt_error_code_t::value_t::E_CANNOT_MOD_CONST, "'" + target_name + "'");

    return &entry.first;
}


/* -------------------------------------------------------------------------- */

variant_t process_operator(rt_prog_ctx_t& ctx, const std::string& operator_name,
    const nu::func_args_t& args)
{
    rt_error_code_t::get_instance().throw_if(
        args.size() != 1, 0, rt_error_code_t::value_t::E_INVALID_ARGS, "");

    var_scope_t::handle_t scope;
    auto* variable_value = resolve_mutable_unary_target(ctx, args[0], scope);

    if (operator_name == NU_BASIC_OP_INC) {
        variable_value->increment();
    } else if (operator_name == NU_BASIC_OP_DEC) {
        variable_value->decrement();
    } else {
        rt_error_code_t::get_instance().throw_if(
            true, 0, rt_error_code_t::value_t::E_FUNC_UNDEF, "");
    }

    return *variable_value;
}


/* -------------------------------------------------------------------------- */

global_function_tbl_t& global_function_tbl_t::get_instance()
{
    if (_instance) {
        return *_instance;
    }

    _instance = new global_function_tbl_t();
    assert(_instance);

    global_function_tbl_t& fmap = *_instance;
    register_builtin_modules(fmap);

    return *_instance;
}


/* -------------------------------------------------------------------------- */

global_operator_tbl_t& global_operator_tbl_t::get_instance()
{
    if (_instance) {
        return *_instance;
    }

    _instance = new global_operator_tbl_t();
    assert(_instance);

    global_operator_tbl_t& opmap = *_instance;
    using arg_t = const variant_t&;

    opmap["mod"] = [](arg_t a, arg_t b) { return a.int_mod(b); };
    opmap["div"] = [](arg_t a, arg_t b) { return a.int_div(b); };

    opmap["<="] = [](arg_t a, arg_t b) { return a <= b; };
    opmap[">="] = [](arg_t a, arg_t b) { return a >= b; };
    opmap["="] = [](arg_t a, arg_t b) { return a == b; };
    opmap["<>"] = [](arg_t a, arg_t b) { return a != b; };
    opmap["<"] = [](arg_t a, arg_t b) { return a < b; };
    opmap[">"] = [](arg_t a, arg_t b) { return a > b; };

    opmap["+"] = [](arg_t a, arg_t b) { return a + b; };
    opmap["-"] = [](arg_t a, arg_t b) { return a - b; };

    opmap["/"] = [](arg_t a, arg_t b) { return a / b; };
    opmap["*"] = [](arg_t a, arg_t b) { return a * b; };
    opmap["^"] = [](arg_t a, arg_t b) { return a.power(b); };
    opmap["\\"] = [](arg_t a, arg_t b) { return a.int_div(b); };

    opmap["and"] = [](arg_t a, arg_t b) { return a && b; };
    opmap["or"] = [](arg_t a, arg_t b) { return a || b; };
    opmap["xor"] = [](arg_t a, arg_t b) { return a != b; };

    opmap["bor"] = [](arg_t a, arg_t b) { return a.to_int() | b.to_int(); };
    opmap["band"] = [](arg_t a, arg_t b) { return a.to_int() & b.to_int(); };
    opmap["bxor"] = [](arg_t a, arg_t b) { return a.to_int() ^ b.to_int(); };
    opmap["bshr"] = [](arg_t a, arg_t b) { return a.to_int() >> b.to_int(); };
    opmap["bshl"] = [](arg_t a, arg_t b) { return a.to_int() << b.to_int(); };

    return *_instance;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
