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

#include "nu_stmt.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

/**
 * Represents the END SELECT statement.
 * At parse time it closes the open SELECT CASE block in the metadata.
 * At run time it resets the runtime matched/value state and continues.
 */
class stmt_end_select_t : public stmt_t {
public:
    stmt_end_select_t() = delete;
    stmt_end_select_t(const stmt_end_select_t&) = delete;
    stmt_end_select_t& operator=(const stmt_end_select_t&) = delete;

    stmt_end_select_t(prog_ctx_t& ctx);

    void run(rt_prog_ctx_t& ctx) override;
    stmt_cl_t get_cl() const noexcept override;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
