//  
// This file is part of MipTknzr Library Project
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __MIP_TKNZR_BLDR_H__
#define __MIP_TKNZR_BLDR_H__


/* -------------------------------------------------------------------------- */

#include "mip_base_tknzr_bldr.h"
#include "mip_tknzr.h"

#include <cassert>
#include <memory>


/* -------------------------------------------------------------------------- */

namespace mip {


/* -------------------------------------------------------------------------- */

//! Builder of tokenizer objects
class tknzr_bldr_t : public base_tknzr_bldr_t
{
private:
    bool _build_tknzr();

    template <class T, class S>
    bool _def_item(const T& value, S& set)
    {
        if (!_build_tknzr()) {
            return false;
        }

        auto it = set.find(value);

        if (it != set.end()) {
            return false;
        }

        set.insert(value);

        return true;
    }

    template <class T, class S>
    bool _def_item(const std::set<T>& value_set, S& set)
    {
        auto res = true;

        for (const auto & v : value_set) {
            if (!_def_item(v, set)) {
                res = false; // just notify some def_item() failed
            }
        }
        
        return res;
    }

    std::unique_ptr< tknzr_t > _tknzr;

public:
    tknzr_bldr_t() noexcept : _tknzr(new tknzr_t()) {
        assert(_tknzr);
    }

    std::unique_ptr< base_tknzr_t > build() override;

    bool def_atom(const string_t& value) override;
    bool def_atom(const std::set<string_t>& value_set) override;

    bool def_blank(const string_t& value) override;
    bool def_blank(const std::set<string_t>& value_set) override;

    bool def_eol(const base_tknzr_t::eol_t& value) override;
    bool def_eol(const std::set<base_tknzr_t::eol_t>& value_set) override;

    bool def_sl_comment(const string_t& prefix) override;
    bool def_sl_comment(const std::set<string_t>& prefix_set) override;

    bool def_ml_comment(const string_t& begin, const string_t& end) override;

    bool def_string(char_t quote, std::shared_ptr<base_esc_cnvrtr_t> et = nullptr) override;
};


/* -------------------------------------------------------------------------- */

} // namespace mip


/* -------------------------------------------------------------------------- */

#endif // __MIP_TKNZR_BLDR_H__
