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

#ifndef __NU_EXPR_VAR_H__
#define __NU_EXPR_VAR_H__


/* -------------------------------------------------------------------------- */

#include "nu_rt_prog_ctx.h"
#include "nu_expr_any.h"
#include "nu_var_scope.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class expr_var_t : public expr_any_t
{
public:
   expr_var_t(const std::string& name) : _name(name)
   {}

   expr_var_t() = delete;
   expr_var_t(const expr_var_t&) = default;
   expr_var_t& operator=( const expr_var_t& ) = default;


   variant_t eval(rt_prog_ctx_t & ctx) const override
   {
      //NOTE: if symbol <name> does not exist,
      //it will be create on-the-fly within current
      //(subroutine) scope

      auto var_scope_type = ctx.proc_scope.get_type(_name);

      var_scope_t::handle_t scope = ctx.proc_scope.get(var_scope_type);

      if ( !scope->is_defined(_name) )
      {
         auto vtype = variable_t::type_by_name(_name);
         variant_t value("", vtype, 0);
         scope->define(_name, value);
      }

      return ( *scope )[_name];
   }


   virtual bool empty() const throw( ) override
   {
      return false;
   }


   const std::string& name() const throw( )
   {
      return _name;
   }

protected:
   std::string _name;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif
