//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_NEXT_H__
#define __NU_STMT_NEXT_H__


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

class stmt_next_t : public stmt_t {
public:
    stmt_next_t() = delete;
    stmt_next_t(const stmt_next_t&) = delete;
    stmt_next_t& operator=(const stmt_next_t&) = delete;

    stmt_next_t(prog_ctx_t& ctx, const std::string& variable)
        : stmt_t(ctx)
        , _variable(variable)
    {
        auto handle = ctx.for_loop_metadata.end_find(ctx.compiletime_pc);

        if (!handle) {
            handle = ctx.for_loop_metadata.compile_end(ctx.compiletime_pc);
        }

        if (handle) {
            handle->pc_end_stmt = ctx.compiletime_pc;
        }
    }

    virtual void run(rt_prog_ctx_t& ctx) override;
    virtual stmt_cl_t get_cl() const noexcept override;

protected:
    std::string _variable;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_NEXT_H__
