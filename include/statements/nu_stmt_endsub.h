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
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_endsub_t : public stmt_t {
public:
    stmt_endsub_t() = delete;
    stmt_endsub_t(const stmt_endsub_t&) = delete;
    stmt_endsub_t& operator=(const stmt_endsub_t&) = delete;

    stmt_endsub_t(prog_ctx_t& ctx);

    void run(rt_prog_ctx_t& ctx) override;
    stmt_cl_t get_cl() const noexcept override;
    bool is_debug_steppable() const noexcept override { return false; }

protected:
    bool run_pending_scope_destructor(rt_prog_ctx_t& ctx,
        const std::vector<std::string>& excluded_names = {});
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
