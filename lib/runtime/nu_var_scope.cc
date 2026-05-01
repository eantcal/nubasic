//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_var_scope.h"

#include <algorithm>


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
    if (!is_defined(name)) {
        _definition_order.push_back(name);
    }

    map()[name] = value;
    return true;
}


/* -------------------------------------------------------------------------- */

void var_scope_t::erase(const std::string& name)
{
    symbol_map_t<icstring_t, var_value_t>::erase(name);

    _definition_order.erase(
        std::remove(_definition_order.begin(), _definition_order.end(), name),
        _definition_order.end());
}


/* -------------------------------------------------------------------------- */

void var_scope_t::clear()
{
    symbol_map_t<icstring_t, var_value_t>::clear();
    _definition_order.clear();
}


/* -------------------------------------------------------------------------- */

std::vector<std::string> var_scope_t::names_in_reverse_definition_order() const
{
    std::vector<std::string> names;
    names.reserve(_definition_order.size());

    for (auto it = _definition_order.rbegin(); it != _definition_order.rend();
        ++it) {
        if (is_defined(*it)) {
            names.push_back(*it);
        }
    }

    return names;
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
