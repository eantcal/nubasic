//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_locate.h"
#include "nu_error_codes.h"
#include "nu_os_console.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_locate_t::run(rt_prog_ctx_t& ctx)
{
    auto rt_error_if
        = [&](bool cond, rt_error_code_t::value_t err, const std::string desc) {
              rt_error_code_t::get_instance().throw_if(
                  cond, ctx.runtime_pc.get_line(), err, desc);
          };

    enum { ARGS_NUM = 2 };

    rt_error_if(
        _args.size() != ARGS_NUM, rt_error_code_t::E_WRG_NUM_ARGS, "LOCATE");

    variant_t val[2];
    int i = 0;

    for (auto arg : _args) {
        if (arg.first)
            val[i] = arg.first->eval(ctx);

        rt_error_if(!variable_t::is_integral(val[i].get_type()),
            rt_error_code_t::E_INVALID_ARGS, "Locate");

        ++i;

        if (i > ARGS_NUM) // paranoid
            break;
    }

    // locate y,x

    int y = val[0].to_int();
    int x = val[1].to_int();

    if (x <= 0)
        x = 1;

    if (y <= 0)
        y = 1;

    _os_locate(y, x);

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
