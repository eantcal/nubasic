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

#ifndef __NU_ICSTRING_H__
#define __NU_ICSTRING_H__


/* -------------------------------------------------------------------------- */

#include <algorithm>
#include <string.h>
#include <string>

#include "nu_os_std.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class icstring_t {
public:
    icstring_t(const std::string& str = std::string()) noexcept : _data(str) {}


    icstring_t(const char* str) noexcept : _data(str) {}


    icstring_t(const icstring_t&) = default;


    icstring_t& operator=(const icstring_t&) = default;


    icstring_t(icstring_t&& s) noexcept { _data = std::move(s._data); }


    icstring_t& operator=(icstring_t&& s) noexcept;


    size_t find(std::string searching_s);


    bool operator<(const icstring_t& s) const noexcept;


    bool operator>=(const icstring_t& s) const noexcept
    {
        return !(this->operator<(s));
    }


    bool operator<=(const icstring_t& s) const noexcept
    {
        return !(s.operator<(*this));
    }


    bool operator==(const icstring_t& s) const noexcept
    {
        return this->operator<=(s) && this->operator>=(s);
    }


    bool operator!=(const icstring_t& s) const noexcept
    {
        return !this->operator==(s);
    }


    bool operator>(const icstring_t& s) const noexcept
    {
        return s.operator<(*this);
    }


    explicit operator std::string() const noexcept { return _data; }


    const std::string& str() const noexcept { return _data; }


    std::string&& str() noexcept { return std::move(_data); }


    size_t size() const noexcept { return _data.size(); }


    bool empty() const noexcept { return _data.empty(); }


    void clear() noexcept { _data.clear(); }


protected:
    std::string _data;
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_ICSTRING_H__
