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

#ifndef __NU_EXPR_FUNCTION_H__
#define __NU_EXPR_FUNCTION_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_var_scope.h"
#include "nu_global_function_tbl.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

//! This class represents a function-call expression
class expr_function_t : public expr_any_t
{
public:
   //! ctor
   expr_function_t(const std::string& name, func_args_t var);

   expr_function_t() = delete;
   expr_function_t(const expr_function_t&) = default;
   expr_function_t& operator=( const expr_function_t& ) = default;

   //! Evaluates the function (using name and arguments given to the ctor)
   variant_t eval(rt_prog_ctx_t & ctx) const override;

   //! Returns false for this type of object
   bool empty() const throw( ) override;

protected:
   std::string _name;
   func_args_t _var;
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_FUNCTION_H__
