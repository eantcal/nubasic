//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_READ_H__
#define __NU_STMT_READ_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include "nu_token_list.h"
#include "nu_var_scope.h"
#include "nu_variable.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_read_file_t : public stmt_t {
public:
    stmt_read_file_t(const stmt_read_file_t&) = delete;
    stmt_read_file_t& operator=(const stmt_read_file_t&) = delete;

    stmt_read_file_t(prog_ctx_t& ctx, int, const var_arg_t&)
        : stmt_t(ctx)
    {
    }

    stmt_read_file_t(const arg_list_t& args, // integer expr which is size of dst buf
        prog_ctx_t& ctx,
        int fd, // integer value which is the file number
        const var_arg_t& var) // destination variable
        : stmt_t(ctx),
          _args(args),
          _fd(fd),
          _var(var)
    {
    }

    void run(rt_prog_ctx_t& ctx) override;

protected:
    arg_list_t _args;
    int _fd;
    var_arg_t _var;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_READ_H__
