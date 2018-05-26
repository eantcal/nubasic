//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __MIP_BASE_TKNZR_BLDR_H__
#define __MIP_BASE_TKNZR_BLDR_H__


/* -------------------------------------------------------------------------- */

#include "mip_base_tknzr.h"
#include "mip_base_esc_cnvrtr.h"

#include <memory>
#include <istream>
#include <set>

/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

//! Abstract base class of tokenizer builder objects
struct base_tknzr_bldr_t
{
    //! dtor
    virtual ~base_tknzr_bldr_t() {}

    //! Build a tokenizer object
    virtual std::unique_ptr< base_tknzr_t > build() = 0;

    //! Add a definition of an atomic token
    virtual bool def_atom(const string_t& value) = 0;

    //! Define a set of atomic tokens
    virtual bool def_atom(const std::set<string_t>& value_set) = 0;

    //! Add a definition of a blank token
    virtual bool def_blank(const string_t& value) = 0;

    //! Define a set of blank tokens
    virtual bool def_blank(const std::set<string_t>& value_set) = 0;

    //! Add a definition of a multi-line comment
    virtual bool def_ml_comment(const string_t& begin, const string_t& end) = 0;

    //! Add a definition of a single-line comment
    virtual bool def_sl_comment(const string_t& prefix) = 0;

    //! Define of a set of single-line comments
    virtual bool def_sl_comment(const std::set<string_t>& prefix_set) = 0;

    //! Add a definition of end-of-line tokens
    virtual bool def_eol(const base_tknzr_t::eol_t& value) = 0;

    //! Define a set of end-of-line tokens
    virtual bool def_eol(const std::set<base_tknzr_t::eol_t>& value_set) = 0;

    //! Add a definition of a string token
    virtual bool def_string(char_t quote, std::shared_ptr<base_esc_cnvrtr_t> et = nullptr) = 0;
};


/* -------------------------------------------------------------------------- */

} // namespace mip


/* -------------------------------------------------------------------------- */

#endif // __MIP_BASE_TKNZR_BLDR_H__
