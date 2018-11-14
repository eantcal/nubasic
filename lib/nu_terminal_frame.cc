//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_basic_defs.h"

#ifndef _WIN32
#ifdef __linux__
#include <linux/limits.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <vector>
#else
#include "nu_about.h"
#include <windows.h>
#endif


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

#ifdef _WIN32

int create_terminal_frame(int argc, char* argv[])
{
    SetConsoleTitle(nu::about::progname);
    return 0;
}

#else


/* -------------------------------------------------------------------------- */

int create_terminal_frame(int argc, char* argv[])
{
#ifndef TINY_NUBASIC_VER
    static char xterm_prog[] = NU_BASIC_XTERM_EXECBIN;
    static char xterm_earg[] = NU_BASIC_XTERM_EXECOPT;

    if (argc > 1 && strcmp(argv[1], NU_BASIC_XTERM_NOFRAME_SWITCH) == 0) {
        return 0;
    }

    else if (argc > 0
        && (argc < 2 || strcmp(argv[1], NU_BASIC_XTERM_FRAME_SWITCH))) 
    {
        std::string progname = argv[0];
        progname += " ";
        progname += NU_BASIC_XTERM_FRAME_SWITCH;

        int i = 1;

        while (--argc) {
            progname += " ";
            progname += argv[i];

            ++i;
        }

        std::vector<char> prog_arg(progname.size());
        strncpy(prog_arg.data(), progname.c_str(), progname.size());
        prog_arg.push_back(0);

        char* const execvp_args[]
            = { xterm_prog, xterm_earg, prog_arg.data(), 0 };

        execvp(xterm_prog, execvp_args);
    }

#endif // ! TINY_NUBASIC_VER
    return 0;
}


/* -------------------------------------------------------------------------- */

#endif


/* -------------------------------------------------------------------------- */

} // namespace nu
