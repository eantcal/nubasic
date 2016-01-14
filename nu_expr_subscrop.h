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

#ifndef __NU_EXPR_SUBSCROP_H__
#define __NU_EXPR_SUBSCROP_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_var_scope.h"
#include "nu_global_function_tbl.h"
#include "nu_expr_function.h"


/* -------------------------------------------------------------------------- */

namespace nu
{

/* -------------------------------------------------------------------------- */

class expr_subscrop_t : public expr_function_t
{
public:
   expr_subscrop_t(const std::string& name, func_args_t var)
      : expr_function_t(name, var)
   {}

   expr_subscrop_t() = delete;
   expr_subscrop_t(const expr_subscrop_t&) = default;
   expr_subscrop_t& operator=( const expr_subscrop_t& ) = default;

   variant_t eval(rt_prog_ctx_t & ctx) const override;
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_SUBSCROP_H__
