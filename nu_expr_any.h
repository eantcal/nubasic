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

#ifndef __NU_EXPR_ANY_H__
#define __NU_EXPR_ANY_H__


/* -------------------------------------------------------------------------- */

#include "nu_stdtype.h"
#include "nu_variant.h"
#include <functional>
#include <memory>
#include <list>
#include <vector>

/* -------------------------------------------------------------------------- */

namespace nu
{

/* -------------------------------------------------------------------------- */

class rt_prog_ctx_t;


/* -------------------------------------------------------------------------- */

struct expr_any_t
{
   using handle_t = std::shared_ptr < expr_any_t > ;
   using func_args_t = std::vector < expr_any_t::handle_t >;

   virtual variant_t eval(rt_prog_ctx_t & ctx) const = 0;
   virtual bool empty() const NU_NOEXCEPT = 0;

   virtual std::string name() const NU_NOEXCEPT = 0;
   virtual func_args_t get_args() const NU_NOEXCEPT = 0;

   virtual ~expr_any_t() {}
};


/* -------------------------------------------------------------------------- */

//! arg_list_t is a collection of expressions with a separator char
//! (char may be used for formatting purposes i.e. print implementation)
using arg_list_t = std::list < std::pair<expr_any_t::handle_t, char> > ;


/* -------------------------------------------------------------------------- */

//! var_arg_t is a pair of <name, index> - index may be any integer expression
//! handle index is provided to resolve array elements
using var_arg_t = std::pair < std::string, expr_any_t::handle_t > ;


/* -------------------------------------------------------------------------- */

//! var_list_t is a collection of var_arg_t
using var_list_t = std::list < var_arg_t > ;


/* -------------------------------------------------------------------------- */

//! Built-in function arguments
using func_args_t = expr_any_t::func_args_t;



/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif
