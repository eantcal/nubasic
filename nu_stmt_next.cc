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
#include "nu_stmt_next.h"
#include "nu_error_codes.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

stmt_next_t::stmt_cl_t stmt_next_t::get_cl() const NU_NOEXCEPT
{
   return stmt_cl_t::FOR_END;
}


/* -------------------------------------------------------------------------- */

void stmt_next_t::run(rt_prog_ctx_t & ctx)
{
   if (ctx.for_loop_tbl.empty())
      rt_error_code_t::get_instance().throw_if(
         true,
         ctx.runtime_pc.get_line(),
         rt_error_code_t::E_NEXT_WITHOUT_FOR, "");

   //If counter was not specified... (NEXT <without-counter>)
   std::string counter_name = _variable;

   if (_variable.empty())
   {
      if (ctx.for_loop_tbl.size() > 1)
         rt_error_code_t::get_instance().throw_if(
            true,
            ctx.runtime_pc.get_line(),
            rt_error_code_t::E_IMPL_CNT_NOT_ALLOWED,
            "Next");

      counter_name = ctx.for_loop_tbl.begin()->first;

      if (counter_name.empty())
         rt_error_code_t::get_instance().throw_if(
            true,
            ctx.runtime_pc.get_line(),
            rt_error_code_t::E_IMPL_CNT_NOT_ALLOWED,
            "Next");

      //Extranct variable name from qualified counter name
      auto pos = counter_name.find("::");

      _variable = pos >= 0 && counter_name.size() > 2 ?
                  counter_name.substr(pos, counter_name.size() - 2) :
                  _variable = counter_name;
   }

   auto scope_id = ctx.proc_scope.get_scope_id();
   auto scope_type = ctx.proc_scope.get_type(_variable);

   if (scope_type != proc_scope_t::type_t::GLOBAL && !scope_id.empty())
      counter_name = scope_id + "::" + _variable;

   var_scope_t::handle_t scope = ctx.proc_scope.get(scope_type);

   auto & forctx = ctx.for_loop_tbl[counter_name];
   auto & counter = (*scope)[_variable].first;

   forctx.pc_next_stmt = ctx.runtime_pc;
   counter += forctx.step;

   bool condition =
      bool(forctx.step.to_double() > 0 ?
           counter <= forctx.end_counter :
           counter >= forctx.end_counter);

   auto handle = ctx.for_loop_metadata.end_find(ctx.runtime_pc);

   if (!handle)
      rt_error_code_t::get_instance().throw_if(
         true,
         ctx.runtime_pc.get_line(),
         rt_error_code_t::E_NEXT_WITHOUT_FOR,
         "");

   bool exit_for_loop = handle->flag[instrblock_t::EXIT];

   if (exit_for_loop)
      handle->flag.set(instrblock_t::EXIT, false);

   //Loop condition check
   if (!exit_for_loop && condition)
   {
      //Modify counter and go to FOR-TO-STEP-line
      ctx.go_to( forctx.pc_for_stmt );
   }
   else
   {
      exit_for_loop = true;
   }

   if (exit_for_loop)
   {
      //LOOP completed, remove ctx and go to next line
      ctx.for_loop_tbl.erase(counter_name);
      ctx.go_to_next();
   }

}


/* -------------------------------------------------------------------------- */

} // namespace nu

