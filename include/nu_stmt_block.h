//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_BLOCK_H__
#define __NU_STMT_BLOCK_H__


/* -------------------------------------------------------------------------- */

#include "nu_rt_prog_ctx.h"
#include "nu_stmt.h"
#include <list>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class stmt_block_t : public stmt_t {
protected:
    std::deque<stmt_t::handle_t> _stmts;

public:
    using handle_t = std::shared_ptr<stmt_block_t>;


    stmt_block_t() = delete;


    stmt_block_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }


    bool empty() const noexcept { return _stmts.empty(); }


    size_t size() const noexcept { return _stmts.size(); }


    int find_stmt_pos(int stmtid) const noexcept;


    stmt_block_t& operator+=(stmt_t::handle_t stmt);


    stmt_cl_t get_cl() const noexcept override
    {
        return _stmts.size() == 1 ? (*_stmts.cbegin())->get_cl()
                                  : stmt_t::stmt_cl_t::BLOCK_OF_CODE;
    }


    bool run_pos(rt_prog_ctx_t& ctx, int pos);


    void run(rt_prog_ctx_t& ctx) override { run_pos(ctx, 0); }
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif // __NU_STMT_BLOCK_H__
