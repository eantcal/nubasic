//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#include "nu_label_tbl.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

void label_tbl_t::get_err_msg(const std::string& key, std::string& err) const
{
    err = "'" + key + "' label not found";
}


/* -------------------------------------------------------------------------- */

} // namespace nu
