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

#ifndef __NU_EXPR_LITERAL_H__
#define __NU_EXPR_LITERAL_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class expr_literal_t : public expr_any_t
{
public:
   //! ctor
   expr_literal_t(const variant_t& value = variant_t(0)) :
      _val(value)
   {}

   expr_literal_t(const expr_literal_t&) = default;
   expr_literal_t& operator=( const expr_literal_t& ) = default;

   //! move ctor
   expr_literal_t(expr_literal_t&& obj) :
      _val(std::move(obj._val))
   {}

   //! move operator
   expr_literal_t& operator=( expr_literal_t&& obj )
   {
      if ( this != &obj )
         _val = std::move(obj._val);

      return *this;
   }

   //! Return the literal expression value
   variant_t eval(rt_prog_ctx_t & ctx) const override
   {
      ( void ) ctx;
      return _val;
   }

   //! Returns false for this expression type
   virtual bool empty() const NU_NOEXCEPT override
   {
      return false;
   }

protected:
   variant_t _val;
};


/* -------------------------------------------------------------------------- */

}

/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_LITERAL_H__
