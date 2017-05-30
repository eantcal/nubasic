//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_FOR_TO_STEP_H__
#define __NU_STMT_FOR_TO_STEP_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_expr_literal.h"
#include "nu_prog_ctx.h"
#include "nu_stmt.h"
#include "nu_token_list.h"
#include "nu_var_scope.h"
#include "nu_variable.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_for_to_step_t : public stmt_t {
public:
    using arg_t = expr_any_t::handle_t;

    stmt_for_to_step_t() = delete;
    stmt_for_to_step_t(const stmt_for_to_step_t&) = delete;
    stmt_for_to_step_t& operator=(const stmt_for_to_step_t&) = delete;

    stmt_for_to_step_t(prog_ctx_t& ctx, const std::string& variable,
        arg_t from_arg, arg_t to_arg, expr_any_t::handle_t step)
        : stmt_t(ctx)
        , _variable(variable)
        , _from_arg(from_arg)
        , _to_arg(to_arg)
        , _step(step)
    {
        assert(step);

        ctx.for_loop_metadata.compile_begin(ctx.compiletime_pc);
    }

    virtual void run(rt_prog_ctx_t& ctx) override;
    virtual stmt_cl_t get_cl() const noexcept override;

protected:
    std::string _variable;
    arg_t _from_arg, _to_arg;
    expr_any_t::handle_t _step = 0;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_FOR_TO_STEP_H__
