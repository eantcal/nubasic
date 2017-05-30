//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_RESERVED_KEYWORDS_H__
#define __NU_RESERVED_KEYWORDS_H__


/* -------------------------------------------------------------------------- */

#include <set>
#include <string>


/* -------------------------------------------------------------------------- */

namespace nu {

/* -------------------------------------------------------------------------- */

struct reserved_keywords_t {
    static const std::set<std::string> list;
};


/* -------------------------------------------------------------------------- */
}


/* -------------------------------------------------------------------------- */

#endif // __NU_RESERVED_KEYWORDS_H__
