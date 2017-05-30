//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_struct.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"

/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_struct_t::stmt_struct_t(prog_ctx_t& ctx, const std::string& id)
    : stmt_t(ctx)
    , _id(id)
{

    // Search function prototype by name
    //
    auto i = ctx.struct_prototypes.data.find(id);

    syntax_error_if(
        (i != ctx.struct_prototypes.data.end()
            && i->second.first.get_line() != ctx.compiletime_pc.get_line()),
        "Struct " + id + " already defined");
    //

    // Remove an old declaration for replacing its prototype
    if (i != ctx.struct_prototypes.data.end())
        ctx.struct_prototypes.data.erase(i);

    ctx.struct_prototypes.data.insert(std::make_pair(id,
        std::make_pair(ctx.compiletime_pc, variant_t(struct_variant_t(id)))));

    ctx.struct_metadata.compile_begin(ctx.compiletime_pc, id);
}


/* -------------------------------------------------------------------------- */

void stmt_struct_t::run(rt_prog_ctx_t& ctx)
{
    auto& subctx = ctx.struct_metadata;
    auto handle = subctx.begin_find(ctx.runtime_pc);

    rt_error_code_t::get_instance().throw_if(!handle, ctx.runtime_pc.get_line(),
        rt_error_code_t::E_STRUCT_UNDEF, _id);

    auto scope_id = ctx.proc_scope.get_scope_id();

    // Skip function body if we are not in the global scope
    if (!scope_id.empty()) {
        handle->flag.set(instrblock_t::EXIT, true);
        ctx.go_to(handle->pc_end_stmt);
        return;
    } else {
        handle->flag.set(instrblock_t::EXIT, false);
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

stmt_struct_t::stmt_cl_t stmt_struct_t::get_cl() const noexcept
{
    return stmt_cl_t::STRUCT_BEGIN;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
