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

#ifndef __NU_STMT_ON_GOTO_H__
#define __NU_STMT_ON_GOTO_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include "nu_stmt_empty.h"
#include "nu_expr_any.h"

#include <string>
#include <memory>
#include <vector>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class stmt_on_goto_t : public stmt_t
{
public:
   stmt_on_goto_t() = delete;
   stmt_on_goto_t(const stmt_on_goto_t&) = delete;
   stmt_on_goto_t& operator=(const stmt_on_goto_t&) = delete;

   using label_list_t = std::vector < std::string > ;

   stmt_on_goto_t(
      prog_ctx_t & ctx,
      expr_any_t::handle_t condition,
      const label_list_t & label_list
   )
      :
      stmt_t(ctx),
      _condition(condition),
      _label_list(label_list)
   {}

   virtual void run(rt_prog_ctx_t& ctx) override;

protected:
   expr_any_t::handle_t _condition;
   label_list_t _label_list;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_ON_GOTO_H__
