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

#ifndef __NU_STMT_FOR_TO_STEP_H__
#define __NU_STMT_FOR_TO_STEP_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include "nu_variable.h"
#include "nu_var_scope.h"
#include "nu_token_list.h"
#include "nu_expr_any.h"
#include "nu_expr_literal.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class stmt_for_to_step_t : public stmt_t
{
public:
   using arg_t = expr_any_t::handle_t;

   stmt_for_to_step_t() = delete;
   stmt_for_to_step_t(const stmt_for_to_step_t&) = delete;
   stmt_for_to_step_t& operator=(const stmt_for_to_step_t&) = delete;

   stmt_for_to_step_t(
      prog_ctx_t & ctx,
      const std::string& variable,
      arg_t from_arg,
      arg_t to_arg,
      expr_any_t::handle_t step)
      :
      stmt_t(ctx),
      _variable(variable),
      _from_arg(from_arg),
      _to_arg(to_arg),
      _step(step)
   {
      assert(step);

      ctx.for_loop_metadata.compile_begin(ctx.compiletime_pc);
   }

   virtual void run(rt_prog_ctx_t& ctx) override;
   virtual stmt_cl_t get_cl() const NU_NOEXCEPT override;

protected:
   std::string _variable;
   arg_t _from_arg, _to_arg;
   expr_any_t::handle_t _step = 0;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_FOR_TO_STEP_H__