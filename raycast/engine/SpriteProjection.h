// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include "ColumnDepthBuffer.h"
#include "Sprite.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

struct SpriteProjectionInput {
    double cameraX = 0.0;
    double cameraY = 0.0;
    double cameraFacingRadians = 0.0;
    double fieldOfViewRadians = 3.14159265358979323846 / 3.0;
    uint32_t screenWidth = 0;
    uint32_t screenHeight = 0;
    double verticalCenterOffsetRows = 0.0;
    double projectionScaleRows = 0.0;
    double worldCellHeight = 1.0;
    double projectionCenter = 0.5;
};

struct SpriteScreenProjection {
    bool visible = false;
    double depth = 0.0;
    double horizontalOffset = 0.0;
    int centerColumn = 0;
    int leftColumn = 0;
    int rightColumn = 0;
    int topRow = 0;
    int bottomRow = 0;
};

class SpriteProjector {
public:
    static SpriteScreenProjection project(
        const Sprite& sprite, const SpriteProjectionInput& input) noexcept
    {
        SpriteScreenProjection projection;

        if (!sprite.visible || input.screenWidth == 0 || input.screenHeight == 0
            || input.fieldOfViewRadians <= 0.0 || sprite.scale <= 0.0) {
            return projection;
        }

        const auto dx = sprite.x - input.cameraX;
        const auto dy = sprite.y - input.cameraY;
        const auto facingCos = std::cos(input.cameraFacingRadians);
        const auto facingSin = std::sin(input.cameraFacingRadians);

        const auto forwardDepth = dx * facingCos + dy * facingSin;
        if (forwardDepth <= nearDepth()) {
            return projection;
        }

        const auto rightOffset = -dx * facingSin + dy * facingCos;
        const auto halfFovTangent = std::tan(input.fieldOfViewRadians * 0.5);
        if (halfFovTangent <= 0.0) {
            return projection;
        }

        const auto normalizedX = rightOffset / (forwardDepth * halfFovTangent);
        const auto projectionScale = input.projectionScaleRows > 0.0
            ? input.projectionScaleRows
            : static_cast<double>(input.screenHeight);
        const auto cellHeight
            = input.worldCellHeight > 0.0 ? input.worldCellHeight : 1.0;
        const auto projectedCellHeight = projectionScale / forwardDepth;
        const auto projectedHeight
            = projectedCellHeight * (sprite.scale / cellHeight);
        const auto projectedWidth = projectedHeight;
        const auto centerColumn = (normalizedX + 1.0) * 0.5
            * static_cast<double>(input.screenWidth);

        projection.depth = forwardDepth;
        projection.horizontalOffset = normalizedX;
        projection.centerColumn = static_cast<int>(std::floor(centerColumn));
        projection.leftColumn
            = static_cast<int>(std::floor(centerColumn - projectedWidth * 0.5));
        projection.rightColumn
            = static_cast<int>(std::ceil(centerColumn + projectedWidth * 0.5));

        const auto verticalCenter
            = static_cast<double>(input.screenHeight) * 0.5
            + input.verticalCenterOffsetRows;
        const auto floorRow
            = verticalCenter + projectedCellHeight * input.projectionCenter;
        projection.topRow
            = static_cast<int>(std::floor(floorRow - projectedHeight));
        projection.bottomRow = static_cast<int>(std::ceil(floorRow));
        projection.visible = projection.rightColumn >= 0
            && projection.leftColumn < static_cast<int>(input.screenWidth)
            && projection.bottomRow >= 0
            && projection.topRow < static_cast<int>(input.screenHeight);

        return projection;
    }

    static bool hasVisibleColumn(const SpriteScreenProjection& projection,
        const ColumnDepthBuffer& depthBuffer) noexcept
    {
        if (!projection.visible || depthBuffer.empty()) {
            return false;
        }

        const auto firstColumn = std::max(0, projection.leftColumn);
        const auto lastColumn = std::min(
            static_cast<int>(depthBuffer.width()) - 1, projection.rightColumn);

        if (firstColumn > lastColumn) {
            return false;
        }

        for (auto column = firstColumn; column <= lastColumn; ++column) {
            if (projection.depth
                < depthBuffer.depth(static_cast<uint32_t>(column))) {
                return true;
            }
        }

        return false;
    }

private:
    static constexpr double nearDepth() noexcept { return 0.0001; }
};
