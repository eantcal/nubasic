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

#ifndef __NU_STMT_RETURN_H__
#define __NU_STMT_RETURN_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class stmt_return_t : public stmt_t
{
public:
   stmt_return_t(prog_ctx_t & ctx)
      : stmt_t(ctx)
   {}


   virtual void run(rt_prog_ctx_t& ctx) override
   {
      ctx.flag.set(rt_prog_ctx_t::FLG_RETURN_REQUEST, true);
   }


protected:
   prog_pointer_t::line_number_t _line_number = 0;

   stmt_return_t(const stmt_return_t&) = delete;
   stmt_return_t& operator=(const stmt_return_t&) = delete;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_RETURN_H__