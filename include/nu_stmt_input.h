//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_INPUT_H__
#define __NU_STMT_INPUT_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include "nu_var_scope.h"
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_input_t : public stmt_t {
public:
    stmt_input_t() = delete;
    stmt_input_t(stmt_input_t&) = delete;
    stmt_input_t& operator=(stmt_input_t&) = delete;

    stmt_input_t(
        prog_ctx_t& ctx, const std::string& input_str, const var_list_t& var)
        : stmt_t(ctx)
        , _input_str(input_str)
        , _vars(var)
    {
    }


    virtual void run(rt_prog_ctx_t& ctx) override;


protected:
    std::string _input_str;
    var_list_t _vars;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif
