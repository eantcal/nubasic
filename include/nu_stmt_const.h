//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_CONST_H__
#define __NU_STMT_CONST_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_prog_ctx.h"
#include "nu_stmt.h"
#include "nu_token_list.h"
#include "nu_var_scope.h"
#include "nu_variable.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_const_t : public stmt_t {
public:
    using arg_t = expr_any_t::handle_t;

    stmt_const_t(prog_ctx_t& ctx, const std::string& var,
        const std::string& vtype, arg_t value)
        : stmt_t(ctx)
        , _var(var)
        , _vtype(vtype)
        , _arg(value)
    {
    }

    stmt_const_t() = delete;
    stmt_const_t(const stmt_const_t&) = delete;
    stmt_const_t& operator=(const stmt_const_t&) = delete;

    void run(rt_prog_ctx_t& ctx) override;

protected:
    std::string _var;
    std::string _vtype;
    arg_t _arg;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_CONST_H__
