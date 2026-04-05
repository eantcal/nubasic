//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_const.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_const_t::run(rt_prog_ctx_t& ctx)
{
   const variant_t value = _arg->eval(ctx);
   const std::string& name = _var;

   const auto scope_type = ctx.proc_scope.get_type(name);

   switch (scope_type) {
   case proc_scope_t::type_t::LOCAL:
   case proc_scope_t::type_t::GLOBAL:
      rt_error_code_t::get_instance().throw_if(true,
         ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_VAR_REDEF,
         "Variable '" + name + "'");
      break;

   case proc_scope_t::type_t::UNDEF:
   default:
      break;
   }

   var_scope_t::handle_t scope = ctx.proc_scope.get_global();

   const auto vtype_code = variable_t::type_by_typename(_vtype);
   const auto value_type = value.get_type();

   std::string init_val;

   switch (vtype_code) {
   case variable_t::type_t::STRING:
      rt_error_code_t::get_instance().throw_if(
         value_type != variable_t::type_t::STRING, ctx.runtime_pc.get_line(),
         rt_error_code_t::value_t::E_TYPE_ILLEGAL, "'" + name + "'");
      init_val = value.to_str();
      scope->define(name,
         var_value_t(variant_t(init_val, variable_t::type_t::STRING, 0), VAR_ACCESS_RO));
      break;

   case variable_t::type_t::DOUBLE:
   case variable_t::type_t::INTEGER:
   case variable_t::type_t::BOOLEAN:
      init_val = value.to_str();
      scope->define(name,
         var_value_t(variant_t(init_val, vtype_code, 0), VAR_ACCESS_RO));
      break;

   case variable_t::type_t::STRUCT:
   case variable_t::type_t::BYTEVECTOR:
   case variable_t::type_t::UNDEFINED:
   default:
      rt_error_code_t::get_instance().throw_if(true,
         ctx.runtime_pc.get_line(), rt_error_code_t::value_t::E_TYPE_ILLEGAL,
         "'" + name + "'");
   }


   ctx.go_to_next();
}


/* -------------------------------------------------------------------------- */

} // namespace nu
