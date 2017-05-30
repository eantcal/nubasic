//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include "nu_expr_any.h"
#include "nu_rt_prog_ctx.h"

#include <cassert>
#include <list>
#include <memory>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_t::stmt_t(prog_ctx_t& ctx) noexcept { _stmt_id = ctx.make_next_stmt_id(); }


/* -------------------------------------------------------------------------- */

stmt_t::stmt_cl_t stmt_t::get_cl() const noexcept { return stmt_cl_t::NA; }


/* -------------------------------------------------------------------------- */

stmt_t::~stmt_t() {}


/* -------------------------------------------------------------------------- */
}
