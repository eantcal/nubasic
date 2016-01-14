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

#include "nu_rt_prog_ctx.h"
#include "nu_stmt_gosub.h"
#include "nu_error_codes.h"
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

void stmt_gosub_t::run(rt_prog_ctx_t& ctx)
{
   if ( !_line_number )
   {
      if ( _label.empty() || !ctx.prog_label.is_defined(_label) )
      {
         rt_error_code_t::get_instance().throw_if(
            _label.empty(),
            ctx.runtime_pc.get_line(),
            rt_error_code_t::E_NO_LABEL,
            "Gosub");

         rt_error_code_t::get_instance().throw_if(
            !ctx.prog_label.is_defined(_label),
            ctx.runtime_pc.get_line(),
            rt_error_code_t::E_LABEL_NOT_DEF,
            "Gosub");
      }

      _line_number = ctx.prog_label[_label];
   }


   ctx.set_return_line(
      std::make_pair( ctx.runtime_pc.get_line(), get_stmt_id() ));

   ctx.go_to(prog_pointer_t(_line_number, 0));
}


/* -------------------------------------------------------------------------- */

} // namespace nu

