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

#ifndef __NU_STMT_ELSE_H__
#define __NU_STMT_ELSE_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include "nu_token_list.h"
#include "nu_expr_any.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */
   
class stmt_elif_t : public stmt_t
{
public:
   stmt_elif_t() = delete;
   stmt_elif_t(const stmt_elif_t&) = delete;
   stmt_elif_t& operator=(const stmt_elif_t&) = delete;

   stmt_elif_t(prog_ctx_t & ctx, expr_any_t::handle_t condition);

   virtual void run(rt_prog_ctx_t& ctx) override;

   //! Identifies the class of the statement
   stmt_cl_t get_cl() const NU_NOEXCEPT override
   {
      return stmt_cl_t::ELSE;
   }

protected:
   expr_any_t::handle_t _condition;
};


/* -------------------------------------------------------------------------- */

class stmt_else_t : public stmt_elif_t
{
public:
   stmt_else_t() = delete;
   stmt_else_t(const stmt_else_t&) = delete;
   stmt_else_t& operator=(const stmt_else_t&) = delete;

   stmt_else_t(prog_ctx_t& ctx) :
      stmt_elif_t(ctx, nullptr) {}
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_ELSE_H__