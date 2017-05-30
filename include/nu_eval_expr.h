//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_EVAL_EXPR_H__
#define __NU_EVAL_EXPR_H__


/* -------------------------------------------------------------------------- */

#include "nu_rt_prog_ctx.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

variant_t eval_expr(rt_prog_ctx_t& ctx, std::string data);


/* -------------------------------------------------------------------------- */

}

#endif // __NU_EVAL_EXPR_H__


/* -------------------------------------------------------------------------- */
