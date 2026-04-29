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
#include <string>
#include <vector>

#include "framework.h"
#include "nuBasicCLI.h"
#include "nu_cli_common.h"
#include "nu_interpreter.h"
#include "nu_os_console.h"
#include "nu_os_gdi.h"
#include "nu_os_std.h"
#include "nu_reserved_keywords.h"
#include "nu_signal_handling.h"
#include "nu_terminal_frame.h"
#include "nu_winconsole_api.h"

#include "Resource.h"
#include "nu_about.h"
#include "nu_basic_defs.h"

#include <cstdio>

/* -------------------------------------------------------------------------- */

static void show_splash(HINSTANCE hInstance)
{
    HBITMAP image = ::LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_SPLASH));
    if (!image)
        return;

    BITMAP bm = {};
    ::GetObject(image, sizeof(bm), &bm);

    const int wdx = bm.bmWidth > 0 ? bm.bmWidth : 800;
    const int wdy = bm.bmHeight > 0 ? bm.bmHeight : 400;

    RECT desktop = {};
    GetClientRect(GetDesktopWindow(), &desktop);
    const int wx = (desktop.right - desktop.left - wdx) / 2;
    const int wy = (desktop.bottom - desktop.top - wdy) / 2;

    HWND hwnd = ::CreateWindowA("STATIC", nu::about::progname,
        WS_VISIBLE | WS_POPUP, wx, wy, wdx, wdy, NULL, NULL, NULL, NULL);

    HDC hdc = GetDC(hwnd);
    HDC hdcMem = ::CreateCompatibleDC(hdc);
    auto hbmOld = ::SelectObject(hdcMem, (HGDIOBJ)image);
    ::BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
    ::SelectObject(hdcMem, hbmOld);
    ::DeleteDC(hdcMem);
    ::ReleaseDC(hwnd, hdc);

    Sleep(2000);

    ::DestroyWindow(hwnd);
    ::DeleteObject(image);
}

/* -------------------------------------------------------------------------- */

static bool should_preserve_inherited_stdio()
{
    const HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    const HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);

    const auto is_valid = [](HANDLE h) {
        return h && h != INVALID_HANDLE_VALUE
            && GetFileType(h) != FILE_TYPE_UNKNOWN;
    };

    return is_valid(hIn) && is_valid(hOut) && is_valid(hErr);
}

/* -------------------------------------------------------------------------- */

template <typename... Args>
static void cli_printf(const char* fmt, Args&&... args)
{
    if (nu::_os_get_screen_mode() == 0)
        printf(fmt, std::forward<Args>(args)...);
    else
        nu_winconsole_printf(fmt, std::forward<Args>(args)...);
}

/* -------------------------------------------------------------------------- */

static void on_console_ctrlc()
{
    nu::_ev_dispatcher(nu::signal_handler_t::event_t::BREAK);
}

static void on_readline_cancel_reprompt()
{
    cli_printf("%s", NU_BASIC_PROMPT_STR);
}

/* -------------------------------------------------------------------------- */

static int nuBASIC_console(int argc, char* argv[])
{
    const auto runtime_options = nu::cli::parse_runtime_options(argc, argv);
    nu::interpreter_t nuBASIC;
    nuBASIC.set_yield_cbk([](void*) {
        if (nu_winconsole_is_active()) {
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

    for (int j = 1; j < argc; ++j) {
        if (std::string(argv[j]) == "-a") {
            ::MessageBoxA(0, nu::cli::build_about_text().c_str(),
                nu::about::progname, MB_ICONINFORMATION | MB_OK);
            return 0;
        }
    }

    const auto bootstrap = nu::cli::build_session_bootstrap(
        argc, argv, [&argv](std::string& param, int& i, int& argc_remaining) {
            if (argc_remaining > 1 && param.size() == 2
                && param.c_str()[0] == '-'
                && param.c_str()[1] == NU_BASIC_HELP_MACRO) {
                const auto item = std::string(argv[++i]);
                const auto help = nu::cli::build_help_text(item);

                if (help == "No help found for '" + item + "'\n") {
                    ::MessageBoxA(
                        0, "Item not found", item.c_str(), MB_ICONEXCLAMATION);
                } else {
                    ::MessageBoxA(
                        0, help.c_str(), item.c_str(), MB_ICONINFORMATION);
                }

                exit(0);
            }

            return false;
        });

    if (!bootstrap.script_args.empty())
        nuBASIC.set_cli_args(bootstrap.script_args);

    std::string command;
    bool first_command = false;
    const bool batch_mode = bootstrap.batch_mode;

    if (!batch_mode || nu::_os_get_screen_mode() != 0)
        nu::_os_cls();

    if (!batch_mode) {
        if (runtime_options.machine_interface) {
            cli_printf("%s",
                nu::cli::machine_event(
                    "ready", { { "version", nuBASIC.version() } })
                    .c_str());
        } else {
            const auto ver_str = nuBASIC.version();
            cli_printf("%s", ver_str.c_str());
            cli_printf(NU_BASIC_MSG_STR__READY NU_BASIC_PROMPT_NEWLINE);
        }
    } else {
        command = bootstrap.command_line;
        first_command = true;
    }

    while (1) {
        if (nu_winconsole_is_active()) {
            if (!nu_winconsole_process_messages()) {
                break;
            }
        }

        if (!first_command) {
            auto input_result = nu::_os_input_interruptible(stdin);
            if (input_result.interrupted) {
                nuBASIC.get_and_reset_break_event();
                if (runtime_options.machine_interface)
                    cli_printf(
                        "%s", nu::cli::machine_event("interrupted").c_str());
                continue;
            }

            command = std::move(input_result.text);
        } else
            first_command = false;

        if (command.empty()) {
            if (nu::_os_get_screen_mode() == 0
                && (feof(stdin) || ferror(stdin))) {
                break;
            }
            continue;
        }

        nuBASIC.get_and_reset_break_event();
        const auto res = nu::cli::exec_command(
            nuBASIC, command,
            [](const std::string& text) { cli_printf("%s", text.c_str()); },
            runtime_options);

        // Text-only mode (no GDI window): exit immediately in batch mode.
        if (!nu_winconsole_is_active() && batch_mode) {
            break;
        }

        // GDI window is open: handle end-of-script regardless of the current
        // screen mode — SCREEN 0 in a BASIC program must not bypass this block.
        if (nu_winconsole_is_active()) {
            const bool need_key_wait
                = nu_winconsole_is_graphics_mode() || batch_mode;

            if (nu_winconsole_is_graphics_mode()) {
                nu_winconsole_show_graphics_end_prompt();
            } else if (batch_mode) {
                cli_printf("\nPress any key to exit...");
            }

            if (need_key_wait) {
                while (!nu_winconsole_key_available()
                    && !nu_winconsole_vkey_available()) {
                    if (!nu_winconsole_process_messages())
                        break;
                    Sleep(10);
                }
                if (nu_winconsole_key_available())
                    nu_winconsole_get_key();
                else if (nu_winconsole_vkey_available())
                    nu_winconsole_get_vkey();
            }
            nu_winconsole_restore_text_mode();
            nu_winconsole_set_app_mouse_input_enabled(0);

            if (batch_mode)
                break;
        }

        switch (res) {
        case nu::interpreter_t::exec_res_t::IO_ERROR:
            if (runtime_options.machine_interface) {
                cli_printf("%s",
                    nu::cli::machine_event("ioError",
                        { { "message", NU_BASIC_ERROR_STR__ERRORLOADING } })
                        .c_str());
            } else {
                cli_printf(
                    NU_BASIC_ERROR_STR__ERRORLOADING NU_BASIC_PROMPT_NEWLINE);
            }
            break;

        case nu::interpreter_t::exec_res_t::SYNTAX_ERROR:
            if (runtime_options.machine_interface) {
                cli_printf("%s",
                    nu::cli::machine_event("syntaxError",
                        { { "message", NU_BASIC_ERROR_STR__SYNTAXERROR } })
                        .c_str());
            } else {
                cli_printf(
                    NU_BASIC_ERROR_STR__SYNTAXERROR NU_BASIC_PROMPT_NEWLINE);
            }
            break;

        case nu::interpreter_t::exec_res_t::CMD_EXEC:
            if (runtime_options.machine_interface) {
                cli_printf("%s", nu::cli::machine_event("ok").c_str());
            } else {
                cli_printf(NU_BASIC_PROMPT_STR NU_BASIC_PROMPT_NEWLINE);
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
        cli_printf("%s", nu::cli::machine_event("terminated").c_str());

    return 0;
}

/* -------------------------------------------------------------------------- */

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    int argc;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);

    char** argv = new char*[argc];

    for (int i = 0; i < argc; ++i) {
        int bufSize = WideCharToMultiByte(
            CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);

        argv[i] = new char[bufSize];

        WideCharToMultiByte(
            CP_UTF8, 0, argvW[i], -1, argv[i], bufSize, nullptr, nullptr);
    }

    LocalFree(argvW);

    bool text_mode = false;
    for (int j = 1; j < argc; ++j) {
        if (std::string(argv[j]) == "-t"
            || std::string(argv[j]) == "--text-mode") {
            text_mode = true;
            break;
        }
    }

    if (text_mode) {
        const std::string exec_file = nu::cli::find_exec_file_arg(argc, argv);
        if (!exec_file.empty() && nu::cli::source_file_uses_graphics(exec_file))
            text_mode = false;
    }

    if (text_mode) {
        nu::_os_set_screen_mode(0);
        if (!should_preserve_inherited_stdio()) {
            AttachConsole(ATTACH_PARENT_PROCESS);
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
            freopen("CONIN$", "r", stdin);
        }
        setvbuf(stdout, nullptr, _IONBF, 0);
        setvbuf(stderr, nullptr, _IONBF, 0);
    } else {
        if (!nu_winconsole_init(hInstance, SW_SHOW)) {
            MessageBoxA(NULL, "Failed to initialize GDI console", "Error",
                MB_OK | MB_ICONERROR);
            return 1;
        }

        nu::set_gdi_target_window((HWND)nu_winconsole_get_hwnd());
        nu::_os_set_screen_mode(1);
        nu_winconsole_set_exit_on_close(1);
        nu_winconsole_set_ctrlc_callback(on_console_ctrlc);
        nu_winconsole_set_readline_cancel_hook(on_readline_cancel_reprompt);

        const auto rt_opts = nu::cli::parse_runtime_options(argc, argv);
        const std::string exec_file = nu::cli::find_exec_file_arg(argc, argv);

        // Change working directory to the script's folder so that relative
        // asset paths (e.g. "skyline.bmp" in plane.bas) resolve correctly.
        if (!exec_file.empty()) {
            const auto sep = exec_file.find_last_of("\\/");
            if (sep != std::string::npos) {
                const std::string dir = exec_file.substr(0, sep);
                SetCurrentDirectoryA(dir.c_str());
            }
        }

        if (!rt_opts.machine_interface && exec_file.empty())
            show_splash(hInstance);
    }

    nu::reserved_keywords_t::list();
    nu::create_terminal_frame(argc, argv);

    const auto errLevel = nuBASIC_console(argc, argv);

    if (nu_winconsole_is_active())
        nu_winconsole_shutdown();

    for (int i = 0; i < argc; ++i) {
        delete[] argv[i];
    }
    delete[] argv;

    return errLevel;
}
