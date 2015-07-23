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

#include "nu_variable.h"
#include "nu_exception.h"

#include <algorithm>
#include <cassert>
#include <set>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

variable_t::type_t variable_t::type_by_typename(std::string name)
{
   std::transform(name.begin(), name.end(), name.begin(), tolower);

   if (name == "integer")
      return type_t::INTEGER;

   if ( name == "long64" )
      return type_t::LONG64;

   if (name == "string")
      return type_t::STRING;

   //if (name == "BYTE")
   //    return type_t::BYTE;

   if (name == "double")
      return type_t::DOUBLE;

   if (name == "float")
      return type_t::FLOAT;

   if (name == "boolean")
      return type_t::BOOLEAN;

   return type_t::UNDEFINED;
}


/* -------------------------------------------------------------------------- */

std::string variable_t::typename_by_type(variable_t::type_t type)
{
   switch (type)
   {
      case type_t::INTEGER:
         return "Integer";

      case type_t::STRING:
         return "String";

      case type_t::BOOLEAN:
         return "Boolean";

      case type_t::FLOAT:
         return "Float";

      case type_t::LONG64:
         return "Long64";

      case type_t::BYTEVECTOR:
         return "ByteVect";

      case type_t::DOUBLE:
         return "Double";

      case type_t::UNDEFINED:
         break;
   }

   return "Undefined";
}


/* -------------------------------------------------------------------------- */

variable_t::type_t variable_t::type_by_name(const std::string& name)
{
   assert(name.size() > 0);

   const char ch = *name.rbegin();

   switch (ch)
   {
      case '%':
         return type_t::INTEGER;

      case '$':
         return type_t::STRING;

      case '@':
         return type_t::BYTEVECTOR;

      case '!':
         return type_t::DOUBLE;

      case '&':
         return type_t::LONG64;

      case '#':
         return type_t::BOOLEAN;
   }

   return type_t::FLOAT;
}


/* -------------------------------------------------------------------------- */

bool variable_t::is_valid_name(std::string name)
{
   if (name.empty())
      return false;

   std::transform(name.begin(), name.end(), name.begin(), tolower);

   if (reserved_keywords_t::list.find(name) != reserved_keywords_t::list.cend())
      return false;

   auto letter = [](char c)
   {
      return c >= 'a' && c <= 'z';
   };

   char first_char = name.c_str()[0];

   if (!letter(first_char) && first_char != '_')
      return false;

   if (name.size() == 1)
      return first_char != '_';

   auto number = [](char c)
   {
      return(c >= '0' && c <= '9');
   };

   for (size_t i = 1; i < name.size(); ++i)
   {
      char c = name.c_str()[i];

      bool valid = letter(c) ||
                   c == '_' ||
                   number(c) ||
                   (i == (name.size() - 1) &&
                    (c == '%' || c == '$' || c== '@' || c=='!' || c == '&' ||  c=='#'));

      if (!valid)
         return false;
   }

   return true;
}


/* -------------------------------------------------------------------------- */

} // namespace nu