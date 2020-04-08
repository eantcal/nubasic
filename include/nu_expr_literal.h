//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_EXPR_LITERAL_H__
#define __NU_EXPR_LITERAL_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class expr_literal_t : public expr_any_t {
public:
    //! ctor
    expr_literal_t(const variant_t& value = variant_t(0))
        : _val(value)
    {
    }

    expr_literal_t(const expr_literal_t&) = default;
    expr_literal_t& operator=(const expr_literal_t&) = default;

    //! move ctor
    expr_literal_t(expr_literal_t&& obj) noexcept
        : _val(std::move(obj._val))
    {
    }

    //! move operator
    expr_literal_t& operator=(expr_literal_t&& obj) noexcept {
        if (this != &obj)
            _val = std::move(obj._val);

        return *this;
    }

    //! Return the literal expression value
    variant_t eval(rt_prog_ctx_t& ctx) const override {
        (void)ctx;
        return _val;
    }

    //! Returns false for this expression type
    bool empty() const noexcept override { 
        return false; 
    }

    std::string name() const noexcept override { 
        return ""; 
    }

    func_args_t get_args() const noexcept override {
        func_args_t dummy;
        return dummy;
    }

protected:
    variant_t _val;
};


/* -------------------------------------------------------------------------- */

}

/* -------------------------------------------------------------------------- */

#endif // __NU_EXPR_LITERAL_H__
