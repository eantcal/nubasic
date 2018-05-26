//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __MIP_BASE_TKNZR_H__
#define __MIP_BASE_TKNZR_H__


/* -------------------------------------------------------------------------- */

#include "mip_token.h"

#include <memory>
#include <istream>


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

//! Abstract base class of tokenizer objects
struct base_tknzr_t
{
    //! End-of-line ids
    enum class eol_t {
        CR,   // '\r'
        LF
    };

    //! dtor
    virtual ~base_tknzr_t() {}

    //! Return (next) token found in the input stream or nullptr in case of error
    virtual std::unique_ptr<token_t> next(_istream & is) = 0;

    //! Return true if there is no more data to process
    virtual bool eos(_istream & is) = 0;
};


/* -------------------------------------------------------------------------- */

} // namespace mip


/* -------------------------------------------------------------------------- */

#endif // __MIP_BASE_TKNZR_H__
