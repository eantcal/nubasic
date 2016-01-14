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

#ifndef __NU_GENERIC_FUNCTOR_H__
#define __NU_GENERIC_FUNCTOR_H__


/* -------------------------------------------------------------------------- */

#include "nu_variant.h"
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class rt_prog_ctx_t;


/* -------------------------------------------------------------------------- */

struct generic_functor_t
{
   using args_t = std::vector < variant_t > ;

   int operator()(rt_prog_ctx_t& ctx, args_t args)
   {
      return apply(ctx, args);
   }

   virtual int apply(rt_prog_ctx_t& ctx, args_t args) = 0;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_GENERIC_FUNCTOR_H__