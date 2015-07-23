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
#include "nu_stmt_wend.h"
#include "nu_error_codes.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

stmt_wend_t::stmt_wend_t(prog_ctx_t & ctx) :
   stmt_t(ctx)
{
   syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
                   "END WHILE or WEND must be first line statement");

   auto handle = ctx.while_metadata.end_find(ctx.compiletime_pc);

   if (!handle)
      handle = ctx.while_metadata.compile_end(ctx.compiletime_pc);

   if (handle)
      handle->pc_end_stmt = ctx.compiletime_pc;
}


/* -------------------------------------------------------------------------- */

void stmt_wend_t::run(rt_prog_ctx_t & ctx)
{
   auto handle = ctx.while_metadata.end_find(ctx.runtime_pc);

   rt_error_code_t::get_instance().throw_if(
      !handle,
      ctx.runtime_pc.get_line(),
      rt_error_code_t::E_NO_MATCH_WHILE,
      "Wend / While End");

   //Loop condition check
   if (!handle->flag[instrblock_t::EXIT])
   {
      //jump to while statement
      ctx.go_to(handle->pc_begin_stmt);
   }

   else
   {
      //LOOP completed, go to next line
      ctx.go_to_next();
   }
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_wend_t::get_cl() const throw()
{
   return stmt_cl_t::WHILE_END;
}


/* -------------------------------------------------------------------------- */

} // namespace nu

