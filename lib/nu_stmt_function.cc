//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_function.h"
#include "nu_error_codes.h"
#include "nu_global_function_tbl.h"
#include "nu_rt_prog_ctx.h"

/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_function_t::stmt_function_t(prog_ctx_t& ctx, const std::string& id)
    : stmt_t(ctx)
    , _id(id)
{

    // Get a reference to global function set
    auto& funcs = global_function_tbl_t::get_instance();

    // Search function prototype by name
    //
    auto i = ctx.proc_prototypes.data.find(id);

    syntax_error_if(
        funcs.is_defined(id) || (i != ctx.proc_prototypes.data.end()
                                    && i->second.first.get_line()
                                        != ctx.compiletime_pc.get_line()),
        "Function " + id + " already defined");
    //

    // Remove an old declaration for replacing its prototype
    if (i != ctx.proc_prototypes.data.end())
        ctx.proc_prototypes.data.erase(i);

    ctx.proc_prototypes.data.insert(std::make_pair(
        id, std::make_pair(ctx.compiletime_pc, func_prototype_t())));

    // Register this procedure as function
    ctx.function_tbl.insert(id);

    // Create metadata for the function (including begin-line and its name)
    ctx.procedure_metadata.compile_begin(ctx.compiletime_pc, id);

    // Create an execution object (using a lamda)
    auto this_func = [](rt_prog_ctx_t& ctx, const std::string& name,
        const func_args_t& args) -> variant_t {
        // Run the function
        ctx.program().run(name, args);

        // Retrieve the return value
        auto i = ctx.function_retval_tbl.find(name);

        syntax_error_if(i == ctx.function_retval_tbl.end(),
            "Return value '" + name + "' not defined");

        auto& stack = i->second;
        auto ret = stack.front();
        stack.pop_front();

        if (stack.empty())
            ctx.function_retval_tbl.erase(name);

        return ret;
    };

    // Register this execution object as a function
    // (just like any other global function,
    //  including precompiled built-in ones)
    funcs.define(id, this_func);
}


/* -------------------------------------------------------------------------- */

void stmt_function_t::run(rt_prog_ctx_t& ctx)
{
    auto& subctx = ctx.procedure_metadata;
    auto handle = subctx.begin_find(ctx.runtime_pc);

    rt_error_code_t::get_instance().throw_if(
        !handle, ctx.runtime_pc.get_line(), rt_error_code_t::E_FUNC_UNDEF, _id);

    auto scope_id = ctx.proc_scope.get_scope_id();

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

void stmt_function_t::define(const std::string& var, const std::string& vtype,
    vec_size_t vect_size, prog_ctx_t& ctx, const std::string& id)
{
    assert(_id == id);

    syntax_error_if(_vars_rep_check.find(var) != _vars_rep_check.end(),
        "Cannot reuse a variable name in subroutine argument list");

    _vars_rep_check.insert(var);

    auto& fproto = ctx.proc_prototypes.data[id].second;

    fproto.parameters.push_back(func_param_t(var, vtype, vect_size));
}


/* -------------------------------------------------------------------------- */

stmt_function_t::stmt_cl_t stmt_function_t::get_cl() const noexcept
{
    return stmt_cl_t::SUB_BEGIN;
}

/* -------------------------------------------------------------------------- */

} // namespace nu
