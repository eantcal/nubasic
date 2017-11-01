//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_RETURN_H__
#define __NU_STMT_RETURN_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_return_t : public stmt_t {
public:
    stmt_return_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }
    
    void run(rt_prog_ctx_t& ctx) override {
        ctx.flag.set(rt_prog_ctx_t::FLG_RETURN_REQUEST, true);
    }

protected:
    prog_pointer_t::line_number_t _line_number = 0;

    stmt_return_t(const stmt_return_t&) = delete;
    stmt_return_t& operator=(const stmt_return_t&) = delete;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_RETURN_H__
