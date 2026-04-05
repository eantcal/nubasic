//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_end.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_end_t::run(rt_prog_ctx_t& ctx)
{
    ctx.flag.set(rt_prog_ctx_t::FLG_END_REQUEST, true);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
