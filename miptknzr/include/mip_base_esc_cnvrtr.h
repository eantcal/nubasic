//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __MIP_BASE_ESC_CNVRTR_H__
#define __MIP_BASE_ESC_CNVRTR_H__


/* -------------------------------------------------------------------------- */

#include "mip_token.h"

#include <memory>
#include <istream>


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

//! Abstract base class of escape sequences converter
struct base_esc_cnvrtr_t {
    virtual ~base_esc_cnvrtr_t() {}
    virtual bool convert(const string_t& str, size_t & rcnt, char_t & ch) const = 0;
    virtual char_t escape_char() const noexcept = 0;
};


/* -------------------------------------------------------------------------- */

} // namespace mip


  /* -------------------------------------------------------------------------- */

#endif // __MIP_BASE_ESC_CNVRTR_H__
