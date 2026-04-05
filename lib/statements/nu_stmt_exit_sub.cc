//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

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
    const auto rt_error_if = 
        [&](bool cond, rt_error_code_t::value_t err, const std::string& desc) {
            rt_error_code_t::get_instance().throw_if(
                cond, ctx.runtime_pc.get_line(), err, desc);
        };

    const auto handle = ctx.procedure_metadata.exit_find(ctx.runtime_pc);

    rt_error_if(!handle, rt_error_code_t::value_t::E_EXIT_SUB_OUTSIDE_SUB, "");

    rt_error_if(handle->pc_end_stmt.get_line() < 1,
        rt_error_code_t::value_t::E_MISSING_END_SUB, "Exit Sub/Function");

    ctx.go_to(handle->pc_end_stmt);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
