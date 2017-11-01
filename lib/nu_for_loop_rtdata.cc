//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_for_loop_rtdata.h"
#include <set>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void for_loop_rtdata_t::trace(std::stringstream& ss)
{
    ss << "For-loop:\n";

    for (auto e : *this) {
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

    for (auto i = cbegin(); i != cend(); ++i) {
        const auto& key = i->first;

        if (key.find(proc_ctx) == 0) {
            remove_list.insert(key);
        }
    }

    for (const auto& e : remove_list) {
        erase(e);
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
