//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_rt_prog_ctx.h"
#include "nu_global_function_tbl.h"
#include "nu_os_std.h"

#include <iomanip>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

rt_prog_ctx_t::rt_prog_ctx_t(
    runnable_t& robj, FILE* stdout_ptr, FILE* stdin_ptr, source_line_t& sl)
    : prog_ctx_t(stdout_ptr, stdin_ptr)
    , _program_code(robj)
    , _source_line(sl)
{
    flag.define(FLG_END_REQUEST);
    flag.define(FLG_RETURN_REQUEST);
    flag.define(FLG_JUMP_REQUEST);
    flag.define(FLG_SKIP_TILL_NEXT);
    flag.define(FLG_STOP_REQUEST);
    step_mode_active = false;
}


/* -------------------------------------------------------------------------- */

void rt_prog_ctx_t::go_to(const prog_pointer_t& pc) noexcept
{
    goingto_pc = pc;
    flag.set(FLG_JUMP_REQUEST, true);
}


/* -------------------------------------------------------------------------- */

void rt_prog_ctx_t::go_to_next() noexcept
{
    goingto_pc.go_to(0);
    flag.set(FLG_JUMP_REQUEST, false);
}


/* -------------------------------------------------------------------------- */

void rt_prog_ctx_t::set_return_line(
    const rt_prog_ctx_t::return_point_t& rp) noexcept
{
    if (rp.first > 0) {
        return_stack.push_front(rp);
    }
}


/* -------------------------------------------------------------------------- */

rt_prog_ctx_t::return_point_t rt_prog_ctx_t::get_return_line() noexcept
{
    if (return_stack.empty()) {
        return std::make_pair(0, 0);
    }

    auto resume_line = return_stack.front();
    return_stack.pop_front();

    return resume_line;
}


/* -------------------------------------------------------------------------- */

void rt_prog_ctx_t::clear_rtdata()
{
    // Reset control-structure flags
    flag.reset_all();

    // Reset "goig-to" program counter
    goingto_pc.set(0, 0);

    // Reset RETURN stack
    return_stack.clear();

    // Reset program counter
    runtime_pc.reset();

    // Clear FOR-loop runtime data
    for_loop_tbl.clear();

    // Close open files and frees related resources
    file_tbl.clear();

    // Clear all variables
    proc_scope.clear();

    // Reset error number
    set_errno(0);

    // Clear function return-value table
    function_retval_tbl.clear();

    // Clear hash tables
    hash_tbls.clear();

    // Clear read-data store
    read_data_store.clear();
    read_data_store_index = 0;
}


/* -------------------------------------------------------------------------- */

void rt_prog_ctx_t::trace_rtdata(std::stringstream& ss)
{
    const prog_pointer_t::line_number_t gotoline = goingto_pc.get_line();

    const auto li = _source_line.find(runtime_pc.get_line());

    if (li != _source_line.end()) {
        ss << std::setw(5) << runtime_pc.get_line() << " ";
        ss << " " << li->second << std::endl;
    }
    else {
        ss << "Current line : " << runtime_pc.get_line() << std::endl;
    }

    ss << "Go-To line : "
       << std::string(gotoline == 0 ? "next" : nu::to_string(gotoline))
       << std::endl;
    ss << "End request pending: "
       << (flag[rt_prog_ctx_t::FLG_END_REQUEST] ? "Y" : "N") << std::endl;
    ss << "Stop request pending: "
        << (flag[rt_prog_ctx_t::FLG_STOP_REQUEST] ? "Y" : "N") << std::endl;
    ss << "Return request pending: "
       << (flag[rt_prog_ctx_t::FLG_RETURN_REQUEST] ? "Y" : "N") << std::endl;
    ss << "Skip-till-NEXT request pending: "
       << (flag[rt_prog_ctx_t::FLG_SKIP_TILL_NEXT] ? "Y" : "N") << std::endl;
    ss << "Jump request pending: "
       << (flag[rt_prog_ctx_t::FLG_JUMP_REQUEST] ? "Y" : "N") << std::endl;
    ss << "Step mode on: " << (step_mode_active ? "Y" : "N") << std::endl;

    const auto var = proc_scope.get();
    const auto &scope_id = proc_scope.get_scope_id();

    if (!scope_id.empty() && !var->empty()) {
        ss << "Variables";
        ss << "[" << scope_id << "]";
        ss << ":" << std::endl;
        ss << *var;
    }

    const auto gvar = proc_scope.get(proc_scope_t::type_t::GLOBAL);

    if (!gvar->empty()) {
        ss << "Variables [<GLOBAL>]:" << std::endl;
        ss << *gvar;
    }

    if (!hash_tbls.empty()) {
        ss << "Hash Tables:" << std::endl;

        for (const auto & tbl : hash_tbls) {
            ss << "hash['" << tbl.first << "'](" << tbl.second.size() << ") = " << std::endl;

            if (!tbl.second.empty()) {
                auto it = tbl.second.begin();

                for (size_t i = 0; it != tbl.second.end() && i < 10; ++i, ++it) {
                    ss << "\t" << it->first << ":" << it->second << std::endl;
                }

                if (tbl.second.size()>10) {
                    ss << "\t" << "..." << std::endl;
                }
            }
        }
    }

    if (!read_data_store.empty()) {
        ss << "Read-Data store:" << std::endl;
        ss << "\t" << "Index = " << read_data_store_index << std::endl;

        size_t size = read_data_store.size();
        if (size > 10)
            size = 10;

        for (size_t i = 0; i < size; ++i) {
            ss << "\t[" << i << "] -> " << read_data_store[i] << std::endl;
        }

        if (size > 10) {
            ss << "\t" << "..." << std::endl;
        }
    }

    if (!function_retval_tbl.empty()) {
        ss << "Function return values : " << std::endl;

        for (const auto& tbl : function_retval_tbl) {
            ss << "\t" << tbl.first << ": ";

            for (const auto& rv : tbl.second) {
                ss << "[" << rv.to_str() << "] ";
            }
        }

        ss << std::endl;
    }

    if (!return_stack.empty()) {
        return_stack.trace(ss);
    }

    if (!for_loop_tbl.empty()) {
        for_loop_tbl.trace(ss);
    }

    if (!file_tbl.empty()) {
        file_tbl.trace(ss);
    }
}


/* -------------------------------------------------------------------------- */

void return_stack_t::trace(std::stringstream& ss)
{
    ss << "RETURN stack: ";

    for (const auto &e : *this) {
        ss << e.first << " (stmt_id=" << e.second << ") ";
    }

    ss << std::endl;
}


/* -------------------------------------------------------------------------- */

} // namespace nu
