//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_close.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_close_t::run(rt_prog_ctx_t& ctx)
{
    const bool res = ctx.file_tbl.close_fd(_fd);

    ctx.set_errno(!res && !errno ? EBADF : errno);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
