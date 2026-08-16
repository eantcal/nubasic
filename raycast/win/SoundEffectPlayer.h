// This file is part of nuRCADE (New (nu) Raycasting Classic Arcade Development Engine).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include <string>

class SoundEffectPlayer {
public:
    static bool playOnce(const std::string& path, int volumePercent = 100,
        std::string* error = nullptr);
};
