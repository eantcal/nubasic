//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "textinfobox.h"
#include "stdafx.h"

#include <cassert>
#include <sstream>


/* -------------------------------------------------------------------------- */

txtinfobox_t::txtinfobox_t(const HWND hParentWnd, const HINSTANCE hInst,
    const std::string& fontName, const int fontSize, const DWORD dwStyle)
{
    // Prefer RichEdit 2.0+ so the Output tab can style error headers.
    LoadLibraryA("Riched20.dll");
    LoadLibraryA("RichEd32.dll");

    const char* class_names[] = { "RichEdit20A", "RICHEDIT" };

    for (const auto* class_name : class_names) {
        _ctrl_hwnd = CreateWindowExA(WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME,
            class_name, NULL,
            WS_CHILD | WS_VISIBLE | dwStyle | WS_HSCROLL | WS_VSCROLL, 0, 0,
            CW_USEDEFAULT, CW_USEDEFAULT, hParentWnd, (HMENU) nullptr, hInst,
            NULL);

        if (_ctrl_hwnd) {
            break;
        }
    }

    assert(_ctrl_hwnd);

    LOGFONT lFont{ 0 };

    lFont.lfHeight = fontSize;
    lFont.lfWidth = 0;
    lFont.lfEscapement = 0;
    lFont.lfOrientation = 0;
    lFont.lfWeight = 0;
    lFont.lfItalic = 0;
    lFont.lfUnderline = 0;
    lFont.lfStrikeOut = 0;
    lFont.lfCharSet = ANSI_CHARSET;
    lFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lFont.lfQuality = ANTIALIASED_QUALITY;
    lFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;

    strncpy(lFont.lfFaceName, fontName.c_str(), sizeof(lFont.lfFaceName) - 1);

    const auto hFont = CreateFontIndirect(&lFont);

    SendMessage(_ctrl_hwnd, WM_SETFONT, (WPARAM)hFont, (DWORD)TRUE);
}


/* -------------------------------------------------------------------------- */

void txtinfobox_t::update(const std::stringstream& os)
{
    SetWindowText(_ctrl_hwnd, os.str().c_str());
}


/* -------------------------------------------------------------------------- */

void txtinfobox_t::arrange(WORD x_pos, WORD y_pos, WORD dx, WORD dy)
{
    MoveWindow(_ctrl_hwnd, x_pos, y_pos, dx, dy, TRUE);
}
