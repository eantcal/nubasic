/*
*  This file is part of nuBASIC
*
*  nuBASIC is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  nuBASIC is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with nuBASIC; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
*
*  Author: <antonino.calderone@ericsson.com>, <acaldmail@gmail.com>
*
*/


/* -------------------------------------------------------------------------- */

#include "nu_rt_prog_ctx.h"
#include "nu_stmt_let.h"
#include "nu_error_codes.h"
#include "nu_global_function_tbl.h"
#include "nu_tokenizer.h"
#include "nu_expr_parser.h"


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

variant_t eval_expr(rt_prog_ctx_t & ctx, std::string data)
{
   try
   {
      nu::token_list_t tl;
      tokenizer_t st(data);
      expr_parser_t ep;
      auto h = ep.compile(st);

      return h->eval(ctx).to_str();
   }
   catch (nu::exception_t&)
   {
      return variant_t(std::string("Syntax Error"));
   }
   catch (std::exception&)
   {
      return variant_t(std::string("Runtime Error"));
   }
}


/* -------------------------------------------------------------------------- */

} // namespace nu
