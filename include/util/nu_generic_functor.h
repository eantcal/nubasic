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

#include "nu_variant.h"
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class rt_prog_ctx_t;


/* -------------------------------------------------------------------------- */

struct generic_functor_t {
    using args_t = std::vector<variant_t>;

    int operator()(rt_prog_ctx_t& ctx, args_t args) { return apply(ctx, args); }

    virtual int apply(rt_prog_ctx_t& ctx, args_t args) = 0;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */
