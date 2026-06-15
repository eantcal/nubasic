// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include <cstdint>
#include <limits>
#include <vector>

class ColumnDepthBuffer {
public:
    void resize(uint32_t width)
    {
        m_depth.resize(width);
        clear();
    }

    void clear() noexcept
    {
        for (auto& depth : m_depth) {
            depth = farDepth();
        }
    }

    uint32_t width() const noexcept
    {
        return static_cast<uint32_t>(m_depth.size());
    }

    bool empty() const noexcept { return m_depth.empty(); }

    double depth(uint32_t column) const noexcept
    {
        if (column < m_depth.size()) {
            return m_depth[column];
        }

        return farDepth();
    }

    void setDepth(uint32_t column, double depth) noexcept
    {
        if (column < m_depth.size()) {
            m_depth[column] = depth;
        }
    }

    void setNearestDepth(uint32_t column, double depth) noexcept
    {
        if (column < m_depth.size() && depth < m_depth[column]) {
            m_depth[column] = depth;
        }
    }

    static constexpr double farDepth() noexcept
    {
        return std::numeric_limits<double>::infinity();
    }

private:
    std::vector<double> m_depth;
};
