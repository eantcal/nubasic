//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_stop.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_stop_t::run(rt_prog_ctx_t& ctx)
{
    ctx.stop_metadata.pc_stop_stmt.insert(ctx.compiletime_pc);
    ctx.flag.set(rt_prog_ctx_t::FLG_STOP_REQUEST, true);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
