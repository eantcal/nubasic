//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_OS_CONSOLE_H__
#define __NU_OS_CONSOLE_H__


/* -------------------------------------------------------------------------- */

#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

void _os_init();
void _os_locate(int y, int x);
void _os_cls();
std::string _os_input_str(int n);
std::string _os_input(FILE* finput_ptr);
int _os_kbhit();
void _os_cursor_visible(bool on);
void _os_config_term(bool on);
void _os_u16write(const std::u16string& output);

/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif //__NU_OS_CONSOLE_H__
