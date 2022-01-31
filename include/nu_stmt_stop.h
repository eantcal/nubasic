//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_STOP_H__
#define __NU_STMT_STOP_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_stop_t : public stmt_t {
public:
    stmt_stop_t() = delete;
    stmt_stop_t(const stmt_stop_t&) = delete;
    stmt_stop_t& operator=(const stmt_stop_t&) = delete;

    stmt_stop_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }

    void run(rt_prog_ctx_t& ctx) override;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_STOP_H__
