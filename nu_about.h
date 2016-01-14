/*
*  This file is part of nuBASIC
*
*  nuBASIC is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  nuBASIC is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with nuBASIC; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  US
*
*  Author: Antonino Calderone <acaldmail@gmail.com>
*
*/

#ifndef __NU_ABOUT_H__
#define __NU_ABOUT_H__


/* -------------------------------------------------------------------------- */

#include "config.h"


/* -------------------------------------------------------------------------- */

namespace nu
{

/* -------------------------------------------------------------------------- */

namespace about
{
#define NUBASIC_PROGRAM_NAME "nuBASIC"

#ifdef TINY_NUBASIC_VER
static const char * const progname = NUBASIC_PROGRAM_NAME " (tiny version)";
#else
static const char * const progname = NUBASIC_PROGRAM_NAME;
#endif
static const char * const author = "Antonino Calderone";
static const char * const contacts = PACKAGE_BUGREPORT;
static const char * const homepage = "http://www.nubasic.eu";
static const char * const license = "GNU General Public License v2.0";
static const char * const version = PACKAGE_VERSION;
static const char * const copyright = "2014, 2015";
static const char * const description =
   "nuBASIC is a programming language from the BASIC family.\n"
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

