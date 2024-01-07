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
#include <iostream>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <variant>

#include "framework.h"
#include "nu_builtin_help.h"
#include "nu_exception.h"
#include "nu_interpreter.h"
#include "nu_os_console.h"
#include "nu_os_std.h"
#include "nu_reserved_keywords.h"
#include "nu_terminal_frame.h"
#include "winconsole.h"

#include <cstdio>
#include <iostream>

void alloc_console()
{
    if (const HWND hwnd = ::GetConsoleWindow(); hwnd) {
        return;
    }

    AllocConsole();
    SetConsoleOutputCP(CP_UTF8);

    SetConsoleTitle("nuBASIC - Press CTRL+C to break execution");

    COORD dwSize; // new screen buffer size
    dwSize.X = 100;
    dwSize.Y = 1000;

    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), dwSize);

    // Redirect the CRT standard input, output, and error handles to the console
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    // Clear the error state for each of the C++ standard stream objects.
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
}

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

static int nuBASIC_console(int argc, char* argv[])
{
    nu::_os_cls();

    nu::interpreter_t nuBASIC;
    std::string command_line;
    bool first_command = false;

    if (argc >= 2) {
        int i = 1;

        while (argc-- > 1) {
            std::string param = argv[i];

            if (param != NU_BASIC_XTERM_FRAME_SWITCH
                && param != NU_BASIC_XTERM_NOFRAME_SWITCH) {
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
                        ::FreeConsole();

                        auto item = std::string(argv[++i]);
                        auto help
                            = nu::builtin_help_t::get_instance().help(item);

                        if (help.empty()) {
                            ::MessageBox(0, "Item not found", item.c_str(),
                                MB_ICONEXCLAMATION);
                        }

                        else {
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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{

    // Parse the command line
    int argc;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);

    // Allocate memory for the narrow string arguments
    char** argv = new char*[argc];

    for (int i = 0; i < argc; ++i) {
        // Compute the size of the required narrow string buffer
        int bufSize = WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);

        // Allocate buffer for narrow string
        argv[i] = new char[bufSize];

        // Convert wide string to narrow string
        WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, argv[i], bufSize, nullptr, nullptr);
    }
        
    LocalFree(argvW);

    alloc_console();
    nu::reserved_keywords_t::list();
    nu::create_terminal_frame(argc, argv);

    const auto errLevel = nuBASIC_console(argc, argv);

    for (int i = 0; i < argc; ++i) {
        delete[] argv[i];
    }
    delete[] argv;

    return errLevel;
}