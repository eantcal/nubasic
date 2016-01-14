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


/* -------------------------------------------------------------------------- */

#ifndef __NUBASIC_TXTINFOBOX_H__
#define __NUBASIC_TXTINFOBOX_H__


/* -------------------------------------------------------------------------- */

#include "stdafx.h"
#include <sstream>


/* -------------------------------------------------------------------------- */

class txtinfobox_t
{
private:
    HWND _ctrl_hwnd;

public:
    txtinfobox_t(
        HWND hParentWnd, 
        HINSTANCE hInst, 
        const std::string& fontName = "Verdana",
        const int fontSize = 14,
        DWORD dwStyle = /*WS_BORDER |*/ ES_READONLY | ES_RIGHT | ES_MULTILINE);

    void update(const std::stringstream& os);
    void arrange(WORD x_pos, WORD y_pos, WORD dx, WORD dy);
};


/* -------------------------------------------------------------------------- */

#endif // __NUBASIC_TXTINFOBOX_H__