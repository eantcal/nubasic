//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_VAR_SCOPE_H__
#define __NU_VAR_SCOPE_H__


/* -------------------------------------------------------------------------- */

#include "nu_icstring.h"
#include "nu_symbol_map.h"
#include "nu_variant.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

enum { VAR_ACCESS_RW, VAR_ACCESS_RO = 1 };

using var_scope_flag_t = unsigned int;
using var_value_t = std::pair<variant_t, var_scope_flag_t>;

/**
 * This class holds the value of variables that belong to the same
 * definition scope
 */
class var_scope_t : public symbol_map_t<icstring_t, var_value_t> {
public:
    using handle_t = std::shared_ptr<var_scope_t>;

    var_scope_t() = default;
    var_scope_t(const var_scope_t&) = default;
    var_scope_t& operator=(const var_scope_t&) = default;

    bool define(const std::string& name, const var_value_t& value) override;

    friend std::stringstream& operator<<(
        std::stringstream& ss, var_scope_t& obj);

protected:
    void get_err_msg(const std::string& key, std::string& err) const override;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_VAR_SCOPE_H__
