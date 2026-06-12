// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include "SpriteSet.h"

#include <string>
#include <vector>

class SpriteMetadataLoader {
public:
    struct Result {
        bool success = false;
        SpriteSet spriteSet;
        std::vector<std::string> errors;
    };

    Result loadFromFile(const std::string& metadataPath) const;
};
