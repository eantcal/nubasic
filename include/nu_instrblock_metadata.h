//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_INSTRBLOCK_METADATA_H__
#define __NU_INSTRBLOCK_METADATA_H__


/* -------------------------------------------------------------------------- */

#include "nu_flag_map.h"
#include "nu_prog_pointer.h"

#include <deque>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <set>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

// Instr block provides context for 'while' and 'if' statements
// with multi-lines syntax implementation

struct instrblock_t {
    using handle_t = std::shared_ptr<instrblock_t>;

    enum { EXIT };

    instrblock_t() noexcept { 
        flag.define(EXIT); 
    }

    instrblock_t(const instrblock_t&) = default;
    instrblock_t& operator=(const instrblock_t&) = default;

    void trace(std::stringstream& ss, bool include_else = false);

    std::string identifier;
    flag_map_t flag;
    prog_pointer_t pc_begin_stmt;
    prog_pointer_t pc_else_stmt; // used by IF...THEN...ELSE... stmt
    prog_pointer_t pc_end_stmt;
};


/* -------------------------------------------------------------------------- */

struct instrblock_metadata_t {
    using begin_tbl_t
        = std::map<prog_pointer_t::line_number_t, instrblock_t::handle_t>;

    using end_tbl_t = std::map<prog_pointer_t::line_number_t,
        prog_pointer_t::line_number_t>;

    using build_stack_t = std::deque<prog_pointer_t::line_number_t>;

protected:
    begin_tbl_t begin_tbl; // line     -> block
    end_tbl_t end_tbl; // end-line -> begin line
    build_stack_t build_stack; // build stack
    end_tbl_t exit_tbl; // exit-line -> begin line

public:
    bool empty() const noexcept { 
        return begin_tbl.empty(); 
    }

    size_t get_exit_point_cnt() const noexcept { 
        return exit_tbl.size(); 
    }


    void trace(std::stringstream& ss, bool include_else = false);

    void clear();
    void compile_begin(
        const prog_pointer_t& pc, const std::string& identifier = "");

    instrblock_t::handle_t compile_end(const prog_pointer_t& pc);
    instrblock_t::handle_t compile_exit_point(const prog_pointer_t& pc);
    instrblock_t::handle_t begin_find(
        const prog_pointer_t::line_number_t& line);
    instrblock_t::handle_t begin_find(const prog_pointer_t& pc);
    instrblock_t::handle_t end_find(const prog_pointer_t& pc);
    instrblock_t::handle_t exit_find(const prog_pointer_t& pc);
};


/* -------------------------------------------------------------------------- */

struct if_instrblock_t {
    prog_pointer_t pc_if_stmt;
    prog_pointer_t pc_endif_stmt;
    std::list<prog_pointer_t> else_list;
    bool condition = false;

    void clear();

    friend std::ostream& operator<<(
        std::ostream& os, const if_instrblock_t& ib);
};


/* -------------------------------------------------------------------------- */

struct stop_instr_metadata_t {
    std::set<prog_pointer_t> pc_stop_stmt;

    friend std::ostream& operator<<(
        std::ostream& os, const stop_instr_metadata_t& ib);
};


/* -------------------------------------------------------------------------- */

struct if_instrblock_metadata_t {
    std::stack<prog_pointer_t> pc_stack;
    std::map<prog_pointer_t, if_instrblock_t> data;
    std::map<prog_pointer_t, prog_pointer_t> block_to_if_line_tbl;

    void clear();

    friend std::ostream& operator<<(
        std::ostream& os, const if_instrblock_metadata_t& md);
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_INSTRBLOCK_METADATA_H__
