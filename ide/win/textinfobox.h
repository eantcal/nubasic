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

#include "stdafx.h"
#include <sstream>


/* -------------------------------------------------------------------------- */

class txtinfobox_t {
private:
    HWND _ctrl_hwnd;

public:
    txtinfobox_t(const HWND hParentWnd, const HINSTANCE hInst,
        const std::string& fontName = "Consolas", const int fontSize = 14,
        const DWORD dwStyle = ES_READONLY | ES_MULTILINE | ES_SAVESEL);

    void update(const std::stringstream& os);
    void arrange(WORD x_pos, WORD y_pos, WORD dx, WORD dy);

    // Get control HWND for advanced operations (e.g., SendMessage)
    HWND get_hwnd() const { return _ctrl_hwnd; }
};


/* -------------------------------------------------------------------------- */
