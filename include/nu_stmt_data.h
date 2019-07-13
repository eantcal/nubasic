//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_DATA_H__
#define __NU_STMT_DATA_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include <list>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_data_t : public stmt_t {
public:
    stmt_data_t(const stmt_data_t&) = delete;
    stmt_data_t& operator=(const stmt_data_t&) = delete;

    stmt_data_t(prog_ctx_t& ctx, const std::string& data = "")
        : stmt_t(ctx)
        , _data(data)
    {
    }

    stmt_data_t(const arg_list_t& args, prog_ctx_t& ctx)
        : stmt_t(ctx)
        , _args(args)
    {
    }

    void run(rt_prog_ctx_t& ctx) override;

protected:
    arg_list_t _args;
    std::string _data;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_DATA_H__
