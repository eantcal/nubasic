//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef __NUBASIC_TXTINFOBOX_H__
#define __NUBASIC_TXTINFOBOX_H__


/* -------------------------------------------------------------------------- */

#include "stdafx.h"
#include <sstream>


/* -------------------------------------------------------------------------- */

class txtinfobox_t {
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