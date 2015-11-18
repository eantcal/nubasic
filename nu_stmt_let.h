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

#ifndef __NU_STMT_LET_H__
#define __NU_STMT_LET_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include "nu_variable.h"
#include "nu_var_scope.h"
#include "nu_token_list.h"
#include "nu_expr_any.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class stmt_let_t : public stmt_t
{
public:
   using arg_t = expr_any_t::handle_t;

   stmt_let_t() = delete;
   stmt_let_t(const stmt_let_t&) = delete;
   stmt_let_t& operator=(const stmt_let_t&) = delete;

   stmt_let_t(
      prog_ctx_t & ctx,
      const std::string& variable,
      arg_t arg,
      expr_any_t::handle_t vect_idx,
      expr_any_t::handle_t element_vect_idx,
      bool struct_member,
      bool constant)
      :
      stmt_t(ctx),
      _variable(variable),
      _arg(arg),
      _vect_idx(vect_idx),
      _element_vect_idx(element_vect_idx),
      _struct_member(struct_member),
      _constant(constant)
   {}

   virtual void run(rt_prog_ctx_t& ctx) override;

protected:
   template<typename ...E>
   void _assign(
      rt_prog_ctx_t & ctx,
      variant_t & var,
      const variant_t & val,
      variable_t::type_t vart,
      E... idx)
   {
      switch ( vart )
      {
         case variable_t::type_t::STRING:
            if ( val.get_type() != variable_t::type_t::STRING )
               rt_error_code_t::get_instance().throw_if(
               true,
               ctx.runtime_pc.get_line(),
               rt_error_code_t::E_TYPE_MISMATCH,
               "'" + _variable + "'");

            var.set_str(val.to_str(), idx...);
            break;

         case variable_t::type_t::FLOAT:
            var.set_real(val.to_real(), idx...);
            break;

         case variable_t::type_t::DOUBLE:
            var.set_double(val.to_double(), idx...);
            break;

         case variable_t::type_t::BYTEVECTOR:
            var.set_bvect(val.to_int(), idx...);
            break;

         case variable_t::type_t::LONG64:
            var.set_long64(val.to_long64(), idx...);
            break;

         case variable_t::type_t::BOOLEAN:
            var.set_bool(val.to_bool(), idx...);
            break;

         case variable_t::type_t::INTEGER:
         default:
            var.set_int(val.to_int(), idx...);
            break;
      }
   }

   std::string _variable;
   arg_t _arg;
   expr_any_t::handle_t _vect_idx;
   expr_any_t::handle_t _element_vect_idx;
   bool _struct_member = false;
   bool _constant = false;
};


/* -------------------------------------------------------------------------- */

}

/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_LET_H__