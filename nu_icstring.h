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

#ifndef __NU_ICSTRING_H__
#define __NU_ICSTRING_H__


/* -------------------------------------------------------------------------- */

#include <string>
#include <string.h>
#include <algorithm>

#include "nu_os_std.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class icstring_t
{
public:
   icstring_t(const std::string& str = std::string()) throw( )
      : _data(str)
   {}


   icstring_t(const char* str) throw( ) :
      _data(str)
   {}


   icstring_t(const icstring_t&) = default;


   icstring_t& operator=( const icstring_t& ) = default;


   icstring_t(icstring_t&& s) throw( )
   {
      _data = std::move(s._data);
   }


   icstring_t& operator=( icstring_t&& s ) throw( );


   size_t find(std::string searching_s);


   bool operator<( const icstring_t& s ) const throw( );


   bool operator>=( const icstring_t& s ) const throw( )
   {
      return !( this->operator<( s ) );
   }


   bool operator<=( const icstring_t& s ) const throw( )
   {
      return !( s.operator<( *this ) );
   }


   bool operator==( const icstring_t& s ) const throw( )
   {
      return this->operator<=( s ) && this->operator>=( s );
   }


   bool operator!=( const icstring_t& s ) const throw( )
   {
      return !this->operator==( s );
   }


   bool operator>( const icstring_t& s ) const throw( )
   {
      return s.operator<( *this );
   }


   explicit operator std::string() const throw( )
   {
      return _data;
   }


   const std::string& str() const throw( )
   {
      return _data;
   }


   std::string&& str() throw( )
   {
      return std::move(_data);
   }


   size_t size() const throw( )
   {
      return _data.size();
   }


   bool empty() const throw( )
   {
      return _data.empty();
   }


   void clear() throw( )
   {
      _data.clear();
   }


protected:
   std::string _data;
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_ICSTRING_H__
