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

#ifndef __NU_INTEPRETER_H__
#define __NU_INTEPRETER_H__


/* -------------------------------------------------------------------------- */

#include "nu_basic_defs.h"
#include "nu_stmt_parser.h"
#include "nu_program.h"
#include "nu_tokenizer.h"

#include <string>
#include <string.h>
#include <map>
#include <cstdio>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

class source_line_t :
   public std::map < runnable_t::line_num_t, std::string >
{};


/* -------------------------------------------------------------------------- */

class renum_tbl_t :
   public std::map < runnable_t::line_num_t, runnable_t::line_num_t >
{};


/* -------------------------------------------------------------------------- */

class interpreter_t : protected signal_handler_t
{
protected:

   volatile bool _break_event = false;


   interpreter_t(interpreter_t&) = delete;


   static std::string read_line(FILE * f);


   rt_prog_ctx_t& get_rt_ctx() NU_NOEXCEPT
   {
      return _prog_ctx;
   }


   virtual bool notify(const event_t& ev) override;


public:

   void register_break_event()
   {
      signal_mgr_t::instance().register_handler(event_t::BREAK, this);
   }


   struct breakpoint_cond_t
   {
      breakpoint_cond_t() = default;
      breakpoint_cond_t(const breakpoint_cond_t&) = default;
      breakpoint_cond_t& operator=( const breakpoint_cond_t& ) = default;

      breakpoint_cond_t(const std::string& cond_str, stmt_t::handle_t cond_stmt)
         : condition_str(cond_str), condition_stmt(cond_stmt)
      {}

      breakpoint_cond_t(breakpoint_cond_t&& other)
      {
         condition_str = std::move(other.condition_str);
         condition_stmt = std::move(other.condition_stmt);
      }

      breakpoint_cond_t& operator=( breakpoint_cond_t&& other )
      {
         if ( this != &other )
         {
            condition_str = std::move(other.condition_str);
            condition_stmt = std::move(other.condition_stmt);
         }

         return *this;
      }

      std::string condition_str;
      nu::stmt_t::handle_t condition_stmt = nullptr;
   };


   using breakpoint_tbl_t =
      std::map < prog_pointer_t::line_number_t, breakpoint_cond_t > ;


   interpreter_t(
      FILE * std_output = stdout,
      FILE * std_input = stdin);


   virtual ~interpreter_t();


   bool get_and_reset_break_event() NU_NOEXCEPT
   {
      volatile bool res = _break_event;
      _break_event = false;
      return res;
   }


   enum class exec_res_t
   {
      NOP,
      SYNTAX_ERROR,
      IO_ERROR,
      RT_ERROR,
      UPDATE_PROG,
      CMD_EXEC,
      BREAKPOINT,
   };


   exec_res_t set_step_mode(bool on);


   exec_res_t exec_command(const std::string& cmd);


   void version();


   prog_pointer_t::line_number_t get_cur_line_n() const NU_NOEXCEPT
   {
      auto line = _prog_ctx.runtime_pc.get_line();

      if ( line < 1 )
         line = _prog_ctx.compiletime_pc.get_line();

      return line;
   }


   prog_pointer_t::line_number_t get_last_line_n() const NU_NOEXCEPT
   {
      auto line = _prog_ctx.runtime_pc.get_last_line();

      if ( line < 1 )
         line = _prog_ctx.compiletime_pc.get_last_line();

      return line;
   }


   void renum_line(std::string& line, const renum_tbl_t& renum_tbl);


   bool is_breakpoint_active() const NU_NOEXCEPT
   {
      return _breakpoints.find(
                _prog_ctx.runtime_pc.get_line()) != _breakpoints.end();
   }


   void renum_prog(runnable_t::line_num_t step);


   bool update_program(const std::string& line, int ln = 0);


   void clear_all();


   const rt_prog_ctx_t& get_ctx() const NU_NOEXCEPT
   {
      return _prog_ctx;
   }


   bool has_runnable_stmt(int line) const NU_NOEXCEPT
   {
      if ( line < 0 || _prog_line.empty() )
         return false;

      prog_line_t::const_iterator it = _prog_line.find(line);

      if ( it == _prog_line.end() )
         return false;

      return ( it->second.first->get_cl() != stmt_t::stmt_cl_t::EMPTY );
   }


protected:
   runnable_t * _prog = nullptr;

   void clear_rtdata();

   void rebuild(runnable_t::line_num_t to_num = 0 /* all */);
   void erase_line(runnable_t::line_num_t num);

   bool run(runnable_t::line_num_t start_from = 0);
   bool run_next(runnable_t::line_num_t line);
   bool cont(runnable_t::line_num_t start_from, runnable_t::stmt_num_t stmtid);

   exec_res_t set_breakpoint(runnable_t::line_num_t line, breakpoint_cond_t && bp);
   exec_res_t erase_breakpoint(runnable_t::line_num_t line);
   bool continue_afterbrk(runnable_t::line_num_t line);

   bool load(FILE * f);
   bool save(const std::string& filepath);


   bool list(
      runnable_t::line_num_t from = 0,
      runnable_t::line_num_t to = 0,
      const std::string grep_filter = "");


   static std::string skip_blank(expr_tknzr_t& tz, nu::token_t & t);


   exec_res_t break_if(prog_pointer_t::line_number_t line, token_list_t & tl);


   FILE* get_stdout_ptr() const NU_NOEXCEPT
   {
      return _stdout_ptr;
   }


   FILE* get_stdin_ptr() const NU_NOEXCEPT
   {
      return _stdin_ptr;
   }


private:
   breakpoint_tbl_t _breakpoints;

   prog_line_t _prog_line;
   source_line_t _source_line;
   rt_prog_ctx_t _prog_ctx;
   nu::stmt_parser_t _parser;
   FILE *_stdout_ptr;
   FILE *_stdin_ptr;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_INTERPRETER_H__


