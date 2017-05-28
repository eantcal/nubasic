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
