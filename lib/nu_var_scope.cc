//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_var_scope.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void var_scope_t::get_err_msg(const std::string& key, std::string& err) const
{
    err = "'" + key + "' out of scope";
}


/* -------------------------------------------------------------------------- */

bool var_scope_t::define(const std::string& name, const var_value_t& value)
{
    map()[name]=value; //TODO
    return true;
}


/* -------------------------------------------------------------------------- */

std::stringstream& operator<<(std::stringstream& ss, var_scope_t& obj)
{
    for (const auto& e : obj.map()) {
        ss << "\t" << e.first.str() << ": "
           << " " << ((e.second.second & VAR_ACCESS_RO) ? " const " : "")
           << e.second.first << std::endl;
    }

    return ss;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
