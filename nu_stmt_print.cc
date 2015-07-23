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
*  Author: <antonino.calderone@ericsson.com>, <acaldmail@gmail.com>
*
*/


/* -------------------------------------------------------------------------- */

#include "nu_rt_prog_ctx.h"
#include "nu_stmt_print.h"
#include "nu_os_console.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

void stmt_print_t::run(rt_prog_ctx_t & ctx)
{
   bool hide_cursor = true;
   FILE* sout = ctx.get_stdout_ptr();


   struct _hide_cursor_guard_t
   {
      bool _hide;

      _hide_cursor_guard_t(bool hide) throw() : _hide(hide)
      {
         if (_hide) _os_cursor_visible(false);
      }

      ~_hide_cursor_guard_t() throw()
      {
         if (_hide) _os_cursor_visible(true);
      }
   };

   if (_fd != 0)
   {
      sout = ctx.file_tbl.resolve_fd(_fd);
      hide_cursor = false;
   }

   if (sout == nullptr)
   {
      ctx.set_errno(EBADF);
      ctx.go_to_next();
      return;
   }

   int ret = 0;

   if (_args.empty())
   {
      _hide_cursor_guard_t guard(hide_cursor);
      ret = ::fprintf(sout, "%s\n", _data.c_str());
      ::fflush(sout);
   }

   else
   {
      _hide_cursor_guard_t guard(hide_cursor);

      for (auto arg : _args)
      {
         std::string separator;
         variant_t val;

         if (arg.first == nullptr)
         {
            val = variant_t("");
            separator = "";
         }
         else
         {
            val = arg.first->eval(ctx);

            switch (arg.second)
            {
               case ',':
                  separator = "\t";
                  break;

               case ';':
                  separator = ""; //GW-BASIC syntax
                  break;

               case ' ':
                  separator = "";
                  break;

               default:
                  separator = "\n";
                  break;
            }
         }

         std::stringstream ss;

         if (sout != stdout && variable_t::is_number(val.get_type()))
         {
            //Implements write# semantic (see implementation of input#)
            switch (val.get_type())
            {
               case variant_t::type_t::FLOAT:
                  ret = ::fprintf(sout, "%f%s",
                                  val.to_real(),
                                  separator.c_str());
                  ::fflush(sout);
                  break;

               case variant_t::type_t::DOUBLE:
                  ret = ::fprintf(sout, "%lf%s",
                                  val.to_double(),
                                  separator.c_str());
                  ::fflush(sout);
                  break;

               case variant_t::type_t::INTEGER:
                  ret = ::fprintf(sout, "%i%s",
                                  val.to_int(),
                                  separator.c_str());
                  ::fflush(sout);
                  break;

               case variant_t::type_t::BOOLEAN:
                  ret = ::fprintf(sout, "%s%s",
                                  val.to_bool() ? "true" : "false",
                                  separator.c_str());
                  ::fflush(sout);
                  break;

               case variant_t::type_t::LONG64:
                  ret = ::fprintf(sout, "%lli%s",
                                  val.to_long64(),
                                  separator.c_str());
                  ::fflush(sout);
                  break;

               case variant_t::type_t::STRING:
               case variant_t::type_t::BYTEVECTOR:
               case variant_t::type_t::UNDEFINED:
               default:
                  assert(0);
                  break;
            }
         }

         else switch (val.get_type())
            {
               case variant_t::type_t::BOOLEAN:
                  ret = ::fprintf(sout, "%s%s",
                                  val.to_bool() ? "true" : "false",
                                  separator.c_str());
                  ::fflush(sout);
                  break;

               case variant_t::type_t::FLOAT:
               case variant_t::type_t::DOUBLE:
                  ss << val.to_double();

                  ret = ::fprintf(sout, "%s%s",
                                  ss.str().c_str(),
                                  separator.c_str());
                  ::fflush(sout);
                  break;

               case variant_t::type_t::INTEGER:
               case variant_t::type_t::LONG64:
                  ss << val.to_long64();

                  ret = ::fprintf(sout, "%s%s",
                                  ss.str().c_str(),
                                  separator.c_str());
                  ::fflush(sout);
                  break;

               case variant_t::type_t::STRING:
               case variant_t::type_t::BYTEVECTOR:
               case variant_t::type_t::UNDEFINED:
               default:
                  if (val.get_type() != variant_t::type_t::UNDEFINED)
                  {
                     ret = ::fprintf(sout, "%s%s",
                                     val.to_str().c_str(),
                                     separator.c_str());
                     ::fflush(sout);
                  }

                  break;
            } //switch
      } //for

   }//else

   ctx.go_to_next();
   ctx.set_errno(ret != 0 ? errno : 0);
}


/* -------------------------------------------------------------------------- */

} // namespace nu


