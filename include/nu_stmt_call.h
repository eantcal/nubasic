//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_CALL_H__
#define __NU_STMT_CALL_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_prog_pointer.h"
#include "nu_stmt.h"
#include <list>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_call_t : public stmt_t {
public:
    stmt_call_t(const stmt_call_t&) = delete;
    stmt_call_t& operator=(const stmt_call_t&) = delete;
    stmt_call_t() = delete;

    stmt_call_t(const std::string& name, prog_ctx_t& ctx, bool fncall = false)
        : stmt_t(ctx)
        , _name(name)
        , _fncall(fncall)
    {
    }

    stmt_call_t(const arg_list_t& args, const std::string& name,
        prog_ctx_t& ctx, bool fncall = false)
        : stmt_t(ctx)
        , _args(args)
        , _name(name)
        , _fncall(fncall)
    {
    }

    virtual void run(rt_prog_ctx_t& ctx) override;
    void run(rt_prog_ctx_t& ctx, const prog_pointer_t::line_number_t& line);

protected:
    arg_list_t _args;
    std::string _name;
    bool _fncall = false;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_CALL_H__
