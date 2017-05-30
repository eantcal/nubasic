//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_GLOBAL_FUNCTION_TBL_H__
#define __NU_GLOBAL_FUNCTION_TBL_H__


/* -------------------------------------------------------------------------- */

#include "nu_exception.h"
#include "nu_expr_any.h"
#include "nu_icstring.h"
#include "nu_rt_prog_ctx.h"
#include "nu_symbol_map.h"
#include "nu_variant.h"

#include <algorithm>
#include <functional>
#include <type_traits>
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

using func_bin_t = std::function<variant_t(const variant_t&, const variant_t&)>;

using func_t = std::function<variant_t(
    rt_prog_ctx_t&, const std::string&, const func_args_t&)>;

using binop_t = std::function<variant_t(const variant_t&, const variant_t&)>;


/* -------------------------------------------------------------------------- */

class global_function_tbl_t : public symbol_map_t<icstring_t, func_t> {
private:
    global_function_tbl_t() = default;
    global_function_tbl_t(const global_function_tbl_t&) = delete;
    global_function_tbl_t& operator=(const global_function_tbl_t&) = delete;

    static global_function_tbl_t* _instance;

protected:
    virtual void get_err_msg(const std::string& key, std::string& err) const
    {
        err = "'" + key + "()' not defined";
    }

public:
    static global_function_tbl_t& get_instance();
};


/* -------------------------------------------------------------------------- */

class global_operator_tbl_t : public symbol_map_t<icstring_t, binop_t> {
private:
    global_operator_tbl_t() = default;
    global_operator_tbl_t(const global_operator_tbl_t&) = delete;
    global_operator_tbl_t& operator=(const global_operator_tbl_t&) = delete;

    static global_operator_tbl_t* _instance;

protected:
    virtual void get_err_msg(const std::string& key, std::string& err) const
    {
        err = "'" + key + "()' not defined";
    }

public:
    static global_operator_tbl_t& get_instance();
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_BUILTIN_FUNC__
