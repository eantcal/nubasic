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

#include "nu_rt_prog_ctx.h"
#include "nu_stmt_struct.h"
#include "nu_error_codes.h"

/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

stmt_struct_t::stmt_struct_t(prog_ctx_t & ctx, const std::string& id)
   :
   stmt_t(ctx),
   _id(id)
{

   //Search function prototype by name
   //
   auto i = ctx.struct_prototypes.data.find(id);

   syntax_error_if(
      (i != ctx.struct_prototypes.data.end() &&
       i->second.first.get_line() != ctx.compiletime_pc.get_line()),
      "Struct " + id + " already defined");
   //

   // Remove an old declaration for replacing its prototype
   if (i != ctx.struct_prototypes.data.end())
      ctx.struct_prototypes.data.erase(i);

   ctx.struct_prototypes.data.insert(
      std::make_pair(
         id,
         std::make_pair(
            ctx.compiletime_pc,
            variant_t(struct_variant_t(id)))
      )
   );

   ctx.struct_metadata.compile_begin(ctx.compiletime_pc, id);
}


/* -------------------------------------------------------------------------- */

void stmt_struct_t::run(rt_prog_ctx_t & ctx)
{
   auto & subctx = ctx.struct_metadata;
   auto handle = subctx.begin_find(ctx.runtime_pc);

   rt_error_code_t::get_instance().throw_if(
      !handle,
      ctx.runtime_pc.get_line(),
      rt_error_code_t::E_STRUCT_UNDEF,
      _id);

   auto scope_id = ctx.proc_scope.get_scope_id();

   // Skip function body if we are not in the global scope
   if (!scope_id.empty())
   {
      handle->flag.set(instrblock_t::EXIT, true);
      ctx.go_to(handle->pc_end_stmt);
      return;
   }
   else
   {
      handle->flag.set(instrblock_t::EXIT, false);
   }

   ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

stmt_struct_t::stmt_cl_t stmt_struct_t::get_cl() const NU_NOEXCEPT
{
   return stmt_cl_t::STRUCT_BEGIN;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
