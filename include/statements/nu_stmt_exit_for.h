//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */
#pragma once

/* -------------------------------------------------------------------------- */

#include "nu_prog_ctx.h"
#include "nu_stmt.h"
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_exit_for_t : public stmt_t {

public:
    stmt_exit_for_t() = delete;
    stmt_exit_for_t(const stmt_exit_for_t&) = delete;
    stmt_exit_for_t& operator=(const stmt_exit_for_t&) = delete;

    stmt_exit_for_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
        ctx.for_loop_metadata.compile_exit_point(ctx.compiletime_pc);
    }

    void run(rt_prog_ctx_t& ctx) override;
};


/* -------------------------------------------------------------------------- */
} // namespace nu


/* -------------------------------------------------------------------------- */
