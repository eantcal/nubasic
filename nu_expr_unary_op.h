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

#ifndef __NU_EXPR_UNARY_OP_H__
#define __NU_EXPR_UNARY_OP_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_var_scope.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class expr_unary_op_t : public expr_any_t
{
public:
   expr_unary_op_t(const std::string& op_name, expr_any_t::handle_t var);

   expr_unary_op_t() = delete;
   expr_unary_op_t(const expr_unary_op_t&) = default;
   expr_unary_op_t& operator=( const expr_unary_op_t& ) = default;

   variant_t eval(rt_prog_ctx_t & ctx) const override;
   bool empty() const NU_NOEXCEPT override;

protected:
   std::string _op_name;
   expr_any_t::handle_t _var;
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_UNARY_OP_H__
