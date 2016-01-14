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

#ifndef __NU_PROG_POINTER_H__
#define __NU_PROG_POINTER_H__


/* -------------------------------------------------------------------------- */

#include <deque>
#include <sstream>

#include "nu_cpp_lang.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class prog_pointer_t
{
public:
   using line_number_t = int;
   using stmt_number_t = int;

   prog_pointer_t(
      line_number_t line = 0,
      stmt_number_t stmt = 0) NU_NOEXCEPT
      :
      _line(line),
      _line_stmt(stmt)
   {}

   prog_pointer_t(const prog_pointer_t&) = default;
   prog_pointer_t& operator=( const prog_pointer_t& ) = default;


   void go_to(line_number_t line, stmt_number_t stmt = 0) NU_NOEXCEPT
   {
      set(line, stmt);
   }


   void set(line_number_t line, stmt_number_t stmt) NU_NOEXCEPT
   {
      _last_line = _line;
      _line = line;
      _line_stmt = stmt;
   }


   void set_stmt_pos(stmt_number_t stmt)
   {
      _line_stmt = stmt;
   }


   line_number_t get_line() const NU_NOEXCEPT
   {
      return _line;
   }


   line_number_t get_last_line() const NU_NOEXCEPT
   {
      return _last_line;
   }


   int get_stmt_pos() const NU_NOEXCEPT
   {
      return _line_stmt;
   }


   void reset() NU_NOEXCEPT
   {
      set(0, 0);
   }


   bool operator<( const prog_pointer_t& pp ) const NU_NOEXCEPT
   {
      return _line > pp._line || ( _line == pp._line && _line_stmt > pp._line_stmt );
   }


   std::string to_string() const NU_NOEXCEPT
   {
      return std::to_string(_line) + ":" + std::to_string(_line_stmt);
   }

private:
   line_number_t _line = 0;
   stmt_number_t _line_stmt = 0;
   stmt_number_t _last_line = 0;
};


/* -------------------------------------------------------------------------- */

struct return_stack_t :
   public std::deque <
   std::pair <
   prog_pointer_t::line_number_t,
   prog_pointer_t::stmt_number_t > >
{
   void trace(std::stringstream& ss);
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_PROG_POINTER_H__