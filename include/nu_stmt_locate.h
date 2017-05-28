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

#ifndef __NU_STMT_LOCATE_H__
#define __NU_STMT_LOCATE_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_os_console.h"
#include "nu_stmt.h"
#include "nu_token_list.h"
#include "nu_var_scope.h"
#include "nu_variable.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_locate_t : public stmt_t {
public:
    stmt_locate_t() = delete;
    stmt_locate_t(const stmt_locate_t&) = delete;
    stmt_locate_t& operator=(const stmt_locate_t&) = delete;

    stmt_locate_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }


    stmt_locate_t(arg_list_t args, prog_ctx_t& ctx)
        : stmt_t(ctx)
        , _args(args)
    {
    }

    virtual void run(rt_prog_ctx_t& ctx) override;

protected:
    arg_list_t _args;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_LOCATE_H__