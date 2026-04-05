//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef __NU_STB_IMAGE_H__
#define __NU_STB_IMAGE_H__


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

unsigned char* image_load(char const* filename, int& x, int& y);
void image_free(unsigned char* image);

/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // ! __NU_STB_IMAGE_H__
