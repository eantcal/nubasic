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

#include "nu_stdlib.h"
#include "nu_rt_prog_ctx.h"
#include "nu_error_codes.h"

#include <string>
#include <stdlib.h>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

int os_shell_t::exec(const std::string& cmd)
{
   return ::system(cmd.c_str());
}


/* -------------------------------------------------------------------------- */

int os_shell_t::apply(rt_prog_ctx_t& ctx, args_t args)
{
   enum { CMD, NARGS };
   (void)ctx;

   rt_error_code_t::get_instance().throw_if(
      args.size() != NARGS ||
      args[CMD].get_type() != variant_t::type_t::STRING,
      ctx.runtime_pc.get_line(),
      rt_error_code_t::E_INVALID_ARGS,
      "SHELL"
   );

   auto c = args[CMD].to_str();

   return exec(c);
}


/* -------------------------------------------------------------------------- */

int os_chdir_t::apply(rt_prog_ctx_t& ctx, args_t args)
{
   enum { CMD, NARGS };
   (void)ctx;

   rt_error_code_t::get_instance().throw_if(
      args.size() != NARGS ||
      args[CMD].get_type() != variant_t::type_t::STRING,
      ctx.runtime_pc.get_line(),
      rt_error_code_t::E_INVALID_ARGS,
      "CHDIR"
   );

   auto dir = args[CMD].to_str();

   return _os_change_dir(dir) ? 0 : errno;
}


/* -------------------------------------------------------------------------- */

int os_fopen_t::apply(rt_prog_ctx_t& ctx, args_t args)
{
   enum { FILENAME, MODE, FILENUMBER, NARGS };

   rt_error_code_t::get_instance().throw_if(
      args.size() != NARGS ||
      args[FILENAME].get_type() != variant_t::type_t::STRING ||
      args[MODE].get_type() != variant_t::type_t::STRING ||
      !variable_t::is_integral(args[FILENUMBER].get_type()),
      ctx.runtime_pc.get_line(),
      rt_error_code_t::E_INVALID_ARGS,
      "FOPEN"
   );

   auto filename   = args[FILENAME].to_str();
   auto mode       = args[MODE].to_str();
   auto filenumber = args[FILENUMBER].to_int();

   bool res = ctx.file_tbl.open_fd(filename, mode, filenumber);

   return !res && !errno ? EBADF : errno;
}


/* -------------------------------------------------------------------------- */

int os_fflush_t::apply(rt_prog_ctx_t& ctx, args_t args)
{
   enum { FILENUMBER, NARGS };

   rt_error_code_t::get_instance().throw_if(
      args.size() != NARGS ||
      ! variable_t::is_integral( args[FILENUMBER].get_type() ),
      ctx.runtime_pc.get_line(),
      rt_error_code_t::E_INVALID_ARGS,
      "FLUSH"
   );

   auto filenumber = args[FILENUMBER].to_int();

   bool res = ctx.file_tbl.flush_fd(filenumber);

   return !res && !errno ? EBADF : errno;
}


/* -------------------------------------------------------------------------- */

int os_fseek_t::apply(rt_prog_ctx_t& ctx, args_t args)
{
   enum { FILENUMBER, SEEKPTR, SEEKORIGIN, NARGS };

   rt_error_code_t::get_instance().throw_if(
      args.size() != NARGS ||
      ! variable_t::is_integral(args[SEEKPTR].get_type()) ||
      ! variable_t::is_integral(args[SEEKORIGIN].get_type()) ||
      ! variable_t::is_integral(args[FILENUMBER].get_type()),
      ctx.runtime_pc.get_line(),
      rt_error_code_t::E_INVALID_ARGS,
      "SEEK"
   );

   auto filenumber = args[FILENUMBER].to_int();
   auto seekptr    = args[SEEKPTR].to_int();
   auto seekorigin = args[SEEKORIGIN].to_int();

   bool res = ctx.file_tbl.seek_fd( seekptr, seekorigin, filenumber );

   return !res && !errno ? EBADF : errno;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
