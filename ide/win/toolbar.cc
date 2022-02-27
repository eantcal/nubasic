//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "toolbar.h"
#include "resource.h"
#include "stdafx.h"

#include <cassert>


/* -------------------------------------------------------------------------- */

toolbar_t::toolbar_t(
    const HWND hWnd, 
    const HINSTANCE hInstance, 
    const UINT idi_toolbar,
    const UINT_PTR res_id, 
    const int n_of_bitmaps, 
    const TBBUTTON buttons[], 
    const int n_of_buttons,
    const int bmwidth, 
    const int bmheight, 
    const int btwidth, 
    const int btheight
)
    : _hinstance(hInstance)
    , _hparent(hWnd)
{
    _toolbar = CreateToolbarEx(
        hWnd, // parent
        WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE
            | TBSTYLE_FLAT,
        idi_toolbar, // toolbar id
        n_of_bitmaps, // number of bitmaps
        hInstance, // mod instance
        res_id, // resource ID for bitmap
        0, 0, btwidth, btheight, // width & height of buttons
        bmwidth, bmheight, // width & height of bitmaps
        sizeof(TBBUTTON)); // structure size

    assert(_toolbar);

    SendMessage(_toolbar, TB_ADDBUTTONS,
        (WPARAM)n_of_buttons, // number of buttons
        (LPARAM)buttons);
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

    switch (((LPNMHDR)lParam)->code) {
    case TTN_GETDISPINFO:
        // Display the ToolTip text.
        lpToolTipText = (LPTOOLTIPTEXT)lParam;

#if 0
        LoadString (globals::h_instance, 
            lpToolTipText->hdr.idFrom,    // string ID == cmd ID
            szBuf,
            sizeof(szBuf));
#endif
        {
            TCHAR text[4] = { 0 }; // TODO
            lpToolTipText->lpszText = /*szBuf*/ (LPSTR)text;
        }
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
    SendMessage(_toolbar, TB_ENABLEBUTTON, id, (LPARAM)MAKELONG(TRUE, 0));
}


/* -------------------------------------------------------------------------- */

void toolbar_t::disable(DWORD id)
{
    SendMessage(_toolbar, TB_ENABLEBUTTON, id, (LPARAM)MAKELONG(FALSE, 0));
}


/* -------------------------------------------------------------------------- */

bool toolbar_t::get_rect(RECT& rect) const
{
    return GetWindowRect(_toolbar, &rect) != 0;
}
