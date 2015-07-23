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
#include "nu_stmt_do.h"
#include "nu_stmt_empty.h"
#include "nu_error_codes.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

stmt_do_t::stmt_do_t(prog_ctx_t & ctx)
   :
   stmt_t(ctx)
{
   syntax_error_if(ctx.compiletime_pc.get_stmt_pos() > 0,
                   "Do-statement must be first line statement");

   ctx.do_loop_while_metadata.compile_begin(ctx.compiletime_pc);
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_do_t::get_cl() const throw()
{
   return stmt_cl_t::DO_BEGIN;
}


/* -------------------------------------------------------------------------- */

void stmt_do_t::run(rt_prog_ctx_t& ctx)
{
   //  Do
   //   ...
   //  Loop While <cond>
   auto & dowctxs = ctx.do_loop_while_metadata;
   auto handle = dowctxs.begin_find(ctx.runtime_pc);

   if (!handle)
      rt_error_code_t::get_instance().throw_if(
         true,
         ctx.runtime_pc.get_line(),
         rt_error_code_t::E_INTERNAL,
         "Do... Loop While");

   handle->flag.set(instrblock_t::EXIT, false);
   ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu

