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
