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

#ifndef __NUBASIC_TOOLBAR_H__
#define __NUBASIC_TOOLBAR_H__

#include "stdafx.h"


/* -------------------------------------------------------------------------- */

class toolbar_t 
{
private:
    HWND _toolbar;
    HINSTANCE _hinstance;
    HWND _hparent;

public:
   toolbar_t(
       HWND hParentWnd, 
       HINSTANCE hInstance, 
       UINT idi_toolbar, 
       UINT_PTR res_id,
       int n_of_bitmaps,
       TBBUTTON buttons[],
       int n_of_buttons,
       int bmwidth = 28, int bmheight = 32,
       int btwidth = 28, int btheight = 32);

   virtual void on_resize();
   virtual void on_customize();
   virtual BOOL on_notify(HWND hWnd, LPARAM lParam);

   void enable(DWORD id);
   void disable(DWORD id);

   bool get_rect(RECT& rect);

   HWND get_hwnd() const throw()
   {
      return _toolbar;
   }
};


/* -------------------------------------------------------------------------- */

#endif
