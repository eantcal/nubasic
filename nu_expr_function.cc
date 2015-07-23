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

#include "nu_expr_function.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

expr_function_t::expr_function_t(const std::string& name, func_args_t var) :
   _name(name),
   _var(var)
{}


/* -------------------------------------------------------------------------- */

bool expr_function_t::empty() const throw()
{
   return false;
}


/* -------------------------------------------------------------------------- */

variant_t expr_function_t::eval(rt_prog_ctx_t & ctx) const
{

   if (!global_function_tbl_t::get_instance().is_defined(_name))
   {
      var_scope_t::handle_t scope =
         ctx.proc_scope.get(ctx.proc_scope.get_type(_name));

      // If it is not a function, could be a vector...
      if (scope->is_defined(_name))
      {
         const variant_t& var_value = (*scope)[_name];

         if (var_value.is_vector())
            return var_value[_var[0]->eval(ctx).to_int()];
      }

      throw exception_t(
         std::string("Error: \"" + _name + "\" undefined symbol"));
   }

   return global_function_tbl_t::get_instance()[_name](ctx, _name, _var);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
