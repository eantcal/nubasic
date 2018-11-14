//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_PROC_PROTOTYPE_TBL_H__
#define __NU_PROC_PROTOTYPE_TBL_H__


/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_prog_pointer.h"
#include "nu_variant.h"

#include <list>
#include <map>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

struct func_param_t {
    std::string var_name; // variable name
    std::string type_name; // variable type name
    expr_any_t::handle_t vsize;

    func_param_t(const std::string& vname, const std::string& tname,
        expr_any_t::handle_t vsz)
        : var_name(vname)
        , type_name(tname)
        , vsize(vsz)
    {
    }

    func_param_t() = delete;
    func_param_t(const func_param_t&) = default;
};


/* -------------------------------------------------------------------------- */

struct func_prototype_t {
    std::string ret_type;
    size_t array_size = 0;
    std::list<func_param_t> parameters;

    func_prototype_t() = default;
    func_prototype_t(const func_prototype_t&) = default;
};


/* -------------------------------------------------------------------------- */

struct proc_prototype_tbl_t {
    std::map<std::string, std::pair<prog_pointer_t, func_prototype_t>> data;

    proc_prototype_tbl_t() = default;
    proc_prototype_tbl_t(const proc_prototype_tbl_t&) = default;
};


/* -------------------------------------------------------------------------- */

using struct_prototype_t = variant_t;


/* -------------------------------------------------------------------------- */

struct struct_prototype_tbl_t {
    std::map<std::string, std::pair<prog_pointer_t, struct_prototype_t>> data;

    struct_prototype_tbl_t() = default;
    struct_prototype_tbl_t(const struct_prototype_tbl_t&) = default;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif // __NU_PROC_PROTOTYPE_TBL_H__
