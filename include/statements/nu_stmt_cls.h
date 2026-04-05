//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_CLS_H__
#define __NU_STMT_CLS_H__


/* -------------------------------------------------------------------------- */

#include "nu_os_console.h"
#include "nu_stmt.h"

#include <stdlib.h>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_cls_t : public stmt_t {
public:
    stmt_cls_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }

    stmt_cls_t() = delete;
    stmt_cls_t(const stmt_cls_t&) = delete;
    stmt_cls_t& operator=(const stmt_cls_t&) = delete;

    virtual void run(rt_prog_ctx_t& ctx) override;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_CLS_H__
