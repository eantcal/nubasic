// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include <cstdint>

using Cell = uint64_t;

namespace MapCell {

using TextureKey = uint8_t;
using TextureResourceKey = uint32_t;

constexpr Cell SOLID_WALL_MASK = 0x00000000000000ffULL;
constexpr Cell CEILING_MASK = 0x000000000000ff00ULL;
constexpr Cell FLOOR_MASK = 0x0000000000ff0000ULL;
constexpr Cell TRANSPARENT_WALL_MASK = 0x00000000ff000000ULL;
constexpr Cell UPPER_WALL_MASK = 0x000000ff00000000ULL;

constexpr TextureKey TRANSPARENT_TEXTURE_KEY = 0xff;

constexpr TextureKey solidWallTexture(Cell cell) noexcept
{
    return static_cast<TextureKey>(cell & SOLID_WALL_MASK);
}

constexpr TextureKey ceilingTexture(Cell cell) noexcept
{
    return static_cast<TextureKey>((cell & CEILING_MASK) >> 8);
}

constexpr TextureKey floorTexture(Cell cell) noexcept
{
    return static_cast<TextureKey>((cell & FLOOR_MASK) >> 16);
}

constexpr TextureKey transparentWallTexture(Cell cell) noexcept
{
    return static_cast<TextureKey>((cell & TRANSPARENT_WALL_MASK) >> 24);
}

constexpr TextureKey upperWallTexture(Cell cell) noexcept
{
    return static_cast<TextureKey>((cell & UPPER_WALL_MASK) >> 32);
}

constexpr bool hasSolidWall(Cell cell) noexcept
{
    return solidWallTexture(cell) != 0;
}

constexpr bool hasTransparentWall(Cell cell) noexcept
{
    return transparentWallTexture(cell) != 0;
}

constexpr bool hasAnyWall(Cell cell) noexcept
{
    return hasSolidWall(cell) || hasTransparentWall(cell);
}

constexpr bool hasUpperWall(Cell cell) noexcept
{
    return upperWallTexture(cell) != 0;
}

constexpr bool isTransparentTexture(TextureKey texture) noexcept
{
    return texture == TRANSPARENT_TEXTURE_KEY;
}

} // namespace MapCell
