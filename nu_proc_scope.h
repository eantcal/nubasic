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

#ifndef __NU_PROC_SCOPE_H__
#define __NU_PROC_SCOPE_H__


/* -------------------------------------------------------------------------- */

#include "nu_var_scope.h"

#include <algorithm>
#include <deque>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

/**
   * This class holds the scope of each running procedure.
   * Procedure scopes are stacked. Every time a procedure
   * is called it enters a new scope (old one is preserved in
   * LIFO order). The procedure exit the scope before it returns.
   */
class proc_scope_t
{
public:
   enum class type_t
   {
      GLOBAL, LOCAL, UNDEF
   };

   proc_scope_t(const proc_scope_t&) = delete;
   proc_scope_t& operator=( const proc_scope_t& ) = delete;

   //! Default ctor
   proc_scope_t();

   //! Get current scope identifier
   //! Global scope has no-name (empty string)
   const std::string& get_scope_id() const throw( );

   //! Clear all scopes and corrisponding variables
   void clear();

   //! Enter a new scope
   void enter_scope(const std::string& sub_name, bool fncall) throw( );

   //! Exit a scope
   void exit_scope() throw( );

   //! Retrieve scope-type info of a variable
   type_t get_type(const std::string& varname) const throw( );

   //! Get current LOCAL or GLOBAL scope
   var_scope_t::handle_t get(type_t type = type_t::LOCAL) const throw( );

   //! Return true for function call, false else
   bool is_func_call(const std::string& sub_name) const;

private:
   std::deque< std::string > _scope_stack;
   std::map < std::string, var_scope_t::handle_t > _vars;

   using rec_cnt_fncflg_t = std::pair < int, bool > ;
   std::map < std::string, rec_cnt_fncflg_t > _rec_tbl;

   var_scope_t::handle_t _global_vars;    // variables
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_PROC_SCOPE_H__
