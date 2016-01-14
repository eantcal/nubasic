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

#ifndef __NU_EXPR_EMPTY_H__
#define __NU_EXPR_EMPTY_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_variant.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class expr_empty_t : public expr_any_t
{
public:
   //! ctors
   expr_empty_t() = default;
   expr_empty_t(const expr_empty_t&) = default;
   expr_empty_t& operator =( const expr_empty_t& ) = default;

   //! It does nothing for an empty object
   virtual variant_t eval(rt_prog_ctx_t &) const override
   {
      return variant_t(0);
   }

   //! Returns true for an empty expression
   virtual bool empty() const NU_NOEXCEPT override
   {
      return true;
   }

   std::string name() const NU_NOEXCEPT override
   {
      return "";
   }

   func_args_t get_args() const NU_NOEXCEPT override
   {
      func_args_t dummy;
      return dummy;
   }
};


/* -------------------------------------------------------------------------- */

}

#endif // __NU_EXPR_EMPTY_H__
