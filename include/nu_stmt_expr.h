//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_EXPR_H__
#define __NU_STMT_EXPR_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include <list>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_expr_t : public stmt_t {
public:
    stmt_expr_t(stmt_expr_t&) = delete;
    stmt_expr_t& operator=(stmt_expr_t&) = delete;

    stmt_expr_t(prog_ctx_t& ctx, const std::string& data = "")
        : stmt_t(ctx)
        , _data(data)
    {
    }

    stmt_expr_t(const arg_list_t& args, prog_ctx_t& ctx)
        : stmt_t(ctx)
        , _args(args)
    {
    }

    virtual void run(rt_prog_ctx_t& ctx) override;

protected:
    arg_list_t _args;
    std::string _data;
};


/* -------------------------------------------------------------------------- */
}

/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_EXPR_H__
