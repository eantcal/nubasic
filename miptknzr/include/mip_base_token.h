//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __MIP_BASE_TOKEN_H__
#define __MIP_BASE_TOKEN_H__


/* -------------------------------------------------------------------------- */

#include "mip_unicode.h"

#include <string>
#include <ostream>


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

//! Abstract base class of token objects
struct base_token_t
{
    enum class tcl_t {
        BLANK,
        END_OF_LINE,
        END_OF_FILE,
        COMMENT,
        STRING,
        ATOM,
        OTHER
    };

    //! Return <quota, escape prefix> of string token
    virtual std::pair<char_t, char_t> get_quote_esc() const noexcept = 0;

    //! Return token line number (first line -> 0)
    virtual size_t line() const noexcept = 0;

    //! Return token char offset (first character -> 0)
    virtual size_t offset() const noexcept = 0;

    //! Return token type
    virtual tcl_t type() const noexcept = 0;

    //! Return token value
    virtual const string_t& value() const noexcept = 0;
    
    //! Get a description of a given token type
    static const char_t* type2str(tcl_t type);

    //! operator<< provided for debugging purposes
    friend _ostream& operator<<(_ostream& os, base_token_t& tkn);
    
    //! dtor
    virtual ~base_token_t() {}
};


/* -------------------------------------------------------------------------- */

} // namespace mip


/* -------------------------------------------------------------------------- */

#endif // __MIP_BASE_TOKEN_H__
