//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_proc_scope.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

proc_scope_t::proc_scope_t()
    : _global_vars(std::make_shared<var_scope_t>())
{
}


/* -------------------------------------------------------------------------- */

const std::string& proc_scope_t::get_scope_id() const noexcept
{
    static std::string _empty_string;

    if (_scope_stack.empty())
        return _empty_string;

    return _scope_stack.front();
}


/* -------------------------------------------------------------------------- */

void proc_scope_t::clear()
{
    // Clear all global-variables
    _global_vars->clear();

    // Clear all scoped-variables
    for (auto& e : _vars)
        e.second->clear();

    _vars.clear();

    _scope_stack.clear();
    _rec_tbl.clear();
}


/* -------------------------------------------------------------------------- */

var_scope_t::handle_t proc_scope_t::get(type_t type) const noexcept
{
    if (type == type_t::GLOBAL)
        return _global_vars;

    const auto& scope_name = get_scope_id();
    auto i = _vars.find(scope_name);

    if (i == _vars.end())
        return _global_vars;

    return i->second;
}


/* -------------------------------------------------------------------------- */

void proc_scope_t::enter_scope(
    const std::string& sub_name, bool fncall) noexcept
{
    auto i = _rec_tbl.find(sub_name);

    int id = 0;

    if (i == _rec_tbl.end()) {
        auto value = std::make_pair(id, fncall);
        _rec_tbl.insert(std::make_pair(sub_name, value));
    } else {
        id = ++i->second.first;
    }

    const std::string rec_id = "[" + nu::to_string(id) + "]";

    const std::string scope_name = sub_name + rec_id;

    _scope_stack.push_front(scope_name);

    _vars.insert(std::make_pair(scope_name, std::make_shared<var_scope_t>()));
}


/* -------------------------------------------------------------------------- */

bool proc_scope_t::is_func_call(const std::string& sub_name) const
{
    auto i = _rec_tbl.find(sub_name);

    if (i == _rec_tbl.end())
        return false;

    return i->second.second;
}


/* -------------------------------------------------------------------------- */

void proc_scope_t::exit_scope() noexcept
{
    if (_scope_stack.empty())
        return;

    const auto& name = _scope_stack.front();

    _vars.erase(name);

    auto pos = name.find('[');

    int value = 0;

    decltype(_rec_tbl.begin()) i;

    if (int(pos) >= 0) {
        auto endpos = name.find(']');
        std::string svalue = name.substr(pos + 1, endpos - pos - 1);
        value = nu::stoi(svalue);
        i = _rec_tbl.find(name.substr(0, pos));
    } else {
        i = _rec_tbl.find(name);
    }

    if (i != _rec_tbl.end()) {
        i->second.first = value - 1;

        if (i->second.first < 0)
            _rec_tbl.erase(i);
    }

    _scope_stack.pop_front();
}


/* -------------------------------------------------------------------------- */

proc_scope_t::type_t proc_scope_t::get_type(const std::string& varname) const
    noexcept
{
    bool global_var = _global_vars->is_defined(varname);

    const auto& scope_name = get_scope_id();

    // Test local scope
    if (!scope_name.empty()) {
        // Search variable in the local scope
        auto i = _vars.find(scope_name);

        if (i != _vars.end() && i->second->is_defined(varname))
            return type_t::LOCAL;
    }

    return global_var ? type_t::GLOBAL : type_t::UNDEF;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
