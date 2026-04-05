//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_EXPR_SUBSCROP_H__
#define __NU_EXPR_SUBSCROP_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_expr_function.h"
#include "nu_global_function_tbl.h"
#include "nu_var_scope.h"


/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

class expr_subscrop_t : public expr_function_t {
public:
    expr_subscrop_t(const std::string& name, func_args_t var)
        : expr_function_t(name, var)
    {
    }

    expr_subscrop_t() = delete;
    expr_subscrop_t(const expr_subscrop_t&) = default;
    expr_subscrop_t& operator=(const expr_subscrop_t&) = default;

    variant_t eval(rt_prog_ctx_t& ctx) const override;
};


/* -------------------------------------------------------------------------- */

} // namespace


/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_SUBSCROP_H__
