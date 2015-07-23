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
#include "nu_stmt_endif.h"
#include "nu_error_codes.h"



/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

stmt_endif_t::stmt_cl_t stmt_endif_t::get_cl() const throw()
{
   return stmt_cl_t::IF_BLOCK_END;
}


/* -------------------------------------------------------------------------- */

stmt_endif_t::stmt_endif_t(prog_ctx_t& ctx)
   : stmt_t(ctx)
{
   auto & ifctxs = ctx.if_metadata;
   
   syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
                   "'End If' statement must be a first statement of the line");

   syntax_error_if(ifctxs.pc_stack.empty(),
                   "'End If': no any if-statement matching");

   auto if_pc = ifctxs.pc_stack.top();
   ifctxs.data[if_pc].pc_endif_stmt = ctx.compiletime_pc;
   ifctxs.block_to_if_line_tbl[ctx.compiletime_pc] = if_pc;
   ifctxs.pc_stack.pop();
}


/* -------------------------------------------------------------------------- */

void stmt_endif_t::run(rt_prog_ctx_t & ctx)
{
   auto & ifctxs = ctx.if_metadata;
   auto ifstmt_pc = ifctxs.block_to_if_line_tbl[ctx.runtime_pc];
   const auto & metadata_it = ifctxs.data.find(ifstmt_pc);

   rt_error_code_t::get_instance().throw_if(
      metadata_it == ifctxs.data.end(),
      ctx.runtime_pc.get_line(),
      rt_error_code_t::E_INTERNAL,
      "If");

   metadata_it->second.condition = false;

   ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu

