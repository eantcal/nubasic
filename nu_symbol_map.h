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

#ifndef __NU_SYMBOL_MAP_H__
#define __NU_SYMBOL_MAP_H__


/* -------------------------------------------------------------------------- */

#include <map>
#include <sstream>


/* -------------------------------------------------------------------------- */

namespace nu
{
   

/* -------------------------------------------------------------------------- */

template  <typename String, typename Symb>
class symbol_map_t
{
public:
   using key_t = String;
   using symbol_t = Symb;

   symbol_map_t() = default;
   symbol_map_t(const symbol_map_t&) = default;
   symbol_map_t& operator=(const symbol_map_t&) = delete;

   virtual ~symbol_map_t() {}


   symbol_map_t(symbol_map_t&& obj)
   {
      _symbols = std::move(obj);
   }


   symbol_map_t& operator=(symbol_map_t&& obj)
   {
      if (this != &obj)
         _symbols = std::move(obj);

      return *this;
   }


   virtual bool define(const std::string& name, const Symb& value)
   {
      auto i = map().insert(std::make_pair(name, Symb()));
      (*(i.first)).second = value;
      return i.second;
   }


   virtual void erase(const std::string& name)
   {
      map().erase(name);
   }


   bool is_defined(const std::string& name) const NU_NOEXCEPT
   {
      return _symbols.find(name) != _symbols.end();
   }


   Symb& operator[] (const std::string& name)
   {
      return _symbols[name];
   }


   const Symb& operator[] (const std::string& name) const
   {
      auto i = map().find(name);

      if (i == map().end())
      {
         std::string err;

         get_err_msg(name, err);

         throw exception_t(err);
      }

      return i->second;
   }


   bool empty() const NU_NOEXCEPT
   {
      return _symbols.empty();
   }


   size_t size() const NU_NOEXCEPT
   {
      return _symbols.size();
   }


   const std::map< String, Symb >& map() const NU_NOEXCEPT
   {
      return _symbols;
   }


   virtual void clear()
   {
      _symbols.clear();
   }


   friend std::stringstream& operator<<(
      std::stringstream& ss,
      symbol_map_t<String, Symb>& obj)
   {
      for (const auto & e : obj.map())
         ss << "\t" << e.first.str() <<": " << e.second << std::endl;

      return ss;
   }


protected:

   std::map< String, Symb >& map() NU_NOEXCEPT
   {
      return _symbols;
   }


   virtual void get_err_msg(
      const std::string& key,
      std::string& err) const = 0;


private:

   std::map< String, Symb > _symbols;

};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif
