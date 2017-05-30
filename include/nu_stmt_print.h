//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_PRINT_H__
#define __NU_STMT_PRINT_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include <list>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_print_t : public stmt_t {
public:
    stmt_print_t(const stmt_print_t&) = delete;
    stmt_print_t& operator=(const stmt_print_t&) = delete;

    stmt_print_t(prog_ctx_t& ctx, int fd = 0, const std::string& data = "")
        : stmt_t(ctx)
        , _fd(fd)
        , _data(data)
    {
    }

    stmt_print_t(const arg_list_t& args, prog_ctx_t& ctx, int fd)
        : stmt_t(ctx)
        , _args(args)
        , _fd(fd)
    {
    }

    virtual void run(rt_prog_ctx_t& ctx) override;

protected:
    arg_list_t _args;
    int _fd = 0; /* 0 means 'use stdout' */
    std::string _data;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_PRINT_H__
