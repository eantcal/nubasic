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

#include "nu_stmt_endsub.h"
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_endfunction_t : public stmt_endsub_t {
public:
    stmt_endfunction_t() = delete;
    stmt_endfunction_t(const stmt_endfunction_t&) = delete;
    stmt_endfunction_t& operator=(const stmt_endfunction_t&) = delete;

    stmt_endfunction_t(prog_ctx_t& ctx);

    void run(rt_prog_ctx_t& ctx) override;
};


/* -------------------------------------------------------------------------- */

} // namespace nu

/* -------------------------------------------------------------------------- */
