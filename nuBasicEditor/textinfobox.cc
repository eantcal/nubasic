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
*  Author: <antonino.calderone@ericsson.com>, <acaldmail@gmail.com>
*
*/


/* -------------------------------------------------------------------------- */

#include "textinfobox.h"
#include "stdafx.h"

#include <sstream>
#include <cassert>


/* -------------------------------------------------------------------------- */

txtinfobox_t::txtinfobox_t(
    HWND hParentWnd, 
    HINSTANCE hInst, 
    const std::string& fontName,
    const int fontSize,
    DWORD dwStyle)
{
    _ctrl_hwnd = CreateWindow(
        TEXT("EDIT"),   // Class name
        NULL,           // Window text
        WS_CHILD | WS_VISIBLE | dwStyle, // Window style
        0,              // x coordinate of the upper-left corner
        0,              // y coordinate of the upper-left corner
        CW_USEDEFAULT,  // Width of the edit control window
        CW_USEDEFAULT,  // Height of the edit control window
        hParentWnd,     // Window handle to parent window
        (HMENU)nullptr, // Control identifier
        hInst,          // Instance handle
        NULL);

    assert(_ctrl_hwnd);

    HANDLE hFont = NULL;
    LOGFONT lFont;

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
    lFont.lfPitchAndFamily = VARIABLE_PITCH | FF_SWISS;

    strncpy(lFont.lfFaceName, fontName.c_str(), sizeof(lFont.lfFaceName) - 1);
        
    hFont = CreateFontIndirect(&lFont);
 
    SendMessage(_ctrl_hwnd,
        WM_SETFONT, (WPARAM)hFont, (DWORD)TRUE);
}


/* -------------------------------------------------------------------------- */

void txtinfobox_t::update(const std::stringstream& os)
{
    SetWindowText(_ctrl_hwnd, os.str().c_str());
}


/* -------------------------------------------------------------------------- */

void txtinfobox_t::arrange(WORD x_pos, WORD y_pos, WORD dx, WORD dy)
{
    MoveWindow(_ctrl_hwnd,
        x_pos,
        y_pos,
        dx,
        dy,
        TRUE);
}
