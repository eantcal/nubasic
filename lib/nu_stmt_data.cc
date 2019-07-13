//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_stmt_data.h"
#include "nu_os_console.h"
#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

    void stmt_data_t::run(rt_prog_ctx_t& ctx)
    {
        int ret = 0;

        for (auto arg : _args) {
            variant_t val;

            if (arg.first == nullptr) {
                val = variant_t("");
            }
            else {
                val = arg.first->eval(ctx);

                switch (arg.second) {
                case ',':
                    break;

                case ';':
                case ' ':
                    rt_error_code_t::get_instance().throw_if(1,
                        ctx.runtime_pc.get_line(), 
                        rt_error_code_t::E_INV_DATA_SEPARATOR, "Data");
                    break;

                default:
                    break;
                }
            }

            switch (val.get_type()) {
                case variant_t::type_t::FLOAT:
                case variant_t::type_t::DOUBLE:
                case variant_t::type_t::INTEGER:
                case variant_t::type_t::BOOLEAN:
                case variant_t::type_t::LONG64:
                case variant_t::type_t::STRING:
                    ctx.read_data_store.push_back(val);
                    break;

                case variant_t::type_t::BYTEVECTOR:
                case variant_t::type_t::UNDEFINED:
                default:
                    rt_error_code_t::get_instance().throw_if(1,
                        ctx.runtime_pc.get_line(), 
                        rt_error_code_t::E_TYPE_ILLEGAL, "Data");
                    break;
            }

        } // for


        ctx.go_to_next();
    }


/* -------------------------------------------------------------------------- */

} // namespace nu
