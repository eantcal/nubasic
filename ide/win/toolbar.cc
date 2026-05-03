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

toolbar_t::toolbar_t(const HWND hWnd, const HINSTANCE hInstance,
    const UINT idi_toolbar, const UINT_PTR res_id, const int n_of_bitmaps,
    const TBBUTTON buttons[], const int n_of_buttons, const int bmwidth,
    const int bmheight, const int btwidth, const int btheight)
    : _hinstance(hInstance)
    , _hparent(hWnd)
{
    // CreateToolbarEx only supports 8-bit bitmaps; skip its bitmap parameter
    // and load a 24-bit image list manually via TB_SETIMAGELIST.
    _toolbar = CreateToolbarEx(hWnd,
        WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS | CCS_ADJUSTABLE
            | TBSTYLE_FLAT,
        idi_toolbar, 0, nullptr, 0, // no bitmap at create-time
        nullptr, 0, // no buttons at create-time
        btwidth, btheight, bmwidth, bmheight, sizeof(TBBUTTON));

    assert(_toolbar);

    // Load the resource bitmap as a DIB section (preserves full colour depth)
    HBITMAP hBmp = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(res_id),
        IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    if (hBmp) {
        _hImageList = ImageList_Create(
            bmwidth, bmheight, ILC_COLOR24 | ILC_MASK, n_of_bitmaps, 0);

        if (_hImageList) {
            // Mask out the classic button-face grey so icons blend with any
            // theme
            ImageList_AddMasked(_hImageList, hBmp, RGB(192, 192, 192));
            SendMessage(_toolbar, TB_SETIMAGELIST, 0, (LPARAM)_hImageList);
        }

        DeleteObject(hBmp);
    }

    SendMessage(_toolbar, TB_ADDBUTTONS, (WPARAM)n_of_buttons, (LPARAM)buttons);
    SendMessage(_toolbar, TB_AUTOSIZE, 0, 0);
}


/* -------------------------------------------------------------------------- */

toolbar_t::~toolbar_t()
{
    if (_hImageList)
        ImageList_Destroy(_hImageList);
}


/* -------------------------------------------------------------------------- */

void toolbar_t::on_resize() { SendMessage(_toolbar, TB_AUTOSIZE, 0L, 0L); }


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
        lpToolTipText = (LPTOOLTIPTEXT)lParam;
        {
            static const struct {
                UINT id;
                const char* tip;
            } tips[] = {
                { IDM_FILE_NEW, "New" },
                { IDM_FILE_OPEN, "Open" },
                { IDM_FILE_SAVE, "Save" },
                { IDM_DEBUG_START, "Start Debugging" },
                { IDM_DEBUG_STOP, "Stop Debugging (Shift+F5)" },
                { IDM_DEBUG_STEP_INTO, "Step Into (F11)" },
                { IDM_DEBUG_STEP_OVER, "Step Over (F10)" },
                { IDM_DEBUG_STEP_OUT, "Step Out (Shift+F11)" },
                { IDM_DEBUG_CONT, "Continue (F5)" },
                { IDM_DEBUG_TOGGLEBRK, "Toggle Breakpoint" },
                { IDM_INTERPRETER_BUILD, "Build (Ctrl+B)" },
                { IDM_DEBUG_EVALSEL, "Evaluate Selection (Ctrl+F11)" },
                { IDM_SEARCH_FIND, "Find" },
                { IDM_DEBUG_TOPMOST, "Console Topmost" },
                { IDM_DEBUG_NOTOPMOST, "IDE Topmost" },
            };
            lpToolTipText->lpszText = const_cast<LPSTR>("");
            for (const auto& t : tips) {
                if (t.id == (UINT)lpToolTipText->hdr.idFrom) {
                    lpToolTipText->lpszText = const_cast<LPSTR>(t.tip);
                    break;
                }
            }
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
