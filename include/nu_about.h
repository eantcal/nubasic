//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_ABOUT_H__
#define __NU_ABOUT_H__


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
    static const char* const license = "GNU General Public License v2.0";
    static const char* const version = PACKAGE_VERSION;
    static const char* const copyright = "2014-2017";
    static const char* const description
        = "nuBASIC is a programming language from the BASIC family.\n"
          "nuBASIC is developed in C++11 and distributed under GPLv2 License."
#ifdef TINY_NUBASIC_VER
          "\nThis version of nuBASIC is 'lite' and does not include \n"
          "any graphics, sound, mouse or windows manager support."
#endif
        ;
}


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_ABOUT_H__

/* -------------------------------------------------------------------------- */
