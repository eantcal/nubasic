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

#include "nu_os_std.h"
#include "nu_stmt.h"

/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_randomize_t : public stmt_t {
public:
    stmt_randomize_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }

    stmt_randomize_t() = delete;
    stmt_randomize_t(const stmt_randomize_t&) = delete;
    stmt_randomize_t& operator=(const stmt_randomize_t&) = delete;

    void run(rt_prog_ctx_t& ctx) override
    {
        (void)ctx;
        _os_randomize();
    }
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
