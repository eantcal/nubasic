//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#ifndef __NU_STMT_SYNTAX_H__
#define __NU_STMT_SYNTAX_H__

#include "nu_prog_ctx.h"
#include "nu_stmt.h"

namespace nu {

struct stmt_syntax_t : public stmt_t {
    stmt_syntax_t(prog_ctx_t& ctx, prog_ctx_t::syntax_mode_t mode);

    void run(rt_prog_ctx_t& ctx) override;
    stmt_cl_t get_cl() const noexcept override { return stmt_cl_t::NA; }

private:
    prog_ctx_t::syntax_mode_t _mode;
};

} // namespace nu

#endif // __NU_STMT_SYNTAX_H__
