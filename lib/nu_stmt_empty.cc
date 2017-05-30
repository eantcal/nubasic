//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_empty.h"
#include "nu_rt_prog_ctx.h"
#include <memory>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_empty_t::run(rt_prog_ctx_t& ctx) { ctx.go_to_next(); };


/* -------------------------------------------------------------------------- */

stmt_empty_t::stmt_cl_t stmt_empty_t::get_cl() const noexcept
{
    return stmt_cl_t::EMPTY;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
