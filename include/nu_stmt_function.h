//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_FUNCTION_H__
#define __NU_STMT_FUNCTION_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_prog_ctx.h"
#include "nu_stmt.h"
#include "nu_token_list.h"
#include "nu_var_scope.h"
#include "nu_variable.h"

#include <algorithm>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_function_t : public stmt_t {
public:
    stmt_function_t() = delete;
    stmt_function_t(const stmt_function_t&) = delete;
    stmt_function_t& operator=(const stmt_function_t&) = delete;

    using vec_size_t = expr_any_t::handle_t;

    stmt_function_t(prog_ctx_t& ctx, const std::string& id);

    void define(const std::string& var, const std::string& vtype,
        vec_size_t vect_size, prog_ctx_t& ctx, const std::string& id);

    void define_ret_type(const std::string& type, prog_ctx_t& ctx, size_t array_size) {
        auto& fproto = ctx.proc_prototypes.data[_id].second;
        fproto.ret_type = type;
        fproto.array_size = array_size;
    }

    stmt_cl_t get_cl() const noexcept override;
    void run(rt_prog_ctx_t& ctx) override;

protected:
    std::string _id;
    std::set<std::string> _vars_rep_check;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_FUNCTION_H__
