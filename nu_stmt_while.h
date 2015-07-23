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

#ifndef __NU_STMT_WHILE_H__
#define __NU_STMT_WHILE_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include "nu_stmt_empty.h"
#include "nu_expr_any.h"
#include "nu_signal_handling.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class stmt_while_t : public stmt_t, protected signal_handler_t
{
public:
   stmt_while_t() = delete;
   stmt_while_t(const stmt_while_t&) = delete;
   stmt_while_t& operator=(const stmt_while_t&) = delete;

   // while <condition> [do]
   //   [stmt(s)]
   // wend
   stmt_while_t(
      prog_ctx_t & ctx,
      expr_any_t::handle_t condition);

   // while <condition> do <stmt>
   stmt_while_t(
      prog_ctx_t & ctx,
      expr_any_t::handle_t condition,
      stmt_t::handle_t while_stmt);

   ~stmt_while_t();

   virtual stmt_cl_t get_cl() const throw() override;
   virtual void run(rt_prog_ctx_t& ctx) override;

protected:
   expr_any_t::handle_t _condition;
   stmt_t::handle_t _while_stmt;
   bool _single_stmt = false;
   bool _break_while_loop = false;

   static void build_ctx(prog_ctx_t & ctx);

   virtual bool notify(const event_t& ev) override;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_WHILE_H__
