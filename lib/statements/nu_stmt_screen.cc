//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_screen.h"
#include "nu_rt_prog_ctx.h"

/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

void stmt_screenlock_t::run(rt_prog_ctx_t& ctx)
{
    _os_screenlock();
    ctx.go_to_next();
}

/* -------------------------------------------------------------------------- */

void stmt_screenunlock_t::run(rt_prog_ctx_t& ctx)
{
    _os_screenunlock();
    ctx.go_to_next();
}

/* -------------------------------------------------------------------------- */

void stmt_refresh_t::run(rt_prog_ctx_t& ctx)
{
    _os_refresh();
    ctx.go_to_next();
}

/* -------------------------------------------------------------------------- */

void stmt_screen_t::run(rt_prog_ctx_t& ctx)
{
    if (!_args.empty()) {
        int mode = static_cast<int>(_args.front().first->eval(ctx).to_int());
        _os_set_screen_mode(mode);
    }
    ctx.go_to_next();
}

/* -------------------------------------------------------------------------- */

} // namespace nu
