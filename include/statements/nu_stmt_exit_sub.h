//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_EXIT_SUB_H__
#define __NU_STMT_EXIT_SUB_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_exit_sub_t : public stmt_t {
public:
    stmt_exit_sub_t() = delete;
    stmt_exit_sub_t(const stmt_exit_sub_t&) = delete;
    stmt_exit_sub_t& operator=(const stmt_exit_sub_t&) = delete;

    stmt_exit_sub_t(prog_ctx_t& ctx);

    void run(rt_prog_ctx_t& ctx) override;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_EXIT_SUB_H__
