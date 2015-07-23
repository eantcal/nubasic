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

#ifndef __NU_STMT_GENINSTR_H__
#define __NU_STMT_GENINSTR_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include "nu_expr_any.h"
#include "nu_exception.h"

#include <string>
#include <list>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class prog_ctx_t;
class rt_prog_ctx_t;


/* -------------------------------------------------------------------------- */

//
// int f(T args[ARGSNUM])
//
template< typename F, typename T, int ARGSNUM >
class stmt_geninstr_t : public stmt_t
{
public:
   using data_t = T;

   stmt_geninstr_t(prog_ctx_t & ctx)
      :
      stmt_t(ctx)
   { }

   stmt_geninstr_t(arg_list_t args, prog_ctx_t & ctx) :
      stmt_t(ctx),
      _args(args)
   { }


protected:
   template <class DT>
   void run_aux(rt_prog_ctx_t & ctx, std::true_type)
   {
      std::vector<data_t> f_args;

      for (auto arg : _args)
      {
         data_t val = data_t(arg.first->eval(ctx));
         f_args.push_back(val);
      }

      ctx.set_errno( F()(ctx, f_args) );
   }


   template <class DT>
   void run_aux(rt_prog_ctx_t & ctx, std::false_type)
   {
      std::vector<data_t> f_args;

      for (auto arg : _args)
      {
         f_args.push_back(arg.first->eval(ctx));
      }

      ctx.set_errno(F()(ctx, f_args));
   }


public:
   void run(rt_prog_ctx_t & ctx)
   {
      //if number of arguments is negative
      //do not check it
      if (ARGSNUM >= 0)
      {
         syntax_error_if(
            _args.size() < ARGSNUM,
            "Instruction expects to be passed " +
            nu::to_string(ARGSNUM) + " arguments");
      }

      run_aux<T>(ctx, std::is_integral<T>());

      ctx.go_to_next();
   }

protected:
   arg_list_t _args;
   stmt_geninstr_t(const stmt_geninstr_t&) = delete;
   stmt_geninstr_t& operator=(const stmt_geninstr_t&) = delete;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_GENINSTR_H__