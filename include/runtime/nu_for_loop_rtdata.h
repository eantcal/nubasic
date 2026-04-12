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

#include "nu_flag_map.h"
#include "nu_prog_pointer.h"
#include "nu_variant.h"

#include <sstream>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

struct for_loop_ctx_t {
    enum { FLG_FIRST_EXEC };

    flag_map_t flag;

    for_loop_ctx_t() noexcept { flag.define(FLG_FIRST_EXEC, true); }

    for_loop_ctx_t(const for_loop_ctx_t&) = default;
    for_loop_ctx_t& operator=(const for_loop_ctx_t&) = default;

    prog_pointer_t pc_for_stmt;
    prog_pointer_t pc_next_stmt;

    variant_t step;
    variant_t end_counter;
};


/* -------------------------------------------------------------------------- */

struct for_loop_rtdata_t : public std::map<std::string, for_loop_ctx_t> {
    void trace(std::stringstream& ss);
    void cleanup_data(const std::string& proc_ctx);
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
