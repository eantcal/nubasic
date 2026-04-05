//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

// Create a dedicated Windows console to ensure nuBasic can execute graphics
// commands effectively when running in command-line mode. This is crucial for
// compatibility with Windows 11, as the Windows Terminal does not allow
// drawing directly in the graphical context. Creating a specific console
// enables proper handling of graphics commands, which are unsupported in
// the default Terminal environment.

/* -------------------------------------------------------------------------- */

#include <cassert>
#include <fcntl.h>
#include <io.h>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <variant>

#include "framework.h"
#include "nuBasicCLI.h"
#include "nu_builtin_help.h"
#include "nu_exception.h"
#include "nu_interpreter.h"
#include "nu_os_console.h"
#include "nu_os_gdi.h"
#include "nu_os_std.h"
#include "nu_reserved_keywords.h"
#include "nu_terminal_frame.h"
#include "nu_winconsole_api.h"

#include "nu_about.h"
#include "nu_basic_defs.h"
#include "nu_signal_handling.h"
#include <cstdio>

/* -------------------------------------------------------------------------- */

// Mode-aware output: routes to real printf in text/test mode (SCREEN 0),
// otherwise to the GDI console window.
template <typename... Args>
static void cli_printf(const char* fmt, Args&&... args)
{
    if (nu::_os_get_screen_mode() == 0)
        printf(fmt, std::forward<Args>(args)...);
    else
        nu_winconsole_printf(fmt, std::forward<Args>(args)...);
}

/* -------------------------------------------------------------------------- */

// Dispatch a BREAK signal to the interpreter when Ctrl+C is pressed.
static void on_console_ctrlc()
{
    nu::_ev_dispatcher(nu::signal_handler_t::event_t::BREAK);
}

// After cmd-style line cancel at the prompt, redraw the REPL prompt.
static void on_readline_cancel_reprompt()
{
    cli_printf("%s", NU_BASIC_PROMPT_STR);
}

/* -------------------------------------------------------------------------- */

static nu::interpreter_t::exec_res_t exec_command(
    nu::interpreter_t& basic, const std::string& command)
{
    try {
        const auto res = basic.exec_command(command);

        if (basic.get_and_reset_break_event())
            cli_printf("Code execution has been interrupted by CTRL+C\n");

        if (res == nu::interpreter_t::exec_res_t::BREAKPOINT)
            cli_printf("Execution stopped at breakpoint, line %i.\n"
                       "Type 'cont' to continue\n",
                basic.get_cur_line_n());
        else if (res == nu::interpreter_t::exec_res_t::STOP_REQ)
            cli_printf("Execution stopped at STOP instruction, line %i.\n"
                       "Type 'cont' to continue\n",
                basic.get_cur_line_n());
        return res;
    }

    // Print out Runtime Error Messages
    catch (nu::runtime_error_t& e) {
        int line = e.get_line_num();
        line = line <= 0 ? basic.get_cur_line_n() : line;

        cli_printf(
            "Runtime Error #%i at %i %s\n", e.get_error_code(), line, e.what());
    }

    // Print out Syntax Error Messages
    catch (std::exception& e) {
        if (basic.get_cur_line_n() > 0)
            cli_printf("At line %i: %s\n", basic.get_cur_line_n(), e.what());
        else
            cli_printf("%s\n", e.what());

        return nu::interpreter_t::exec_res_t::RT_ERROR;
    }

    catch (...) {
        cli_printf("Runtime Error\n");
        return nu::interpreter_t::exec_res_t::RT_ERROR;
    }

    return nu::interpreter_t::exec_res_t::CMD_EXEC;
}

/* -------------------------------------------------------------------------- */

static int nuBASIC_console(int argc, char* argv[])
{
    nu::_os_cls();

    nu::interpreter_t nuBASIC;
    // Pump the GDI message queue, but not after every single statement:
    // tight loops (e.g. rosettatxt.bas) would call PeekMessage ~70k times and
    // repaint constantly. Coalesced refresh + time/budget cap keeps output
    // smooth enough while improving throughput.
    nuBASIC.set_yield_cbk([](void*) {
        if (nu::_os_get_screen_mode() != 0) {
            static DWORD last_pump = 0;
            static unsigned stmt_budget = 0;
            const DWORD now = GetTickCount();
            ++stmt_budget;
            const bool due_time
                = (last_pump == 0) || ((now - last_pump) >= 16u);
            const bool due_budget = (stmt_budget >= 96u);
            if (due_time || due_budget) {
                stmt_budget = 0;
                last_pump = now;
                nu_winconsole_process_messages();
                return;
            }
        }
        ::Sleep(0);
    });

    // Handle -a (about) before anything else: it works as a standalone flag.
    for (int j = 1; j < argc; ++j) {
        if (std::string(argv[j]) == "-a") {
            char msg[512];
            snprintf(msg, sizeof(msg),
                "%s %s\nCopyright (C) %s  %s\n\n%s\n\nLicense: %s\nContact:  "
                "%s",
                nu::about::progname, nu::about::version, nu::about::copyright,
                nu::about::author, nu::about::description, nu::about::license,
                nu::about::contacts);
            ::MessageBoxA(
                0, msg, nu::about::progname, MB_ICONINFORMATION | MB_OK);
            return 0;
        }
    }

    std::string command_line;
    bool first_command = false;

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
                        --argc;
                        break;

                    case NU_BASIC_LOAD_MACRO:
                        param = "LOAD \"" + std::string(argv[++i]) + "\"";
                        --argc;
                        break;

                    case NU_BASIC_HELP_MACRO: {
                        auto item = std::string(argv[++i]);
                        auto help
                            = nu::builtin_help_t::get_instance().help(item);

                        if (help.empty()) {
                            ::MessageBox(0, "Item not found", item.c_str(),
                                MB_ICONEXCLAMATION);
                        } else {
                            ::MessageBox(0, help.c_str(), item.c_str(),
                                MB_ICONINFORMATION);
                        }

                        exit(0);
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
    // batch_mode: a command was supplied on the CLI → run it and exit.
    const bool batch_mode = !command_line.empty();

    if (command_line.empty()) {
        const auto ver_str = nuBASIC.version();
        cli_printf("%s", ver_str.c_str());
        cli_printf(NU_BASIC_MSG_STR__READY NU_BASIC_PROMPT_NEWLINE);
    } else {
        command = command_line;
        first_command = true;
    }

    while (1) {
        // In GDI mode, pump the message loop; in text mode this is a no-op.
        if (nu::_os_get_screen_mode() != 0) {
            if (!nu_winconsole_process_messages()) {
                break; // Window closed
            }
        }

        if (!first_command)
            command = nu::_os_input(stdin);
        else
            first_command = false;

        if (command.empty())
            continue;

        nuBASIC.get_and_reset_break_event();
        const auto res = exec_command(nuBASIC, command);

        // In text mode with a CLI command: exit as soon as it completes so
        // that test scripts / CI pipelines are not left hanging on stdin.
        if (nu::_os_get_screen_mode() == 0 && batch_mode) {
            break;
        }

        // In GDI mode: if a graphics program ran, show "Press any key" overlay.
        if (nu::_os_get_screen_mode() != 0) {
            if (nu_winconsole_is_graphics_mode()) {
                nu_winconsole_show_graphics_end_prompt();
                while (!nu_winconsole_key_available()
                    && !nu_winconsole_vkey_available()) {
                    if (!nu_winconsole_process_messages())
                        break; // window closed
                    Sleep(10);
                }
                // Consume the key so it doesn't bleed into the REPL input.
                if (nu_winconsole_key_available())
                    nu_winconsole_get_key();
                else if (nu_winconsole_vkey_available())
                    nu_winconsole_get_vkey();
            }
            nu_winconsole_restore_text_mode();
        }

        switch (res) {
        case nu::interpreter_t::exec_res_t::IO_ERROR:
            cli_printf(
                NU_BASIC_ERROR_STR__ERRORLOADING NU_BASIC_PROMPT_NEWLINE);
            break;

        case nu::interpreter_t::exec_res_t::SYNTAX_ERROR:
            cli_printf(NU_BASIC_ERROR_STR__SYNTAXERROR NU_BASIC_PROMPT_NEWLINE);
            break;

        case nu::interpreter_t::exec_res_t::CMD_EXEC:
            cli_printf(NU_BASIC_PROMPT_STR NU_BASIC_PROMPT_NEWLINE);
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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{

    // Parse the command line
    int argc;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);

    // Allocate memory for the narrow string arguments
    char** argv = new char*[argc];

    for (int i = 0; i < argc; ++i) {
        // Compute the size of the required narrow string buffer
        int bufSize = WideCharToMultiByte(
            CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);

        // Allocate buffer for narrow string
        argv[i] = new char[bufSize];

        // Convert wide string to narrow string
        WideCharToMultiByte(
            CP_UTF8, 0, argvW[i], -1, argv[i], bufSize, nullptr, nullptr);
    }

    LocalFree(argvW);

    // Check for -t / --text-mode before GDI init: headless/console mode.
    // SCREEN 0 is set; GDI window is not created.  Text I/O goes to the real
    // Windows console (stdout/stdin), so tests and CI pipelines can capture it.
    bool text_mode = false;
    for (int j = 1; j < argc; ++j) {
        if (std::string(argv[j]) == "-t"
            || std::string(argv[j]) == "--text-mode") {
            text_mode = true;
            break;
        }
    }

    if (text_mode) {
        nu::_os_set_screen_mode(0);
        // For a WINDOWS-subsystem binary the CRT stdio streams are not
        // connected to anything by default.  Reconnect them to the Win32
        // standard handles that the parent process (PowerShell, cmd.exe,
        // bash via WSL, etc.) already set up — which may be pipes, files,
        // or a console.  This lets the test runner capture stdout/stderr
        // normally.  Only allocate a new console as a last resort (e.g.
        // double-clicked from Explorer with no inherited handles).
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
        HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

        if (hOut == NULL || hOut == INVALID_HANDLE_VALUE) {
            // No inherited stdout: create a console window.
            AllocConsole();
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
            freopen("CONIN$", "r", stdin);
        } else {
            // Reconnect the CRT FILE* streams to the inherited Win32 handles.
            // _open_osfhandle takes ownership, so _close the intermediate fd.
            int fdOut
                = _open_osfhandle(reinterpret_cast<intptr_t>(hOut), _O_TEXT);
            int fdErr
                = _open_osfhandle(reinterpret_cast<intptr_t>(hErr), _O_TEXT);
            int fdIn = _open_osfhandle(
                reinterpret_cast<intptr_t>(hIn), _O_RDONLY | _O_TEXT);
            if (fdOut >= 0) {
                _dup2(fdOut, _fileno(stdout));
                _close(fdOut);
            }
            if (fdErr >= 0) {
                _dup2(fdErr, _fileno(stderr));
                _close(fdErr);
            }
            if (fdIn >= 0) {
                _dup2(fdIn, _fileno(stdin));
                _close(fdIn);
            }
            setvbuf(stdout, nullptr, _IONBF, 0);
            setvbuf(stderr, nullptr, _IONBF, 0);
        }
    } else {
        // Initialize GDI console
        if (!nu_winconsole_init(hInstance, SW_SHOW)) {
            MessageBoxA(NULL, "Failed to initialize GDI console", "Error",
                MB_OK | MB_ICONERROR);
            return 1;
        }

        // Set as target for GDI drawing operations
        nu::set_gdi_target_window((HWND)nu_winconsole_get_hwnd());

        // Standalone CLI: closing the top-level console window must exit the
        // process.
        nu_winconsole_set_exit_on_close(1);

        // Route Ctrl+C in the GDI window to the BASIC break-event system.
        nu_winconsole_set_ctrlc_callback(on_console_ctrlc);
        nu_winconsole_set_readline_cancel_hook(on_readline_cancel_reprompt);
    }

    nu::reserved_keywords_t::list();
    nu::create_terminal_frame(argc, argv);

    const auto errLevel = nuBASIC_console(argc, argv);

    if (!text_mode)
        nu_winconsole_shutdown();

    for (int i = 0; i < argc; ++i) {
        delete[] argv[i];
    }
    delete[] argv;

    return errLevel;
}
