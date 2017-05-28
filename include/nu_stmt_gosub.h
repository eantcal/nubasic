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

#ifndef __NU_STMT_GOSUB_H__
#define __NU_STMT_GOSUB_H__


/* -------------------------------------------------------------------------- */

#include "nu_prog_pointer.h"
#include "nu_stmt.h"
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_gosub_t : public stmt_t {
public:
    stmt_gosub_t() = delete;
    stmt_gosub_t(const stmt_gosub_t&) = delete;
    stmt_gosub_t& operator=(const stmt_gosub_t&) = delete;

    stmt_gosub_t(prog_ctx_t& ctx, prog_pointer_t::line_number_t ln)
        : stmt_t(ctx)
        , _line_number(ln)
    {
    }

    stmt_gosub_t(prog_ctx_t& ctx, const std::string& label)
        : stmt_t(ctx)
        , _label(label)
    {
    }

    virtual void run(rt_prog_ctx_t& ctx) override;

protected:
    prog_pointer_t::line_number_t _line_number = 0;
    std::string _label;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_GOTO_H__