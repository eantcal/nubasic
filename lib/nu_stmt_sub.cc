//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_sub.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_sub_t::stmt_sub_t(prog_ctx_t& ctx, const std::string& id)
    : stmt_t(ctx)
    , _id(id)
{
    const auto i = ctx.proc_prototypes.data.find(id);

    const auto found = i != ctx.proc_prototypes.data.end();

    syntax_error_if(found && i->second.first.get_line() != ctx.compiletime_pc.get_line(),
        "Sub-routine " + id + " already defined");

    // Remove old declaration for replacing its prototype
    if (found) {
        ctx.proc_prototypes.data.erase(i);
    }

    ctx.proc_prototypes.data.insert(std::make_pair(
        id, std::make_pair(ctx.compiletime_pc, func_prototype_t())));

    ctx.procedure_metadata.compile_begin(ctx.compiletime_pc, id);
}


/* -------------------------------------------------------------------------- */

void stmt_sub_t::run(rt_prog_ctx_t& ctx)
{
    auto& subctx = ctx.procedure_metadata;
    const auto handle = subctx.begin_find(ctx.runtime_pc);

    rt_error_code_t::get_instance().throw_if(
        !handle, ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_SUB_UNDEF, _id);

    const auto scope_id = ctx.proc_scope.get_scope_id();

    // Skip function body if we are in the global scope
    if (scope_id.empty()) {
        handle->flag.set(instrblock_t::EXIT, true);
        ctx.go_to(handle->pc_end_stmt);
        return;
    }
    else {
        handle->flag.set(instrblock_t::EXIT, false);
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

void stmt_sub_t::define(const std::string& var, const std::string& vtype,
    vec_size_t vect_size, prog_ctx_t& ctx, const std::string& id)
{
    assert(_id == id);

    syntax_error_if(_vars_rep_check.find(var) != _vars_rep_check.end(),
        "Cannot reuse same variable in a argument list");

    _vars_rep_check.insert(var);

    // TODO VAR prototype must include types
    auto& fproto = ctx.proc_prototypes.data[id].second;

    fproto.parameters.emplace_back(func_param_t(var, vtype, vect_size));
}


/* -------------------------------------------------------------------------- */

stmt_sub_t::stmt_cl_t stmt_sub_t::get_cl() const noexcept
{
    return stmt_cl_t::SUB_BEGIN;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
