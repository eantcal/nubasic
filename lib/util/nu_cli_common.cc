//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_cli_common.h"
#include "nu_about.h"
#include "nu_basic_defs.h"
#include "nu_builtin_help.h"
#include "nu_exception.h"
#include "nu_reserved_keywords.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <stdio.h>

/* -------------------------------------------------------------------------- */

namespace nu::cli {

/* -------------------------------------------------------------------------- */

static bool is_machine_interface_arg(const std::string& arg)
{
    return arg == "--machine-interface" || arg == "--ide-machine-interface";
}

static std::string escape_machine_value(const std::string& value)
{
    std::string escaped;
    escaped.reserve(value.size());

    for (const char ch : value) {
        switch (ch) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\n':
            escaped += "\\n";
            break;
        default:
            escaped += ch;
            break;
        }
    }

    return escaped;
}

std::string ascii_lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

std::string strip_basic_comment(const std::string& line)
{
    bool in_string = false;

    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '"') {
            in_string = !in_string;
        } else if (line[i] == '\'' && !in_string) {
            return line.substr(0, i);
        }
    }

    return line;
}

std::string drop_leading_line_number(std::string line)
{
    size_t pos = 0;
    while (pos < line.size() && std::isspace((unsigned char)line[pos]))
        ++pos;

    const size_t number_begin = pos;
    while (pos < line.size() && std::isdigit((unsigned char)line[pos]))
        ++pos;

    if (pos > number_begin
        && (pos == line.size() || std::isspace((unsigned char)line[pos]))) {
        while (pos < line.size() && std::isspace((unsigned char)line[pos]))
            ++pos;
        return line.substr(pos);
    }

    return line;
}

bool contains_basic_keyword(const std::string& line, const std::string& keyword)
{
    size_t pos = line.find(keyword);
    while (pos != std::string::npos) {
        const auto is_ident = [](char c) {
            return std::isalnum((unsigned char)c) || c == '_' || c == '%'
                || c == '$';
        };

        const bool left_ok = pos == 0 || !is_ident(line[pos - 1]);
        const size_t right = pos + keyword.size();
        const bool right_ok = right >= line.size() || !is_ident(line[right]);

        if (left_ok && right_ok)
            return true;

        pos = line.find(keyword, pos + keyword.size());
    }

    return false;
}

bool source_line_uses_graphics(const std::string& raw_line)
{
    auto line
        = ascii_lower(drop_leading_line_number(strip_basic_comment(raw_line)));
    size_t pos = 0;
    while (pos < line.size() && std::isspace((unsigned char)line[pos]))
        ++pos;

    if (pos >= line.size())
        return false;

    if (line.compare(pos, 3, "rem") == 0
        && (pos + 3 == line.size()
            || std::isspace((unsigned char)line[pos + 3]))) {
        return false;
    }

    for (const auto& keyword : nu::reserved_keywords_t::graphics()) {
        if (contains_basic_keyword(line, keyword))
            return true;
    }

    return false;
}

bool source_file_uses_graphics(const std::string& filepath)
{
    std::ifstream in(filepath);
    if (!in)
        return false;

    std::string line;
    while (std::getline(in, line)) {
        if (source_line_uses_graphics(line))
            return true;
    }

    return false;
}

std::string find_exec_file_arg(int argc, char* argv[])
{
    for (int j = 1; j + 1 < argc; ++j) {
        if (argv[j][0] == '-' && argv[j][1] == NU_BASIC_EXEC_MACRO
            && argv[j][2] == '\0') {
            return argv[j + 1];
        }
    }

    return "";
}

runtime_options_t parse_runtime_options(int argc, char* argv[])
{
    runtime_options_t result;

    for (int i = 1; i < argc; ++i) {
        if (is_machine_interface_arg(argv[i])) {
            result.machine_interface = true;
            break;
        }
    }

    return result;
}

std::string build_usage_text(const char* progname)
{
    char buffer[2048];
    snprintf(buffer, sizeof(buffer),
        "%s %s - %s\n"
        "\n"
        "Usage:\n"
        "  %s [options] [-e <file>]\n"
        "\n"
        "Options:\n"
        "  -e <file>          Execute a nuBASIC source file and exit\n"
        "  -l <file>          Load a nuBASIC source file into the REPL\n"
        "  -t, --text-mode    Run in text/batch mode (no terminal window)\n"
        "  --machine-interface Emit machine-friendly debugger events\n"
        "  -nx                Skip spawning a new terminal window\n"
        "  -h [topic]         Show built-in language help (topic is optional)\n"
        "  -?, --help [topic] Same as -h\n"
        "  -v, --version      Print version information and exit\n"
        "\n"
        "Examples:\n"
        "  %s                        Start the interactive REPL\n"
        "  %s -e hello.bas           Execute hello.bas\n"
        "  %s -t -e hello.bas        Execute hello.bas in batch/CI mode\n"
        "  %s -h PRINT               Show help for the PRINT statement\n"
        "  %s --help                 List all commands and functions\n"
        "\n"
        "Homepage: %s\n",
        nu::about::progname, nu::about::version, nu::about::description,
        progname, progname, progname, progname, progname, progname,
        nu::about::homepage);

    return buffer;
}

std::string build_help_text(const std::string& topic)
{
    const auto text = nu::builtin_help_t::get_instance().help(topic);

    if (text.empty() && !topic.empty())
        return "No help found for '" + topic + "'\n";

    return text + "\n";
}

std::string build_about_text()
{
    char msg[512];
    snprintf(msg, sizeof(msg),
        "%s %s\nCopyright (C) %s  %s\n\n%s\n\nLicense: %s\nContact:  %s",
        nu::about::progname, nu::about::version, nu::about::copyright,
        nu::about::author, nu::about::description, nu::about::license,
        nu::about::contacts);
    return msg;
}

std::string machine_event(const std::string& event_name,
    const std::vector<std::pair<std::string, std::string>>& fields)
{
    std::string text
        = "@@nubasic event=\"" + escape_machine_value(event_name) + "\"";

    for (const auto& [key, value] : fields) {
        text += " " + key + "=\"" + escape_machine_value(value) + "\"";
    }

    text += "\n";
    return text;
}

session_bootstrap_t build_session_bootstrap(int argc, char* argv[],
    const std::function<bool(std::string&, int&, int&)>& extra_option_handler)
{
    session_bootstrap_t result;

    for (int j = 1; j < argc; ++j) {
        if (argv[j][0] == '-' && argv[j][1] == NU_BASIC_EXEC_MACRO
            && argv[j][2] == '\0' && j + 1 < argc) {
            result.script_args.push_back(argv[j + 1]);
            for (int k = j + 2; k < argc; ++k)
                result.script_args.push_back(argv[k]);
            break;
        }
    }

    if (argc >= 2) {
        int i = 1;

        while (argc-- > 1) {
            std::string param = argv[i];

            if (param != NU_BASIC_XTERM_FRAME_SWITCH
                && param != NU_BASIC_XTERM_NOFRAME_SWITCH && param != "-t"
                && param != "--text-mode" && !is_machine_interface_arg(param)) {
                bool handled = false;
                if (extra_option_handler) {
                    handled = extra_option_handler(param, i, argc);
                }

                if (!handled && argc > 1 && param.size() == 2
                    && param.c_str()[0] == '-') {
                    switch (param.c_str()[1]) {
                    case NU_BASIC_EXEC_MACRO: {
                        std::string fp = argv[++i];
                        for (auto& c : fp)
                            if (c == '\\')
                                c = '/';
                        param = "EXEC \"" + fp + "\"";
                        argc = 1;
                        handled = true;
                        break;
                    }
                    case NU_BASIC_LOAD_MACRO: {
                        std::string fp = argv[++i];
                        for (auto& c : fp)
                            if (c == '\\')
                                c = '/';
                        param = "LOAD \"" + fp + "\"";
                        --argc;
                        handled = true;
                        break;
                    }
                    }
                }

                if (!handled || !param.empty())
                    result.command_line += param + " ";
            }

            ++i;
        }
    }

    result.batch_mode = !result.command_line.empty();
    return result;
}

nu::interpreter_t::exec_res_t exec_command(nu::interpreter_t& basic,
    const std::string& command, const output_fn_t& output,
    const runtime_options_t& options)
{
    try {
        const auto res = basic.exec_command(command);
        const bool interrupted = basic.get_and_reset_break_event();

        if (interrupted) {
            output("Code execution has been interrupted by CTRL+C\n");
            if (options.machine_interface
                && res != nu::interpreter_t::exec_res_t::STOP_REQ
                && res != nu::interpreter_t::exec_res_t::BREAKPOINT) {
                output(machine_event("interrupted"));
            }
        }

        if (res == nu::interpreter_t::exec_res_t::BREAKPOINT) {
            output("Execution stopped at breakpoint, line "
                + std::to_string(basic.get_cur_line_n())
                + ".\nType 'cont' to continue\n");
            if (options.machine_interface) {
                output(machine_event("stopped",
                    { { "reason", "breakpoint" },
                        { "line", std::to_string(basic.get_cur_line_n()) } }));
            }
        } else if (res == nu::interpreter_t::exec_res_t::STOP_REQ) {
            output("Execution stopped at STOP instruction, line "
                + std::to_string(basic.get_cur_line_n())
                + ".\nType 'cont' to continue\n");
            if (options.machine_interface) {
                const auto stop_reason
                    = basic.get_rt_ctx().step_mode_active ? "step" : "stop";
                output(machine_event("stopped",
                    { { "reason", stop_reason },
                        { "line", std::to_string(basic.get_cur_line_n()) } }));
            }
        }
        return res;
    }

    catch (nu::runtime_error_t& e) {
        int line = e.get_line_num();
        line = line <= 0 ? basic.get_cur_line_n() : line;

        output("Runtime Error #" + std::to_string(e.get_error_code()) + " at "
            + std::to_string(line) + " " + e.what() + "\n");
        if (options.machine_interface) {
            output(machine_event("runtimeError",
                { { "line", std::to_string(line) },
                    { "code", std::to_string(e.get_error_code()) },
                    { "message", e.what() } }));
        }
        return nu::interpreter_t::exec_res_t::RT_ERROR;
    }

    catch (std::exception& e) {
        if (basic.get_cur_line_n() > 0)
            output("At line " + std::to_string(basic.get_cur_line_n()) + ": "
                + e.what() + "\n");
        else
            output(std::string(e.what()) + "\n");

        if (options.machine_interface) {
            output(machine_event("runtimeError",
                { { "line",
                      std::to_string(std::max(0, basic.get_cur_line_n())) },
                    { "message", e.what() } }));
        }
        return nu::interpreter_t::exec_res_t::RT_ERROR;
    }

    catch (...) {
        output("Runtime Error\n");
        if (options.machine_interface) {
            output(machine_event(
                "runtimeError", { { "message", "Runtime Error" } }));
        }
        return nu::interpreter_t::exec_res_t::RT_ERROR;
    }

    return nu::interpreter_t::exec_res_t::CMD_EXEC;
}

/* -------------------------------------------------------------------------- */

} // namespace nu::cli

/* -------------------------------------------------------------------------- */
