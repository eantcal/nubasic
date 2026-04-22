//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_about.h"
#include "nu_builtin_help.h"
#include "nu_exception.h"
#include "nu_interpreter.h"
#include "nu_os_console.h"
#include "nu_os_std.h"
#include "nu_reserved_keywords.h"
#include "nu_terminal_frame.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <variant>

#ifdef _WIN32
#include <windows.h>
#endif

/* -------------------------------------------------------------------------- */

#ifdef _WIN32

static std::string ascii_lower(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

static std::string strip_basic_comment(const std::string& line)
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

static std::string drop_leading_line_number(std::string line)
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

static bool contains_basic_keyword(
    const std::string& line, const std::string& keyword)
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

static bool source_line_uses_graphics(const std::string& raw_line)
{
    static const char* const graphics_keywords[]
        = { "ellipse", "fillellipse", "fillrect", "getpixel", "line",
              "movewindow", "plotimage", "rect", "refresh", "screen",
              "screenlock", "screenunlock", "setpixel", "textout" };

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

    for (const auto* keyword : graphics_keywords) {
        if (contains_basic_keyword(line, keyword))
            return true;
    }

    return false;
}

static bool source_file_uses_graphics(const std::string& filepath)
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

static std::string find_exec_file_arg(int argc, char* argv[])
{
    for (int j = 1; j + 1 < argc; ++j) {
        if (argv[j][0] == '-' && argv[j][1] == NU_BASIC_EXEC_MACRO
            && argv[j][2] == '\0') {
            return argv[j + 1];
        }
    }

    return "";
}

static std::string quote_arg(const std::string& arg)
{
    std::string quoted = "\"";
    for (const char c : arg) {
        if (c == '"')
            quoted += '\\';
        quoted += c;
    }
    quoted += '"';
    return quoted;
}

static std::string sibling_gdi_exe(const char* argv0)
{
    char full_path[MAX_PATH] = { 0 };
    if (GetModuleFileNameA(nullptr, full_path, MAX_PATH)) {
        std::string path = full_path;
        const auto slash = path.find_last_of("\\/");
        if (slash != std::string::npos)
            return path.substr(0, slash + 1) + "nubasicgdi.exe";
    }

    std::string path = argv0 ? argv0 : "nubasic.exe";
    const auto slash = path.find_last_of("\\/");
    if (slash != std::string::npos)
        return path.substr(0, slash + 1) + "nubasicgdi.exe";

    return "nubasicgdi.exe";
}

static bool launch_gdi_cli(int argc, char* argv[])
{
    const std::string gdi_exe = sibling_gdi_exe(argv[0]);
    std::string command_line = quote_arg(gdi_exe);

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-t" || arg == "--text-mode")
            continue;
        command_line += " " + quote_arg(arg);
    }

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    std::vector<char> mutable_cmd(command_line.begin(), command_line.end());
    mutable_cmd.push_back('\0');

    if (!CreateProcessA(nullptr, mutable_cmd.data(), nullptr, nullptr, FALSE, 0,
            nullptr, nullptr, &si, &pi)) {
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

#endif

/* -------------------------------------------------------------------------- */

static nu::interpreter_t::exec_res_t exec_command(
    nu::interpreter_t& basic, const std::string& command)
{
    try {
        const auto res = basic.exec_command(command);

        if (basic.get_and_reset_break_event())
            printf("Code execution has been interrupted by CTRL+C\n");

        if (res == nu::interpreter_t::exec_res_t::BREAKPOINT)
            printf("Execution stopped at breakpoint, line %i.\n"
                   "Type 'cont' to continue\n",
                basic.get_cur_line_n());
        else if (res == nu::interpreter_t::exec_res_t::STOP_REQ)
            printf("Execution stopped at STOP instruction, line %i.\n"
                   "Type 'cont' to continue\n",
                basic.get_cur_line_n());
        return res;
    }

    // Print out Runtime Error Messages
    catch (nu::runtime_error_t& e) {
        int line = e.get_line_num();
        line = line <= 0 ? basic.get_cur_line_n() : line;

        printf(
            "Runtime Error #%i at %i %s\n", e.get_error_code(), line, e.what());
    }

    // Print out Syntax Error Messages
    catch (std::exception& e) {
        if (basic.get_cur_line_n() > 0)
            printf("At line %i: %s\n", basic.get_cur_line_n(), e.what());

        else
            printf("%s\n", e.what());

        return nu::interpreter_t::exec_res_t::RT_ERROR;
    }

    catch (...) {
        printf("Runtime Error\n");
        return nu::interpreter_t::exec_res_t::RT_ERROR;
    }

    return nu::interpreter_t::exec_res_t::CMD_EXEC;
}

/* -------------------------------------------------------------------------- */

static void print_usage(const char* progname)
{
    printf(
        "%s %s - %s\n"
        "\n"
        "Usage:\n"
        "  %s [options] [-e <file>]\n"
        "\n"
        "Options:\n"
        "  -e <file>          Execute a nuBASIC source file and exit\n"
        "  -l <file>          Load a nuBASIC source file into the REPL\n"
        "  -t, --text-mode    Run in text/batch mode (no terminal window)\n"
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
}

static void print_help(const std::string& topic)
{
    const auto text = nu::builtin_help_t::get_instance().help(topic);

    if (text.empty() && !topic.empty())
        printf("No help found for '%s'\n", topic.c_str());
    else
        printf("%s\n", text.c_str());
}

/* -------------------------------------------------------------------------- */

static int nuBASIC_console(int argc, char* argv[])
{
    // Handle informational flags before clearing the screen
    for (int j = 1; j < argc; ++j) {
        const std::string a = argv[j];

        if (a == "--usage" || a == "-u") {
            print_usage(argv[0]);
            exit(0);
        }

        if (a == "--version" || a == "-v") {
            printf("%s %s\n", nu::about::progname, nu::about::version);
            exit(0);
        }

        if (a == "--help" || a == "-?" || a == "-h") {
            std::string topic;

            if (j + 1 < argc && argv[j + 1][0] != '-')
                topic = argv[j + 1];

            if (topic.empty())
                print_usage(argv[0]);
            else
                print_help(topic);

            exit(0);
        }
    }

    nu::interpreter_t nuBASIC;
    std::string command_line;
    bool first_command = false;

    // Pre-pass: collect script args for main() when using -e file.bas [args...]
    // argv[0] for main() is conventionally the script filename.
    {
        std::vector<std::string> script_args;
        for (int j = 1; j < argc; ++j) {
            if (argv[j][0] == '-' && argv[j][1] == NU_BASIC_EXEC_MACRO
                && argv[j][2] == '\0' && j + 1 < argc) {
                script_args.push_back(argv[j + 1]); // script name as argv[0]
                for (int k = j + 2; k < argc; ++k)
                    script_args.push_back(argv[k]);
                break;
            }
        }
        if (!script_args.empty())
            nuBASIC.set_cli_args(script_args);
    }

    if (argc >= 2) {
        int i = 1;

        while (argc-- > 1) {
            std::string param = argv[i];

            if (param != NU_BASIC_XTERM_FRAME_SWITCH
                && param != NU_BASIC_XTERM_NOFRAME_SWITCH && param != "-t"
                && param != "--text-mode") {
                if (argc > 1 && param.size() == 2 && param.c_str()[0] == '-') {
                    switch (param.c_str()[1]) {
                    case NU_BASIC_EXEC_MACRO: {
                        std::string fp = argv[++i];
                        for (auto& c : fp)
                            if (c == '\\')
                                c = '/';
                        param = "EXEC \"" + fp + "\"";
                        argc = 1; // stop after this — remaining args are CLI
                                  // args, not commands
                        break;
                    }
                    case NU_BASIC_LOAD_MACRO: {
                        std::string fp = argv[++i];
                        for (auto& c : fp)
                            if (c == '\\')
                                c = '/';
                        param = "LOAD \"" + fp + "\"";
                        --argc;
                        break;
                    }
                    }
                }

                command_line += param + " ";
            }

            ++i;
        }
    }

    std::string command;
    const bool batch_mode = !command_line.empty();

    if (!batch_mode)
        nu::_os_cls();

    if (command_line.empty()) {
        const auto ver_str = nuBASIC.version();
        printf("%s", ver_str.c_str());
        printf(NU_BASIC_MSG_STR__READY NU_BASIC_PROMPT_NEWLINE);
    }

    else {
        command = command_line;
        first_command = true;
    }

    while (1) {
        if (!first_command)
            command = nu::_os_input(stdin);

        else
            first_command = false;

        if (command.empty())
            continue;

        nuBASIC.get_and_reset_break_event();
        const auto res = exec_command(nuBASIC, command);

        // In batch mode (-e / -l from command line), exit after the command
        if (batch_mode)
            break;

        switch (res) {
        case nu::interpreter_t::exec_res_t::IO_ERROR:
            printf(NU_BASIC_ERROR_STR__ERRORLOADING NU_BASIC_PROMPT_NEWLINE);
            break;

        case nu::interpreter_t::exec_res_t::SYNTAX_ERROR:
            printf(NU_BASIC_ERROR_STR__SYNTAXERROR NU_BASIC_PROMPT_NEWLINE);
            break;

        case nu::interpreter_t::exec_res_t::CMD_EXEC:
            printf(NU_BASIC_PROMPT_STR NU_BASIC_PROMPT_NEWLINE);
            break;

        case nu::interpreter_t::exec_res_t::NOP:
        case nu::interpreter_t::exec_res_t::BREAKPOINT:
        case nu::interpreter_t::exec_res_t::STOP_REQ:
        case nu::interpreter_t::exec_res_t::RT_ERROR:
        case nu::interpreter_t::exec_res_t::UPDATE_PROG:
            break;
        }
    }

    return 0;
}

/* -------------------------------------------------------------------------- */

int main(int argc, char* argv[])
{
#ifdef _WIN32
    const std::string exec_file = find_exec_file_arg(argc, argv);
    if (!exec_file.empty() && source_file_uses_graphics(exec_file)
        && launch_gdi_cli(argc, argv)) {
        return 0;
    }

    // Console-subsystem build: always text mode — no GDI window.
    nu::_os_set_screen_mode(0);
#endif
    nu::reserved_keywords_t::list();
    nu::create_terminal_frame(argc, argv);
    return nuBASIC_console(argc, argv);
}
