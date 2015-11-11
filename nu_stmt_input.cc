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
#include "nu_stmt_input.h"
#include "nu_os_console.h"
#include <cstdio>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

void stmt_input_t::run(rt_prog_ctx_t & ctx)
{
   FILE * sout = ctx.get_stdout_ptr();

   fprintf(sout, "%s", _input_str.c_str());

   for (auto const & variable : _vars)
   {
      std::string value = _os_input(ctx.get_stdin_ptr());

      while (!value.empty() &&
             (value.c_str()[value.size() - 1] == '\n'))
      {
         value.erase(value.end() - 1);
      }

      auto index = variable.second;
      auto name = variable.first;

      bool is_vector = index != nullptr;

      if (is_vector)
      {
         var_scope_t::handle_t scope =
            ctx.proc_scope.get(ctx.proc_scope.get_type(name));

         syntax_error_if(
            !scope->is_defined(name),
            "'" + name + "' must be defined using DIM");

         size_t idx = index->eval(ctx).to_int();

         variant_t var = (*scope)[name];

         //check size
         syntax_error_if(
            idx >= var.vector_size(),
            "'" + name + "(" + nu::to_string(idx) + ")' is"
            "out of range");
         
         variant_t::type_t t = var.get_type();

         if (t==variant_t::type_t::UNDEFINED)
            t = variable_t::type_by_name(name);

         switch (t)
         {
            case variant_t::type_t::STRING:
               var.set_str(value, idx);
               break;

            case variant_t::type_t::FLOAT:
               var.set_real(nu::stof(value), idx);
               break;

            case variant_t::type_t::DOUBLE:
               var.set_double(nu::stod(value), idx);
               break;

            case variant_t::type_t::INTEGER:
               var.set_int(nu::stoi(value), idx);
               break;

            case variant_t::type_t::BYTEVECTOR:
               var.set_bvect(nu::stoi(value), idx);
               break;

            case variant_t::type_t::BOOLEAN:
               var.set_bool(strcasecmp(value.c_str(), "false") != 0 &&
                            strcasecmp(value.c_str(), "0") != 0, idx);
               break;

            case variant_t::type_t::LONG64:
               var.set_long64(nu::stoll(value), idx);
               break;

            case variant_t::type_t::UNDEFINED:
               syntax_error_if(
                  true,
                  "'" + name + "(" + nu::to_string(idx) + ")' is"
                  " undefined");
               break;
         }

         scope->define(name, var);
      }
      else
      {
         var_scope_t::handle_t scope =
            ctx.proc_scope.get(ctx.proc_scope.get_type(name));

         variant_t var = (*scope)[name];
         variant_t::type_t t = var.get_type();

         if (t == variant_t::type_t::UNDEFINED)
            t = variable_t::type_by_name(name);

         switch (t)
         {
            case variable_t::type_t::STRING:
               scope->define(name, value);
               break;

            case variable_t::type_t::DOUBLE:
               try
               {
                  scope->define(name, nu::stod(value));
               }
               catch (...)
               {
                  scope->define(name, double(0));
               }

               break;

            case variable_t::type_t::FLOAT:
               try
               {
                  scope->define(name, nu::stof(value));
               }
               catch (...)
               {
                  scope->define(name, float(0));
               }

               break;

            case variable_t::type_t::LONG64:
               try
               {
                  scope->define(name, nu::stoll(value));
               }
               catch (...)
               {
                  scope->define(name, 0);
               }

               break;

            case variable_t::type_t::BOOLEAN:
               try
               {
                  scope->define(name,
                                strcasecmp(value.c_str(),"false")!=0 &&
                                strcasecmp(value.c_str(), "0") != 0);
               }
               catch (...)
               {
                  scope->define(name, false);
               }

               break;

            case variable_t::type_t::INTEGER:
            default:
               try
               {
                  scope->define(name, nu::stoi(value));
               }
               catch (...)
               {
                  scope->define(name, 0);
               }

               break;
         }
      }
   }

   ctx.go_to_next();

}


/* -------------------------------------------------------------------------- */

} // namespace nu