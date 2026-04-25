//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#pragma once

#include "nu_interpreter.h"

#include <functional>
#include <string>
#include <utility>
#include <vector>

/* -------------------------------------------------------------------------- */

namespace nu::cli {

/* -------------------------------------------------------------------------- */

using output_fn_t = std::function<void(const std::string&)>;

struct session_bootstrap_t {
    std::vector<std::string> script_args;
    std::string command_line;
    bool batch_mode = false;
};

struct runtime_options_t {
    bool machine_interface = false;
};

std::string ascii_lower(std::string s);
std::string strip_basic_comment(const std::string& line);
std::string drop_leading_line_number(std::string line);
bool contains_basic_keyword(
    const std::string& line, const std::string& keyword);
bool source_line_uses_graphics(const std::string& raw_line);
bool source_file_uses_graphics(const std::string& filepath);
std::string find_exec_file_arg(int argc, char* argv[]);
runtime_options_t parse_runtime_options(int argc, char* argv[]);

std::string build_usage_text(const char* progname);
std::string build_help_text(const std::string& topic);
std::string build_about_text();
std::string machine_event(const std::string& event_name,
    const std::vector<std::pair<std::string, std::string>>& fields = {});

session_bootstrap_t build_session_bootstrap(int argc, char* argv[],
    const std::function<bool(std::string&, int&, int&)>& extra_option_handler
    = {});

nu::interpreter_t::exec_res_t exec_command(nu::interpreter_t& basic,
    const std::string& command, const output_fn_t& output,
    const runtime_options_t& options = {});

/* -------------------------------------------------------------------------- */

} // namespace nu::cli

/* -------------------------------------------------------------------------- */
