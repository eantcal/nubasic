// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include "WorldMap.h"

#include <string>
#include <vector>

class WorldJsonLoader {
public:
    struct Result {
        bool success = false;
        std::string activeLayerId;
        std::vector<std::string> errors;
    };

    Result loadFromFile(const std::string& path, WorldMap& target,
        const std::string& layerId = std::string()) const;
    Result loadFromString(const std::string& jsonText, WorldMap& target,
        const std::string& layerId = std::string()) const;
};
