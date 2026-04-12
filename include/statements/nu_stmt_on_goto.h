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
#include "nu_stmt_empty.h"

#include <memory>
#include <string>
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_on_goto_t : public stmt_t {
public:
    stmt_on_goto_t() = delete;
    stmt_on_goto_t(const stmt_on_goto_t&) = delete;
    stmt_on_goto_t& operator=(const stmt_on_goto_t&) = delete;

    using label_list_t = std::vector<std::string>;

    stmt_on_goto_t(prog_ctx_t& ctx, expr_any_t::handle_t condition,
        const label_list_t& label_list)
        : stmt_t(ctx)
        , _condition(condition)
        , _label_list(label_list)
    {
    }

    void run(rt_prog_ctx_t& ctx) override;

protected:
    expr_any_t::handle_t _condition;
    label_list_t _label_list;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
