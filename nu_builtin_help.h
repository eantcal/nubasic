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

#ifndef __NU_BUILTIN_HELP_H__
#define __NU_BUILTIN_HELP_H__


/* -------------------------------------------------------------------------- */

#include "nu_variant.h"
#include "nu_exception.h"
#include "nu_expr_any.h"
#include "nu_symbol_map.h"
#include "nu_icstring.h"
#include "nu_prog_ctx.h"

#include <functional>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu
{

enum class lang_item_t : int
{
   COMMAND, INSTRUCTION, FUNCTION, OPERATOR
};


/* -------------------------------------------------------------------------- */

using help_item_t = std::pair < lang_item_t, icstring_t > ;


/* -------------------------------------------------------------------------- */

class builtin_help_t :
   protected std::map < help_item_t, std::pair<std::string, std::string> >
{
private:
   builtin_help_t() = default;
   builtin_help_t(const builtin_help_t&) = delete;
   builtin_help_t& operator=( const builtin_help_t& ) = delete;

   static builtin_help_t* _instance;

   virtual void get_err_msg(
      const std::string& key,
      std::string& err) const
   {
      err = "item '";
      err += key + "' not found";
   }

public:
   std::string help(const std::string& s_item = "");
   std::string apropos(const std::string& s_item);
   static builtin_help_t& get_instance();
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_BUILTIN_HELP__
