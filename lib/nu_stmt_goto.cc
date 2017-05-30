//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_goto.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_goto_t::run(rt_prog_ctx_t& ctx)
{
    if (!_line_number) {
        if (_label.empty() || !ctx.prog_label.is_defined(_label)) {

            rt_error_code_t::get_instance().throw_if(_label.empty(),
                ctx.runtime_pc.get_line(), rt_error_code_t::E_NO_LABEL, "Goto");

            rt_error_code_t::get_instance().throw_if(
                !ctx.prog_label.is_defined(_label), ctx.runtime_pc.get_line(),
                rt_error_code_t::E_LABEL_NOT_DEF, "Goto");
        }

        _line_number = ctx.prog_label[_label];
    }

    ctx.go_to(prog_pointer_t(_line_number, 0));
}


/* -------------------------------------------------------------------------- */

} // namespace nu
