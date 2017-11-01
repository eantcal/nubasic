//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_IF_THEN_ELSE_H__
#define __NU_STMT_IF_THEN_ELSE_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include "nu_stmt_empty.h"

#include <memory>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_if_then_else_t : public stmt_t {
public:
    stmt_if_then_else_t() = delete;
    stmt_if_then_else_t(const stmt_if_then_else_t&) = delete;
    stmt_if_then_else_t& operator=(const stmt_if_then_else_t&) = delete;

    stmt_if_then_else_t(prog_ctx_t& ctx, expr_any_t::handle_t condition,
        handle_t then_stmt, handle_t else_stmt = nullptr);

    void run(rt_prog_ctx_t& ctx) override;
    stmt_cl_t get_cl() const noexcept override;

protected:
    expr_any_t::handle_t _condition;
    handle_t _then_stmt;
    handle_t _else_stmt;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_IF_THEN_ELSE_H__
