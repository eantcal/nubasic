//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_CLOSE_H__
#define __NU_STMT_CLOSE_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include "nu_stmt_empty.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_close_t : public stmt_t {
public:
    stmt_close_t() = delete;
    stmt_close_t(const stmt_close_t&) = delete;
    stmt_close_t& operator=(const stmt_close_t&) = delete;

    stmt_close_t(prog_ctx_t& ctx, unsigned int fd)
        : stmt_t(ctx)
        , _fd(fd)
    {
    }

    virtual void run(rt_prog_ctx_t& ctx) override;

protected:
    unsigned int _fd = 0;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_CLOSE_H__
