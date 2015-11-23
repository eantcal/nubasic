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
#include "nu_stmt_let.h"
#include "nu_error_codes.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

void stmt_let_t::run(rt_prog_ctx_t & ctx)
{
   variant_t val = _arg->eval(ctx);

   size_t idx = 0;

   var_scope_t::handle_t scope;
   variant_t * var = nullptr;

   if (_struct_member)
   {
      if (_vect_idx) 
         idx = _vect_idx->eval(ctx).to_int();

      var = ctx.get_struct_member_value(_variable, scope, idx);

      rt_error_code_t::get_instance().throw_if(
         !var,
         ctx.runtime_pc.get_line(),
         rt_error_code_t::E_TYPE_MISMATCH,
         "'" + _variable + "'");
   }

   if (!var)
   {
      if (scope == nullptr)
         scope = ctx.proc_scope.get(
            ctx.proc_scope.get_type(_variable));

      auto & v = (*scope)[_variable];

      const bool const_var = (v.second & VAR_ACCESS_RO) == VAR_ACCESS_RO;

      if (const_var)
         rt_error_code_t::get_instance().throw_if(
            true,
            ctx.runtime_pc.get_line(),
            rt_error_code_t::E_CANNOT_MOD_CONST,
            "'" + _variable + "'");

      var = &(v.first);
   }

   variable_t::type_t vart = var->get_type();

   if (vart == variable_t::type_t::UNDEFINED)
      vart = variable_t::type_by_name(_variable);

   bool is_vector = var->is_vector();

   if (_element_vect_idx != nullptr)
   {
      is_vector = true;
      _vect_idx = _element_vect_idx;
   }

   if (is_vector)
   {
      if (_vect_idx)
      {
         size_t idx = _vect_idx->eval(ctx).to_int();

         rt_error_code_t::get_instance().throw_if(
            idx >= var->vector_size(),
            ctx.runtime_pc.get_line(),
            rt_error_code_t::E_VEC_IDX_OUT_OF_RANGE,
            "'" + _variable + "(" + nu::to_string(idx) + ")'");

         _assign<size_t>(ctx, *var, val, vart, idx);
      }
      else
      {
         // expression can be only another vector with same size

         rt_error_code_t::get_instance().throw_if(
            var->vector_size() != val.vector_size() ||
            var->get_type() != val.get_type(),
            ctx.runtime_pc.get_line(),
            rt_error_code_t::E_TYPE_MISMATCH,
            "'" + _variable + "'");

         scope->define(_variable, var_value_t( val, VAR_ACCESS_RW ));
      }

   }
   else
   {
      _assign<>(ctx, *var, val, vart);
   }

   ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
