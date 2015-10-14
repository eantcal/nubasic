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

#include "nu_stmt.h"
#include "nu_rt_prog_ctx.h"
#include "nu_expr_any.h"

#include <memory>
#include <list>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

stmt_t::stmt_t(prog_ctx_t & ctx) NU_NOEXCEPT
{
   _stmt_id = ctx.make_next_stmt_id();
}


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_t::get_cl() const NU_NOEXCEPT
{
   return stmt_cl_t::NA;
}


/* -------------------------------------------------------------------------- */

stmt_t::~stmt_t()
{}


/* -------------------------------------------------------------------------- */

}


