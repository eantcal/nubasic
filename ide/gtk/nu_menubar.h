//  
// This file is part of nuBASIC 
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_MENUBAR_H__
#define __NU_MENUBAR_H__


/* -------------------------------------------------------------------------- */

#include <gtk/gtk.h>
#include <cassert>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class menubar_t
{
public:
    GtkWidget * get_internal_obj() const noexcept {
        return _menubar;
    }

    menubar_t(GtkWidget* menubar = gtk_menu_bar_new()) :
        _menubar(menubar)
    {
        assert(_menubar);
    }

private:
    GtkWidget * _menubar = nullptr;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_MENUBAR_H__ 
