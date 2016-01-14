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

#include "stdafx.h"
#include "resource.h"
#include "toolbar.h"

#include <cassert>


/* -------------------------------------------------------------------------- */

toolbar_t::toolbar_t(
    HWND hWnd,
    HINSTANCE hInstance,
    UINT idi_toolbar,
    UINT_PTR res_id,
    int n_of_bitmaps,
    TBBUTTON buttons[],
    int n_of_buttons,
    int bmwidth, int bmheight,
    int btwidth, int btheight
    )
    :
    _hinstance(hInstance)
{
    _hparent = hWnd;

    _toolbar = CreateToolbarEx(
        hWnd,                  // parent
        WS_CHILD | WS_BORDER | WS_VISIBLE |
        TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE | TBSTYLE_FLAT ,
        idi_toolbar,           // toolbar id
        n_of_bitmaps,          // number of bitmaps
        hInstance,             // mod instance
        res_id,                // resource ID for bitmap
        0, 0,
        btwidth, btheight,     // width & height of buttons
        bmwidth, bmheight,     // width & height of bitmaps
        sizeof(TBBUTTON));     // structure size

    assert(_toolbar);

    SendMessage(
        _toolbar,
        TB_ADDBUTTONS,
        (WPARAM)n_of_buttons,  // number of buttons
        (LPARAM) buttons);
}


/* -------------------------------------------------------------------------- */

void toolbar_t::on_resize()
{
    SendMessage(_toolbar, TB_AUTOSIZE, 0L, 0L);
}


/* -------------------------------------------------------------------------- */

void toolbar_t::on_customize()
{
    // Double-click on the toolbar -- display the customization dialog.
    SendMessage(_toolbar, TB_CUSTOMIZE, 0L, 0L);
}


/* -------------------------------------------------------------------------- */

BOOL toolbar_t::on_notify(HWND hWnd, LPARAM lParam)
{
    static CHAR szBuf[128];

    LPTOOLTIPTEXT lpToolTipText;

    RECT rcl = { 0, 0, 100, 30 };

    switch (((LPNMHDR)lParam)->code)
    {
    case TTN_GETDISPINFO:
        // Display the ToolTip text.
        lpToolTipText = (LPTOOLTIPTEXT)lParam;

#if 0
        LoadString (globals::h_instance, 
            lpToolTipText->hdr.idFrom,    // string ID == cmd ID
            szBuf,
            sizeof(szBuf));
#endif
        lpToolTipText->lpszText = /*szBuf*/ "TODO";
        break;

    case TBN_QUERYDELETE:
        // Toolbar customization -- can we delete this button?
        return TRUE;
        break;

    case TBN_GETBUTTONINFO:
        // The toolbar needs information about a button.
        return FALSE;
        break;

    case TBN_QUERYINSERT:
        // Can this button be inserted? Just say yo.
        return TRUE;
        break;

    case TBN_CUSTHELP:
        // Need to display custom help.
        break;

    case TBN_TOOLBARCHANGE:
        on_resize();
        break;

    default:
        return TRUE;
        break;
    } // switch

    return TRUE;
}


/* -------------------------------------------------------------------------- */

void toolbar_t::enable(DWORD id)
{
    SendMessage(_toolbar, TB_ENABLEBUTTON,
        id, (LPARAM)MAKELONG(TRUE, 0));
}


/* -------------------------------------------------------------------------- */

void toolbar_t::disable(DWORD id)
{
    SendMessage(_toolbar, TB_ENABLEBUTTON,
        id, (LPARAM)MAKELONG(FALSE, 0));
}


/* -------------------------------------------------------------------------- */

bool toolbar_t::get_rect(RECT& rect)
{
    return GetWindowRect(_toolbar, &rect) != 0;
}
