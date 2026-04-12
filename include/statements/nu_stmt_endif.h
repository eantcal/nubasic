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
#include "nu_stmt.h"
#include "nu_token_list.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_endif_t : public stmt_t {
public:
    stmt_endif_t() = delete;
    stmt_endif_t(const stmt_endif_t&) = delete;
    stmt_endif_t& operator=(const stmt_endif_t&) = delete;

    stmt_endif_t(prog_ctx_t& ctx);

    virtual void run(rt_prog_ctx_t& ctx) override;
    virtual stmt_cl_t get_cl() const noexcept override;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
