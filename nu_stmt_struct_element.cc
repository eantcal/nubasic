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

#include "nu_rt_prog_ctx.h"
#include "nu_stmt_struct_element.h"
#include "nu_error_codes.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

stmt_struct_element_t::stmt_struct_element_t(
   prog_ctx_t & ctx,
   const std::string& name,
   variable_t::type_t type,
   vec_size_t vect_size)
{
   auto element_it =
      ctx.struct_prototypes.data.find(ctx.compiling_struct_name);

   syntax_error_if(
      element_it == ctx.struct_prototypes.data.end(),
         name,
         0,
         "Struct... End Struct");

   element_it->second.second.define_struct_member(
      name,
      variant_t(string_t(), type, vect_size));
}


/* -------------------------------------------------------------------------- */

void stmt_struct_element_t::run(rt_prog_ctx_t & ctx)
{
   ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
