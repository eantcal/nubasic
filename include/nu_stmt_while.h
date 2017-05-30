//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_WHILE_H__
#define __NU_STMT_WHILE_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_signal_handling.h"
#include "nu_stmt.h"
#include "nu_stmt_empty.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_while_t : public stmt_t, protected signal_handler_t {
public:
    stmt_while_t() = delete;
    stmt_while_t(const stmt_while_t&) = delete;
    stmt_while_t& operator=(const stmt_while_t&) = delete;

    // while <condition> [do]
    //   [stmt(s)]
    // wend
    stmt_while_t(prog_ctx_t& ctx, expr_any_t::handle_t condition);

    // while <condition> do <stmt>
    stmt_while_t(prog_ctx_t& ctx, expr_any_t::handle_t condition,
        stmt_t::handle_t while_stmt);

    ~stmt_while_t();

    virtual stmt_cl_t get_cl() const noexcept override;
    virtual void run(rt_prog_ctx_t& ctx) override;

protected:
    expr_any_t::handle_t _condition;
    stmt_t::handle_t _while_stmt;
    bool _single_stmt = false;
    bool _break_while_loop = false;

    static void build_ctx(prog_ctx_t& ctx);

    virtual bool notify(const event_t& ev) override;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_WHILE_H__
