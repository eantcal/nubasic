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

#ifndef __NU_FLAG_MAP_H__
#define __NU_FLAG_MAP_H__


/* -------------------------------------------------------------------------- */

#include <map>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu
{

/* -------------------------------------------------------------------------- */

class flag_map_t
{
public:
   flag_map_t() = default;
   flag_map_t(const flag_map_t&) = default;
   flag_map_t& operator=( const flag_map_t& ) = default;


   void reset_all() NU_NOEXCEPT
   {
      _data = 0;
   }


   void define(int id, bool val = false) NU_NOEXCEPT
   {
      assert(size_t(id) < sizeof(_data) * 8);

      _mask |= ( 1LL << id );

      if ( val )
         _data |= _mask;

      else
         _data &= ~_mask;
   }


   bool get(int id) const NU_NOEXCEPT
   {
      assert(size_t(id) < sizeof(_data) * 8);

      decltype( _data ) idmask = 1LL << id;

      if ( ( _mask & idmask ) == 0 )
         return false;

      return ( ( _data & idmask ) != 0 );
   }


   inline bool operator[](int id) const NU_NOEXCEPT
   {
      return get(id);
   }


   void set(int id, bool value)
   {
      assert(size_t(id) < sizeof(_data) * 8);

      decltype( _data ) idmask = 1 << id;

      if ( ( _mask & idmask ) == 0 )
         return;

      if ( value )
         _data |= idmask;

      else
         _data &= ~idmask;
   }


private:
   size_t _mask = 0;
   size_t _data = 0;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_FLAG_MAP_H__
