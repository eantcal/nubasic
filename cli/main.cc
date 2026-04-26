//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_about.h"
#include "nu_cli_common.h"
#include "nu_interpreter.h"
#include "nu_os_console.h"
#include "nu_os_std.h"
#include "nu_reserved_keywords.h"
#include "nu_terminal_frame.h"

#include <stdlib.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

/* -------------------------------------------------------------------------- */

#ifdef _WIN32

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

static bool has_text_mode_arg(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-t" || arg == "--text-mode")
            return true;
    }

    return false;
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

static int nuBASIC_console(int argc, char* argv[])
{
    const auto runtime_options = nu::cli::parse_runtime_options(argc, argv);

    for (int j = 1; j < argc; ++j) {
        const std::string a = argv[j];

        if (a == "--usage" || a == "-u") {
            printf("%s", nu::cli::build_usage_text(argv[0]).c_str());
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
                printf("%s", nu::cli::build_usage_text(argv[0]).c_str());
            else
                printf("%s", nu::cli::build_help_text(topic).c_str());

            exit(0);
        }
    }

    nu::interpreter_t nuBASIC;
    const auto bootstrap = nu::cli::build_session_bootstrap(argc, argv);
    bool first_command = false;

    if (!bootstrap.script_args.empty())
        nuBASIC.set_cli_args(bootstrap.script_args);

    std::string command;
    const bool batch_mode = bootstrap.batch_mode;

    if (!batch_mode)
        nu::_os_cls();

    if (!batch_mode) {
        if (runtime_options.machine_interface) {
            printf("%s",
                nu::cli::machine_event(
                    "ready", { { "version", nuBASIC.version() } })
                    .c_str());
        } else {
            const auto ver_str = nuBASIC.version();
            printf("%s", ver_str.c_str());
            printf(NU_BASIC_MSG_STR__READY NU_BASIC_PROMPT_NEWLINE);
        }
    } else {
        command = bootstrap.command_line;
        first_command = true;
    }

    while (1) {
        if (!first_command) {
            auto input_result = nu::_os_input_interruptible(stdin);
            if (input_result.interrupted) {
                nuBASIC.get_and_reset_break_event();
                if (runtime_options.machine_interface)
                    printf("%s", nu::cli::machine_event("interrupted").c_str());
                continue;
            }

            command = std::move(input_result.text);
        } else
            first_command = false;

        if (command.empty()) {
            if (feof(stdin) || ferror(stdin))
                break;
            continue;
        }

        nuBASIC.get_and_reset_break_event();
        const auto res = nu::cli::exec_command(
            nuBASIC, command,
            [](const std::string& text) { printf("%s", text.c_str()); },
            runtime_options);

        if (batch_mode)
            break;

        switch (res) {
        case nu::interpreter_t::exec_res_t::IO_ERROR:
            if (runtime_options.machine_interface) {
                printf("%s",
                    nu::cli::machine_event("ioError",
                        { { "message", NU_BASIC_ERROR_STR__ERRORLOADING } })
                        .c_str());
            } else {
                printf(
                    NU_BASIC_ERROR_STR__ERRORLOADING NU_BASIC_PROMPT_NEWLINE);
            }
            break;

        case nu::interpreter_t::exec_res_t::SYNTAX_ERROR:
            if (runtime_options.machine_interface) {
                printf("%s",
                    nu::cli::machine_event("syntaxError",
                        { { "message", NU_BASIC_ERROR_STR__SYNTAXERROR } })
                        .c_str());
            } else {
                printf(NU_BASIC_ERROR_STR__SYNTAXERROR NU_BASIC_PROMPT_NEWLINE);
            }
            break;

        case nu::interpreter_t::exec_res_t::CMD_EXEC:
            if (runtime_options.machine_interface) {
                printf("%s", nu::cli::machine_event("ok").c_str());
            } else {
                printf(NU_BASIC_PROMPT_STR NU_BASIC_PROMPT_NEWLINE);
            }
            break;

        case nu::interpreter_t::exec_res_t::NOP:
        case nu::interpreter_t::exec_res_t::BREAKPOINT:
        case nu::interpreter_t::exec_res_t::STOP_REQ:
        case nu::interpreter_t::exec_res_t::RT_ERROR:
        case nu::interpreter_t::exec_res_t::UPDATE_PROG:
            break;
        }
    }

    if (runtime_options.machine_interface)
        printf("%s", nu::cli::machine_event("terminated").c_str());

    return 0;
}

/* -------------------------------------------------------------------------- */

int main(int argc, char* argv[])
{
#ifdef _WIN32
    const std::string exec_file = nu::cli::find_exec_file_arg(argc, argv);
    const bool force_text_mode = has_text_mode_arg(argc, argv);
    // Keep CLI/test/CI behavior deterministic: when -t is present we must stay
    // on the text executable and never auto-route into the GDI frontend.
    if (!force_text_mode && !exec_file.empty()
        && nu::cli::source_file_uses_graphics(exec_file)
        && launch_gdi_cli(argc, argv)) {
        return 0;
    }

    nu::_os_set_screen_mode(0);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
#endif
    nu::reserved_keywords_t::list();
    nu::create_terminal_frame(argc, argv);
    return nuBASIC_console(argc, argv);
}
