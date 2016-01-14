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
*  Author: Antonino Calderone <acaldmail@gmail.com>
*
*/


/* -------------------------------------------------------------------------- */

#ifndef __NU_STMT_BLOCK_H__
#define __NU_STMT_BLOCK_H__


/* -------------------------------------------------------------------------- */

#include "nu_rt_prog_ctx.h"
#include "nu_stmt.h"
#include <list>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class stmt_block_t : public stmt_t
{
protected:
   std::deque< stmt_t::handle_t > _stmts;

public:
   using handle_t = std::shared_ptr < stmt_block_t > ;


   stmt_block_t() = delete;


   stmt_block_t(prog_ctx_t & ctx) :
      stmt_t(ctx)
   {}


   bool empty() const NU_NOEXCEPT
   {
      return _stmts.empty();
   }


   size_t size() const NU_NOEXCEPT
   {
      return _stmts.size();
   }


   int find_stmt_pos(int stmtid) const NU_NOEXCEPT;
   

   stmt_block_t& operator +=( stmt_t::handle_t stmt );


   stmt_cl_t get_cl() const NU_NOEXCEPT override
   {
      return _stmts.size() == 1 ?
             ( *_stmts.cbegin() )->get_cl() :
             stmt_t::stmt_cl_t::BLOCK_OF_CODE;
   }


   bool run_pos(rt_prog_ctx_t& ctx, int pos);


   void run(rt_prog_ctx_t& ctx) override
   {
      run_pos(ctx, 0);
   }

};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_STMT_BLOCK_H__
