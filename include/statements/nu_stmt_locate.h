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

#include "nu_expr_any.h"
#include "nu_os_console.h"
#include "nu_stmt.h"
#include "nu_token_list.h"
#include "nu_var_scope.h"
#include "nu_variable.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_locate_t : public stmt_t {
public:
    stmt_locate_t() = delete;
    stmt_locate_t(const stmt_locate_t&) = delete;
    stmt_locate_t& operator=(const stmt_locate_t&) = delete;

    stmt_locate_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }

    stmt_locate_t(arg_list_t args, prog_ctx_t& ctx)
        : stmt_t(ctx)
        , _args(args)
    {
    }

    void run(rt_prog_ctx_t& ctx) override;

protected:
    arg_list_t _args;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
