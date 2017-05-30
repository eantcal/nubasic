//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_OPEN_H__
#define __NU_STMT_OPEN_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_stmt.h"
#include "nu_stmt_empty.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_open_t : public stmt_t {
public:
    stmt_open_t() = delete;
    stmt_open_t(const stmt_open_t&) = delete;
    stmt_open_t& operator=(const stmt_open_t&) = delete;

    stmt_open_t(prog_ctx_t& ctx, expr_any_t::handle_t filename,
        const std::string& mode, const std::string& access, unsigned int fd)
        : stmt_t(ctx)
        , _filename(filename)
        , _mode(mode)
        , _access(access)
        , _fd(fd)
    {
    }

    virtual void run(rt_prog_ctx_t& ctx) override;

protected:
    expr_any_t::handle_t _filename;
    std::string _mode;
    std::string _access;
    unsigned int _fd = 0;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_OPEN_H__
