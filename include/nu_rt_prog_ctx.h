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

#ifndef __NU_RT_PROG_CTX_H__
#define __NU_RT_PROG_CTX_H__


/* -------------------------------------------------------------------------- */

#include "nu_prog_ctx.h"
#include "nu_runnable.h"

#include <unordered_map>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class rt_prog_ctx_t : public prog_ctx_t {
public:
    using return_point_t = std::pair<prog_pointer_t::line_number_t,
        prog_pointer_t::stmt_number_t>;


    rt_prog_ctx_t(const rt_prog_ctx_t&) = delete;
    rt_prog_ctx_t& operator=(const rt_prog_ctx_t&) = delete;


    // Flags used at run-time for handle control-structures
    enum {
        FLG_END_REQUEST,
        FLG_RETURN_REQUEST,
        FLG_JUMP_REQUEST,
        FLG_SKIP_TILL_NEXT
    };


    flag_map_t flag;
    bool step_mode_active = false;

    // Hash API
    using map_t = std::unordered_map<std::string, nu::variant_t>;
    std::unordered_map<std::string, map_t> hash_tbls;


    // Program counter (line, stmt)
    prog_pointer_t runtime_pc;


    // Going-to program counter (used with flags to handle control structures)
    prog_pointer_t goingto_pc;


    // Run-time descriptor file table
    file_dscrptr_tbl_t file_tbl;


    // FOR-Loop run-time data
    for_loop_rtdata_t for_loop_tbl;


    // Per-function return-value stack
    //
    using func_retval_map_t = std::map<std::string, std::deque<variant_t>>;
    func_retval_map_t function_retval_tbl;
    //

    using source_line_t = std::map<prog_pointer_t::line_number_t, std::string>;


    rt_prog_ctx_t(
        runnable_t& robj, FILE* stdout_ptr, FILE* stdin_ptr, source_line_t& sl);


    void go_to(const prog_pointer_t& pc) noexcept;
    void go_to_next() noexcept;


    runnable_t& program() { return _program_code; }


    void set_return_line(const return_point_t& return_point) noexcept;


    return_point_t get_return_line() noexcept;


    void clear_rtdata();


    // Print-out traces of run-time data
    void trace_rtdata(std::stringstream& ss);


    // Set running statement error number
    void set_errno(int errno_) noexcept { _errno = errno_; }


    // Get last error number of running program
    int get_errno() const noexcept { return _errno; }


    // Trace nested procedure calls
    return_stack_t return_stack;


    // Used by IDE to evaluate expressions
    variant_t exported_result;

private:
    runnable_t& _program_code;
    source_line_t& _source_line;

    // Running program errno
    int _errno = 0;
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_RT_PROG_CTX_H__
