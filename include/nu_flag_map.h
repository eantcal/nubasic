//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_FLAG_MAP_H__
#define __NU_FLAG_MAP_H__


/* -------------------------------------------------------------------------- */

#include <cassert>
#include <map>


/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

class flag_map_t {
public:
    flag_map_t() = default;
    flag_map_t(const flag_map_t&) = default;
    flag_map_t& operator=(const flag_map_t&) = default;


    void reset_all() noexcept { _data = 0; }


    void define(int id, bool val = false) noexcept
    {
        assert(size_t(id) < sizeof(_data) * 8);

        _mask |= (1LL << id);

        if (val)
            _data |= _mask;

        else
            _data &= ~_mask;
    }


    bool get(int id) const noexcept
    {
        assert(size_t(id) < sizeof(_data) * 8);

        decltype(_data) idmask = 1LL << id;

        if ((_mask & idmask) == 0)
            return false;

        return ((_data & idmask) != 0);
    }


    inline bool operator[](int id) const noexcept { return get(id); }


    void set(int id, bool value)
    {
        assert(size_t(id) < sizeof(_data) * 8);

        decltype(_data) idmask = 1ULL << id;

        if ((_mask & idmask) == 0)
            return;

        if (value)
            _data |= idmask;

        else
            _data &= ~idmask;
    }


private:
    size_t _mask = 0;
    size_t _data = 0;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif // __NU_FLAG_MAP_H__
