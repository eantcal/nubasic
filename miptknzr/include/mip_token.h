//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __MIP_TOKEN_H__
#define __MIP_TOKEN_H__


/* -------------------------------------------------------------------------- */

#include "mip_unicode.h"

#include <string>
#include <ostream>
#include <string>


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

class token_t
{
public:
    enum class tcl_t {
        BLANK,
        END_OF_LINE,
        END_OF_FILE,
        COMMENT,
        STRING,
        ATOM,
        OTHER
    };

    token_t(
        const tcl_t& type,
        const string_t& value,
        size_t line,
        size_t column,
        char_t quote = 0,
        char_t esc = 0)
        noexcept
        :
        _type(type),
        _value(value),
        _line(line),
        _offset(column),
        _quote(quote),
        _esc(esc)
    {}

    //! return quote and escape sequence prefix
    std::pair<char_t, char_t> get_quote_esc() const noexcept {
        return std::pair<char_t, char_t>(_quote, _esc);
    }

    //! return token type
    tcl_t type() const noexcept {
        return _type;
    }

    //! return token value
    const string_t& value() const noexcept {
        return _value;
    }

    //! return token line number
    size_t line() const noexcept {
        return _line;
    }

    //! return the token offset in the source text line
    size_t offset() const noexcept {
        return _offset;
    }


    friend _ostream& operator<<(_ostream& os, token_t& tkn);

private:
    static const char_t* type2str(tcl_t type);

    //! token type
    tcl_t _type = tcl_t::OTHER;

    //! token value
    string_t _value;

    //! text line number
    size_t _line = 0;

    //! token offset in the text line
    size_t _offset = 0;

    //! quote of any string token
    char_t _quote = 0;

    //! escape sequence prefix; defined for token representing string
    char_t _esc = 0;

};


/* -------------------------------------------------------------------------- */

} // namespace mip


/* -------------------------------------------------------------------------- */

#endif // __MIP_TOKEN_H__
