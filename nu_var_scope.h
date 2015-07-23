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

#ifndef __NU_VAR_SCOPE_H__
#define __NU_VAR_SCOPE_H__


/* -------------------------------------------------------------------------- */

#include "nu_variant.h"
#include "nu_symbol_map.h"
#include "nu_icstring.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

/**
 * This class holds the value of variables that belong to the same
 * definition scope
 */
class var_scope_t : public symbol_map_t < icstring_t, variant_t >
{
public:
   using handle_t = std::shared_ptr < var_scope_t > ;

   var_scope_t() = default;
   var_scope_t(const var_scope_t&) = default;
   var_scope_t& operator=(const var_scope_t&) = default;

   bool define(const std::string& name, const variant_t& value) override;
   friend std::stringstream& operator<<(std::stringstream& ss, var_scope_t& obj);

protected:
   virtual void get_err_msg(const std::string& key, std::string& err) const;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_VAR_SCOPE_H__
