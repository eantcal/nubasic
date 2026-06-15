// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

class Texture {
public:
    using Pixel = uint32_t;

    Texture() = default;

    Texture(uint32_t width, uint32_t height) { resize(width, height); }

    void resize(uint32_t width, uint32_t height)
    {
        m_width = width;
        m_height = height;
        m_pixels.resize(static_cast<size_t>(m_width) * m_height);
    }

    bool empty() const noexcept { return m_pixels.empty(); }

    uint32_t width() const noexcept { return m_width; }

    uint32_t height() const noexcept { return m_height; }

    Pixel* pixels() noexcept { return m_pixels.data(); }

    const Pixel* pixels() const noexcept { return m_pixels.data(); }

    Pixel getPixel(uint32_t x, uint32_t y) const noexcept
    {
        if (x < m_width && y < m_height) {
            return m_pixels[x + static_cast<size_t>(y) * m_width];
        }

        return 0;
    }

    bool hasAlpha() const noexcept { return m_hasAlpha; }

    void setHasAlpha(bool hasAlpha) noexcept { m_hasAlpha = hasAlpha; }

    void setPixel(uint32_t x, uint32_t y, Pixel color) noexcept
    {
        if (x < m_width && y < m_height) {
            m_pixels[x + static_cast<size_t>(y) * m_width] = color;
        }
    }

    void fillBuffer(void* destBuf, int offset, int sourceWidth) const
    {
        fillBuffer(destBuf, m_width, m_height, offset, sourceWidth);
    }

    void fillBuffer(void* destBuf, uint32_t destWidth, uint32_t destHeight,
        int offset, int sourceWidth) const
    {
        if (!destBuf || sourceWidth <= 0) {
            return;
        }

        auto* dest = static_cast<Pixel*>(destBuf);
        const auto sampleWidth
            = static_cast<uint32_t>(std::min<int>(sourceWidth, m_width));

        if (sampleWidth == 0 || m_height == 0) {
            return;
        }

        // Fast path for power-of-two sample dimensions (the common case for
        // sky/horizon textures): replace the per-pixel modulo + bounds check
        // with masks and direct indexing. Bit-for-bit identical to the generic
        // path because `u % p == u & (p-1)` for unsigned u and power-of-two p,
        // and every sampled coordinate is provably in-bounds.
        const bool pow2 = (sampleWidth & (sampleWidth - 1)) == 0
            && (m_height & (m_height - 1)) == 0;
        if (pow2) {
            const uint32_t xMask = sampleWidth - 1;
            const uint32_t yMask = m_height - 1;
            const uint32_t uOffset = static_cast<uint32_t>(offset);
            const Pixel* src = m_pixels.data();
            for (uint32_t y = 0; y < destHeight; ++y) {
                const size_t srcRow = static_cast<size_t>(y & yMask) * m_width;
                Pixel* destRow = dest + static_cast<size_t>(y) * destWidth;
                for (uint32_t x = 0; x < destWidth; ++x) {
                    destRow[x] = src[((x + uOffset) & xMask) + srcRow];
                }
            }
            return;
        }

        for (uint32_t y = 0; y < destHeight; ++y) {
            for (uint32_t x = 0; x < destWidth; ++x) {
                dest[x + static_cast<size_t>(y) * destWidth]
                    = getPixel((x + offset) % sampleWidth, y % m_height);
            }
        }
    }

private:
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    bool m_hasAlpha = false;
    std::vector<Pixel> m_pixels;
};
