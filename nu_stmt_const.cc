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

#include "nu_rt_prog_ctx.h"
#include "nu_stmt_const.h"
#include "nu_error_codes.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

void stmt_const_t::run(rt_prog_ctx_t & ctx)
{
   variant_t value = _arg->eval(ctx);
   const std::string& name = _var;

   auto scope_type = ctx.proc_scope.get_type(name);

   switch (scope_type)
   {
      case proc_scope_t::type_t::LOCAL:
      case proc_scope_t::type_t::GLOBAL:
         rt_error_code_t::get_instance().throw_if(
            true,
            ctx.runtime_pc.get_line(),
            rt_error_code_t::E_VAR_REDEF,
            "Variable '" + name + "'");
         break;

      case proc_scope_t::type_t::UNDEF:
      default:
         break;
   }

   var_scope_t::handle_t scope = ctx.proc_scope.get_global();

   auto vtype_code = variable_t::type_by_typename(_vtype);
   auto value_type = value.get_type();

   std::string init_val;

   switch (vtype_code)
   {
      case variable_t::type_t::STRING:
         rt_error_code_t::get_instance().throw_if(
            value_type != variable_t::type_t::STRING,
            ctx.runtime_pc.get_line(),
            rt_error_code_t::E_TYPE_ILLEGAL,
            "'" + name + "'");
         init_val = value.to_str();

      case variable_t::type_t::FLOAT:
      case variable_t::type_t::DOUBLE:
      case variable_t::type_t::INTEGER:
      case variable_t::type_t::LONG64:
      case variable_t::type_t::BOOLEAN:
         init_val = value.to_str();
         scope->define(
            name, 
            var_value_t(
               variant_t(init_val, vtype_code, 0),
               VAR_ACCESS_RO));
         break;
         
      case variable_t::type_t::STRUCT:
      case variable_t::type_t::BYTEVECTOR:
      case variable_t::type_t::UNDEFINED:
      default:
         rt_error_code_t::get_instance().throw_if(
            true,
            ctx.runtime_pc.get_line(),
            rt_error_code_t::E_TYPE_ILLEGAL,
            "'" + name + "'");
   }
   

   ctx.go_to_next();

}


/* -------------------------------------------------------------------------- */

} // namespace nu
