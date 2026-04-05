//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_EXPR_VAR_H__
#define __NU_EXPR_VAR_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_rt_prog_ctx.h"
#include "nu_var_scope.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class expr_var_t : public expr_any_t {
public:
    expr_var_t(const std::string& name)
        : _name(name)
    {
    }

    expr_var_t() = delete;
    expr_var_t(const expr_var_t&) = default;
    expr_var_t& operator=(const expr_var_t&) = default;


    variant_t eval(rt_prog_ctx_t& ctx) const override;
    
    bool empty() const noexcept override { 
        return false; 
    }
    
    std::string name() const noexcept override { 
        return _name; 
    }


    func_args_t get_args() const noexcept override {
        func_args_t dummy;
        return dummy;
    }

protected:
    std::string _name;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif
