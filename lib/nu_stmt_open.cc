//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_open.h"
#include "nu_error_codes.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void stmt_open_t::run(rt_prog_ctx_t& ctx)
{
    auto rt_error_if
        = [&](bool cond, rt_error_code_t::value_t err, const std::string desc) {
              rt_error_code_t::get_instance().throw_if(
                  cond, ctx.runtime_pc.get_line(), err, desc);
    };

    ctx.go_to_next();

    auto filename = _filename->eval(ctx);

    rt_error_if(filename.get_type() != variant_t::type_t::STRING,
        rt_error_code_t::E_TYPE_MISMATCH, "OPEN");

    std::string mode = "r";

    if (_mode == "input") {
        rt_error_if(_access == "write" || _access == "read write",
            rt_error_code_t::E_TYPE_ILLEGAL, "OPEN");

        mode = "r";
    } 
    else if (_mode == "output") {
        rt_error_if(_access == "read" || _access == "read write",
            rt_error_code_t::E_TYPE_ILLEGAL, "OPEN");

        mode = "w";
    } 
    else if (_mode == "append") {
        mode = "a";

        if (_access.find("read") != std::string::npos) {
            mode += "+";
        }
    } 
    else if (_mode == "random") {
        mode = "r";

        if (_access.find("write") != std::string::npos) {
            mode += "+";
        }
    } 
    else if (_mode == "binary") {
        mode = "r";

        if (_access.find("write") != std::string::npos) {
            mode += "+";
        }

        mode += "b";
    }

    const bool res = ctx.file_tbl.open_fd(filename.to_str(), mode, _fd);

    ctx.set_errno(!res && !errno ? EBADF : errno);
}


/* -------------------------------------------------------------------------- */

} // namespace nu
