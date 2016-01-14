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
*  Author: Antonino Calderone <acaldmail@gmail.com>
*
*/


/* -------------------------------------------------------------------------- */

#include "nu_expr_var.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


   /* -------------------------------------------------------------------------- */

   variant_t expr_var_t::eval(rt_prog_ctx_t & ctx) const
   {
      var_scope_t::handle_t scope;

      if (scope == nullptr)
         scope = ctx.proc_scope.get(
            ctx.proc_scope.get_type(_name));

      if (scope->is_defined(_name))
      {
         if (scope == nullptr)
            scope = ctx.proc_scope.get(
               ctx.proc_scope.get_type(_name));

         return (*scope)[_name].first;
      }

      auto vtype = variable_t::type_by_name(_name);
      variant_t value("", vtype, 0);
      scope->define(_name, var_value_t(value, VAR_ACCESS_RW));

      return (*scope)[_name].first;
   }

   /* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

