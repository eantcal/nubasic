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

#ifndef __NU_GLOBAL_FUNCTION_TBL_H__
#define __NU_GLOBAL_FUNCTION_TBL_H__


/* -------------------------------------------------------------------------- */

#include "nu_variant.h"
#include "nu_exception.h"
#include "nu_expr_any.h"
#include "nu_symbol_map.h"
#include "nu_icstring.h"
#include "nu_rt_prog_ctx.h"

#include <functional>
#include <vector>
#include <algorithm>
#include <type_traits>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

using func_bin_t = std::function < variant_t(const variant_t&, const variant_t&) > ;

using func_t = std::function < variant_t(
                  rt_prog_ctx_t &,
                  const std::string&,
                  const func_args_t &) > ;

using binop_t = std::function < variant_t(const variant_t&, const variant_t&) > ;


/* -------------------------------------------------------------------------- */

class global_function_tbl_t : public symbol_map_t < icstring_t, func_t >
{
private:
   global_function_tbl_t() = default;
   global_function_tbl_t(const global_function_tbl_t&) = delete;
   global_function_tbl_t& operator=( const global_function_tbl_t& ) = delete;

   static global_function_tbl_t* _instance;

protected:
   virtual void get_err_msg(const std::string& key, std::string& err) const
   {
      err = "'" + key + "()' not defined";
   }

public:
   static global_function_tbl_t& get_instance();
};


/* -------------------------------------------------------------------------- */

class global_operator_tbl_t : public symbol_map_t < icstring_t, binop_t >
{
private:
   global_operator_tbl_t() = default;
   global_operator_tbl_t(const global_operator_tbl_t&) = delete;
   global_operator_tbl_t& operator=( const global_operator_tbl_t& ) = delete;

   static global_operator_tbl_t* _instance;

protected:
   virtual void get_err_msg(const std::string& key, std::string& err) const
   {
      err = "'" + key + "()' not defined";
   }

public:
   static global_operator_tbl_t& get_instance();
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_BUILTIN_FUNC__
