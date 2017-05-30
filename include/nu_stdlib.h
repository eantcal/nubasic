//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STDLIB_H__
#define __NU_STDLIB_H__


/* -------------------------------------------------------------------------- */

#include "nu_generic_functor.h"

#include <string>
#include <vector>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

//! Executes a shell command
struct os_shell_t : public generic_functor_t {
    static int exec(const std::string& cmd);

    virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */

//! Changes current working directory
struct os_chdir_t : public generic_functor_t {
    virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */

//! Implements FOPEN
struct os_fopen_t : public generic_functor_t {
    virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */

//! Implements SEEK
struct os_fseek_t : public generic_functor_t {
    virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */

//! Implements FLUSH
struct os_fflush_t : public generic_functor_t {
    virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */

//! Implements READ
struct os_fread_t : public generic_functor_t {
    virtual int apply(rt_prog_ctx_t& ctx, args_t args) override;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_STDLIB_H__
