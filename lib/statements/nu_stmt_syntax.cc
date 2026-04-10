//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_stmt_syntax.h"
#include "nu_rt_prog_ctx.h"

namespace nu {

stmt_syntax_t::stmt_syntax_t(prog_ctx_t& ctx, prog_ctx_t::syntax_mode_t mode)
    : stmt_t(ctx)
    , _mode(mode)
{
    ctx.set_syntax_mode(mode);
}

void stmt_syntax_t::run(rt_prog_ctx_t& ctx)
{
    ctx.set_syntax_mode(_mode);
    ctx.go_to_next();
}

} // namespace nu
