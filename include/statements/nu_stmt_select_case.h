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

#include <memory>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

/**
 * Represents the SELECT CASE <expression> statement.
 * At parse time it registers the current PC in the select_case metadata.
 * At run time it evaluates the expression and stores the result in the
 * runtime context so that subsequent CASE statements can compare against it.
 */
class stmt_select_case_t : public stmt_t {
public:
    stmt_select_case_t() = delete;
    stmt_select_case_t(const stmt_select_case_t&) = delete;
    stmt_select_case_t& operator=(const stmt_select_case_t&) = delete;

    stmt_select_case_t(prog_ctx_t& ctx, expr_any_t::handle_t expression);

    void run(rt_prog_ctx_t& ctx) override;
    stmt_cl_t get_cl() const noexcept override;

protected:
    expr_any_t::handle_t _expression;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
