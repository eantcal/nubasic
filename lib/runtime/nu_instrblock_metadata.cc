//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */


#include "nu_instrblock_metadata.h"

/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */
// instrblock_t

void instrblock_t::trace(std::stringstream& ss, bool include_else)
{
    ss << "exit flag : " << (flag[EXIT] ? "Y" : "N");
    ss << "  begin pc: " << pc_begin_stmt.get_line();
    ss << "  end pc: " << pc_end_stmt.get_line();

    if (include_else) {
        ss << "  else pc: " << pc_else_stmt.get_line();
    }
}


/* -------------------------------------------------------------------------- */
// instrblock_metadata_t

void instrblock_metadata_t::trace(std::stringstream& ss, bool include_else)
{
    for (auto& e : begin_tbl) {
        ss << "\t";
        e.second->trace(ss, include_else);
        ss << std::endl;
    }

    for (auto& e : exit_tbl) {
        ss << "\t";
        ss << "Exit point: " << e.first << " Block PC:" << e.second;
        ss << std::endl;
    }
}


/* -------------------------------------------------------------------------- */

void instrblock_metadata_t::clear()
{
    begin_tbl.clear();
    end_tbl.clear();
    build_stack.clear();
    exit_tbl.clear();
}


/* -------------------------------------------------------------------------- */

void instrblock_metadata_t::compile_begin(
    const prog_pointer_t& pc, const std::string& identifier)
{
    build_stack.push_front(pc.get_line());

    instrblock_t::handle_t handle = std::make_shared<instrblock_t>();

    handle->pc_begin_stmt = pc;
    handle->identifier = identifier;

    begin_tbl.insert(std::make_pair(pc.get_line(), handle));
}


/* -------------------------------------------------------------------------- */

instrblock_t::handle_t instrblock_metadata_t::compile_end(
    const prog_pointer_t& pc)
{
    if (build_stack.empty()) {
        return nullptr;
    }

    const auto begin_line = build_stack.front();
    const auto i = begin_tbl.find(begin_line);

    if (i == begin_tbl.end()) {
        return nullptr;
    }

    i->second->pc_end_stmt = pc;

    end_tbl.insert(std::make_pair(pc.get_line(), begin_line));

    build_stack.pop_front();

    return i->second;
}


/* -------------------------------------------------------------------------- */

instrblock_t::handle_t instrblock_metadata_t::compile_exit_point(
    const prog_pointer_t& pc)
{
    if (build_stack.empty()) {
        return nullptr;
    }

    const auto begin_line = build_stack.front();
    const auto i = begin_tbl.find(begin_line);

    if (i == begin_tbl.end()) {
        return nullptr;
    }

    exit_tbl.insert(std::make_pair(pc.get_line(), begin_line));

    return i->second;
}


/* -------------------------------------------------------------------------- */

instrblock_t::handle_t instrblock_metadata_t::begin_find(
    const prog_pointer_t::line_number_t& line) const
{
    const auto i = begin_tbl.find(line);

    if (i == begin_tbl.end()) {
        return nullptr;
    }

    return i->second;
}


/* -------------------------------------------------------------------------- */

instrblock_t::handle_t instrblock_metadata_t::begin_find(
    const prog_pointer_t& pc) const
{
    return begin_find(pc.get_line());
}


/* -------------------------------------------------------------------------- */

instrblock_t::handle_t instrblock_metadata_t::end_find(const prog_pointer_t& pc) const
{
    const auto i = end_tbl.find(pc.get_line());
    return i == end_tbl.end() ? nullptr : begin_find(i->second);
}


/* -------------------------------------------------------------------------- */

instrblock_t::handle_t instrblock_metadata_t::exit_find(
    const prog_pointer_t& pc) const
{
    const auto i = exit_tbl.find(pc.get_line());
    return i == exit_tbl.end() ? nullptr : begin_find(i->second);
}


/* -------------------------------------------------------------------------- */

void if_instrblock_t::clear()
{
    else_list.clear();
    condition = false;
    pc_if_stmt.reset();
    pc_endif_stmt.reset();
}


/* -------------------------------------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const if_instrblock_t& ib)
{
    os << "if line:" << ib.pc_if_stmt.get_line();
    os << " endif line:" << ib.pc_endif_stmt.get_line();
    os << " else lines:";

    for (const auto& ei : ib.else_list) {
        os << ei.get_line() << " ";
    }

    return os;
}


/* -------------------------------------------------------------------------- */

void if_instrblock_metadata_t::clear()
{
    data.clear();
    block_to_if_line_tbl.clear();

    while (!pc_stack.empty()) {
        pc_stack.pop();
    }
}


/* -------------------------------------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const if_instrblock_metadata_t& md)
{
    os << "if block:";

    for (const auto& e : md.data) {
        os << e.second << std::endl;
    }

    return os;
}


/* -------------------------------------------------------------------------- */

std::ostream& operator<<(std::ostream& os, const stop_instr_metadata_t& md)
{
    os << "stop at:";

    for (const auto& e : md.pc_stop_stmt) {
        os << e.get_line();
    }

    return os;
}


} // namespace nu
