//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_struct_element.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

stmt_struct_element_t::stmt_struct_element_t(prog_ctx_t& ctx,
    const std::string& name, const std::string& type, vec_size_t vect_size)
{
    auto element_it
        = ctx.struct_prototypes.data.find(ctx.compiling_struct_name);

    syntax_error_if(element_it == ctx.struct_prototypes.data.end(), name, 0,
        "Struct... End Struct");

    auto user_def_type = ctx.struct_prototypes.data.find(type);

    auto type_code = variable_t::type_by_typename(type);

    if (user_def_type != ctx.struct_prototypes.data.end()) {
        element_it->second.second.define_struct_member(
            name, user_def_type->second.second);
    } 
    else {
        syntax_error_if(type_code == variable_t::type_t::UNDEFINED, name, 0,
            "Struct... End Struct");

        element_it->second.second.define_struct_member(
            name, variant_t(string_t(), type_code, vect_size));
    }
}


/* -------------------------------------------------------------------------- */

void stmt_struct_element_t::run(rt_prog_ctx_t& ctx) 
{ 
    ctx.go_to_next(); 
}


/* -------------------------------------------------------------------------- */

} // namespace nu
