//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_BUILTIN_HELP_H__
#define __NU_BUILTIN_HELP_H__


/* -------------------------------------------------------------------------- */

#include "nu_exception.h"
#include "nu_expr_any.h"
#include "nu_icstring.h"
#include "nu_prog_ctx.h"
#include "nu_symbol_map.h"
#include "nu_variant.h"

#include <algorithm>
#include <functional>
#include <string>
#include <type_traits>
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {

enum class lang_item_t : int { 
    COMMAND, 
    INSTRUCTION, 
    FUNCTION, 
    OPERATOR 
};


/* -------------------------------------------------------------------------- */

using help_item_t = std::pair<lang_item_t, icstring_t>;


/* -------------------------------------------------------------------------- */

class builtin_help_t
    : protected std::map<help_item_t, std::pair<std::string, std::string>> {
private:
    builtin_help_t() = default;
    builtin_help_t(const builtin_help_t&) = delete;
    builtin_help_t& operator=(const builtin_help_t&) = delete;

    static builtin_help_t* _instance;

    virtual void get_err_msg(const std::string& key, std::string& err) const {
        err = "item '";
        err += key + "' not found";
    }

public:
    std::string help(const std::string& s_item = "");
    std::string apropos(const std::string& s_item);
    static builtin_help_t& get_instance();
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_BUILTIN_HELP__
