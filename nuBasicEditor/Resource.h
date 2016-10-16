/*
*  This file is part of nuBASIC Editor
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

//{{NO_DEPENDENCIES}}


#pragma warning(disable : 4005)

#define IDC_STATIC -1
#define IDC_LINE 1000
#define IDM_FILE_NEW 40001
#define IDM_FILE_OPEN 40002
#define IDM_FILE_SAVE 40003
#define IDM_FILE_SAVEAS 40004
#define IDM_FILE_EXIT 40005
#define IDM_EDIT_CUT 40006
#define IDM_EDIT_COPY 40007
#define IDM_EDIT_UNDO 40008
#define IDM_EDIT_REDO 40009
#define IDM_EDIT_PASTE 40010
#define IDM_EDIT_DELETE 40011
#define IDM_EDIT_SELECTALL 40012
#define ID_MENU 40013
#define IDM_DEBUG_RUN 40014
#define IDM_DEBUG_CONT 40015
#define IDM_SEARCH_GOTOLINE 40016
#define IDC_AUTOCOMPLETE 40017
#define IDM_HELP_SEARCH_KEYWORD 40018
#define IDM_SEARCH_FIND 40019
#define IDM_SEARCH_FINDANDREPLACE 40020
#define IDM_SEARCH_ADDMARKER 40021
#define IDM_ABOUT_ABOUT 40022
#define IDM_SEARCH_FINDNEXTMARKER 40023
#define IDM_SEARCH_FINDPREVIOUSMARKER 40024
#define IDM_SEARCH_DELALLMARKERS 40025
#define IDM_SETTINGS_FONT 40026
#define IDM_SETTINGS_RESETDEFAULTS 40027
#define IDM_INTERPRETER_BUILD 40028
#define IDM_DEBUG_START 40029
#define IDM_DEBUG_TOGGLEBRK 40030
#define IDM_DEBUG_REMOVEALLBRK 40031
#define IDM_DEBUG_EVALSEL 40032
#define IDM_DEBUG_GOTO_PC 40033
#define IDM_DEBUG_STEP 40034
#define IDM_DEBUG_TOPMOST 40035
#define IDM_DEBUG_NOTOPMOST 40036
#define IDM_CLEAR_INFOBOX 40037
#define IDM_COPY_INFOBOX_CLIPBOARD 40038
#define IDM_CTX_HELP 40039
#define IDM_HIDE_INFOBOX 40040
#define IDM_RESIZE_INFOBOX 40041


#define IDM_MENU_SEARCH_POS 4

// Used to map function reference
// IDM >= 41000 are dinamically assigned
#define IDM_INTERPRETER_BROWSER_FUN 41000

#define IDD_GOTOLINE 104
#define IDI_NUBASIC_ICON 100
#define IDI_NUBASIC_SPLASH 103
#define IDI_NUBASIC_TOOLBAR 105
#define IDD_DIALOG_RUNNING 107
#define IDD_VSPLITTER 108
#define IDD_HSPLITTER 109
#define IDD_INFOBOX 110


// Next default values for new objects
//
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE 106
#define _APS_NEXT_COMMAND_VALUE 40025
#define _APS_NEXT_CONTROL_VALUE 1001
#define _APS_NEXT_SYMED_VALUE 101
#endif
#endif
