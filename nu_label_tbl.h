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

#ifndef __NU_LABEL_TBL_H__
#define __NU_LABEL_TBL_H__


/* -------------------------------------------------------------------------- */

#include "nu_variant.h"
#include "nu_exception.h"
#include "nu_symbol_map.h"
#include "nu_icstring.h"
#include <algorithm>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class label_tbl_t : public symbol_map_t < icstring_t, unsigned int >
{
public:
   label_tbl_t(const label_tbl_t&) = delete;
   label_tbl_t& operator=( const label_tbl_t& ) = delete;

   void set_explicit_line_reference_mode() NU_NOEXCEPT
   {
      _explicit_line_reference = true;
   }

   bool get_explicit_line_reference_mode() const NU_NOEXCEPT
   {
      return _explicit_line_reference;
   }

   void clear()
   {
      symbol_map_t<icstring_t, unsigned int>::clear();
      _explicit_line_reference = false;
   }


   label_tbl_t() = default;

protected:
   bool _explicit_line_reference = false;

   virtual void get_err_msg(const std::string& key, std::string& err) const;

};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_LABEL_TBL_H__
