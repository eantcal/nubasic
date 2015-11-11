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

#include "nu_expr_struct.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

bool expr_struct_t::empty() const NU_NOEXCEPT
{
   return false;
}


/* -------------------------------------------------------------------------- */

variant_t expr_struct_t::eval(rt_prog_ctx_t & ctx) const
{
   const auto & var_name = _member_call_list[0].first;
   const auto & index = _member_call_list[0].second;

   var_scope_t::handle_t scope =
      ctx.proc_scope.get(ctx.proc_scope.get_type(var_name));

   variant_t * value = nullptr;

   if (scope->is_defined(var_name))
   {
      const variant_t& var_value = (*scope)[var_name];
      value = &(var_value[index->eval(ctx).to_int()]);
   }

   if (!value)
      throw exception_t(
         std::string("Error: \"" + var_name + "\" undefined variable"));

   if (!value->is_struct())
      throw exception_t(
         std::string("Error: \"" + var_name + "\" is not a struct instance"));
   
   size_t inner_struct_level = 1;
   
   do {
      if (_member_call_list.size()>= inner_struct_level)
         throw exception_t(
            std::string("Error in struct reference of \"" + var_name + "\""));

      const auto & struct_type_name = value->struct_type_name();

      assert(!struct_type_name.empty());

      auto it = ctx.struct_prototypes.data.find(struct_type_name);

      if (it == ctx.struct_prototypes.data.end())
         throw exception_t(
            std::string(
               "Error: \"" + var_name + ":" + 
               struct_type_name + "\" not defined"));

      const auto & struct_prototype = it->second.second;
      const auto & member = _member_call_list[inner_struct_level];
      const auto & member_name = member.first;
      auto member_index = member.second->eval(ctx);

      auto value_handle = 
         value->struct_member(member_name/*, member_index.to_int()*/);

      if (!value_handle) throw exception_t(
         std::string(
            "Error: '" +
            var_name + "' is not element of struct '" +
            struct_type_name + "'"));

      if (!value_handle->is_struct())
         return *value_handle;

      value = value_handle.get();

      ++inner_struct_level;

   } while (true);


   return *value;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
