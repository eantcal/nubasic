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
#include "nu_stmt_call.h"
#include "nu_error_codes.h"
#include "nu_global_function_tbl.h"
#include "nu_expr_function.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

void stmt_call_t::run(rt_prog_ctx_t& ctx)
{
   run(ctx, ctx.runtime_pc.get_line());
}


/* -------------------------------------------------------------------------- */

void stmt_call_t::run(
   rt_prog_ctx_t& ctx,
   const prog_pointer_t::line_number_t& line)
{

   auto rt_error_if = [&](
                         bool cond,
                         rt_error_code_t::value_t err,
                         const std::string& desc)
   {
      rt_error_code_t::get_instance().throw_if(
         cond,
         ctx.runtime_pc.get_line(),
         err,
         desc);
   };

   auto i = ctx.proc_prototypes.data.find(_name);

   bool undef = false;

   // First verify if "CALL" statement is applied to built-in function
   // (however its return value will be ignored, but function will be executed
   //  like a procedure)
   if (i == ctx.proc_prototypes.data.end())
   {
      auto & gf = global_function_tbl_t::get_instance();
      undef = ! gf.is_defined(_name);

      if (undef == false)
      {
         func_args_t function_args;

         for (const auto & arg : _args)
            function_args.push_back(arg.first);

         expr_any_t::handle_t fhandle(
            std::make_shared<expr_function_t>(
               _name, function_args));

         assert(fhandle);

         fhandle->eval(ctx);

         ctx.go_to_next();

         return;
      }
   }

   rt_error_if(
      undef,
      rt_error_code_t::E_SUB_UNDEF,
      _name);

   auto & argument_list = i->second.second;

   rt_error_if(
      argument_list.parameters.size() != _args.size(),
      rt_error_code_t::E_WRG_NUM_ARGS,
      _name);

   //Evaluate sub arguments
   std::list<variant_t> values;

   for (auto arg : _args)
   {
      variant_t val = 
         arg.first == nullptr ? 
            variant_t("") : arg.first->eval(ctx);

      values.push_back(val);
   }


   // Enter calling-subroutine scope
   ctx.set_return_line( std::make_pair( line, get_stmt_id()) );
   ctx.go_to(i->second.first);
   ctx.proc_scope.enter_scope(_name, _fncall);

   auto arg_it = argument_list.parameters.begin();

   if (! values.empty())
   {
      auto sub_xscope = ctx.proc_scope.get();
      auto values_it = values.begin();

      for (auto arg : _args)
      {
         variant_t val = *values_it;

         const auto & variable_name = arg_it->var_name;
         const auto & variable_type = arg_it->type_name;

         int vsize = 0;

         if (arg_it->vsize)
         {
            auto size = arg_it->vsize->eval(ctx);
            vsize = size.to_int();
         }

         auto var_type = variable_type.empty() ?
            variable_t::type_by_name(variable_name) :
            variable_t::type_by_typename(variable_type); // TODO struct

         rt_error_if(
            vsize && (!val.is_vector() || int(val.vector_size()) != vsize),
            rt_error_code_t::E_TYPE_MISMATCH,
            _name + ": '" + variable_name + "' array ");

         switch (val.get_type())
         {
            case variant_t::type_t::INTEGER:
            case variant_t::type_t::LONG64:
            case variant_t::type_t::FLOAT:
            case variant_t::type_t::DOUBLE:
            case variant_t::type_t::BOOLEAN:
               rt_error_if(
                  ! variable_t::is_number( var_type ),
                  rt_error_code_t::E_TYPE_MISMATCH,
                  _name + ": Parameter '" + variable_name + "'");
               break;

            case variant_t::type_t::BYTEVECTOR:
               rt_error_if(
                  var_type != variable_t::type_t::BYTEVECTOR,
                  rt_error_code_t::E_TYPE_MISMATCH,
                  _name + ", Parameter '" + variable_name + "'");
               break;

            case variant_t::type_t::STRING:
            default:
               if (val.get_type() == variant_t::type_t::UNDEFINED)
               {
                  rt_error_if(
                     true,
                     rt_error_code_t::E_TYPE_ILLEGAL,
                     _name + ", Parameter '" + variable_name + "'");
               }
               else
               {
                  rt_error_if(
                     var_type != variable_t::type_t::STRING,
                     rt_error_code_t::E_TYPE_MISMATCH,
                     _name + ", Parameter '" + variable_name+"'");
               }

               break;
         } //switch

         sub_xscope->define(variable_name, var_value_t(val, VAR_ACCESS_RW));

         ++arg_it;
         ++values_it;

      } //for
   }

}


/* -------------------------------------------------------------------------- */

} // namespace nu
