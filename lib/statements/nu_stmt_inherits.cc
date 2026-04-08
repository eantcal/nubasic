//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_inherits.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"

/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

stmt_inherits_t::stmt_inherits_t(prog_ctx_t& ctx, const std::string& base_name)
    : stmt_t(ctx)
    , _base_name(base_name)
{
    const std::string& derived = ctx.compiling_class_name;

    syntax_error_if(derived.empty(), "'Inherits' used outside of a class body");

    syntax_error_if(base_name == derived,
        "Class '" + derived + "' cannot inherit from itself");

    // Cycle detection: walk existing chain from base upward
    {
        std::string cls = base_name;
        while (!cls.empty()) {
            syntax_error_if(cls == derived,
                "Circular inheritance detected involving class '" + derived
                    + "'");
            auto it = ctx.class_bases.find(cls);
            if (it == ctx.class_bases.end())
                break;
            cls = it->second;
        }
    }

    // Base class must already be defined
    syntax_error_if(ctx.struct_prototypes.data.find(base_name)
            == ctx.struct_prototypes.data.end(),
        "Base class '" + base_name + "' is not defined");

    // Only one Inherits clause per class
    syntax_error_if(ctx.class_bases.count(derived) != 0,
        "Class '" + derived + "' already has a base class");

    ctx.class_bases[derived] = base_name;
}


/* -------------------------------------------------------------------------- */

void stmt_inherits_t::run(rt_prog_ctx_t& ctx) { ctx.go_to_next(); }


/* -------------------------------------------------------------------------- */

} // namespace nu
