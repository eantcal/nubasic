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

#ifndef __NU_STMT_SUB_H__
#define __NU_STMT_SUB_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include "nu_variable.h"
#include "nu_var_scope.h"
#include "nu_token_list.h"

#include <string>
#include <algorithm>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class stmt_sub_t : public stmt_t
{
public:
   stmt_sub_t() = delete;
   stmt_sub_t(const stmt_sub_t&) = delete;
   stmt_sub_t& operator=(const stmt_sub_t&) = delete;

   using vec_size_t = expr_any_t::handle_t;

   stmt_sub_t(prog_ctx_t & ctx, const std::string& id);

   void define(
      const std::string& var,
      const std::string& vtype,
      vec_size_t vect_size,
      prog_ctx_t & ctx,
      const std::string& id);

   virtual stmt_cl_t get_cl() const NU_NOEXCEPT override;
   virtual void run(rt_prog_ctx_t& ctx) override;

protected:
   std::string _id;
   std::set< std::string > _vars_rep_check;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_SUB_H__