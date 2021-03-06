//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_ELSE_H__
#define __NU_STMT_ELSE_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include "nu_token_list.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_elif_t : public stmt_t {
public:
    stmt_elif_t() = delete;
    stmt_elif_t(const stmt_elif_t&) = delete;
    stmt_elif_t& operator=(const stmt_elif_t&) = delete;

    stmt_elif_t(prog_ctx_t& ctx, expr_any_t::handle_t condition);

    void run(rt_prog_ctx_t& ctx) override;

    //! Identifies the class of the statement
    stmt_cl_t get_cl() const noexcept override { 
        return stmt_cl_t::ELSE; 
    }

protected:
    expr_any_t::handle_t _condition;
};


/* -------------------------------------------------------------------------- */

class stmt_else_t : public stmt_elif_t {
public:
    stmt_else_t() = delete;
    stmt_else_t(const stmt_else_t&) = delete;
    stmt_else_t& operator=(const stmt_else_t&) = delete;

    stmt_else_t(prog_ctx_t& ctx)
        : stmt_elif_t(ctx, nullptr)
    {
    }
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_ELSE_H__
