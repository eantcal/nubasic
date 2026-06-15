// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include <cstdint>

using Color = uint32_t;

inline Color makeColor(double r, double g, double b) noexcept
{
    return static_cast<uint8_t>(r)
        | (static_cast<uint16_t>(static_cast<uint8_t>(g)) << 8)
        | (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 16);
}

inline uint8_t colorRed(Color color) noexcept
{
    return static_cast<uint8_t>(color & 0xff);
}

inline uint8_t colorGreen(Color color) noexcept
{
    return static_cast<uint8_t>((color >> 8) & 0xff);
}

inline uint8_t colorBlue(Color color) noexcept
{
    return static_cast<uint8_t>((color >> 16) & 0xff);
}

inline uint8_t colorAlpha(Color color) noexcept
{
    return static_cast<uint8_t>((color >> 24) & 0xff);
}
