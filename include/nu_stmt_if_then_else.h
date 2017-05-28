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

    virtual void run(rt_prog_ctx_t& ctx) override;
    virtual stmt_cl_t get_cl() const noexcept override;

protected:
    expr_any_t::handle_t _condition;
    handle_t _then_stmt;
    handle_t _else_stmt;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_IF_THEN_ELSE_H__
