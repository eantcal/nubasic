//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_EMPTY_H__
#define __NU_STMT_EMPTY_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include <memory>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

struct stmt_empty_t : public stmt_t {
    stmt_empty_t() = delete;
    stmt_empty_t(const stmt_empty_t&) = delete;
    stmt_empty_t& operator=(const stmt_empty_t&) = delete;

    stmt_empty_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }

    virtual void run(rt_prog_ctx_t& ctx) override;
    virtual stmt_cl_t get_cl() const noexcept override;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif // __NU_STMT_EMPTY_H__
