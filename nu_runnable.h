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

#ifndef __NU_RUNNABLE_H__
#define __NU_RUNNABLE_H__


/* -------------------------------------------------------------------------- */

#include "nu_signal_handling.h"
#include "nu_prog_pointer.h"
#include "nu_expr_any.h"

#include <string>
#include <map>
#include <cstdio>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class runnable_t : protected signal_handler_t
{
public:

   using line_num_t = prog_pointer_t::line_number_t;
   using stmt_num_t = prog_pointer_t::stmt_number_t;


   runnable_t()
   {
      signal_mgr_t::instance().register_handler(event_t::BREAK, this);
   }


   virtual ~runnable_t()
   {
      signal_mgr_t::instance().unregister_handler(event_t::BREAK, this);
   }


   virtual bool run(line_num_t start_from = 0) = 0;


   virtual bool cont(line_num_t start_from, stmt_num_t stmt_id) = 0;


   virtual bool run(
      const std::string& name,
      const std::vector<expr_any_t::handle_t>& args) = 0;


protected:

   virtual bool notify(const event_t& ev) override
   {
      _break_event = ev == event_t::BREAK;
      return true;
   }


   bool break_event() const throw( )
   {
      return _break_event;
   }


   void reset_break_event() throw( )
   {
      _break_event = false;
   }


   void set_break_event() throw( )
   {
      _break_event = true;
   }


private:


   bool _break_event = false;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_RUNNABLE_H__

