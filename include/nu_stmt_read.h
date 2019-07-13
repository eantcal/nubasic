//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_READ_H__
#define __NU_STMT_READ_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include "nu_var_scope.h"
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_read_t : public stmt_t {
public:
    stmt_read_t() = delete;
    stmt_read_t(stmt_read_t&) = delete;
    stmt_read_t& operator=(stmt_read_t&) = delete;

    stmt_read_t(
        prog_ctx_t& ctx, const var_list_t& var)
        : stmt_t(ctx)
        , _vars(var)
    {
    }

    void run(rt_prog_ctx_t& ctx) override;

protected:
    var_list_t _vars;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif
