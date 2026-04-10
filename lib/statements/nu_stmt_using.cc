//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_stmt_using.h"
#include "nu_rt_prog_ctx.h"

namespace nu {

stmt_using_t::stmt_using_t(prog_ctx_t& ctx, const std::string& module_name)
    : stmt_t(ctx)
    , _module_name(module_name)
{
    ctx.import_builtin_module(module_name);
}

void stmt_using_t::run(rt_prog_ctx_t& ctx)
{
    ctx.import_builtin_module(_module_name);
    ctx.go_to_next();
}

} // namespace nu
