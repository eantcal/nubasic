/*
*  This file is part of nuBASIC
*
*  nuBASIC is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  nuBASIC is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with nuBASIC; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
*
*  Author: Antonino Calderone <acaldmail@gmail.com>
*
*/


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

class stmt_read_t : public stmt_t {
public:
    stmt_read_t(const stmt_read_t&) = delete;
    stmt_read_t& operator=(const stmt_read_t&) = delete;

    stmt_read_t(prog_ctx_t& ctx, int, const var_arg_t&)
        : stmt_t(ctx)
    {
    }

    stmt_read_t(const arg_list_t& args, // integer expr which is size of dst buf
        prog_ctx_t& ctx,
        int fd, // integer value which is the file number
        const var_arg_t& var) // destination variable
        : stmt_t(ctx),
          _args(args),
          _fd(fd),
          _var(var)
    {
    }

    virtual void run(rt_prog_ctx_t& ctx) override;

protected:
    arg_list_t _args;
    int _fd;
    var_arg_t _var;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_READ_H__
