//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_DELAY_H__
#define __NU_STMT_DELAY_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_os_std.h"
#include "nu_signal_handling.h"
#include "nu_stmt.h"
#include "nu_token_list.h"
#include "nu_var_scope.h"
#include "nu_variable.h"


#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_delay_t : public stmt_t, protected signal_handler_t {
public:
    stmt_delay_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
        signal_mgr_t::instance().register_handler(event_t::BREAK, this);
    }

    stmt_delay_t(arg_list_t args, prog_ctx_t& ctx)
        : stmt_t(ctx)
        , _args(args)
    {
        signal_mgr_t::instance().register_handler(event_t::BREAK, this);
    }

    virtual void run(rt_prog_ctx_t& ctx) override;

    virtual ~stmt_delay_t()
    {
        signal_mgr_t::instance().unregister_handler(event_t::BREAK, this);
    }

protected:
    arg_list_t _args;
    volatile bool _break_delay = false;

    virtual bool notify(const event_t& ev) override;

    stmt_delay_t(const stmt_delay_t&) = delete;
    stmt_delay_t& operator=(const stmt_delay_t&) = delete;
};


/* -------------------------------------------------------------------------- */

class stmt_mdelay_t : public stmt_delay_t {
public:
    stmt_mdelay_t(prog_ctx_t& ctx)
        : stmt_delay_t(ctx)
    {
    }

    stmt_mdelay_t(arg_list_t args, prog_ctx_t& ctx)
        : stmt_delay_t(args, ctx)
    {
    }

    virtual void run(rt_prog_ctx_t& ctx) override;

    stmt_mdelay_t(const stmt_mdelay_t&) = delete;
    stmt_mdelay_t& operator=(const stmt_mdelay_t&) = delete;
};


/* -------------------------------------------------------------------------- */
}

/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_DELAY_H__
