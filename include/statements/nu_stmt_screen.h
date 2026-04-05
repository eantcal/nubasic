//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_SCREEN_H__
#define __NU_STMT_SCREEN_H__

/* -------------------------------------------------------------------------- */

#include "nu_expr_any.h"
#include "nu_os_console.h"
#include "nu_stmt.h"

/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

// SCREENLOCK — suppress the automatic per-primitive back-buffer→screen blit.
// Drawing commands (LINE, RECT, …) continue writing to the back-buffer but the
// screen is not refreshed until SCREENUNLOCK or REFRESH is executed.
class stmt_screenlock_t : public stmt_t {
public:
    stmt_screenlock_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }

    stmt_screenlock_t() = delete;
    stmt_screenlock_t(const stmt_screenlock_t&) = delete;
    stmt_screenlock_t& operator=(const stmt_screenlock_t&) = delete;

    virtual void run(rt_prog_ctx_t& ctx) override;
};

/* -------------------------------------------------------------------------- */

// SCREENUNLOCK — re-enable the automatic refresh and force one immediate blit
// so the whole frame accumulated during the lock appears at once.
class stmt_screenunlock_t : public stmt_t {
public:
    stmt_screenunlock_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }

    stmt_screenunlock_t() = delete;
    stmt_screenunlock_t(const stmt_screenunlock_t&) = delete;
    stmt_screenunlock_t& operator=(const stmt_screenunlock_t&) = delete;

    virtual void run(rt_prog_ctx_t& ctx) override;
};

/* -------------------------------------------------------------------------- */

// REFRESH — force an immediate blit of the back-buffer to the screen.
// Works regardless of whether the screen lock is active.
class stmt_refresh_t : public stmt_t {
public:
    stmt_refresh_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }

    stmt_refresh_t() = delete;
    stmt_refresh_t(const stmt_refresh_t&) = delete;
    stmt_refresh_t& operator=(const stmt_refresh_t&) = delete;

    virtual void run(rt_prog_ctx_t& ctx) override;
};

/* -------------------------------------------------------------------------- */

// SCREEN n — switch console/graphics mode, like GW-BASIC SCREEN.
//   SCREEN 0  text/hybrid mode: I/O via real console, GDI ops are no-ops.
//   SCREEN 1  GDI console mode: I/O and graphics through the GDI window.
class stmt_screen_t : public stmt_t {
public:
    stmt_screen_t(prog_ctx_t& ctx)
        : stmt_t(ctx)
    {
    }

    stmt_screen_t(arg_list_t args, prog_ctx_t& ctx)
        : stmt_t(ctx)
        , _args(args)
    {
    }

    stmt_screen_t() = delete;
    stmt_screen_t(const stmt_screen_t&) = delete;
    stmt_screen_t& operator=(const stmt_screen_t&) = delete;

    virtual void run(rt_prog_ctx_t& ctx) override;

protected:
    arg_list_t _args;
};

/* -------------------------------------------------------------------------- */

} // namespace nu

/* -------------------------------------------------------------------------- */

#endif // __NU_STMT_SCREEN_H__
