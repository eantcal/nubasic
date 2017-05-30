//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_icstring.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

icstring_t& icstring_t::operator=(icstring_t&& s) noexcept
{
    if (this != &s)
        _data = std::move(s._data);

    return *this;
}


/* -------------------------------------------------------------------------- */

size_t icstring_t::find(std::string searching_s)
{
    std::string s = _data;

    std::transform(s.begin(), s.end(), s.begin(), ::tolower);

    std::transform(
        searching_s.begin(), searching_s.end(), searching_s.begin(), ::tolower);

    return s.find(searching_s);
}


/* -------------------------------------------------------------------------- */

bool icstring_t::operator<(const icstring_t& s) const noexcept
{
    if (s._data.empty())
        return false;

    if (_data.empty())
        return true;

    return strcasecmp(_data.c_str(), s._data.c_str()) < 0;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
