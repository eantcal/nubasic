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

#include "nu_interpreter.h"
#include "nu_program.h"
#include "nu_about.h"
#include "nu_builtin_help.h"
#include "nu_os_console.h"
#include "nu_os_std.h"
#include "nu_stdlib.h"
#include "nu_stmt_block.h"

#include <string>
#include <string.h>
#include <memory>
#include <iostream>
#include <cassert>
#include <algorithm>


/* -------------------------------------------------------------------------- */

namespace nu
{

static void quote_string(
   std::string& str,
   const char qt='\"',
   const std::string escape_s="\\" )
{
   const std::string s = str;
   str.clear();

   for (auto it = s.cbegin(); it != s.cend(); ++it)
   {
      if (*it == qt) str += escape_s;

      str += *it;
   }
}


/* -------------------------------------------------------------------------- */

interpreter_t::interpreter_t(
   FILE * std_output,
   FILE * std_input)
   :
   _prog(new program_t(_prog_line, _prog_ctx, true)),
   _prog_ctx(*_prog, std_output, std_input, _source_line),
   _parser(),
   _stdout_ptr(std_output),
   _stdin_ptr(std_input)
{
   signal_mgr_t::instance().register_handler(event_t::BREAK, this);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::notify(const event_t& ev)
{
   _break_event = ev == event_t::BREAK;
   return _break_event;
}


/* -------------------------------------------------------------------------- */

interpreter_t::~interpreter_t()
{
   signal_mgr_t::instance().unregister_handler(event_t::BREAK, this);
   delete _prog;
}


/* -------------------------------------------------------------------------- */

std::string interpreter_t::skip_blank(expr_tknzr_t& tz, nu::token_t & t)
{
   std::string blank;

   while (!tz.eol() && t.type() == tkncl_t::BLANK)
   {
      if (t.type() == tkncl_t::BLANK)
         blank += t.org_id();

      t = tz.next();
   }

   if (t.type() == tkncl_t::BLANK)
      blank += t.org_id();

   return blank;
}


/* -------------------------------------------------------------------------- */

void interpreter_t::version()
{
   using namespace about;
   fprintf(get_rt_ctx().get_stdout_ptr(),
           "%s - %s\n"    //prog ver
           "(c) %s <%s>" //(c) author <mail>
           " - %s\n"     // - <year> <CR>
           "%s\n"        // <homepage>  <CR>
           "%s\n",       // <descr>  <CR>
           progname, about::version,
           author, contacts,
           copyright,
           homepage,
           description);
}


/* -------------------------------------------------------------------------- */

void interpreter_t::clear_rtdata()
{
   get_rt_ctx().clear_rtdata();
}


/* -------------------------------------------------------------------------- */

void interpreter_t::clear_all()
{
   _prog_line.clear();
   _source_line.clear();
   get_rt_ctx().clear_rtdata();
   get_rt_ctx().clear_metadata();
   _breakpoints.clear();
}


/* -------------------------------------------------------------------------- */

void interpreter_t::rebuild(runnable_t::line_num_t to_num)
{
   //Clear precompiled program
   _prog_line.clear();

   //Clear all labels, program contexts including get, array, etc...
   get_rt_ctx().clear_rtdata();
   get_rt_ctx().clear_metadata();

   //Re-build program
   for (const auto& sl : _source_line)
   {
      if (to_num <= 0 || sl.first < to_num)
         update_program(sl.second, sl.first);
   }
}


/* -------------------------------------------------------------------------- */

void interpreter_t::erase_line(runnable_t::line_num_t num)
{
   _prog_line.erase(num);
   _source_line.erase(num);
}


//----------------------------------------------------------------------

void interpreter_t::renum_line(std::string& line, const renum_tbl_t& renum_tbl)
{
   std::string new_line;
   tokenizer_t tknzr(line);

   enum
   {
      EXPECTED_GOXX,
      EXPECTED_TO_OR_SUB,
      EXPECTED_LINENUM,
   }
   state = EXPECTED_GOXX;

   while (!tknzr.eol())
   {
      token_t t = tknzr.next();

      if (t.type() == tkncl_t::BLANK)
      {
         new_line += t.org_id();
         continue;
      }

      if (t.type() == tkncl_t::STRING_COMMENT)
      {
         new_line += t.org_id();

         while (!tknzr.eol())
         {
            token_t t = tknzr.next();
            new_line += t.org_id();
         }

         break;
      }

      if (t.type() == tkncl_t::STRING_LITERAL)
      {
         auto id = t.org_id();

         quote_string(id);

         new_line += "\"" + id + "\"";

         continue;
      }

      switch (state)
      {
         case EXPECTED_GOXX:
            if (t.type() == tkncl_t::IDENTIFIER)
            {
               if (t.identifier() == "goto")
                  state = EXPECTED_LINENUM;

               else if (t.identifier() == "gosub")
                  state = EXPECTED_LINENUM;

               else if (t.identifier() == "go")
                  state = EXPECTED_TO_OR_SUB;

               else
                  state = EXPECTED_GOXX;
            }

            new_line += t.org_id();

            break;

         case EXPECTED_TO_OR_SUB:
            state = t.identifier() == "to" || t.identifier() == "sub" ?
                    EXPECTED_LINENUM : EXPECTED_GOXX;

            new_line += t.org_id();

            break;

         case EXPECTED_LINENUM:
            state = EXPECTED_GOXX;

            if (t.type() == tkncl_t::INTEGRAL)
            {
               prog_pointer_t::line_number_t ln = 0;

               try
               {
                  ln = nu::stoi(t.org_id());
               }
               catch (...)
               {
                  ln = 0;
               }

               auto i = renum_tbl.find(ln);

               new_line += i != renum_tbl.end() ?
                           nu::to_string(i->second) :
                           t.org_id();
            }
            else
            {
               new_line += t.org_id();
            }

            break;

         default:
            new_line += t.org_id();
            break;
      }
   }

   line = std::move(new_line);
}


//----------------------------------------------------------------------

void interpreter_t::renum_prog(runnable_t::line_num_t step)
{
   if (step < 1)
      step = 10;

   runnable_t::line_num_t ln = step;

   source_line_t renumered_source;
   renum_tbl_t renum_tbl;

   for (auto line : _source_line)
   {
      renumered_source.insert(std::make_pair(ln, line.second));
      renum_tbl.insert(std::make_pair(line.first, ln));
      ln += step;
   }

   for (auto i = renumered_source.begin(); i != renumered_source.end(); ++i)
      renum_line(i->second, renum_tbl);

   // Replace renumbered breakpoints
   breakpoint_tbl_t breakpoints;

   for (const auto & rl : renum_tbl)
   {
      auto i = _breakpoints.find(rl.first);

      if (i != _breakpoints.end())
         breakpoints.insert( std::make_pair( rl.second, i->second) );
   }

   _breakpoints = breakpoints;

   _source_line = std::move(renumered_source);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::update_program(const std::string& line, int ln)
{
   tokenizer_t ltknzr(line);

   bool empty_line = ltknzr.eol();

   if (ln < 1 && empty_line)
      return false;

   nu::token_t token(ltknzr.next());
   skip_blank(ltknzr, token);

   runnable_t::line_num_t line_num = 0;
   int pos = 0;

   if (ln > 0)
   {
      line_num = ln;
   }
   else
   {
      try
      {
         line_num = nu::stoi(token.identifier());
      }
      catch (std::exception &)
      {
         return false;
      }

      pos = int ( ltknzr.tell() );
      skip_blank(ltknzr, token);

      //If you just typed the line number...
      if (ltknzr.eol())
      {
         //It means "remove that line"
         erase_line(line_num);
         rebuild();

         return true;
      }
   }

   std::string code = line.substr(pos, line.size() - pos);

   get_rt_ctx().compiletime_pc.set(line_num, 0);

   // compile may throw exceptions....
   tokenizer_t tknzr(code, pos);
   auto h = _parser.compile_line(tknzr, get_rt_ctx());

   _prog_line  [line_num].first = h;

   if (ln == 0 && ! code.empty() && code.c_str()[0]==' ')
      code = code.substr(1, code.size()-1);

   _source_line[line_num] = code;

   // Set breakpoint if any
   auto it = _breakpoints.find(line_num);

   if (it != _breakpoints.end())
   {
      _prog_line[line_num].second.break_point = true;
      _prog_line[line_num].second.condition_stmt = it->second.condition_stmt;
      _prog_line[line_num].second.condition_str = it->second.condition_str;
   }

   return true;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::save(const std::string& filepath)
{
   FILE* f = fopen(filepath.c_str(), "w+b");

   if (!f)
      return false;

   bool ret = true;

   for (const auto& line : _source_line)
   {
      //If program contains explicit reference to a line number (e.g. goto
      //or gosub statement has a line number parameter instead of label)
      //line numbers will be saved into the program file
      if (get_rt_ctx().prog_label.get_explicit_line_reference_mode() && 
         0 > fprintf(f, "%u ", line.first) )
      {
         ret = false;
         break;
      }

      if (0 > fwrite(line.second.c_str(), line.second.size(), 1, f))
      {
         ret = false;
         break;
      }

      if (0 > fwrite("\n", 1, 1, f))
      {
         ret = false;
         break;
      }
   }

   fclose(f);

   return ret;
}


/* -------------------------------------------------------------------------- */

std::string interpreter_t::read_line(FILE * f)
{
   std::string line;

   while (!feof(f) && ferror(f) == 0)
   {
      char c = fgetc(f);

      if (int(c >= 32) && int(c <= 127))
         line.push_back(c);

      if (c == '\n')
         break;
   }

   return line;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::load(FILE* f)
{
   std::string first_line = read_line(f);
   bool old_format = false;

   if (!first_line.empty())
   {
      tokenizer_t ltknzr(first_line);

      if (!ltknzr.eol())
      {
         nu::token_t token(ltknzr.next());
         skip_blank(ltknzr, token);

         if (token.type() == tkncl_t::INTEGRAL)
            old_format = true;
      }
   }

   //reset file ptr
   fseek(f, 0, SEEK_SET);

   int ln = 0;

   while (! feof(f) && ferror(f)==0)
   {
      std::string line = read_line(f);

      if (line.empty() && feof(f))
         break;

      if (line.empty() && ferror(f))
      {
         fclose(f);
         return false;
      }

      if ((!old_format || !line.empty()) &&
            !update_program(line, old_format ? 0 : ++ln))
      {
         fclose(f);
         return false;
      }
   }

   fclose(f);
   return true;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::list(
   runnable_t::line_num_t from,
   runnable_t::line_num_t to,
   const std::string grep_filter)
{

   for (const auto & line : _source_line)
   {
      bool show[2] = { true, true };

      if (from)
         show[0] = line.first >= from;

      if (to)
         show[1] = line.first <= to;

      if (show[0] && show[1])
      {
         bool bshow = true;

         if (!grep_filter.empty())
         {
            std::string s = line.second;
            std::string key = grep_filter;
            std::transform(s.begin(), s.end(), s.begin(), tolower);
            std::transform(key.begin(), key.end(), key.begin(), tolower);

            bshow = s.find(key) != std::string::npos;
         }

         if (bshow)
         {
            dbginfo_t dbg;
            program_t * ptr = dynamic_cast<program_t*>(_prog);

            if (ptr)
               ptr->get_dbg_info(line.first, dbg);

            fprintf(
               get_rt_ctx().get_stdout_ptr(),
               "%c%6u %s\n",
               dbg.break_point ? '*' : ' ',
               line.first,
               line.second.c_str());

            if (!dbg.condition_str.empty())
            {
               fprintf(
                  get_rt_ctx().get_stdout_ptr(),
                  "Breakpoint at %i %s\n",
                  line.first,
                  dbg.condition_str.c_str());
            }
         }
      }
   }

   return true;
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::set_breakpoint(
   runnable_t::line_num_t line,
   interpreter_t::breakpoint_cond_t && bp )
{
   dbginfo_t dbg;
   program_t * ptr = dynamic_cast<program_t*>(_prog);

   if (ptr)
   {
      bool res = ptr->get_dbg_info(line, dbg);

      if (!res)
         return exec_res_t::RT_ERROR;

      dbg.break_point = true;
      dbg.condition_stmt = bp.condition_stmt;
      dbg.condition_str = bp.condition_str;

      if (ptr->set_dbg_info(line, dbg))
      {
         _breakpoints.insert(std::make_pair(line, bp));
         return exec_res_t::CMD_EXEC;
      }

      return exec_res_t::RT_ERROR;
   }

   return exec_res_t::SYNTAX_ERROR;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::continue_afterbrk(runnable_t::line_num_t line)
{
   dbginfo_t dbg;
   program_t * ptr = dynamic_cast<program_t*>(_prog);

   if (ptr)
   {
      bool res = ptr->get_dbg_info(line, dbg);

      if (!res)
         return false;

      if (dbg.break_point)
      {
         dbg.break_point = false;
         dbg.continue_after_break = true;

         return ptr->set_dbg_info(line, dbg);
      }

      return true;
   }

   return false;
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::erase_breakpoint(
   runnable_t::line_num_t line)
{
   dbginfo_t dbg;
   program_t * ptr = dynamic_cast<program_t*>(_prog);

   if (ptr)
   {
      bool res = ptr->get_dbg_info(line, dbg);

      if (!res)
         return exec_res_t::RT_ERROR;

      dbg.break_point = false;

      if (ptr->set_dbg_info(line, dbg))
      {
         _breakpoints.erase(line);
         return exec_res_t::CMD_EXEC;
      }

      return exec_res_t::RT_ERROR;
   }

   return exec_res_t::SYNTAX_ERROR;
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::break_if(
   prog_pointer_t::line_number_t line,
   token_list_t & tl)
{
   if (tl.empty() ||
         tl.begin()->type() != tkncl_t::IDENTIFIER ||
         tl.begin()->identifier() != "if")
   {
      return exec_res_t::SYNTAX_ERROR;
   }

   std::string condition_str;

   for (const auto & t : tl)
   {
      if (t.type() == tkncl_t::STRING_LITERAL)
      {
         auto id = t.org_id();

         quote_string(id);

         condition_str += "\"" + id + "\"";
      }
      else
      {
         condition_str += t.org_id();
      }
   }

   auto token = *tl.begin();

   std::string id = "then";
   token_t tk_then(
      id,
      tkncl_t::IDENTIFIER,
      token.position(),
      token.expression_ptr());

   tl.data().push_back(tk_then);

   id = " ";
   token_t tk_blank(
      id,
      tkncl_t::BLANK,
      token.position(),
      token.expression_ptr());

   tl.data().push_back(tk_blank);

   id = "end";
   token_t tk_end(
      id,
      tkncl_t::IDENTIFIER,
      token.position(),
      token.expression_ptr());

   tl.data().push_back(tk_end);

   nu::stmt_t::handle_t cond_stmt(_parser.compile_line(tl, get_rt_ctx()));

   return set_breakpoint(line, breakpoint_cond_t(condition_str, cond_stmt));
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::set_step_mode(bool on)
{
   get_rt_ctx().step_mode_active = on;
   return exec_res_t::CMD_EXEC;
}


/* -------------------------------------------------------------------------- */

interpreter_t::exec_res_t interpreter_t::exec_command(const std::string& cmd)
{
   std::string command = cmd;
   tokenizer_t tknzr(command, 0);

   if (tknzr.eol())
      return exec_res_t::NOP;

   nu::token_t token(tknzr.next());
   skip_blank(tknzr, token);

   bool add_ok = false;

   //? -> PRINT ;-)
   if (token.identifier() == "?")
   {
      auto i = command.find("?");
      command =
         command.substr(0, i) + "print " +
         command.substr(i + 1, command.size() - i - 1);
      token.set_identifier("print", token_t::case_t::NOCHANGE);
      token.set_type(tkncl_t::IDENTIFIER);
   }
   else if (token.identifier() == "!")
   {
      auto i = command.find("!");

      command = command.substr(i + 1, command.size() - i - 1);

      os_shell_t::exec(command);

      return exec_res_t::CMD_EXEC;
   }
   else if (token.type() == tkncl_t::INTEGRAL)
   {
      unsigned int line = std::stoi(token.identifier());

      if (line>0)
         rebuild(line - 1);

      add_ok = update_program(command);
   }

   if (add_ok)
      return exec_res_t::UPDATE_PROG;

   if (token.type() == tkncl_t::IDENTIFIER)
   {
      std::string cmd = token.identifier();

      if (cmd == "exit")
      {
         exit(0);
         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "cd")
      {
         token = tknzr.next();
         skip_blank(tknzr, token);

         std::string arg = token.identifier();

         while (!tknzr.eol())
         {
            token = tknzr.next();
            arg += token.identifier();
         }

         return _os_change_dir(arg) ?
                exec_res_t::CMD_EXEC : exec_res_t::IO_ERROR;
      }

      if (cmd == "pwd")
      {
         std::string wd = _os_get_working_dir();
         fprintf(get_stdout_ptr(), "%s\n", wd.c_str());

         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "help")
      {
         std::string help_content;

         token = tknzr.next();
         skip_blank(tknzr, token);

         help_content = builtin_help_t::get_instance().help(
                           token.type() != tkncl_t::UNDEFINED ?
                           token.identifier() : "");

         fprintf(get_stdout_ptr(), "%s\n", help_content.c_str());

         return exec_res_t::CMD_EXEC;
      }


      if (cmd == "apropos")
      {
         std::string help_content;

         token = tknzr.next();
         skip_blank(tknzr, token);

         help_content = builtin_help_t::get_instance().apropos(
                           token.type() != tkncl_t::UNDEFINED ?
                           token.identifier() : "");

         fprintf(get_stdout_ptr(), "%s\n", help_content.c_str());

         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "ver")
      {
         version();
         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "new")
      {
         clear_all();
         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "clr")
      {
         clear_rtdata();
         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "tron")
      {
         get_rt_ctx().tracing_on = true;
         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "troff")
      {
         get_rt_ctx().tracing_on = false;
         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "load")
      {

         token = tknzr.next();

         if (tknzr.eol())
            return exec_res_t::SYNTAX_ERROR;

         skip_blank(tknzr, token);

         std::string arg = token.identifier();

         while (!tknzr.eol())
         {
            token = tknzr.next();
            arg += token.identifier();
         }

         FILE* f = fopen(arg.c_str(), "r");

         if (!f)
            return exec_res_t::IO_ERROR;

         clear_all();

         return load(f) ? exec_res_t::CMD_EXEC : exec_res_t::IO_ERROR;
      }

      if (cmd == "exec")
      {
         token = tknzr.next();

         if (tknzr.eol())
            return exec_res_t::SYNTAX_ERROR;

         skip_blank(tknzr, token);

         FILE* f = fopen(token.identifier().c_str(), "r");

         if (!f)
            return exec_res_t::IO_ERROR;

         clear_all();

         if (!load(f))
            return exec_res_t::IO_ERROR;

         run(0);

         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "save")
      {
         token = tknzr.next();

         if (tknzr.eol())
            return exec_res_t::SYNTAX_ERROR;

         skip_blank(tknzr, token);

         std::string arg = token.identifier();

         while (!tknzr.eol())
         {
            token = tknzr.next();
            arg += token.identifier();
         }

         return save(arg) ?
                exec_res_t::CMD_EXEC : exec_res_t::IO_ERROR;
      }


      if (cmd == "build")
      {
         skip_blank(tknzr, token);

         if (!tknzr.eol())
            return exec_res_t::SYNTAX_ERROR;

         rebuild();

         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "resume")
      {
         if (!run_next(get_rt_ctx().runtime_pc.get_line()))
            rebuild();

         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "cont")
      {
         auto line = get_rt_ctx().runtime_pc.get_line();
         auto stmt_id = get_rt_ctx().runtime_pc.get_stmt_pos();

         if (line == 0 || continue_afterbrk(line))
         {
            if (!cont(line, stmt_id))
               rebuild();
         }

         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "run")
      {
         token = tknzr.next();

         if (tknzr.eol())
         {
            rebuild();
            run(0);

            return is_breakpoint_active() ?
                   exec_res_t::BREAKPOINT :
                   exec_res_t::CMD_EXEC;
         }

         skip_blank(tknzr, token);

         if (token.type() == tkncl_t::INTEGRAL)
         {
            // Command "RUN <num>" does not clear the context
            run(nu::stoi(token.identifier()));

            return is_breakpoint_active() ?
                   exec_res_t::BREAKPOINT :
                   exec_res_t::CMD_EXEC;
         }

         return exec_res_t::SYNTAX_ERROR;
      }

      if (cmd == "ston")
      {
         //set step mode off
         set_step_mode(true);
         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "stoff")
      {
         //set step mode off
         set_step_mode(false);
         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "break")
      {
         token_list_t tl;
         tknzr.get_tknlst(tl);
         stmt_parser_t::remove_blank(tl);

         if (tl.empty())
         {
            for (auto & b : _breakpoints)
               list(b.first, b.first);

            return exec_res_t::CMD_EXEC;
         }

         token_t token = *tl.begin();

         prog_pointer_t::line_number_t line = 0;

         if (token.type() == tkncl_t::INTEGRAL)
         {
            line = nu::stoi(token.identifier());
            --tl;
            stmt_parser_t::remove_blank(tl);

            if (tl.empty())
               return set_breakpoint(line, breakpoint_cond_t("", nullptr));
         }

         else
         {
            return exec_res_t::SYNTAX_ERROR;
         }

         return break_if(line, tl);
      }

      if (cmd == "rmbrk")
      {
         token = tknzr.next();

         if (tknzr.eol())
            return exec_res_t::SYNTAX_ERROR;

         skip_blank(tknzr, token);

         if (token.type() == tkncl_t::INTEGRAL)
         {
            const auto line = nu::stoi(token.identifier());
            return erase_breakpoint(line);
         }

         return exec_res_t::SYNTAX_ERROR;
      }

      if (cmd == "clrbrk")
      {
         auto breakpoints = _breakpoints;

         for (const auto & line : breakpoints)
            erase_breakpoint(line.first);

         return exec_res_t::CMD_EXEC;
      }

      if (cmd == "vars")
      {
         std::stringstream ss;
         get_rt_ctx().trace_rtdata(ss);
         fprintf(get_rt_ctx().get_stdout_ptr(), "%s\n", ss.str().c_str());

         return exec_res_t::CMD_EXEC;
      }


      if (cmd == "meta")
      {
         std::stringstream ss;
         get_rt_ctx().trace_metadata(ss);
         fprintf(get_rt_ctx().get_stdout_ptr(), "%s\n", ss.str().c_str());

         return exec_res_t::CMD_EXEC;
      }


      if (cmd == "renum")
      {
         token = tknzr.next();

         if (tknzr.eol())
         {
            renum_prog(0);
            rebuild();
            return exec_res_t::CMD_EXEC;
         }

         skip_blank(tknzr, token);

         if (token.type() == tkncl_t::INTEGRAL)
         {
            renum_prog(nu::stoi(token.identifier()));
            rebuild();
            return exec_res_t::CMD_EXEC;
         }

         return exec_res_t::SYNTAX_ERROR;
      }


      if (cmd == "list")
      {
         token = tknzr.next();

         if (tknzr.eol())
         {
            return list() ?
                   exec_res_t::CMD_EXEC : exec_res_t::SYNTAX_ERROR;

         }

         skip_blank(tknzr, token);

         runnable_t::line_num_t from_line = 0;
         runnable_t::line_num_t to_line = 0;

         bool parse_minus = true;

         if (token.type() == tkncl_t::INTEGRAL)
         {
            to_line = from_line = nu::stoi(token.identifier());
         }
         else
         {
            if (token.type() == tkncl_t::OPERATOR &&
                  token.identifier() == "-")
            {
               from_line = 0; // from begin
               parse_minus = false;
            }
            else
            {
               return exec_res_t::SYNTAX_ERROR;
            }
         }

         token = tknzr.next();
         skip_blank(tknzr, token);

         if (parse_minus)
         {
            if (token.type() == tkncl_t::OPERATOR &&
                  token.identifier() == "-")
            {
               to_line = 0; // until the end
            }

            if (tknzr.eol())
            {
               return list(from_line, to_line) ?
                      exec_res_t::CMD_EXEC : exec_res_t::SYNTAX_ERROR;

            }


            token = tknzr.next();
            skip_blank(tknzr, token);
         }

         if (token.type() == tkncl_t::INTEGRAL)
            to_line = nu::stoi(token.identifier());
         else
            return exec_res_t::SYNTAX_ERROR;

         return list(from_line, to_line) ?
                exec_res_t::CMD_EXEC : exec_res_t::SYNTAX_ERROR;

      }

      if (cmd == "grep")
      {
         token = tknzr.next();

         if (tknzr.eol())
            return exec_res_t::SYNTAX_ERROR;

         skip_blank(tknzr, token);

         return list(0, 0, token.identifier()) ?
                exec_res_t::CMD_EXEC : exec_res_t::SYNTAX_ERROR;

      }
   }

   //Run BASIC in-line instruction
   tokenizer_t inlinetknzr(command, 0);
   nu::stmt_t::handle_t h(_parser.compile_line(inlinetknzr, get_rt_ctx()));

   NU_TRACE_CTX(get_rt_ctx());

   h->run(get_rt_ctx());

   NU_TRACE_CTX(get_rt_ctx());

   return exec_res_t::CMD_EXEC;
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::run(runnable_t::line_num_t line)
{
   struct _guard_t
   {
      _guard_t()
      {
         _os_config_term(false);
      }
      ~_guard_t()
      {
         _os_config_term(true);
      }
   } _guard;

   program_t prog(_prog_line, _prog_ctx, false);

   return prog.run(line);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::cont(runnable_t::line_num_t line, runnable_t::stmt_num_t stmtid)
{
   struct _guard_t
   {
      _guard_t()
      {
         _os_config_term(false);
      }
      ~_guard_t()
      {
         _os_config_term(true);
      }
   } _guard;

   program_t prog(_prog_line, _prog_ctx, false);

   return prog.cont(line, stmtid);
}


/* -------------------------------------------------------------------------- */

bool interpreter_t::run_next(runnable_t::line_num_t line)
{
   struct _guard_t
   {
      _guard_t()
      {
         _os_config_term(false);
      }
      ~_guard_t()
      {
         _os_config_term(true);
      }
   } _guard;

   program_t prog(_prog_line, _prog_ctx, false);

   return prog.run_next(line);
}


/* -------------------------------------------------------------------------- */

} // namespace nu


