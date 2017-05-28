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

#include "nu_stmt_locate.h"
#include "nu_error_codes.h"
#include "nu_os_console.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_locate_t::run(rt_prog_ctx_t& ctx)
{
    auto rt_error_if
        = [&](bool cond, rt_error_code_t::value_t err, const std::string desc) {
              rt_error_code_t::get_instance().throw_if(
                  cond, ctx.runtime_pc.get_line(), err, desc);
          };

    enum { ARGS_NUM = 2 };

    rt_error_if(
        _args.size() != ARGS_NUM, rt_error_code_t::E_WRG_NUM_ARGS, "LOCATE");

    variant_t val[2];
    int i = 0;

    for (auto arg : _args) {
        if (arg.first)
            val[i] = arg.first->eval(ctx);

        rt_error_if(!variable_t::is_integral(val[i].get_type()),
            rt_error_code_t::E_INVALID_ARGS, "Locate");

        ++i;

        if (i > ARGS_NUM) // paranoid
            break;
    }

    // locate y,x

    int y = val[0].to_int();
    int x = val[1].to_int();

    if (x <= 0)
        x = 1;

    if (y <= 0)
        y = 1;

    _os_locate(y, x);

    ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
