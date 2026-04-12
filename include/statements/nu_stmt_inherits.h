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

// Records the inheritance relationship "DerivedClass Inherits BaseClass".
// All compile-time work (recording class_bases) is done in the constructor.
// At runtime the statement simply advances the program counter.

struct stmt_inherits_t : public stmt_t {
    stmt_inherits_t(prog_ctx_t& ctx, const std::string& base_name);

    void run(rt_prog_ctx_t& ctx) override;

    stmt_cl_t get_cl() const noexcept override { return stmt_cl_t::INHERITS; }

private:
    std::string _base_name;
};

/* -------------------------------------------------------------------------- */

} // namespace nu

/* -------------------------------------------------------------------------- */
