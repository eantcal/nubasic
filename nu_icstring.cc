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

#include "nu_icstring.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

icstring_t& icstring_t::operator=(icstring_t&& s) throw()
{
   if (this != &s)
      _data = std::move(s._data);

   return *this;
}


/* -------------------------------------------------------------------------- */

size_t icstring_t::find(std::string searching_s)
{
   std::string s = _data;

   std::transform(s.begin(), s.end(), s.begin(), ::tolower);

   std::transform(
      searching_s.begin(),
      searching_s.end(),
      searching_s.begin(),
      ::tolower);

   return s.find(searching_s);
}


/* -------------------------------------------------------------------------- */

bool icstring_t::operator<(const icstring_t& s) const throw()
{
   if (s._data.empty())
      return false;

   if (_data.empty())
      return true;

   return strcasecmp(_data.c_str(), s._data.c_str()) < 0;
}


/* -------------------------------------------------------------------------- */

} // namespace nu

