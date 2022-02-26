//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STDTYPE_H__
#define __NU_STDTYPE_H__

/* -------------------------------------------------------------------------- */

#include <string>
#include <memory>


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

using integer_t = int64_t;
using double_t = double;
using string_t = std::string;
using byte_t = uint8_t;
using bool_t = bool;


struct object_t {
    using handle_t = std::shared_ptr<object_t>;
    virtual ~object_t() {}
};


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // __NU_STDTYPE_H__
