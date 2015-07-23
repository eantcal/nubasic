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
#include "nu_stmt_input_file.h"
#include "nu_os_console.h"
#include <cstdio>


/* -------------------------------------------------------------------------- */

namespace nu
{

void stmt_input_file_t::run(rt_prog_ctx_t & ctx)
{
   FILE * s_in = ctx.get_stdin_ptr();

   if (_fd > 0)
   {
      s_in = ctx.file_tbl.resolve_fd(_fd);

      if (!s_in)
      {
         ctx.set_errno(EBADF);
         ctx.go_to_next();
         return;
      }
   }

   int ret = 0;

   for (auto const & variable : _vars)
   {
      auto vtype = variable_t::type_by_name(variable.first);

      std::string svalue;
      int ivalue = 0;
      long long llvalue = 0;
      float fvalue = 0.0F;
      double dvalue = 0.0L;

      switch (vtype)
      {
         case nu::variable_t::type_t::INTEGER:
            ret = fscanf(s_in, "%i", &ivalue);
            break;

         case nu::variable_t::type_t::LONG64:
            ret = fscanf(s_in, "%lli", &llvalue);
            break;

         case nu::variable_t::type_t::FLOAT:
            ret = fscanf(s_in, "%f", &fvalue);
            break;

         case nu::variable_t::type_t::DOUBLE:
            ret = fscanf(s_in, "%lf", &dvalue);
            break;

         case nu::variable_t::type_t::STRING:
         case nu::variable_t::type_t::BOOLEAN:
            svalue = _os_input(s_in);
            break;

         case nu::variable_t::type_t::UNDEFINED:
         default:
            throw exception_t("input# failed reading variable");
            break;
      }

      auto index = variable.second;
      auto name = variable.first;

      bool is_vector = index != nullptr;

      if (is_vector)
      {
         syntax_error_if(
            !ctx.proc_scope.get()->is_defined(name),
            "'" + name + "' must be defined using DIM");

         size_t idx = index->eval(ctx).to_int();

         variant_t var = (*(ctx.proc_scope.get()))[name];

         //check size
         syntax_error_if(
            idx >= var.vector_size(),
            "'" + name + "(" + nu::to_string(idx) + ")' is"
            "out of range");

         switch (vtype)
         {
            case nu::variable_t::type_t::INTEGER:
               var.set_int(ivalue, idx);
               break;

            case nu::variable_t::type_t::LONG64:
               var.set_long64(llvalue, idx);
               break;

            case nu::variable_t::type_t::DOUBLE:
               var.set_double(dvalue, idx);
               break;

            case nu::variable_t::type_t::FLOAT:
               var.set_real(fvalue, idx);
               break;

            case nu::variable_t::type_t::STRING:
               var.set_str(svalue, idx);
               break;

            case nu::variable_t::type_t::BOOLEAN:
               var.set_bool(
                  strcasecmp(svalue.c_str(),"false")!=0 &&
                  strcasecmp(svalue.c_str(), "0") != 0, idx);
               break;

            case nu::variable_t::type_t::BYTEVECTOR:
               var.set_bvect(ivalue, idx);
               break;

            case nu::variable_t::type_t::UNDEFINED:
               break;
         }

         ctx.proc_scope.get()->define(name, var);
      }

      else
         switch (vtype)
         {
            case variable_t::type_t::STRING:
               (ctx.proc_scope.get())->define(name, svalue);
               break;

            case variable_t::type_t::DOUBLE:
               try
               {
                  (ctx.proc_scope.get())->define(name, dvalue);
               }
               catch (...)
               {
                  (ctx.proc_scope.get())->define(name, double(0));
               }

               break;

            case variable_t::type_t::FLOAT:
               try
               {
                  (ctx.proc_scope.get())->define(name, fvalue);
               }
               catch (...)
               {
                  (ctx.proc_scope.get())->define(name, float(0));
               }

               break;

            case variable_t::type_t::BOOLEAN:
               try
               {
                  (ctx.proc_scope.get())->define(name,
                                                 strcasecmp(svalue.c_str(), "false") != 0 &&
                                                 strcasecmp(svalue.c_str(), "0") != 0);
               }
               catch (...)
               {
                  (ctx.proc_scope.get())->define(name, false);
               }

               break;

            case variable_t::type_t::LONG64:
               try
               {
                  (ctx.proc_scope.get())->define(name, llvalue);
               }
               catch (...)
               {
                  (ctx.proc_scope.get())->define(name, (long long)0);
               }

               break;

            case variable_t::type_t::INTEGER:
            default:
               try
               {
                  (ctx.proc_scope.get())->define(name, ivalue);
               }
               catch (...)
               {
                  (ctx.proc_scope.get())->define(name, 0);
               }

               break;
         }
   }

   ctx.set_errno(ret != 0 ? errno : 0);

   ctx.go_to_next();

}


/* -------------------------------------------------------------------------- */

} // namespace nu