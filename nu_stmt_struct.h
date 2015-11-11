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

#ifndef __NU_STMT_STRUCT_H__
#define __NU_STMT_STRUCT_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include "nu_variable.h"
#include "nu_var_scope.h"

#include <string>
#include <algorithm>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class stmt_struct_t : public stmt_t
{
public:
   stmt_struct_t() = delete;
   stmt_struct_t(const stmt_struct_t&) = delete;
   stmt_struct_t& operator=(const stmt_struct_t&) = delete;

   stmt_struct_t(prog_ctx_t & ctx, const std::string& id);

   virtual stmt_cl_t get_cl() const NU_NOEXCEPT override;
   virtual void run(rt_prog_ctx_t& ctx) override;

protected:
   std::string _id;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_STRUCT_H__