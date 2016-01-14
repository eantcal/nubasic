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
#include "nu_stmt_else.h"
#include "nu_error_codes.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

stmt_elif_t::stmt_elif_t(
   prog_ctx_t & ctx,
   expr_any_t::handle_t condition)
   :
   stmt_t(ctx),
   _condition(condition)
{
   syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
                   "'Elif' must be first statement of the code line");

   auto & ifctxs = ctx.if_metadata;
   
   syntax_error_if(ifctxs.pc_stack.empty(),
                   "'Elif' no any if-statement matching");

   auto if_pc = ifctxs.pc_stack.top();

   ifctxs.data[if_pc].else_list.push_back(ctx.compiletime_pc);
   ifctxs.block_to_if_line_tbl[ctx.compiletime_pc] = if_pc;
}


/* -------------------------------------------------------------------------- */

void stmt_elif_t::run(rt_prog_ctx_t& ctx)
{
   auto & ifctxs = ctx.if_metadata;
   auto ifstmt_pc = ifctxs.block_to_if_line_tbl[ctx.runtime_pc];
   const auto & metadata_it = ifctxs.data.find(ifstmt_pc);

   rt_error_code_t::get_instance().throw_if(
      metadata_it == ifctxs.data.end(),
      ctx.runtime_pc.get_line(),
      rt_error_code_t::E_INTERNAL,
      "Elif");

   if (metadata_it->second.condition)
      ctx.go_to(metadata_it->second.pc_endif_stmt);

   else
   {
      if (_condition && static_cast<bool>(_condition->eval(ctx)) == false)
      {
         metadata_it->second.condition = false;

         if (metadata_it->second.else_list.empty())
         {
            ctx.go_to(metadata_it->second.pc_endif_stmt);
         }
         else
         {
            auto it = metadata_it->second.else_list.cbegin();

            for (; it != metadata_it->second.else_list.cend(); ++it)
            {
               if (it->get_line() == ctx.runtime_pc.get_line())
               {
                  ++it;

                  ctx.go_to(it == metadata_it->second.else_list.cend() ? 
                     metadata_it->second.pc_endif_stmt : 
                     it->get_line());

                  break;
               }
            }

            if (it == metadata_it->second.else_list.cend())
               ctx.go_to(metadata_it->second.pc_endif_stmt);
         }
      }
      else
      {
         metadata_it->second.condition = true;
         ctx.go_to_next();
      }
   }
}


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

