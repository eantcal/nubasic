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

#ifndef __NU_EXPR_BIN_H__
#define __NU_EXPR_BIN_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_global_function_tbl.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class expr_bin_t : public expr_any_t
{
public:
   using func_t = func_bin_t;

   //! ctor
   expr_bin_t(
      func_t f,
      expr_any_t::handle_t var1,
      expr_any_t::handle_t var2) :
      _func(f),
      _var1(var1),
      _var2(var2)
   {}


   expr_bin_t() = delete;
   expr_bin_t(const expr_bin_t&) = default;
   expr_bin_t& operator=( const expr_bin_t& ) = default;

   //! Returns f(var1, var2) appling ctor given arguments
   virtual variant_t eval(rt_prog_ctx_t & ctx) const
   {
      return _func(_var1->eval(ctx), _var2->eval(ctx));
   }

   //! Returns false for a binary expression
   virtual bool empty() const throw( ) override
   {
      return false;
   }


protected:
   func_bin_t _func;
   expr_any_t::handle_t _var1, _var2;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif
