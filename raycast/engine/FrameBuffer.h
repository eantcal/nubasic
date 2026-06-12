// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include <cstdint>
#include <vector>

class FrameBuffer {
public:
    using Pixel = uint32_t;

    FrameBuffer() = default;

    FrameBuffer(uint32_t width, uint32_t height) { resize(width, height); }

    void resize(uint32_t width, uint32_t height)
    {
        m_width = width;
        m_height = height;
        m_pixels.resize(static_cast<size_t>(m_width) * m_height);
    }

    bool empty() const noexcept { return m_pixels.empty(); }

    uint32_t width() const noexcept { return m_width; }

    uint32_t height() const noexcept { return m_height; }

    uint32_t pitchBytes() const noexcept { return m_width * sizeof(Pixel); }

    Pixel* pixels() noexcept { return m_pixels.data(); }

    const Pixel* pixels() const noexcept { return m_pixels.data(); }

    void* data() noexcept { return m_pixels.data(); }

    const void* data() const noexcept { return m_pixels.data(); }

    size_t byteSize() const noexcept { return m_pixels.size() * sizeof(Pixel); }

    void setPixel(uint32_t x, uint32_t y, Pixel color) noexcept
    {
        if (x < m_width && y < m_height) {
            m_pixels[x + static_cast<size_t>(y) * m_width] = color;
        }
    }

    Pixel pixel(uint32_t x, uint32_t y) const noexcept
    {
        if (x < m_width && y < m_height) {
            return m_pixels[x + static_cast<size_t>(y) * m_width];
        }

        return 0;
    }

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::vector<Pixel> m_pixels;
};
