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

#ifndef __NU_FOR_LOOP_RTDATA_H__
#define __NU_FOR_LOOP_RTDATA_H__


/* -------------------------------------------------------------------------- */

#include "nu_prog_pointer.h"
#include "nu_flag_map.h"
#include "nu_variant.h"

#include <sstream>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

struct for_loop_ctx_t
{
   enum
   {
      FLG_FIRST_EXEC
   };

   flag_map_t flag;

   for_loop_ctx_t() throw( )
   {
      flag.define(FLG_FIRST_EXEC, true);
   }

   for_loop_ctx_t(const for_loop_ctx_t&) = default;
   for_loop_ctx_t& operator=( const for_loop_ctx_t& ) = default;

   prog_pointer_t pc_for_stmt;
   prog_pointer_t pc_next_stmt;

   variant_t step;
   variant_t end_counter;
};


/* -------------------------------------------------------------------------- */

struct for_loop_rtdata_t : public std::map < std::string, for_loop_ctx_t >
{
   void trace(std::stringstream& ss);
   void cleanup_data(const std::string& proc_ctx);
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_FOR_LOOP_RTDATA_H__
