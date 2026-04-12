//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_class.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"

/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_class_t::stmt_class_t(prog_ctx_t& ctx, const std::string& id)
    : stmt_t(ctx)
    , _id(id)
{
    // Register as a struct prototype so instances can hold data members
    auto i = ctx.struct_prototypes.data.find(id);
    const auto found = i != ctx.struct_prototypes.data.end();

    syntax_error_if(
        found && i->second.first.get_line() != ctx.compiletime_pc.get_line(),
        "Class " + id + " already defined");

    if (found) {
        ctx.struct_prototypes.data.erase(i);
    }

    ctx.struct_prototypes.data.insert(std::make_pair(id,
        std::make_pair(
            ctx.compiletime_pc, variant_t(struct_variant_t(id, true)))));
    ctx.class_names.insert(id);

    ctx.class_metadata.compile_begin(ctx.compiletime_pc, id);
    ctx.compiling_class_name = id;
    ctx.compiling_class_member_is_public = true;
}


/* -------------------------------------------------------------------------- */

void stmt_class_t::run(rt_prog_ctx_t& ctx)
{
    auto& subctx = ctx.class_metadata;
    auto handle = subctx.begin_find(ctx.runtime_pc);

    rt_error_code_t::get_instance().throw_if(!handle, ctx.runtime_pc.get_line(),
        rt_error_code_t::value_t::E_CLASS_UNDEF, _id);

    auto scope_id = ctx.proc_scope.get_scope_id();

    // Skip class body if we are not in the global scope
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

stmt_class_t::stmt_cl_t stmt_class_t::get_cl() const noexcept
{
    return stmt_cl_t::CLASS_BEGIN;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
