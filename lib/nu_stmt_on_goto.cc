//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_on_goto.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_on_goto_t::run(rt_prog_ctx_t& ctx)
{

    rt_error_code_t::get_instance().throw_if(_label_list.empty(),
        ctx.runtime_pc.get_line(), rt_error_code_t::E_NO_LABEL, "On ... Goto");

    auto lbl_idx = _condition->eval(ctx);

    rt_error_code_t::get_instance().throw_if(
        !variable_t::is_integral(lbl_idx.get_type()), ctx.runtime_pc.get_line(),
        rt_error_code_t::E_TYPE_ILLEGAL, "On ... Goto");

    size_t idx = static_cast<size_t>(lbl_idx.to_int());

    rt_error_code_t::get_instance().throw_if(
        int(idx) < 0 || idx >= _label_list.size(), ctx.runtime_pc.get_line(),
        rt_error_code_t::E_VAL_OUT_OF_RANGE,
        "On " + nu::to_string(idx) + " Goto ...");

    auto label = _label_list[idx];

    rt_error_code_t::get_instance().throw_if(!ctx.prog_label.is_defined(label),
        ctx.runtime_pc.get_line(), rt_error_code_t::E_LABEL_NOT_DEF,
        "On ... Goto '" + label + "'");

    auto line_number = ctx.prog_label[label];

    ctx.go_to(prog_pointer_t(line_number, 0));
}


/* -------------------------------------------------------------------------- */

} // namespace nu
