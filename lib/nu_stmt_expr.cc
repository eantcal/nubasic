//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_expr.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_expr_t::run(rt_prog_ctx_t& ctx)
{
    for (auto arg : _args) {
        if (arg.first != nullptr)
            arg.first->eval(ctx);
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */
