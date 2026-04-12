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

class stmt_goto_t : public stmt_t {
public:
    stmt_goto_t() = delete;
    stmt_goto_t(stmt_goto_t&) = delete;
    stmt_goto_t& operator=(stmt_goto_t&) = delete;

    stmt_goto_t(prog_ctx_t& ctx, prog_pointer_t::line_number_t ln)
        : stmt_t(ctx)
        , _line_number(ln)
    {
    }

    stmt_goto_t(prog_ctx_t& ctx, const std::string& label)
        : stmt_t(ctx)
        , _label(label)
    {
    }

    void run(rt_prog_ctx_t& ctx) override;

protected:
    prog_pointer_t::line_number_t _line_number = 0;
    std::string _label;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
