//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_EXPR_ANY_H__
#define __NU_EXPR_ANY_H__


/* -------------------------------------------------------------------------- */

#include "nu_stdtype.h"
#include "nu_variant.h"
#include <functional>
#include <list>
#include <memory>
#include <vector>

/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

class rt_prog_ctx_t;


/* -------------------------------------------------------------------------- */

struct expr_any_t {
    using handle_t = std::shared_ptr<expr_any_t>;
    using func_args_t = std::vector<expr_any_t::handle_t>;

    virtual variant_t eval(rt_prog_ctx_t& ctx) const = 0;
    virtual bool empty() const noexcept = 0;

    virtual std::string name() const noexcept = 0;
    virtual func_args_t get_args() const noexcept = 0;

    virtual ~expr_any_t() {}
};


/* -------------------------------------------------------------------------- */

//! arg_list_t is a collection of expressions with a separator char
//! (char may be used for formatting purposes i.e. print implementation)
using arg_list_t = std::list<std::pair<expr_any_t::handle_t, char>>;


/* -------------------------------------------------------------------------- */

//! var_arg_t is a pair of <name, index> - index may be any integer expression
//! handle index is provided to resolve array elements
using var_arg_t = std::pair<std::string, expr_any_t::handle_t>;


/* -------------------------------------------------------------------------- */

//! var_list_t is a collection of var_arg_t
using var_list_t = std::list<var_arg_t>;


/* -------------------------------------------------------------------------- */

//! Built-in function arguments
using func_args_t = expr_any_t::func_args_t;


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif
