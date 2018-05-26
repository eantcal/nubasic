//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __MIP_ESC_CNVRTR_H__
#define __MIP_ESC_CNVRTR_H__


/* -------------------------------------------------------------------------- */

#include "mip_base_esc_cnvrtr.h"

#include <memory>
#include <istream>


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

//! Escape sequence converter
class esc_cnvrtr_t : public base_esc_cnvrtr_t
{
private:
    static void _tail(string_t & str, size_t cnt = 1);
    static bool _octal2dec(const string_t & str, unsigned int& res, size_t & cnt);
    static bool _hex2dec(const string_t & str, unsigned int& res, size_t & cnt);

    char_t _esc_char = _T('\\');

public:
    //! ctor
    //! @param esc_char is escape sequence prefix character
    esc_cnvrtr_t(char_t esc_char = _T('\\')) noexcept :
        _esc_char(esc_char)
    {
    }

    //! Return the escape sequence prefix character
    char_t escape_char() const noexcept override {
        return _esc_char;
    }

    //! dtor
    virtual ~esc_cnvrtr_t() {}

    //! Convert an escape sequence into a character
    //! @param str is a string which begins with an escape sequence
    //! @param rcnt is the number of character of escape sequnce (including esc prefix)
    //! @param ch is a converted character
    //! @return true in case of success, false otherwise
    bool convert(const string_t& str, size_t & rcnt, char_t & ch) const override;
};



/* -------------------------------------------------------------------------- */

} // namespace mip


/* -------------------------------------------------------------------------- */

#endif // __MIP_ESC_CONVERTER_H__
