//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_delay.h"
#include "nu_error_codes.h"
#include "nu_os_std.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

static void __stmt_delay_t_run(rt_prog_ctx_t& ctx, arg_list_t& args,
    const std::string delay_desc, std::function<void(int)> delay_f,
    volatile bool& break_event, int poll_break_intv)
{
    rt_error_code_t::get_instance().throw_if(args.empty(),
        ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_WRG_NUM_ARGS, delay_desc);

    variant_t val = args.begin()->first->eval(ctx);

    int intv = val.to_int();
    int iters = intv / poll_break_intv;
    int left_intv = intv % poll_break_intv;

    while (!break_event && iters--) {
        delay_f(poll_break_intv);
    }

    if (left_intv && !break_event) {
        delay_f(left_intv);
    }

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

bool stmt_delay_t::notify(const event_t& ev)
{
    _break_delay = ev == event_t::BREAK;
    return _break_delay;
}


/* -------------------------------------------------------------------------- */

void stmt_delay_t::run(rt_prog_ctx_t& ctx)
{
    _break_delay = false;
    __stmt_delay_t_run(ctx, _args, "Delay", _os_delay, _break_delay, 1);
}


/* -------------------------------------------------------------------------- */

void stmt_mdelay_t::run(rt_prog_ctx_t& ctx)
{
    _break_delay = false;
    __stmt_delay_t_run(ctx, _args, "MDelay", _os_mdelay, _break_delay, 1000);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
