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
#include "nu_variant.h"

#include <string>
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

/**
 * A single match condition for a CASE clause.
 *
 * Three forms:
 *   EQUAL          Case 1, 2, 3
 *   RANGE          Case 1 To 5
 *   IS_COMPARISON  Case Is > 5
 */
struct case_condition_t {
    enum class kind_t { EQUAL, RANGE, IS_COMPARISON };

    kind_t kind = kind_t::EQUAL;

    // EQUAL / RANGE low / IS_COMPARISON comparand
    expr_any_t::handle_t expr1;

    // RANGE high bound (used only when kind == RANGE)
    expr_any_t::handle_t expr2;

    // Comparison operator string for IS_COMPARISON (">", ">=", "<", "<=", "=",
    // "<>")
    std::string op;

    bool matches(const variant_t& select_val, rt_prog_ctx_t& ctx) const;
};


/* -------------------------------------------------------------------------- */

/**
 * Represents a CASE (or CASE ELSE) clause inside a SELECT CASE block.
 *
 * Constructed with an empty condition list when it is a CASE ELSE clause.
 */
class stmt_case_t : public stmt_t {
public:
    using conditions_t = std::vector<case_condition_t>;

    stmt_case_t() = delete;
    stmt_case_t(const stmt_case_t&) = delete;
    stmt_case_t& operator=(const stmt_case_t&) = delete;

    // CASE <condition-list>
    stmt_case_t(prog_ctx_t& ctx, conditions_t conditions);

    // CASE ELSE
    explicit stmt_case_t(prog_ctx_t& ctx);

    void run(rt_prog_ctx_t& ctx) override;
    stmt_cl_t get_cl() const noexcept override;

    bool is_else() const noexcept { return _is_else; }

protected:
    conditions_t _conditions;
    bool _is_else = false;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
