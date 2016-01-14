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

#include "nu_for_loop_rtdata.h"
#include <set>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

void for_loop_rtdata_t::trace(std::stringstream& ss)
{
   ss << "For-loop:\n";

   for (auto e : *this)
   {
      ss << "\tid=<" << e.first << ">";
      ss << " For-line:" << e.second.pc_for_stmt.get_line();
      ss << " For-stmt:" << e.second.pc_for_stmt.get_stmt_pos();
      ss << " Next-line:" << e.second.pc_next_stmt.get_line();
      ss << " Next-stmt:" << e.second.pc_next_stmt.get_stmt_pos();
      ss << " step:" << e.second.step.to_str();
      ss << " endc:" << e.second.end_counter.to_str() << std::endl;
   }
}


/* -------------------------------------------------------------------------- */

void for_loop_rtdata_t::cleanup_data(const std::string& proc_ctx)
{
   std::set<std::string> remove_list;

   for (auto i = cbegin(); i != cend(); ++i)
   {
      const auto & key = i->first;

      if (key.find(proc_ctx) == 0)
         remove_list.insert(key);
   }

   for (const auto & e : remove_list)
      erase(e);
}


/* -------------------------------------------------------------------------- */

} // namespace nu

