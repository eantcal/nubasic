//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_WEND_H__
#define __NU_STMT_WEND_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include "nu_token_list.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_wend_t : public stmt_t {
public:
    stmt_wend_t(prog_ctx_t& ctx);

    stmt_wend_t() = delete;
    stmt_wend_t(const stmt_wend_t&) = delete;
    stmt_wend_t& operator=(const stmt_wend_t&) = delete;

    void run(rt_prog_ctx_t& ctx) override;
    stmt_cl_t get_cl() const noexcept override;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_WEND_H__
