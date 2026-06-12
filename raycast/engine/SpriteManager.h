// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include "SpriteSet.h"

#include <map>
#include <string>

class SpriteManager {
public:
    struct Selection {
        const SpriteSet* spriteSet = nullptr;
        const SpriteDirectionDefinition* direction = nullptr;
        uint32_t resolution = 0;
        const std::string* filePath = nullptr;
    };

    bool addSpriteSet(SpriteSet spriteSet);
    const SpriteSet* spriteSet(const std::string& name) const noexcept;
    Selection selectFrame(const std::string& spriteSetName,
        double relativeAngleDegrees, double distanceCells) const noexcept;

private:
    std::map<std::string, SpriteSet> m_spriteSets;
};
