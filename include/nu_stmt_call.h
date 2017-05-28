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

#ifndef __NU_STMT_CALL_H__
#define __NU_STMT_CALL_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_prog_pointer.h"
#include "nu_stmt.h"
#include <list>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_call_t : public stmt_t {
public:
    stmt_call_t(const stmt_call_t&) = delete;
    stmt_call_t& operator=(const stmt_call_t&) = delete;
    stmt_call_t() = delete;

    stmt_call_t(const std::string& name, prog_ctx_t& ctx, bool fncall = false)
        : stmt_t(ctx)
        , _name(name)
        , _fncall(fncall)
    {
    }

    stmt_call_t(const arg_list_t& args, const std::string& name,
        prog_ctx_t& ctx, bool fncall = false)
        : stmt_t(ctx)
        , _args(args)
        , _name(name)
        , _fncall(fncall)
    {
    }

    virtual void run(rt_prog_ctx_t& ctx) override;
    void run(rt_prog_ctx_t& ctx, const prog_pointer_t::line_number_t& line);

protected:
    arg_list_t _args;
    std::string _name;
    bool _fncall = false;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_CALL_H__
