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

#include "nu_stmt_exit_sub.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"

/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_exit_sub_t::stmt_exit_sub_t(prog_ctx_t& ctx)
    : stmt_t(ctx)
{
    ctx.procedure_metadata.compile_exit_point(ctx.compiletime_pc);
}


/* -------------------------------------------------------------------------- */

void stmt_exit_sub_t::run(rt_prog_ctx_t& ctx)
{
    auto rt_error_if = [&](
        bool cond, rt_error_code_t::value_t err, const std::string& desc) {
        rt_error_code_t::get_instance().throw_if(
            cond, ctx.runtime_pc.get_line(), err, desc);
    };

    auto handle = ctx.procedure_metadata.exit_find(ctx.runtime_pc);

    rt_error_if(!handle, rt_error_code_t::E_EXIT_SUB_OUTSIDE_SUB, "");

    rt_error_if(handle->pc_end_stmt.get_line() < 1,
        rt_error_code_t::E_MISSING_END_SUB, "Exit Sub/Function");

    ctx.go_to(handle->pc_end_stmt);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
