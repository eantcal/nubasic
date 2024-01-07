//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#pragma once


/* -------------------------------------------------------------------------- */

#include "config.h"


/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

namespace about {
#define NUBASIC_PROGRAM_NAME "nuBASIC"

#ifdef TINY_NUBASIC_VER
    static const char* const progname = NUBASIC_PROGRAM_NAME " (tiny version)";
#else
    static const char* const progname = NUBASIC_PROGRAM_NAME;
#endif
    static const char* const author = "Antonino Calderone";
    static const char* const contacts = PACKAGE_BUGREPORT;
    static const char* const homepage = "http://www.nubasic.eu";
    static const char* const license = "The MIT License (MIT)";
    static const char* const version = PACKAGE_VERSION;
    static const char* const copyright = "2014-2024";
    static const char* const description
        = "nuBASIC is a programming language from the BASIC family.\n"
          "nuBASIC has been written in C++ and distributed under MIT License."
#ifdef TINY_NUBASIC_VER
          "\nThis version of nuBASIC does not include \n"
          "any graphics, sound, mouse or windows manager support."
#endif
        ;
}


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */
