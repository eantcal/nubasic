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

#include "nu_prog_ctx.h"
#include "nu_os_std.h"
#include "nu_global_function_tbl.h"

#include <iomanip>


/* -------------------------------------------------------------------------- */

namespace nu
{


/* -------------------------------------------------------------------------- */

prog_ctx_t::prog_ctx_t(FILE * stdout_ptr, FILE * stdin_ptr)
   :
   _stdout_ptr(stdout_ptr),
   _stdin_ptr(stdin_ptr)
{
}



/* -------------------------------------------------------------------------- */

void prog_ctx_t::clear_metadata()
{
   // Remove all compile-time control-structures metadata

   for_loop_metadata.clear();
   while_metadata.clear();
   do_loop_while_metadata.clear();
   if_metadata.clear();
   procedure_metadata.clear();
   proc_prototypes.data.clear();
   struct_prototypes.data.clear();
   struct_metadata.clear();
   prog_label.clear();
   compiletime_pc.reset();
   compiling_struct_name.clear();

   // Remove user-defined-functions
   auto & funcs = global_function_tbl_t::get_instance();

   for (auto & f : function_tbl)
      funcs.erase(f);

   function_tbl.clear();

   _stmt_id_cnt = 0;
}


/* -------------------------------------------------------------------------- */

std::vector<std::string> split(const std::string &s, char delim='.') 
{
   std::vector<std::string> elems;
   std::stringstream ss(s);
   std::string item;

   while (std::getline(ss, item, delim))
      elems.push_back(item);

   return elems;
}

/* -------------------------------------------------------------------------- */

variant_t * prog_ctx_t::get_struct_member_value(
   const std::string& qualified_variable_name,
   var_scope_t::handle_t& scope,
   size_t index)
{
   auto reflist = split(qualified_variable_name);
   const auto& member = reflist[0];

   scope = proc_scope.get(proc_scope.get_type(member));

   variant_t * value = nullptr;

   if (scope->is_defined(member))
   {
      value = &((*scope)[member]);

      size_t level = 1;
      while (value && value->is_struct() && level < reflist.size())
      {
         const auto & struct_type_name = value->struct_type_name();
         assert(!struct_type_name.empty());

         auto it = struct_prototypes.data.find(struct_type_name);

         if (it == struct_prototypes.data.end())
            return nullptr;

         //const auto & struct_prototype = it->second.second;
         const auto & member_name = reflist[level++];

         auto value_handle = value->struct_member(member_name);
         value = value_handle.get();
      }
   }
         
   return value;
}


/* -------------------------------------------------------------------------- */

void prog_ctx_t::trace_metadata(std::stringstream& ss)
{
   ss << "Explicit line number reference detected: "
      << (prog_label.get_explicit_line_reference_mode() ? "Y" : "N")
      << std::endl;

   ss << "Last compiled line : " << compiletime_pc.get_line() << std::endl;
   ss << "Last compiled stmt : " << compiletime_pc.get_stmt_pos() << std::endl;

   if (!proc_prototypes.data.empty())
   {
      ss << "Procedures Prototypes:\n";

      for (const auto& s : proc_prototypes.data)
      {
         ss << "\tline:" << std::setw(5) << s.second.first.get_line() << " ";
         ss << (function_tbl.find(s.first) != function_tbl.end() ?
                "Func " : "Sub  ");

         ss << s.first << "(";

         const auto & parameters = s.second.second.parameters;

         auto size = parameters.size();

         for (const auto& p : parameters)
         {
            ss << p.var_name;

            if (!p.type_name.empty())
               ss << " As " << p.type_name;

            if (--size > 0) ss << ",";
         }

         ss << ")\n";
      }
   }

   if (!struct_prototypes.data.empty())
   {
      ss << "Structure Prototypes:\n";

      for (const auto& s : struct_prototypes.data)
      {
         ss << "\tline:" << std::setw(5) << s.second.first.get_line() << " ";
         ss << "Struct " << s.first << "\n";
         s.second.second.describe_type(ss);
         ss << "\n";
      }
   }

   if (!procedure_metadata.empty())
   {
      ss << "Sub Procedures:\n";
      procedure_metadata.trace(ss);
      ss << std::endl;
   }

   if (!struct_metadata.empty())
   {
      ss << "Structures:\n";
      struct_metadata.trace(ss);
      ss << std::endl;
   }

   if (!prog_label.empty())
   {
      ss << "Labels:\n";
      ss << prog_label;
   }

   if (!while_metadata.empty())
   {
      ss << "While loop metadata:\n";
      while_metadata.trace(ss);
      ss << std::endl;
   }

   if (!for_loop_metadata.empty())
   {
      ss << "For-loop metadata:\n";
      for_loop_metadata.trace(ss);
      ss << std::endl;
   }

   if (!do_loop_while_metadata.empty())
   {
      ss << "Do-Loop-While metadata:\n";
      do_loop_while_metadata.trace(ss);
      ss << std::endl;
   }

   ss << "If-statement metadata:\n" << if_metadata;
}


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

