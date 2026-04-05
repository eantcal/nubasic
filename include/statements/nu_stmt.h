//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_H__
#define __NU_STMT_H__


/* -------------------------------------------------------------------------- */

#include "nu_error_codes.h"
#include "nu_exception.h"
#include "nu_expr_any.h"
#include "nu_stdtype.h"

#include <cassert>
#include <list>
#include <memory>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class prog_ctx_t;


/* -------------------------------------------------------------------------- */

/**
 * This is the base class of a hierarchy of statement classes.
 */
class stmt_t {
public:
    stmt_t() = default;
    stmt_t(const stmt_t&) = delete;
    stmt_t& operator=(const stmt_t&) = delete;
    stmt_t(prog_ctx_t& ctx) noexcept;


    int get_stmt_id() const noexcept { 
        return _stmt_id; 
    }


    enum class stmt_cl_t {
        NA,
        EMPTY,
        BLOCK_OF_CODE,
        WHILE_BEGIN,
        WHILE_END,
        FOR_BEGIN,
        FOR_END,
        IF_BLOCK_BEGIN,
        IF_BLOCK_END,
        SUB_BEGIN,
        SUB_END,
        DO_BEGIN,
        DO_END,
        ELSE,
        STRUCT_BEGIN,
        STRUCT_END
    };

    using handle_t = std::shared_ptr<stmt_t>;

    //! Executes the statement
    virtual void run(rt_prog_ctx_t&) = 0;

    //! Identify the class of statement
    virtual stmt_cl_t get_cl() const noexcept;

    // dtor
    virtual ~stmt_t();

private:
    int _stmt_id = 0;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif // __NU_STMT_H__
