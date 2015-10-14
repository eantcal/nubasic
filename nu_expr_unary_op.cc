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

#include "nu_expr_unary_op.h"
#include "nu_global_function_tbl.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

expr_unary_op_t::expr_unary_op_t(const std::string& op_name, expr_any_t::handle_t var) :
   _op_name(op_name),
   _var(var)
{}


/* -------------------------------------------------------------------------- */

bool expr_unary_op_t::empty() const NU_NOEXCEPT
{
   return false;
}


/* -------------------------------------------------------------------------- */

variant_t expr_unary_op_t::eval(rt_prog_ctx_t & ctx) const
{

   if (!global_function_tbl_t::get_instance().is_defined(_op_name))
   {
      throw exception_t(
         std::string("Error: \"" + _op_name + "\" undefined symbol"));
   }

   func_args_t args;
   args.push_back(_var);

   return global_function_tbl_t::get_instance()[_op_name](ctx, _op_name, args);
}


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */


