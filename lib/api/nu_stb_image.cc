//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//


/* -------------------------------------------------------------------------- */

#ifndef TINY_NUBASIC_VER
#include "nu_stb_image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


/* -------------------------------------------------------------------------- */

namespace nu {


/* -------------------------------------------------------------------------- */

unsigned char* image_load(char const* filename, int& x, int& y)
{
    int comp = 0;
    stbi_uc* image = stbi_load(filename, &x, &y, &comp, 4);

    return image;
}


/* -------------------------------------------------------------------------- */

void image_free(unsigned char* image)
{
    if (image) {
        stbi_image_free(image);
    }
}


/* -------------------------------------------------------------------------- */

} // namespace nu

#endif // !TINY_NUBASIC_VER
