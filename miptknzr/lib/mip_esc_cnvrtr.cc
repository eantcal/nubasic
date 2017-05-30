//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#include "mip_esc_cnvrtr.h"

#include <memory>
#include <istream>


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

void esc_cnvrtr_t::_tail(string_t & str, size_t cnt)
{
    if (str.size() <= cnt) {
        str.clear();
    }
    else {
        str = str.substr(cnt, str.size() - cnt);
    }
}


/* -------------------------------------------------------------------------- */

bool esc_cnvrtr_t::_octal2dec(const string_t & str, unsigned int& res, size_t & cnt)
{
    if (str.size() < 2) {
        return false;
    }

    cnt = str.size();
    if (cnt > 3) {
        cnt = 3;
    }

    res = 0;
    for (size_t i = 0; i < cnt; ++i) {

        auto ch = str[i];

        if (ch<_T('0') || ch>_T('7')) {
            return false;
        }

        res += (ch - _T('0')) << (3 * (cnt - i - 1));
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool esc_cnvrtr_t::_hex2dec(const string_t & str, unsigned int& res, size_t & cnt)
{
    if (str.size() < 2 || (str[0] != 'x' && str[0] != 'X')) {
        return false;
    }

    cnt = str.size();
    string_t hex = str.substr(1, cnt - 1);

    size_t i = 0;
    for (; i < cnt; ++i) {
        const auto ch = ::tolower(hex[i]);

        bool ok = (ch >= _T('0') && ch <= _T('9')) || 
                  (ch >= _T('a') && ch < _T('f'));
        if (!ok)
            break;
    }

    hex = hex.substr(0, i);
    cnt = i + 1;

    try {
        res = std::stoi(hex, 0, 16);
    }
    catch (std::exception&) {
        return false;
    }

    return true;
}


/* -------------------------------------------------------------------------- */

bool esc_cnvrtr_t::convert(const string_t& str, size_t & rcnt, char_t & ch) const
{
    if (str.size() <= 1) {
        return false;
    }

    const char_t prefix = str[1];
    rcnt = 2;

    switch (prefix) {
    case _T('\''):
        ch = _T('\'');
        break;
    case _T('"'):
        ch = _T('"');
        break;
    case _T('\\'):
        ch = _T('\\');
        break;
    case _T('n'):
        ch = _T('\n');
        break;
    case _T('r'):
        ch = _T('\r');
        break;
    case _T('t'):
        ch = _T('\t');
        break;
    case _T('b'):
        ch = _T('\b');
        break;
    case _T('f'):
        ch = _T('\f');
        break;
#if 0
    case _T('a'):
        ch = _T('\a');
        break;
    case _T('?'):
        ch = _T('?');
        break;
    case _T('0'):
        ch = _T('\0');
        break;
#endif
    default:
        if (str.size() >= 3 && (str[1] >= _T('0') && str[1] <= _T('7'))) {
            unsigned int res = 0;
            const bool ok = _octal2dec(str.c_str() + 1, res, rcnt);

            if (!ok) {
                return false;
            }

            ch = static_cast<char_t>(res);
            ++rcnt;

            break;
        }
        else if (str.size() >= 2 && (str[1] == _T('x') || str[1] == _T('X'))) {
            unsigned int res = 0;
            const bool ok = _hex2dec(str.c_str() + 1, res, rcnt);

            if (!ok) {
                return false;
            }

            ch = static_cast<char_t>(res);
            ++rcnt;

            break;
        }
        else {
            ch = prefix;
        }
        return false;
    }


    return true;
}


/* -------------------------------------------------------------------------- */

} // namespace mip


/* -------------------------------------------------------------------------- */
