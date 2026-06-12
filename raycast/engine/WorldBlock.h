// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include "MapCell.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace WinRayCast {

using BlockId = uint8_t;

enum class WallSpanKind : uint8_t { Solid = 0, Transparent = 1 };

enum class WallFace : uint8_t { North = 0, East = 1, South = 2, West = 3 };

struct WallTextureAnimation {
    // -1 means all faces; otherwise use the WallFace ordinal.
    int face = -1;
    double frameDurationSeconds = 0.12;
    bool loop = true;
    std::vector<MapCell::TextureKey> textureKeys;
};

struct WallSpan {
    WallSpan() = default;

    WallSpan(MapCell::TextureKey texture, int32_t spanBottom, int32_t spanTop,
        WallSpanKind spanKind, bool collides) noexcept
        : textureKey(texture)
        , bottom(spanBottom)
        , top(spanTop)
        , kind(spanKind)
        , collision(collides)
    {
    }

    MapCell::TextureKey textureKey = 0;
    std::array<MapCell::TextureKey, 4> faceTextureKeys{};
    MapCell::TextureKey interiorTextureKey = 0;
    std::array<bool, 4> facesEnabled{ true, true, true, true };
    std::vector<WallTextureAnimation> baseAnimations;
    std::vector<WallTextureAnimation> overlayAnimations;
    int32_t bottom = 0;
    int32_t top = 0;
    WallSpanKind kind = WallSpanKind::Solid;
    bool collision = true;

    bool isFaceEnabled(WallFace face) const noexcept
    {
        const auto index = static_cast<std::size_t>(face);
        return index < facesEnabled.size() ? facesEnabled[index] : true;
    }

    MapCell::TextureKey textureForFace(WallFace face) const noexcept
    {
        const auto index = static_cast<std::size_t>(face);
        if (index < facesEnabled.size() && !facesEnabled[index]) {
            return 0;
        }

        const auto faceTexture
            = index < faceTextureKeys.size() ? faceTextureKeys[index] : 0;
        return faceTexture != 0 ? faceTexture : textureKey;
    }
};

struct DoorDefinition {
    bool enabled = false;
    bool blocksWhenClosed = true;
    std::string requiredKey;
    double triggerDistanceCells = 1.25;
    double openTimeSeconds = 0.45;
    double closeDelaySeconds = 1.0;
    std::string openSound;
    int openSoundVolumePercent = 80;
    std::vector<MapCell::TextureKey> animationTextureKeys;
    std::map<std::string, MapCell::TextureKey> lockedOverlayTextureKeysByKey;
};

struct BlockSurface {
    MapCell::TextureKey textureKey = 0;
    int32_t height = 0;
    bool present = false;
};

struct BlockDefinition {
    std::string name;
    BlockSurface floor;
    BlockSurface ceiling;
    std::vector<WallSpan> walls;
    DoorDefinition door;
    std::string horizonImage;
    bool hasAnySolidSpan = false;
    bool hasAnyTransparentSpan = false;
    bool hasAnyCollidingSpan = false;
};

} // namespace WinRayCast
