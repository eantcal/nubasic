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
#include "nu_stmt_while.h"
#include "nu_stmt_empty.h"
#include "nu_error_codes.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

void stmt_while_t::build_ctx(prog_ctx_t & ctx)
{
   syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
                   "While must be first line statement");

   ctx.while_metadata.compile_begin(ctx.compiletime_pc);
}


/* -------------------------------------------------------------------------- */

stmt_while_t::stmt_while_t(
   prog_ctx_t & ctx,
   expr_any_t::handle_t condition)
   :
   stmt_t(ctx),
   _condition(condition)
{
   build_ctx(ctx);
}


/* -------------------------------------------------------------------------- */

// while <condition> do <stmt>
stmt_while_t::stmt_while_t(
   prog_ctx_t & ctx,
   expr_any_t::handle_t condition,
   stmt_t::handle_t while_stmt
)
   :
   stmt_t(ctx),
   _condition(condition),
   _while_stmt(while_stmt),
   _single_stmt(while_stmt != nullptr)
{
   if (_single_stmt)
      signal_mgr_t::instance().register_handler(event_t::BREAK, this);
   else
      build_ctx(ctx);
}


/* -------------------------------------------------------------------------- */

stmt_while_t::~stmt_while_t()
{
   if (_single_stmt)
      signal_mgr_t::instance().unregister_handler(event_t::BREAK, this);
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_while_t::get_cl() const NU_NOEXCEPT
{
   return stmt_cl_t::WHILE_BEGIN;
}


/* -------------------------------------------------------------------------- */

void stmt_while_t::run(rt_prog_ctx_t& ctx)
{

   //  While <condition> [Do]
   //   ...
   //  End While | Wend

   if (!_single_stmt)
   {
      auto & wctxs = ctx.while_metadata;
      auto handle = wctxs.begin_find(ctx.runtime_pc);

      rt_error_code_t::get_instance().throw_if(
         !handle,
         ctx.runtime_pc.get_line(),
         rt_error_code_t::E_INTERNAL,
         "While");

      if (!static_cast<bool>(_condition->eval(ctx)))
      {
         handle->flag.set(instrblock_t::EXIT, true);
         ctx.go_to(handle->pc_end_stmt);
      }
      else
      {
         handle->flag.set(instrblock_t::EXIT, false);
         ctx.go_to_next();
      }
   }
   //  while <condition> do <stmt>
   else while (static_cast<bool>(_condition->eval(ctx)) && ! _break_while_loop)
   {
      _while_stmt->run(ctx);
   }

   if (_break_while_loop)
   {
      ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, true);
      _break_while_loop = false;
   }
}


/* -------------------------------------------------------------------------- */

bool stmt_while_t::notify(const event_t& ev)
{
   _break_while_loop = ev == event_t::BREAK;
   return _break_while_loop;
}


/* -------------------------------------------------------------------------- */

} // namespace nu

