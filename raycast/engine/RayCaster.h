// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include "MapCell.h"
#include "Player.h"
#include "WorldBlock.h"
#include "WorldMap.h"

#include <limits>
#include <utility>

class RayCaster {
public:
    using Point2d = std::pair<double, double>;

    struct RayHit {
        bool found = false;
        bool vertical = true;
        Cell cell = 0;
        Point2d point;
        double distance = std::numeric_limits<double>::infinity();
        const WinRayCast::BlockDefinition* block = nullptr;
        WinRayCast::WallFace face = WinRayCast::WallFace::West;
        int row = -1;
        int column = -1;
    };

    explicit RayCaster(const Player& player) noexcept
        : m_player(player)
    {
    }

    RayHit castSolidWallRay(WorldMap& map, int ray) const noexcept;

private:
    void firstVerticalIntersection(
        WorldMap& map, int ray, double slope, Point2d& point) const noexcept;

    void firstHorizontalIntersection(WorldMap& map, int ray,
        double inverseSlope, Point2d& point) const noexcept;

    void nextVerticalIntersection(WorldMap& map, const Point2d& previous,
        int ray, double slope, Point2d& point) const noexcept;

    void nextHorizontalIntersection(WorldMap& map, const Point2d& previous,
        int ray, double inverseSlope, Point2d& point) const noexcept;

    Cell verticalWall(
        WorldMap& map, const Point2d& point, int ray) const noexcept;
    Cell horizontalWall(
        WorldMap& map, const Point2d& point, int ray) const noexcept;
    const WinRayCast::BlockDefinition* verticalBlock(
        WorldMap& map, const Point2d& point, int ray) const noexcept;
    const WinRayCast::BlockDefinition* horizontalBlock(
        WorldMap& map, const Point2d& point, int ray) const noexcept;

    double horizontalDistance(
        const Point2d& intersection, int ray) const noexcept;
    double verticalDistance(
        const Point2d& intersection, int ray) const noexcept;

    bool isInsideMap(const WorldMap& map, const Point2d& point) const noexcept;

    const Player& m_player;
};
