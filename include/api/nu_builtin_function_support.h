//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#ifndef __NU_BUILTIN_FUNCTION_SUPPORT_H__
#define __NU_BUILTIN_FUNCTION_SUPPORT_H__

#include "nu_global_function_tbl.h"

#include <type_traits>
#include <vector>

namespace nu {

inline void check_arg_num(
    const func_args_t& args, int expected_arg_num, const std::string& fname)
{
    std::string error = "'" + fname + "': expects to be passed "
        + nu::to_string(expected_arg_num) + " argument(s)";

    switch (expected_arg_num) {
    case 0:
        error = "'" + fname + "': expects to be passed no arguments";
        break;

    case 1:
        error = "'" + fname + "': expects to be passed one argument";
        break;
    }

    const bool valid_args = (expected_arg_num == 0 && args.size() == 0)
        || (expected_arg_num == 0 && args.size() == 1 && args[0]->empty())
        || (expected_arg_num == 1 && args.size() == 1 && !args[0]->empty())
        || (expected_arg_num > 1 && int(args.size()) == expected_arg_num);

    syntax_error_if(!valid_args, error);
}

inline void get_functor_vargs(rt_prog_ctx_t& ctx, const std::string& name,
    const func_args_t& args, const std::vector<variant_t::type_t>& check_vect,
    std::vector<variant_t>& vargs)
{
    check_arg_num(args, int(check_vect.size()), name);

    for (auto& arg : args) {
        vargs.push_back(arg->eval(ctx));
    }

    int i = 0;

    for (auto& vargt : check_vect) {
        bool invalid_check = true;

        if (vargt == variant_t::type_t::UNDEFINED) {
            continue;
        }

        if (variable_t::is_number(vargt)
            && variable_t::is_number(vargs[i].get_type())) {
            invalid_check = false;
        } else {
            invalid_check = vargs[i].get_type() != vargt;
        }

        syntax_error_if(invalid_check,
            "'" + name + "': expects to be passed argument "
                + nu::to_string(i + 1) + " as "
                + variant_t::get_type_desc(vargt));

        ++i;
    }
}

template <class F, typename RT, typename T>
variant_t functor_RT_T(rt_prog_ctx_t& ctx, const std::string& name,
    const func_args_t& args, variant_t::type_t argt)
{
    std::vector<variant_t::type_t> check_vect = { argt };
    std::vector<variant_t> vargs;
    get_functor_vargs(ctx, name, args, check_vect, vargs);

    auto arg = static_cast<T>(vargs[0]);

    return variant_t(RT(F()(arg)));
}

template <class F, typename RT, typename T1, typename T2>
variant_t functor_RT_T1_T2(rt_prog_ctx_t& ctx, const std::string& name,
    const func_args_t& args, variant_t::type_t argt1, variant_t::type_t argt2)
{
    std::vector<variant_t::type_t> check_vect = { argt1, argt2 };
    std::vector<variant_t> vargs;
    get_functor_vargs(ctx, name, args, check_vect, vargs);

    auto arg1 = static_cast<T1>(vargs[0]);
    auto arg2 = static_cast<T2>(vargs[1]);

    return variant_t(RT(F()(arg1, arg2)));
}

template <class F, typename RT, typename T1, typename T2, typename T3>
variant_t functor_RT_T1_T2_T3(rt_prog_ctx_t& ctx, const std::string& name,
    const func_args_t& args, variant_t::type_t argt1, variant_t::type_t argt2,
    variant_t::type_t argt3)
{
    std::vector<variant_t::type_t> check_vect = { argt1, argt2, argt3 };
    std::vector<variant_t> vargs;
    get_functor_vargs(ctx, name, args, check_vect, vargs);

    const auto arg1 = static_cast<T1>(vargs[0]);
    const auto arg2 = static_cast<T2>(vargs[1]);
    const auto arg3 = static_cast<T3>(vargs[2]);

    return variant_t(RT(F()(arg1, arg2, arg3)));
}

template <class F, typename RT, typename T1, typename T2, typename T3,
    typename T4>
variant_t functor_RT_T1_T2_T3_T4(rt_prog_ctx_t& ctx, const std::string& name,
    const func_args_t& args, variant_t::type_t argt1, variant_t::type_t argt2,
    variant_t::type_t argt3, variant_t::type_t argt4)
{
    std::vector<variant_t::type_t> check_vect = { argt1, argt2, argt3, argt4 };
    std::vector<variant_t> vargs;
    get_functor_vargs(ctx, name, args, check_vect, vargs);

    const auto arg1 = static_cast<T1>(vargs[0]);
    const auto arg2 = static_cast<T2>(vargs[1]);
    const auto arg3 = static_cast<T3>(vargs[2]);
    const auto arg4 = static_cast<T3>(vargs[3]);

    return variant_t(RT(F()(arg1, arg2, arg3, arg4)));
}

template <class F>
variant_t functor_int_int(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T<F, int, int>(
        ctx, name, args, variant_t::type_t::INTEGER);
}

template <class F>
variant_t functor_int_float(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T<F, int, float>(
        ctx, name, args, variant_t::type_t::DOUBLE);
}

template <class F>
variant_t functor_int_double(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T<F, int, double>(
        ctx, name, args, variant_t::type_t::DOUBLE);
}

template <class F>
variant_t functor_int_string(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T<F, int, std::string>(
        ctx, name, args, variant_t::type_t::STRING);
}

template <class F>
variant_t functor_string_int(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T<F, std::string, int>(
        ctx, name, args, variant_t::type_t::INTEGER);
}

template <class F>
variant_t functor_string_float(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T<F, std::string, float>(
        ctx, name, args, variant_t::type_t::DOUBLE);
}

template <class F>
variant_t functor_string_double(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T<F, std::string, double>(
        ctx, name, args, variant_t::type_t::DOUBLE);
}

template <class F>
variant_t functor_string_string(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T<F, std::string, std::string>(
        ctx, name, args, variant_t::type_t::STRING);
}

template <class F>
variant_t functor_float_string(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T<F, float, std::string>(
        ctx, name, args, variant_t::type_t::STRING);
}

template <class F>
variant_t functor_double_string(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T<F, double, std::string>(
        ctx, name, args, variant_t::type_t::STRING);
}

template <class F>
variant_t functor_string_string_int(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T1_T2<F, std::string, std::string, int>(
        ctx, name, args, variant_t::type_t::STRING, variant_t::type_t::INTEGER);
}

template <class F>
variant_t functor_string_double_int(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T1_T2<F, std::string, double, int>(
        ctx, name, args, variant_t::type_t::DOUBLE, variant_t::type_t::INTEGER);
}

template <class F>
variant_t functor_int_string_string(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T1_T2<F, int, std::string, std::string>(
        ctx, name, args, variant_t::type_t::STRING, variant_t::type_t::STRING);
}

template <class F>
variant_t functor_int_string_int(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T1_T2<F, int, std::string, int>(
        ctx, name, args, variant_t::type_t::STRING, variant_t::type_t::INTEGER);
}

template <class F>
variant_t functor_int_int_int_int(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T1_T2_T3<F, int, int, int, int>(ctx, name, args,
        variant_t::type_t::INTEGER, variant_t::type_t::INTEGER,
        variant_t::type_t::INTEGER);
}

template <class F>
variant_t functor_int_int_int_int_int(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T1_T2_T3_T4<F, int, int, int, int, int>(ctx, name, args,
        variant_t::type_t::INTEGER, variant_t::type_t::INTEGER,
        variant_t::type_t::INTEGER, variant_t::type_t::INTEGER);
}

template <class F>
variant_t functor_int_int_int(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T1_T2<F, int, int, int>(ctx, name, args,
        variant_t::type_t::INTEGER, variant_t::type_t::INTEGER);
}

template <class F>
variant_t functor_string_string_int_int(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T1_T2_T3<F, std::string, std::string, int, int>(ctx, name,
        args, variant_t::type_t::STRING, variant_t::type_t::INTEGER,
        variant_t::type_t::INTEGER);
}

template <class F>
variant_t functor_string_string_int_string(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return functor_RT_T1_T2_T3<F, std::string, std::string, int, std::string>(
        ctx, name, args, variant_t::type_t::STRING, variant_t::type_t::INTEGER,
        variant_t::type_t::STRING);
}

template <class T, class F>
variant_t math_functor_aux(rt_prog_ctx_t& ctx, const std::string& name,
    const func_args_t& args, std::true_type)
{
    return functor_RT_T<F, T, T>(ctx, name, args, variant_t::type_t::INTEGER);
}

template <class T, class F>
variant_t math_functor_aux(rt_prog_ctx_t& ctx, const std::string& name,
    const func_args_t& args, std::false_type)
{
    return functor_RT_T<F, T, T>(ctx, name, args, variant_t::type_t::DOUBLE);
}

template <class T, class F>
variant_t math_functor(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    return math_functor_aux<T, F>(ctx, name, args, std::is_integral<T>());
}

template <class T, class F>
variant_t math_functor2(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args)
{
    if (std::is_integral<T>::value) {
        return functor_RT_T1_T2<F, T, T, T>(ctx, name, args,
            variant_t::type_t::INTEGER, variant_t::type_t::INTEGER);
    }

    return functor_RT_T1_T2<F, T, T, T>(
        ctx, name, args, variant_t::type_t::DOUBLE, variant_t::type_t::DOUBLE);
}

variant_t conv_functor(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args);
variant_t restore_functor(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args);
variant_t evaluate_expression(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args);
variant_t evaluate_and_export_result(
    rt_prog_ctx_t& ctx, const std::string& name, const func_args_t& args);
variant_t process_operator(rt_prog_ctx_t& ctx, const std::string& operator_name,
    const func_args_t& args);

} // namespace nu

#endif // __NU_BUILTIN_FUNCTION_SUPPORT_H__
