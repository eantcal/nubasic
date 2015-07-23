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

#include "nu_rt_prog_ctx.h"
#include "nu_stmt_dim.h"
#include "nu_error_codes.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

void stmt_dim_t::run(rt_prog_ctx_t & ctx)
{
   auto rt_error_if = [&](
                         bool cond,
                         rt_error_code_t::value_t err,
                         const std::string& desc)
   {
      rt_error_code_t::get_instance().throw_if(
         cond,
         ctx.runtime_pc.get_line(),
         err,
         desc);
   };

   for (const auto & v : _vars)
   {
      int vsize = v.second.second->eval(ctx).to_int();
      const std::string& name = v.first;

      auto scope_type = ctx.proc_scope.get_type(name);

      switch (scope_type)
      {
         case proc_scope_t::type_t::GLOBAL:
            rt_error_if(ctx.proc_scope.get_scope_id().empty(),
                        rt_error_code_t::E_VAR_REDEF,
                        "Dim: '" + name + "'");
            break;

         case proc_scope_t::type_t::LOCAL:
            rt_error_if(true,
                        rt_error_code_t::E_VAR_REDEF,
                        "Dim: '" + name + "'");
            break;

         case proc_scope_t::type_t::UNDEF:
         default:
            break;
      }

      rt_error_if(vsize<=0,
                  rt_error_code_t::E_INV_VECT_SIZE,
                  "Dim: '" + name + "'");

      var_scope_t::handle_t scope =
         ctx.proc_scope.get(ctx.proc_scope.get_type(name));

      auto vtype = v.second.first;
      std::string init_val = "0";

      switch (vtype)
      {
         case variable_t::type_t::STRING:
            init_val = "";

         case variable_t::type_t::FLOAT:
         case variable_t::type_t::INTEGER:
         case variable_t::type_t::LONG64:
         case variable_t::type_t::BOOLEAN:
         case variable_t::type_t::BYTEVECTOR:
            scope->define(name, variant_t(init_val, vtype, vsize));
            break;

         default:
            rt_error_if(true,
                        rt_error_code_t::E_INV_VECT_SIZE,
                        "Dim: '" + v.first + "'");
      }
   }

   ctx.go_to_next();

}


/* -------------------------------------------------------------------------- */

} // namespace nu
