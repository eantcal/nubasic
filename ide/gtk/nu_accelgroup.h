//  
// This file is part of nuBASIC IDE Project
// Copyright (c) 2017 - Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

#ifndef __NU_ACCELGROUP_H__
#define __NU_ACCELGROUP_H__


/* -------------------------------------------------------------------------- */

#include <gtk/gtk.h>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

class accelgroup_t
{
public:
    accelgroup_t(GtkAccelGroup * group = gtk_accel_group_new())
        : _group(group)
    {
    }

    GtkAccelGroup * get_internal_obj() const noexcept {
        return _group;
    }
private:
    GtkAccelGroup * _group = nullptr;
};


/* -------------------------------------------------------------------------- */

} // namespace nu


/* -------------------------------------------------------------------------- */

#endif // __NU_ACCELGROUP_H__ 
