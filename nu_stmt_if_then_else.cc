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
#include "nu_stmt_if_then_else.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

stmt_if_then_else_t::stmt_if_then_else_t(
   prog_ctx_t & ctx,
   expr_any_t::handle_t condition,
   handle_t then_stmt,
   handle_t else_stmt)
   :
   stmt_t(ctx),
   _condition(condition),
   _then_stmt(then_stmt),
   _else_stmt(else_stmt)
{
   if (!_else_stmt)
      _else_stmt = std::make_shared<stmt_empty_t>(ctx);

   if (then_stmt->get_cl() == stmt_cl_t::EMPTY)
   {
      auto & ifctxs = ctx.if_metadata;
      ifctxs.data[ctx.compiletime_pc].pc_if_stmt = ctx.compiletime_pc;
      ifctxs.pc_stack.push(ctx.compiletime_pc);
   }

   auto cl = then_stmt->get_cl();

   syntax_error_if(
      cl == stmt_cl_t::DO_BEGIN ||
      cl == stmt_cl_t::DO_BEGIN ||
      cl == stmt_cl_t::DO_END ||
      cl == stmt_cl_t::DO_END ||
      cl == stmt_cl_t::WHILE_BEGIN ||
      cl == stmt_cl_t::WHILE_BEGIN ||
      cl == stmt_cl_t::WHILE_END ||
      cl == stmt_cl_t::WHILE_END ||
      cl == stmt_cl_t::SUB_BEGIN ||
      cl == stmt_cl_t::SUB_BEGIN ||
      cl == stmt_cl_t::SUB_END ||
      cl == stmt_cl_t::SUB_END,
      "This construct is not allowed");
}


/* -------------------------------------------------------------------------- */

stmt_if_then_else_t::stmt_cl_t stmt_if_then_else_t::get_cl() const throw()
{
   return stmt_cl_t::IF_BLOCK_BEGIN;
}


/* -------------------------------------------------------------------------- */

void stmt_if_then_else_t::run(rt_prog_ctx_t& ctx)
{
   ctx.go_to_next();

   auto & ifctxs = ctx.if_metadata;
   const auto & metadata_it = ifctxs.data.find(ctx.runtime_pc);

   if (_then_stmt->get_cl() == stmt_cl_t::EMPTY &&
      metadata_it != ifctxs.data.end() )
   {
      if (static_cast<bool>(_condition->eval(ctx)) == false)
      {
         auto & metadata = metadata_it->second;
         metadata.condition = false;
         
         ctx.go_to(metadata.else_list.empty()  
            ? metadata.pc_endif_stmt : 
            *metadata.else_list.begin());
      }
      else
      {
         metadata_it->second.condition = true;

         ctx.go_to_next();
      }
   }
   else
   {
      if (static_cast<bool>(_condition->eval(ctx)) == true)
         _then_stmt->run(ctx);
      else
         _else_stmt->run(ctx);
   }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
