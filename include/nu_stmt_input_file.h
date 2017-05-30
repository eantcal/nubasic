//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_INPUT_FILE_H__
#define __NU_STMT_INPUT_FILE_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt_input.h"
#include "nu_var_scope.h"
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {

class stmt_input_file_t : public stmt_t {
public:
    stmt_input_file_t(prog_ctx_t& ctx, int fd, var_list_t vars)
        : stmt_t(ctx)
        , _fd(fd)
        , _vars(vars)
    {
    }


    virtual void run(rt_prog_ctx_t& ctx) override;


protected:
    int _fd = 0;
    var_list_t _vars;

    stmt_input_file_t() = delete;
    stmt_input_file_t(stmt_input_file_t&) = delete;
    stmt_input_file_t& operator=(stmt_input_file_t&) = delete;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_STMT_INPUT_FILE_H__
