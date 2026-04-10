//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#ifndef __NU_STMT_USING_H__
#define __NU_STMT_USING_H__

#include "nu_prog_ctx.h"
#include "nu_stmt.h"

#include <string>

namespace nu {

struct stmt_using_t : public stmt_t {
    stmt_using_t(prog_ctx_t& ctx, const std::string& module_name);

    void run(rt_prog_ctx_t& ctx) override;
    stmt_cl_t get_cl() const noexcept override { return stmt_cl_t::NA; }

private:
    std::string _module_name;
};

} // namespace nu

#endif // __NU_STMT_USING_H__
