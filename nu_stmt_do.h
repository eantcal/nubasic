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

#ifndef __NU_STMT_DO_H__
#define __NU_STMT_DO_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_signal_handling.h"
#include "nu_stmt.h"
#include "nu_stmt_empty.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_do_t : public stmt_t {
public:
    stmt_do_t() = delete;
    stmt_do_t(const stmt_do_t&) = delete;
    stmt_do_t& operator=(const stmt_do_t&) = delete;

    // DO
    //   [stmt(s)]
    // LOOP WHILE <condition>
    stmt_do_t(prog_ctx_t& ctx);

    virtual stmt_cl_t get_cl() const noexcept override;
    virtual void run(rt_prog_ctx_t& ctx) override;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_DO_H__