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

#ifndef __NU_EXPR_STRUCT_H__
#define __NU_EXPR_STRUCT_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_var_scope.h"
#include "nu_global_function_tbl.h" // TODO


#include <string>
#include <list>

/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

//! This class represents a function-call expression
class expr_struct_t : public expr_any_t
{
public:
   // Parser should produce a list of pairs. Each of them is 
   // something like <identifier, vector index>
   //
   // id1(index).id2.id3(index3) --> 
   //
   // <"id1",index>, <"id2",0>, <"id3",index3>
   //
   using member_idx_t = 
      std::pair< std::string /* id */, expr_any_t::handle_t /* vector idx */ >;

   using member_call_list_t = std::vector<member_idx_t>;

   //! ctor
   expr_struct_t(const member_call_list_t& member_call_list) NU_NOEXCEPT :
      _member_call_list(member_call_list)
   {}

   expr_struct_t() = delete;
   expr_struct_t(const expr_struct_t&) = default;
   expr_struct_t& operator=( const expr_struct_t& ) = default;

   //! Evaluates the function (using name and arguments given to the ctor)
   variant_t eval(rt_prog_ctx_t & ctx) const override;

   //! Returns false for this type of object
   bool empty() const NU_NOEXCEPT override;

protected:
   member_call_list_t _member_call_list;
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_STRUCT_H__
