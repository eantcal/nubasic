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

#include <cassert>
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <variant>

#ifdef WIN32
#pragma message(                                                               \
    "Warning: Use the nuBasicCLI CMake target to build the Windows GDI command-line nuBASIC (Windows 11–compatible)")
#include <Windows.h>
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

    nu::_os_cls();

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
                    case NU_BASIC_EXEC_MACRO:
                        param = "EXEC \"" + std::string(argv[++i]) + "\"";
                        argc = 1; // stop after this — remaining args are CLI
                                  // args, not commands
                        break;

                    case NU_BASIC_LOAD_MACRO:
                        param = "LOAD \"" + std::string(argv[++i]) + "\"";
                        --argc;
                        break;
                    }
                }

                command_line += param + " ";
            }

            ++i;
        }
    }

    std::string command;
    const bool batch_mode = !command_line.empty();

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
    nu::reserved_keywords_t::list();
    nu::create_terminal_frame(argc, argv);
    return nuBASIC_console(argc, argv);
}
