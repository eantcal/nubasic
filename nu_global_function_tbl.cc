/*
*  This file is part of nuBASIC
*
*  nuBASIC is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  nuBASIC is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with nuBASIC; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
*
*  Author: <antonino.calderone@ericsson.com>, <acaldmail@gmail.com>
*
*/


/* -------------------------------------------------------------------------- */

#include "nu_about.h"
#include "nu_variant.h"
#include "nu_global_function_tbl.h"
#include "nu_os_console.h"
#include "nu_os_std.h"
#include "nu_eval_expr.h"
#include "nu_expr_var.h"
#include "nu_basic_defs.h"

#ifndef TINY_NUBASIC_VER
#include "nu_os_gdi.h"
#endif

#include <functional>
#include <ctime>
#include <string>
#include <cstdlib>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

global_function_tbl_t* global_function_tbl_t::_instance = nullptr;
global_operator_tbl_t* global_operator_tbl_t::_instance = nullptr;


/* -------------------------------------------------------------------------- */

inline static void check_arg_num(
   const nu::func_args_t & args,
   int expected_arg_num,
   const std::string & fname)
{
   std::string error =
      "'" + fname +
      "': expects to be passed " +
      nu::to_string(expected_arg_num) + " argument(s)";

   switch (expected_arg_num)
   {
      case 0:
         error =
            "'" + fname +
            "': expects to be passed no arguments";
         break;

      case 1:
         error =
            "'" + fname +
            "': expects to be passed one argument";
         break;
   }

   bool valid_args =
      (expected_arg_num == 0 && args.size() == 0) ||
      (expected_arg_num == 0 && args.size() == 1 && args[0]->empty()) ||
      (expected_arg_num == 1 && args.size() == 1 && !args[0]->empty()) ||
      (expected_arg_num > 1 && int(args.size()) == expected_arg_num);

   syntax_error_if(!valid_args, error);
}


/* -------------------------------------------------------------------------- */

void get_functor_vargs(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args,
   const std::vector<variant_t::type_t> & check_vect,
   std::vector< variant_t > & vargs)
{
   check_arg_num(args, int ( check_vect.size() ), name);

   for (auto & arg : args)
      vargs.push_back(arg->eval(ctx));

   int i = 0;

   for (auto & vargt : check_vect)
   {
      bool invalid_check = true;

      if (vargt == variant_t::type_t::UNDEFINED)
      {
         // Do not care about this argument
         continue;
      }

      // Accept implicit conversion from/to types double/float/int
      if ( variable_t::is_number(vargt) &&
            variable_t::is_number(vargs[i].get_type()) )
      {
         invalid_check = false;
      }
      else
      {
         invalid_check = vargs[i].get_type() != vargt;
      }

      syntax_error_if(
         invalid_check,
         "'" + name +
         "': expects to be passed argument " + nu::to_string(i+1) +
         " as " + variant_t::get_type_desc(vargt));

      ++i;
   }
}


/* -------------------------------------------------------------------------- */

// RT F()(T arg)
template <class F, typename RT, typename T>
variant_t functor_RT_T(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args,
   variant_t::type_t argt)
{
   std::vector< variant_t::type_t > check_vect = { argt };
   std::vector< variant_t > vargs;
   get_functor_vargs(ctx, name, args, check_vect, vargs);

   auto arg = static_cast<T>(vargs[0]);

   return nu::variant_t(RT(F()(arg)));
}


/* -------------------------------------------------------------------------- */

// RT F()(T1 arg1, T2 arg2)
template <class F, typename RT, typename T1, typename T2>
variant_t functor_RT_T1_T2(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args,
   variant_t::type_t argt1,
   variant_t::type_t argt2)
{
   std::vector< variant_t::type_t > check_vect = { argt1, argt2 };
   std::vector< variant_t > vargs;
   get_functor_vargs(ctx, name, args, check_vect, vargs);

   auto arg1 = static_cast<T1>(vargs[0]);
   auto arg2 = static_cast<T2>(vargs[1]);

   return nu::variant_t(RT(F()(arg1, arg2)));
}


/* -------------------------------------------------------------------------- */

// RT F()(T1 arg1, T2 arg2, T3 arg3)
template <class F, typename RT, typename T1, typename T2, typename T3>
variant_t functor_RT_T1_T2_T3(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args,
   variant_t::type_t argt1,
   variant_t::type_t argt2,
   variant_t::type_t argt3)
{
   std::vector< variant_t::type_t > check_vect = { argt1, argt2, argt3 };
   std::vector< variant_t > vargs;
   get_functor_vargs(ctx, name, args, check_vect, vargs);

   auto arg1 = static_cast<T1>(vargs[0]);
   auto arg2 = static_cast<T2>(vargs[1]);
   auto arg3 = static_cast<T3>(vargs[2]);

   return nu::variant_t(RT(F()(arg1, arg2, arg3)));
}


/* -------------------------------------------------------------------------- */

// RT F()(T1 arg1, T2 arg2, T3 arg3)
template <class F, typename RT, typename T1, typename T2, typename T3, typename T4>
variant_t functor_RT_T1_T2_T3_T4(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args,
   variant_t::type_t argt1,
   variant_t::type_t argt2,
   variant_t::type_t argt3,
   variant_t::type_t argt4)
{
   std::vector< variant_t::type_t > check_vect = { argt1, argt2, argt3, argt4 };
   std::vector< variant_t > vargs;
   get_functor_vargs(ctx, name, args, check_vect, vargs);

   auto arg1 = static_cast<T1>(vargs[0]);
   auto arg2 = static_cast<T2>(vargs[1]);
   auto arg3 = static_cast<T3>(vargs[2]);
   auto arg4 = static_cast<T3>(vargs[3]);

   return nu::variant_t(RT(F()(arg1, arg2, arg3, arg4)));
}


/* -------------------------------------------------------------------------- */

// int F()(float)
template <class F>
variant_t functor_int_float(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T<F, int, float>(
             ctx,
             name,
             args,
             variant_t::type_t::FLOAT);
}


/* -------------------------------------------------------------------------- */

// int F()(double)
template <class F>
variant_t functor_int_double(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T<F, int, double>(
             ctx,
             name,
             args,
             variant_t::type_t::DOUBLE);
}


/* -------------------------------------------------------------------------- */

// int F()(string)
template <class F>
variant_t functor_int_string(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T<F, int, std::string>(
             ctx,
             name,
             args,
             variant_t::type_t::STRING);
}


/* -------------------------------------------------------------------------- */

// string F()(int)
template <class F>
variant_t functor_string_int(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T<F, std::string, int>(
             ctx,
             name,
             args,
             variant_t::type_t::INTEGER);
}


/* -------------------------------------------------------------------------- */

// string F()(float)
template <class F>
variant_t functor_string_float(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T<F, std::string, float>(
             ctx,
             name,
             args,
             variant_t::type_t::FLOAT);
}


/* -------------------------------------------------------------------------- */

// string F()(double)
template <class F>
variant_t functor_string_double(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T<F, std::string, double>(
             ctx,
             name,
             args,
             variant_t::type_t::DOUBLE);
}


/* -------------------------------------------------------------------------- */

// string F()(string)
template <class F>
variant_t functor_string_string(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T<F, std::string, std::string>(
             ctx,
             name,
             args,
             variant_t::type_t::STRING);
}


/* -------------------------------------------------------------------------- */

// float F()(string)
template <class F>
variant_t functor_float_string(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T<F, float, std::string>(
             ctx,
             name,
             args,
             variant_t::type_t::STRING);
}


/* -------------------------------------------------------------------------- */

// double F()(string)
template <class F>
variant_t functor_double_string(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T<F, double, std::string>(
             ctx,
             name,
             args,
             variant_t::type_t::STRING);
}


/* -------------------------------------------------------------------------- */

// string F()(string, int)
template <class F>
variant_t functor_string_string_int(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T1_T2<F, std::string, std::string, int>(
             ctx,
             name,
             args,
             variant_t::type_t::STRING,
             variant_t::type_t::INTEGER);
}


/* -------------------------------------------------------------------------- */

// int F()(string, string)
template <class F>
variant_t functor_int_string_string(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T1_T2<F, int, std::string, std::string>(
             ctx,
             name,
             args,
             variant_t::type_t::STRING,
             variant_t::type_t::STRING);
}


/* -------------------------------------------------------------------------- */

// int F()(string, int)
template <class F>
variant_t functor_int_string_int(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T1_T2<F, int, std::string, int>(
             ctx,
             name,
             args,
             variant_t::type_t::STRING,
             variant_t::type_t::INTEGER);
}


/* -------------------------------------------------------------------------- */

// int F()(int, int, int)
template <class F>
variant_t functor_int_int_int_int(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T1_T2_T3<F, int, int, int, int>(
             ctx,
             name,
             args,
             variant_t::type_t::INTEGER,
             variant_t::type_t::INTEGER,
             variant_t::type_t::INTEGER);
}


/* -------------------------------------------------------------------------- */

// int F()(int, int, int, int)
template <class F>
variant_t functor_int_int_int_int_int(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T1_T2_T3_T4<F, int, int, int, int, int>(
             ctx,
             name,
             args,
             variant_t::type_t::INTEGER,
             variant_t::type_t::INTEGER,
             variant_t::type_t::INTEGER,
             variant_t::type_t::INTEGER);
}

/* -------------------------------------------------------------------------- */

// string F()(string, int, int)
template <class F >
variant_t functor_string_string_int_int(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T1_T2_T3<F, std::string, std::string, int, int>(
             ctx,
             name,
             args,
             variant_t::type_t::STRING,
             variant_t::type_t::INTEGER,
             variant_t::type_t::INTEGER);
}


/* -------------------------------------------------------------------------- */

// string F()(string, int, string)
template <class F >
variant_t functor_string_string_int_string(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return functor_RT_T1_T2_T3<F, std::string, std::string, int, std::string>(
             ctx,
             name,
             args,
             variant_t::type_t::STRING,
             variant_t::type_t::INTEGER,
             variant_t::type_t::STRING);
}

/* -------------------------------------------------------------------------- */

// T F()(T) where T is int
template <class T, class F>
variant_t math_functor_aux(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args,
   std::true_type)
{
   return functor_RT_T<F, T, T>(ctx, name, args, { variant_t::type_t::INTEGER });
}


/* -------------------------------------------------------------------------- */

// T F()(T) where T is double
template <class T, class F>
variant_t math_functor_aux(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args,
   std::false_type)
{
   return functor_RT_T<F, T, T>(ctx, name, args, { variant_t::type_t::DOUBLE });
}


/* -------------------------------------------------------------------------- */

// T F()(T) where T might be either float or int
template <class T, class F>
variant_t math_functor(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   return math_functor_aux<T, F>(ctx, name, args, std::is_integral<T>());
}


/* -------------------------------------------------------------------------- */

// T F()(T,T) where T might be either float or int
template <class T, class F>
variant_t math_functor2(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   if (std::is_integral<T>::value)
   {
      return functor_RT_T1_T2<F, T, T, T>(
                ctx,
                name,
                args,
                variant_t::type_t::INTEGER,
                variant_t::type_t::INTEGER
             );
   }

   return functor_RT_T1_T2<F, T, T, T>(
             ctx,
             name,
             args,
             variant_t::type_t::DOUBLE,
             variant_t::type_t::DOUBLE );
}



/* -------------------------------------------------------------------------- */

static variant_t evaluate_expression(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   std::vector< variant_t::type_t > check_vect = { variant_t::type_t::STRING };
   std::vector< variant_t > vargs;
   get_functor_vargs(ctx, name, args, check_vect, vargs);

   const std::string & arg = vargs[0].to_str();
   return nu::eval_expr(ctx, arg);
}


/* -------------------------------------------------------------------------- */

static variant_t evaluate_and_export_result(
   rt_prog_ctx_t & ctx,
   const std::string& name,
   const nu::func_args_t & args)
{
   ctx.exported_result = evaluate_expression(ctx, name, args);
   return ctx.exported_result;
}


/* -------------------------------------------------------------------------- */

static variant_t process_operator(
   rt_prog_ctx_t & ctx,
   const std::string& operator_name,
   const nu::func_args_t & args)
{
   rt_error_code_t::get_instance().throw_if(
      args.size() != 1,
      0,
      rt_error_code_t::E_INVALID_ARGS,
      "");


   auto var = args[0];
   auto var_ptr = dynamic_cast<expr_var_t*>(var.get());

   rt_error_code_t::get_instance().throw_if(
      var_ptr == nullptr,
      0,
      rt_error_code_t::E_INVALID_ARGS,
      "");

   const auto variable_name = var_ptr->name();

   // Resolve scope using the most nested one
   var_scope_t::handle_t scope =
      ctx.proc_scope.get(proc_scope_t::type_t::LOCAL);

   rt_error_code_t::get_instance().throw_if(
      !scope->is_defined(variable_name),
      0,
      rt_error_code_t::E_INV_IDENTIF,
      "");

   auto variable_value = &(*scope)[variable_name].first;

   if (operator_name == NU_BASIC_OP_INC)
   {
      variable_value->increment();
   }
   else if (operator_name == NU_BASIC_OP_DEC)
   {
      variable_value->decrement();
   }
   else
   {
      rt_error_code_t::get_instance().throw_if(
         true,
         0,
         rt_error_code_t::E_FUNC_UNDEF,
         "");
   }

   return *variable_value;
}


/* -------------------------------------------------------------------------- */

global_function_tbl_t& global_function_tbl_t::get_instance()
{
   if (! _instance)
   {
      _instance = new global_function_tbl_t();
      assert(_instance);

      global_function_tbl_t & fmap = *_instance;

      /*

      __FUNCTOR_BUILDER(sin)

      ===>

      struct _sin { float operator()(float x) NU_NOEXCEPT { return ::sin(x); } };
      fmap["sin"] = functor<float, _sin>;

      */


#ifdef __FUNCTOR_BUILDER
#warning "__FUNCTOR_BUILDER already defined..."
#undef __FUNCTOR_BUILDER
#endif

#define __FUNCTOR_BUILDER(_FNC_) \
            struct _##_FNC_{ float operator()(float x) NU_NOEXCEPT { return _FNC_(x); } }; \
            fmap[#_FNC_] = math_functor<float, _##_FNC_>;

      __FUNCTOR_BUILDER(truncf);

#ifdef __DOUBLE_FUNCTOR_BUILDER
#warning "__FUNCTOR_BUILDER already defined..."
#undef __DOUBLE_FUNCTOR_BUILDER
#endif


#define __DOUBLE_FUNCTOR_BUILDER(_FNC_) \
            struct _##_FNC_{ double operator()(double x) NU_NOEXCEPT { return _FNC_(x); } }; \
            fmap[#_FNC_] = math_functor<double, _##_FNC_>;

      __DOUBLE_FUNCTOR_BUILDER(sin);
      __DOUBLE_FUNCTOR_BUILDER(cos);
      __DOUBLE_FUNCTOR_BUILDER(tan);
      __DOUBLE_FUNCTOR_BUILDER(log);
      __DOUBLE_FUNCTOR_BUILDER(log10);
      __DOUBLE_FUNCTOR_BUILDER(exp);
      __DOUBLE_FUNCTOR_BUILDER(abs);
      __DOUBLE_FUNCTOR_BUILDER(asin);
      __DOUBLE_FUNCTOR_BUILDER(acos);
      __DOUBLE_FUNCTOR_BUILDER(atan);
      __DOUBLE_FUNCTOR_BUILDER(sinh);
      __DOUBLE_FUNCTOR_BUILDER(cosh);
      __DOUBLE_FUNCTOR_BUILDER(tanh);
      __DOUBLE_FUNCTOR_BUILDER(sqrt);


      struct _sign
      {
         double operator()(double x) NU_NOEXCEPT
         {
            return x > 0.0F ? 1.0F : (x == 0.0F ? 0.0F : -1.0F);
         }
      };


      fmap["sign"] = math_functor<double, _sign>;


      struct _min
      {
         double operator()(double x, double y) NU_NOEXCEPT
         {
            return x < y ? x : y;
         }
      };


      fmap["min"] = math_functor2<double, _min>;


      struct _max
      {
         double operator()(double x, double y) NU_NOEXCEPT
         {
            return x > y ? x : y;
         }
      };


      fmap["max"] = math_functor2<double, _max>;


      struct _pow
      {
         double operator()(double x, double y) NU_NOEXCEPT
         {
            return ::pow(x,y);
         }
      };


      fmap["pow"] = math_functor2<double, _pow>;


      struct _int_truncate
      {
         int operator()(double x) NU_NOEXCEPT
         {
            // Truncate to greatest integer less or equal to Argument
            // Example:
            // int(-5) => -5
            // int(-5.1) => -6
            // int(5.9) => 5
            return int((x < 0.0F) ?
                       int((abs(x)) - int(abs(x))>0.0F ?
                           int(x)-1 : int(x)) : int(x));
         }
      };

      // Truncate to greatest integer less or equal to Argument
      fmap["int"] = functor_int_double<_int_truncate>;


      // sqr is an alias of sqrt
      fmap["sqr"] = math_functor<double, _sqrt>;


      struct _rnd
      {
         double operator()(double x) NU_NOEXCEPT
         {
            if (x < 0.0F)
            {
               // Seed the random-number generator with the
               // current time so that the numbers will be
               // different every time we run.
               ::srand((unsigned)time(NULL));
               ::rand();
            }

            return double(::rand()) / RAND_MAX;
         }
      };

      /*
          RND - Random Number in [0.0 .. 1.0]
      Syntax:
          RND(<Seed>)
      If (<Seed> < 0) the Random number generator is initialized
          Examples:
              RND(-625) -> 3.85114436E-06
              RND(0) -> 0.464844882
              RND(0) -> 0.0156260729
      */

      fmap["rnd"] = math_functor<double, _rnd>;


      struct _boolean_not
      {
         int operator()(double x) NU_NOEXCEPT
         {
            return int((x == 0.0F) ? 1 : 0);
         }
      };

      fmap["not"] = functor_int_double<_boolean_not>;


      struct _bitwise_not
      {
         int operator()(int x) NU_NOEXCEPT
         {
            return ~x;
         }
      };

      fmap["b_not"] = math_functor<int, _bitwise_not>;


      struct _len_str
      {
         int operator()(const std::string x) NU_NOEXCEPT
         {
            return int( x.size() );
         }
      };

      fmap["len$"] = functor_int_string<_len_str>;
      fmap["len"] = functor_int_string<_len_str>;


      struct _input_str
      {
         std::string operator()(int n) NU_NOEXCEPT
         {
            return _os_input_str(n);
         }
      };

      fmap["input$"] = functor_string_int<_input_str>;


      struct _asc_str
      {
         int operator()(const std::string x) NU_NOEXCEPT
         {
            return (x.empty() ? 0 : x.c_str()[0]) & 0xff;
         }
      };

      fmap["asc$"] = functor_int_string<_asc_str>;
      fmap["asc"] = functor_int_string<_asc_str>;


      struct _space_str
      {
         std::string operator()(int nspace) NU_NOEXCEPT
         {
            std::string s;
            nspace = nspace < 0 ? 0 : nspace;

            for (int i = 0; i < nspace; ++i)
               s += " ";

            return s;
         }
      };

      //functor_string_int
      fmap["spc"] = functor_string_int<_space_str>;
      fmap["space$"] = functor_string_int<_space_str>;


      struct _chr_str
      {
         std::string operator()(int code) NU_NOEXCEPT
         {
            char s[2] = { 0 };
            s[0] = code;
            return s;
         }
      };

      fmap["chr$"] = functor_string_int<_chr_str>;
      fmap["chr"] = functor_string_int<_chr_str>;


      struct _left_str
      {
         std::string operator()(const std::string& s,int n) NU_NOEXCEPT
         {
            if (n <= 0)
               return std::string();
            else if (n > int(s.size()))
               n = int ( s.size() );

            return s.substr(0, n);
         }
      };

      fmap["left"] = functor_string_string_int<_left_str>;
      fmap["left$"] = functor_string_string_int<_left_str>;


      struct _lcase_str
      {
         std::string operator()(const std::string& s) NU_NOEXCEPT
         {
            std::string ret = s;
            std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
            return ret;
         }
      };

      fmap["lcase"] = functor_string_string<_lcase_str>;
      fmap["lcase$"] = functor_string_string<_lcase_str>;


      struct _ucase_str
      {
         std::string operator()(const std::string& s) NU_NOEXCEPT
         {
            std::string ret = s;
            std::transform(ret.begin(), ret.end(), ret.begin(), ::toupper);
            return ret;
         }
      };

      fmap["ucase"] = functor_string_string<_ucase_str>;
      fmap["ucase$"] = functor_string_string<_ucase_str>;


      fmap["eval"] = evaluate_expression;

      // Used by IDE
      fmap["__eval_export"] = evaluate_and_export_result;


      // Built-in unary operators

      fmap[NU_BASIC_OP_INC] = process_operator;
      fmap[NU_BASIC_OP_DEC] = process_operator;


      struct _right_str
      {
         std::string operator()(const std::string& s, int n) NU_NOEXCEPT
         {
            if (n <= 0)
               return s;
            else if (n > int(s.size()))
               n = int ( s.size() );

            int skip = int ( s.size() ) - n;

            return s.substr(skip, n);
         }
      };

      fmap["right"] = functor_string_string_int<_right_str>;
      fmap["right$"] = functor_string_string_int<_right_str>;


      struct _instrcs
      {
         int operator()(
            const std::string& s,
            const std::string& search_str) NU_NOEXCEPT
         {
            if (search_str.empty())
               return 0;
            else if (s.empty() || search_str.size()>s.size())
               return -1;
            else if (search_str.size() == s.size() && s == search_str)
               return 0;

            std::string mystr = s;
            int pos = 0;

            while (search_str.size() <= mystr.size())
            {
               if (mystr.substr(0, search_str.size()) == search_str)
                  return pos;

               ++pos;
               mystr = mystr.substr(1, mystr.size() - 1);
            }

            return -1;
         }
      };

      fmap["instrcs"] = functor_int_string_string<_instrcs>;


      struct _instr : public _instrcs
      {
         int operator()(
            const std::string& s,
            const std::string& search_str) NU_NOEXCEPT
         {
            std::string mys = s;
            std::string ss = search_str;
            std::transform(mys.begin(), mys.end(), mys.begin(), toupper);
            std::transform(ss.begin(), ss.end(), ss.begin(), toupper);

            return _instrcs::operator()(mys, ss);
         }
      };

      fmap["instr"] = functor_int_string_string<_instr>;


      struct _sub_str
      {
         //SUBSTR(s$,pos,n)
         //return a string of n characters from s$ beginning
         //with the pos-th character (0 is first character)
         //Comment: pos >= 0, n >= 0

         std::string operator()(const std::string& s, int pos, int n) NU_NOEXCEPT
         {
            if (pos<1)
               pos = 0;

            if (pos >= int(s.size()))
               return std::string();

            if (n < 0)
               n = 0;

            if ((pos + n) >= int(s.size()))
               n = int(s.size()) - pos;

            return s.substr(pos, n);
         }
      };


      fmap["substr$"] = functor_string_string_int_int<_sub_str>;


      struct _mid_str
      {
         //MID(s$,pos,n)
         //return a string of n characters from s$ beginning
         //with the pos-th character (1 is first character)
         //Comment: pos >= 1, n >= 0

         std::string operator()(const std::string& s, int pos, int n) NU_NOEXCEPT
         {
            --pos;

            if (pos<1)
               pos = 0;

            if (pos >= int(s.size()))
               return std::string();

            if (n < 0)
               n = 0;

            if ((pos+n) >= int(s.size()))
               n = int ( s.size() )-pos;

            return s.substr(pos, n);
         }
      };


      fmap["mid$"] = functor_string_string_int_int<_mid_str>;
      fmap["mid"] = functor_string_string_int_int<_mid_str>;


      struct _patch_str
      {
         //PSTR$(s$,pos,c)
         //return a string s$ with the pos-th character replaced by c
         //with pos >= 0

         std::string operator()(const std::string& s, int pos, const std::string& c_s) NU_NOEXCEPT
         {

            if (pos >= int(s.size()))
            {
               pos = int(s.size() - 1);
            }

            if (pos<1)
            {
               pos = 0;
            }


            std::string ret = s;

            char c = c_s.empty() ? '\0' : c_s[0];

            ret[pos] = c;

            return ret;
         }
      };

      fmap["pstr$"] = functor_string_string_int_string<_patch_str>;


      struct _val_str
      {
         double operator()(const std::string& x) NU_NOEXCEPT
         {
            try
            {
               return nu::stod(x);
            }

            catch (...)
            {
               return 0.0;
            }
         }
      };


      fmap["val"] = functor_double_string<_val_str>;
      fmap["val%"] = functor_int_string<_val_str>;


      struct _to_str
      {
         std::string operator()(double x) NU_NOEXCEPT
         {
            if (::floor(x) == x)
            {
               return to_string(int(x));
            }

            return to_string(x);
         }
      };


      fmap["str"] = functor_string_double<_to_str>;
      fmap["str$"] = functor_string_double<_to_str>;


      struct _to_hex_str
      {
         std::string operator()(double x) NU_NOEXCEPT
         {
            std::stringstream ss;
            ss << std::hex << int(x);
            return ss.str();
         }
      };

      fmap["hex$"] = functor_string_double<_to_hex_str>;
      fmap["hex"] = functor_string_double<_to_hex_str>;


      auto functor_errno = [](
                              rt_prog_ctx_t & ctx,
                              const std::string& name,
                              const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(ctx.get_errno());
      };

      fmap["errno"] = functor_errno;


      auto functor_errno_str = [](
                                  rt_prog_ctx_t & ctx,
                                  const std::string& name,
                                  const nu::func_args_t & args)
      {
         std::vector< variant_t > vargs;
         get_functor_vargs(ctx, name, args, { variant_t::type_t::INTEGER }, vargs);

         return nu::variant_t(strerror(vargs[0].to_int()));
      };

      fmap["errno$"] = functor_errno_str;


      auto functor_eof = [](
                            rt_prog_ctx_t & ctx,
                            const std::string& name,
                            const nu::func_args_t & args)
      {
         std::vector< variant_t > vargs;
         get_functor_vargs(ctx, name, args, { variant_t::type_t::INTEGER }, vargs);

         auto file = ctx.file_tbl.resolve_fd(vargs[0].to_int());

         if (!file)
         {
            ctx.set_errno(EINVAL);
            return nu::variant_t(-1);
         }

         return nu::variant_t(feof(file));
      };


      fmap["eof"] = functor_eof;


      auto functor_ferror = [](
                               rt_prog_ctx_t & ctx,
                               const std::string& name,
                               const nu::func_args_t & args)
      {
         std::vector< variant_t > vargs;
         get_functor_vargs(ctx, name, args, { variant_t::type_t::INTEGER }, vargs);

         auto file = ctx.file_tbl.resolve_fd(vargs[0].to_int());

         if (!file)
         {
            ctx.set_errno(EINVAL);
            return nu::variant_t(-1);
         }

         return nu::variant_t(ferror(file));
      };

      fmap["ferror"] = functor_ferror;


      auto functor_ftell = [](
                              rt_prog_ctx_t & ctx,
                              const std::string& name,
                              const nu::func_args_t & args)
      {
         std::vector< variant_t > vargs;
         get_functor_vargs(ctx, name, args, { variant_t::type_t::INTEGER }, vargs);

         auto file = ctx.file_tbl.resolve_fd(vargs[0].to_int());

         if (!file)
         {
            ctx.set_errno(EINVAL);
            return nu::variant_t(-1);
         }

         return nu::variant_t(int(ftell(file)));
      };

      fmap["ftell"] = functor_ftell;


      auto functor_fsize = [](
                              rt_prog_ctx_t & ctx,
                              const std::string& name,
                              const nu::func_args_t & args)
      {
         std::vector< variant_t > vargs;
         get_functor_vargs(ctx, name, args, { variant_t::type_t::INTEGER }, vargs);

         auto file = ctx.file_tbl.resolve_fd(vargs[0].to_int());

         if (!file)
         {
            ctx.set_errno(EINVAL);
            return nu::variant_t(-1);
         }

         int cur_pos = ftell(file);

         if (cur_pos <0 || fseek(file, 0, SEEK_END) < 0)
         {
            ctx.set_errno(EINVAL);
            return nu::variant_t(-1);
         }

         auto size = ftell(file);

         if (fseek(file, cur_pos, SEEK_SET) < 0)
         {
            ctx.set_errno(EINVAL);
            return nu::variant_t(-1);
         }

         return variant_t(integer_t(size));
      };

      fmap["fsize"] = functor_fsize;


      auto functor_inkeys = [](
                               rt_prog_ctx_t & ctx,
                               const std::string& name,
                               const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);

         int ch = _os_kbhit();
         std::string s;

         if (ch)
            s.push_back(ch);

         return variant_t(s, variant_t::type_t::STRING);
      };

      fmap["inkey$"] = functor_inkeys;


      auto functor_pwd = [](
                            rt_prog_ctx_t & ctx,
                            const std::string& name,
                            const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);

         auto wd = _os_get_working_dir();

         return variant_t(wd, variant_t::type_t::STRING);
      };

      fmap["pwd$"] = functor_pwd;


      struct _getenv_str
      {
         std::string operator()(const std::string& x) NU_NOEXCEPT
         {
            std::string ret;

            char * var = ::getenv(x.c_str());

            if (var)
               ret = var;

            return ret;
         }
      };

      fmap["getenv$"] = functor_string_string<_getenv_str>;
      fmap["getenv"] = functor_string_string<_getenv_str>;


      struct _setenv_str
      {
         int operator()(const std::string& var, const std::string& val) NU_NOEXCEPT
         {
            return _os_setenv(var.c_str(), val.c_str());
         }
      };

      fmap["setenv"] = functor_int_string_string<_setenv_str>;


      struct _unsetenv_str
      {
         int operator()(const std::string& var) NU_NOEXCEPT
         {
            return _os_unsetenv(var.c_str());
         }
      };

      fmap["unsetenv"] = functor_int_string<_unsetenv_str>;


      struct _erase_file
      {
         int operator()(const std::string& var) NU_NOEXCEPT
         {
            return _os_erase_file(var.c_str());
         }
      };

      fmap["erase"] = functor_int_string<_erase_file>;


      struct _erase_dir
      {
         int operator()(const std::string& var) NU_NOEXCEPT
         {
            return _os_erase_dir(var.c_str());
         }
      };

      fmap["rmdir"] = functor_int_string<_erase_dir>;


      struct _make_dir
      {
         int operator()(const std::string& var) NU_NOEXCEPT
         {
            return _os_make_dir(var.c_str());
         }
      };

      fmap["mkdir"] = functor_int_string<_make_dir>;



#ifndef TINY_NUBASIC_VER
      auto functor_getswidth = [](
                                  rt_prog_ctx_t & ctx,
                                  const std::string& name,
                                  const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(_os_get_screen_width());
      };

      fmap["getswidth"] = functor_getswidth;


      auto functor_getsheight = [](
                                   rt_prog_ctx_t & ctx,
                                   const std::string& name,
                                   const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(_os_get_screen_height());
      };

      fmap["getsheight"] = functor_getsheight;


      auto functor_getwindowx = [](
                                   rt_prog_ctx_t & ctx,
                                   const std::string& name,
                                   const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(_os_get_window_x());
      };

      fmap["getwindowx"] = functor_getwindowx;


      auto functor_getwindowy = [](
                                   rt_prog_ctx_t & ctx,
                                   const std::string& name,
                                   const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(_os_get_window_y());
      };

      fmap["getwindowy"] = functor_getwindowy;


      auto functor_getwindowdx = [](
                                    rt_prog_ctx_t & ctx,
                                    const std::string& name,
                                    const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(_os_get_window_dx());
      };

      fmap["getwindowdx"] = functor_getwindowdx;


      auto functor_getwindowdy = [](
                                    rt_prog_ctx_t & ctx,
                                    const std::string& name,
                                    const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(_os_get_window_dy());
      };

      fmap["getwindowdy"] = functor_getwindowdy;


      auto functor_getmousex = [](
                                  rt_prog_ctx_t & ctx,
                                  const std::string& name,
                                  const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(_os_get_mouse_x());
      };

      fmap["getmousex"] = functor_getmousex;


      auto functor_getmousey = [](
                                  rt_prog_ctx_t & ctx,
                                  const std::string& name,
                                  const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(_os_get_mouse_y());
      };

      fmap["getmousey"] = functor_getmousey;


      auto functor_getmousebtn = [](
                                    rt_prog_ctx_t & ctx,
                                    const std::string& name,
                                    const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(_os_get_mouse_btn());
      };

      fmap["getmousebtn"] = functor_getmousebtn;
#endif // TINY_NUBASIC_VER


      auto functor_pi = [](
                           rt_prog_ctx_t & ctx,
                           const std::string& name,
                           const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(3.1415926535897F);
      };

      fmap["pi"] = functor_pi;


      auto functor_plat_id = [](
                                rt_prog_ctx_t & ctx,
                                const std::string& name,
                                const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
#ifdef WIN32
         return 1;
#else // assuming Linux (TODO modify to add other platforms)
         return 2;
#endif
      };

      fmap["getplatid"] = functor_plat_id;

      auto functor_ver = [](
                            rt_prog_ctx_t & ctx,
                            const std::string& name,
                            const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(about::version);
      };

      fmap["ver$"] = functor_ver;


#define NUBASIC_DEF_FTIME(__item)\
                auto functor_get_##__item = [](\
                    rt_prog_ctx_t & ctx,\
                    const std::string& name,\
                    const nu::func_args_t & args)\
                {\
                    check_arg_num(args, 0, name);\
                    return nu::variant_t(integer_t(\
                        nu::_os_get_ ## __item()));\
                };

      NUBASIC_DEF_FTIME(day);
      NUBASIC_DEF_FTIME(month);
      NUBASIC_DEF_FTIME(year);
      NUBASIC_DEF_FTIME(wday);
      NUBASIC_DEF_FTIME(yday);
      NUBASIC_DEF_FTIME(hour);
      NUBASIC_DEF_FTIME(min);
      NUBASIC_DEF_FTIME(sec);
      NUBASIC_DEF_FTIME(time);

      fmap["sysday"] = functor_get_day;
      fmap["sysmonth"] = functor_get_month;
      fmap["sysyear"] = functor_get_year;
      fmap["syswday"] = functor_get_wday;
      fmap["sysyday"] = functor_get_yday;
      fmap["syshour"] = functor_get_hour;
      fmap["sysmin"] = functor_get_min;
      fmap["syssec"] = functor_get_sec;
      fmap["time"] = functor_get_time;

      auto functor_sys_time = [](
                                 rt_prog_ctx_t & ctx,
                                 const std::string& name,
                                 const nu::func_args_t & args)
      {
         check_arg_num(args, 0, name);
         return nu::variant_t(_os_get_systime());
      };

      fmap["systime$"] = functor_sys_time;


      auto functor_sizeof_bv = [](
                                  rt_prog_ctx_t & ctx,
                                  const std::string& name,
                                  const nu::func_args_t & args)
      {
         std::vector< variant_t > vargs;
         get_functor_vargs(ctx, name, args, { variant_t::type_t::UNDEFINED }, vargs);

         return nu::variant_t(integer_t(vargs[0].vector_size()));
      };


      fmap["sizeof@"] = functor_sizeof_bv;


      auto functor_sizeof = [](
                               rt_prog_ctx_t & ctx,
                               const std::string& name,
                               const nu::func_args_t & args)
      {
         std::vector< variant_t > vargs;
         get_functor_vargs(ctx, name, args, { variant_t::type_t::UNDEFINED }, vargs);

         return nu::variant_t(integer_t( vargs[0].vector_size() ));
      };


      fmap["sizeof"] = functor_sizeof;


#ifndef TINY_NUBASIC_VER
      struct _msg_box
      {
         int operator()(
            const std::string& title,
            const std::string& message) NU_NOEXCEPT
         {
            return _os_msg_box(title, message, 0);
         }
      };


      fmap["msgbox"] = functor_int_string_string<_msg_box>;


      struct _play_sound
      {
         int operator()(
            const std::string& filename,
            int flg) NU_NOEXCEPT
         {
            return _os_play_sound(filename, flg);
         }
      };


      fmap["playsound"] = functor_int_string_int<_play_sound>;


      struct _move_window
      {
         int operator()(int x, int y, int dx, int dy) NU_NOEXCEPT
         {
            return _os_move_window(x,y,dx,dy);
         }
      };


      fmap["movewindow"] = functor_int_int_int_int_int<_move_window>;
#endif


      struct _rgb
      {
         int operator()(int r, int g, int b) NU_NOEXCEPT
         {
            return 
               ((int)(((uint8_t)(r) | 
               ((uint16_t)((uint8_t)(g)) << 8)) | 
               (((uint32_t)(uint8_t)(b)) << 16)));
         }
      };


      fmap["rgb"] = functor_int_int_int_int<_rgb>;


   }

   return *_instance;
}


global_operator_tbl_t& global_operator_tbl_t::get_instance()
{
   if (!_instance)
   {
      _instance = new global_operator_tbl_t();
      assert(_instance);

      global_operator_tbl_t & opmap = *_instance;
      using arg_t = const variant_t&;

      opmap["mod"] = [](arg_t a, arg_t b)
      {
         return a.int_mod(b);
      };
      opmap["div"] = [](arg_t a, arg_t b)
      {
         return a.int_div(b);
      };
      opmap["<="] = [](arg_t a, arg_t b)
      {
         return a <= b;
      };
      opmap[">="] = [](arg_t a, arg_t b)
      {
         return a >= b;
      };
      opmap["="] = [](arg_t a, arg_t b)
      {
         return a == b;
      };
      opmap["<>"] = [](arg_t a, arg_t b)
      {
         return a != b;
      };
      opmap["<"] = [](arg_t a, arg_t b)
      {
         return a < b;
      };
      opmap[">"] = [](arg_t a, arg_t b)
      {
         return a > b;
      };
      opmap["+"] = [](arg_t a, arg_t b)
      {
         return a + b;
      };
      opmap["-"] = [](arg_t a, arg_t b)
      {
         return a - b;
      };
      opmap["/"] = [](arg_t a, arg_t b)
      {
         return a / b;
      };
      opmap["*"] = [](arg_t a, arg_t b)
      {
         return a * b;
      };
      opmap["^"] = [](arg_t a, arg_t b)
      {
         return a.power(b);
      };
      opmap["\\"] = [](arg_t a, arg_t b)
      {
         return a.int_div(b);
      };
      opmap["and"] = [](arg_t a, arg_t b)
      {
         return a && b;
      };
      opmap["or"] = [](arg_t a, arg_t b)
      {
         return a || b;
      };
      opmap["xor"] = [](arg_t a, arg_t b)
      {
         return a != b;
      };

      opmap["bor"]  = [](arg_t a, arg_t b)
      {
         return a.to_int() | b.to_int();
      };
      opmap["band"] = [](arg_t a, arg_t b)
      {
         return a.to_int() & b.to_int();
      };
      opmap["bxor"] = [](arg_t a, arg_t b)
      {
         return a.to_int() ^ b.to_int();
      };
      opmap["bshr"] = [](arg_t a, arg_t b)
      {
         return a.to_int() >> b.to_int();
      };
      opmap["bshl"] = [](arg_t a, arg_t b)
      {
         return a.to_int() << b.to_int();
      };

   }

   return *_instance;
}


/* -------------------------------------------------------------------------- */

} // nu

