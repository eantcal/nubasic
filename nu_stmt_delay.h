/*
*  This file is part of nubasic
*
*  nubasic is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  nubasic is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with nubasic; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
*
*  Author: Antonino Calderone <acaldmail@gmail.com>
*
*/


/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_DELAY_H__
#define __NU_STMT_DELAY_H__


/* -------------------------------------------------------------------------- */

#include "nu_stmt.h"
#include "nu_variable.h"
#include "nu_var_scope.h"
#include "nu_token_list.h"
#include "nu_expr_any.h"
#include "nu_os_std.h"

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class stmt_delay_t : public stmt_t, protected signal_handler_t
{
public:
   stmt_delay_t(prog_ctx_t & ctx) :
      stmt_t(ctx)
   {
      signal_mgr_t::instance().register_handler(event_t::BREAK, this);
   }

   stmt_delay_t(arg_list_t args, prog_ctx_t & ctx)
      :
      stmt_t(ctx),
      _args(args)
   {
      signal_mgr_t::instance().register_handler(event_t::BREAK, this);
   }

   virtual void run(rt_prog_ctx_t& ctx) override;

   virtual ~stmt_delay_t()
   {
      signal_mgr_t::instance().unregister_handler(event_t::BREAK, this);
   }

protected:
   arg_list_t _args;
   volatile bool _break_delay = false;

   virtual bool notify(const event_t& ev) override;

   stmt_delay_t(const stmt_delay_t&) = delete;
   stmt_delay_t& operator=(const stmt_delay_t&) = delete;
};


/* -------------------------------------------------------------------------- */

class stmt_mdelay_t : public stmt_delay_t
{
public:
   stmt_mdelay_t(prog_ctx_t & ctx) :
      stmt_delay_t(ctx)
   {}

   stmt_mdelay_t(arg_list_t args, prog_ctx_t & ctx)
      :
      stmt_delay_t(args, ctx)
   {}

   virtual void run(rt_prog_ctx_t& ctx) override;

   stmt_mdelay_t(const stmt_mdelay_t&) = delete;
   stmt_mdelay_t& operator=(const stmt_mdelay_t&) = delete;
};


/* -------------------------------------------------------------------------- */

}

/* -------------------------------------------------------------------------- */

#endif //__NU_STMT_DELAY_H__