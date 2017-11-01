//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_DIM_H__
#define __NU_STMT_DIM_H__


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

class stmt_dim_t : public stmt_t {
public:
    using vec_size_t = expr_any_t::handle_t;
    using vinfo_t = std::pair<std::string, vec_size_t>;

    using dim_var_map_t = std::map<std::string, vinfo_t>;

    stmt_dim_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }

    stmt_dim_t() = delete;
    stmt_dim_t(const stmt_dim_t&) = delete;
    stmt_dim_t& operator=(const stmt_dim_t&) = delete;

    void run(rt_prog_ctx_t& ctx) override;

    void define(const std::string& var, const std::string& vtype,
        vec_size_t vect_size, prog_ctx_t& ctx)
    {
        (void)ctx;
        _vars.insert(std::make_pair(var, std::make_pair(vtype, vect_size)));
    }

protected:
    dim_var_map_t _vars;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_DIM_H__
