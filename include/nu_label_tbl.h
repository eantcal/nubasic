//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_LABEL_TBL_H__
#define __NU_LABEL_TBL_H__


/* -------------------------------------------------------------------------- */

#include "nu_exception.h"
#include "nu_icstring.h"
#include "nu_symbol_map.h"
#include "nu_variant.h"
#include <algorithm>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class label_tbl_t : public symbol_map_t<icstring_t, unsigned int> {
public:
    label_tbl_t(const label_tbl_t&) = delete;
    label_tbl_t& operator=(const label_tbl_t&) = delete;

    void set_explicit_line_reference_mode() noexcept
    {
        _explicit_line_reference = true;
    }

    bool get_explicit_line_reference_mode() const noexcept
    {
        return _explicit_line_reference;
    }

    void clear()
    {
        symbol_map_t<icstring_t, unsigned int>::clear();
        _explicit_line_reference = false;
    }


    label_tbl_t() = default;

protected:
    bool _explicit_line_reference = false;

    virtual void get_err_msg(const std::string& key, std::string& err) const;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_LABEL_TBL_H__
