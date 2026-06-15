// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "RayCaster.h"

namespace {
constexpr double kFarDistance = 1000000.0;

struct SampledCell {
    Cell cell = 0;
    const WinRayCast::BlockDefinition* block = nullptr;
    int row = 0;
    int column = 0;
};

int clampCell(int value, int count) noexcept
{
    if (value >= count) {
        return count - 1;
    }

    if (value < 0) {
        return 0;
    }

    return value;
}

bool isSolidHit(const WorldMap& map, const SampledCell& sample) noexcept
{
    return map.isSolidCellHit(sample.row, sample.column, sample.cell);
}

WinRayCast::WallFace verticalHitFace(const Player& player, int ray) noexcept
{
    return ray >= player.deg90() && ray < player.deg270()
        ? WinRayCast::WallFace::East
        : WinRayCast::WallFace::West;
}

WinRayCast::WallFace horizontalHitFace(const Player& player, int ray) noexcept
{
    return ray >= player.deg180() && ray < player.deg360()
        ? WinRayCast::WallFace::South
        : WinRayCast::WallFace::North;
}

SampledCell sampleVerticalCell(WorldMap& map, const Player& player,
    const RayCaster::Point2d& point, int ray) noexcept
{
    int col = static_cast<int>(point.first / map.getCellDx());
    int row = static_cast<int>(point.second / map.getCellDy());

    if (ray >= player.deg90() && ray < player.deg270()) {
        --col;
    }

    col = clampCell(col, map.getColCount());
    row = clampCell(row, map.getRowCount());

    return { map[row][col], map.blockAtCell(row, col), row, col };
}

SampledCell sampleHorizontalCell(WorldMap& map, const Player& player,
    const RayCaster::Point2d& point, int ray) noexcept
{
    int col = static_cast<int>(point.first / map.getCellDx());
    int row = static_cast<int>(point.second / map.getCellDy());

    if (ray >= player.deg180() && ray < player.deg360()) {
        --row;
    }

    col = clampCell(col, map.getColCount());
    row = clampCell(row, map.getRowCount());

    return { map[row][col], map.blockAtCell(row, col), row, col };
}
} // namespace

void RayCaster::firstVerticalIntersection(
    WorldMap& map, int ray, double slope, Point2d& point) const noexcept
{
    const double xp = m_player.getX();
    const double yp = m_player.getY();

    const double xi = ray >= m_player.deg90() && ray < m_player.deg270()
        ? static_cast<double>(map.getPlayerCellPos().first) * map.getCellDx()
        : static_cast<double>(map.getPlayerCellPos().first + 1)
            * map.getCellDx();

    const double yi = slope * (xi - xp) + yp;

    point.first = xi;
    point.second = yi;
}

void RayCaster::firstHorizontalIntersection(
    WorldMap& map, int ray, double inverseSlope, Point2d& point) const noexcept
{
    const double xp = m_player.getX();
    const double yp = m_player.getY();

    const double yi = ray >= m_player.deg180() && ray < m_player.deg360()
        ? static_cast<double>(map.getPlayerCellPos().second) * map.getCellDy()
        : static_cast<double>(map.getPlayerCellPos().second + 1)
            * map.getCellDy();

    const double xi = inverseSlope * (yi - yp) + xp;

    point.first = xi;
    point.second = yi;
}

void RayCaster::nextVerticalIntersection(WorldMap& map, const Point2d& previous,
    int ray, double slope, Point2d& point) const noexcept
{
    if (ray >= m_player.deg90() && ray < m_player.deg270()) {
        point.second = previous.second - slope * map.getCellDx();
        point.first = previous.first - map.getCellDx();
    } else {
        point.second = previous.second + slope * map.getCellDx();
        point.first = previous.first + map.getCellDx();
    }
}

void RayCaster::nextHorizontalIntersection(WorldMap& map,
    const Point2d& previous, int ray, double inverseSlope,
    Point2d& point) const noexcept
{
    if (ray >= m_player.deg180() && ray < m_player.deg360()) {
        point.first = previous.first - inverseSlope * map.getCellDy();
        point.second = previous.second - map.getCellDy();
    } else {
        point.first = previous.first + inverseSlope * map.getCellDy();
        point.second = previous.second + map.getCellDy();
    }
}

Cell RayCaster::verticalWall(
    WorldMap& map, const Point2d& point, int ray) const noexcept
{
    int col = static_cast<int>(point.first / map.getCellDx());
    int row = static_cast<int>(point.second / map.getCellDy());

    if (ray >= m_player.deg90() && ray < m_player.deg270()) {
        --col;
    }

    col = clampCell(col, map.getColCount());
    row = clampCell(row, map.getRowCount());

    return map[row][col];
}

const WinRayCast::BlockDefinition* RayCaster::verticalBlock(
    WorldMap& map, const Point2d& point, int ray) const noexcept
{
    int col = static_cast<int>(point.first / map.getCellDx());
    int row = static_cast<int>(point.second / map.getCellDy());

    if (ray >= m_player.deg90() && ray < m_player.deg270()) {
        --col;
    }

    col = clampCell(col, map.getColCount());
    row = clampCell(row, map.getRowCount());

    return map.blockAtCell(row, col);
}

Cell RayCaster::horizontalWall(
    WorldMap& map, const Point2d& point, int ray) const noexcept
{
    int col = static_cast<int>(point.first / map.getCellDx());
    int row = static_cast<int>(point.second / map.getCellDy());

    if (ray >= m_player.deg180() && ray < m_player.deg360()) {
        --row;
    }

    col = clampCell(col, map.getColCount());
    row = clampCell(row, map.getRowCount());

    return map[row][col];
}

const WinRayCast::BlockDefinition* RayCaster::horizontalBlock(
    WorldMap& map, const Point2d& point, int ray) const noexcept
{
    int col = static_cast<int>(point.first / map.getCellDx());
    int row = static_cast<int>(point.second / map.getCellDy());

    if (ray >= m_player.deg180() && ray < m_player.deg360()) {
        --row;
    }

    col = clampCell(col, map.getColCount());
    row = clampCell(row, map.getRowCount());

    return map.blockAtCell(row, col);
}

double RayCaster::horizontalDistance(
    const Point2d& intersection, int ray) const noexcept
{
    return (intersection.second - m_player.getY()) * m_player.invsin(ray);
}

double RayCaster::verticalDistance(
    const Point2d& intersection, int ray) const noexcept
{
    return (intersection.first - m_player.getX()) * m_player.invcos(ray);
}

bool RayCaster::isInsideMap(
    const WorldMap& map, const Point2d& point) const noexcept
{
    return point.first >= 0 && point.first <= map.getMaxX() && point.second >= 0
        && point.second <= map.getMaxY();
}

RayCaster::RayHit RayCaster::castSolidWallRay(
    WorldMap& map, int ray) const noexcept
{
    Point2d verticalPoint;
    Point2d horizontalPoint;

    firstVerticalIntersection(map, ray, m_player.tan(ray), verticalPoint);
    firstHorizontalIntersection(
        map, ray, m_player.invtan(ray), horizontalPoint);

    bool verticalActive = isInsideMap(map, verticalPoint);
    bool horizontalActive = isInsideMap(map, horizontalPoint);

    while (verticalActive || horizontalActive) {
        const auto verticalDist = verticalActive
            ? verticalDistance(verticalPoint, ray)
            : kFarDistance;
        const auto horizontalDist = horizontalActive
            ? horizontalDistance(horizontalPoint, ray)
            : kFarDistance;

        if (verticalDist <= horizontalDist) {
            const auto sample
                = sampleVerticalCell(map, m_player, verticalPoint, ray);
            if (isSolidHit(map, sample)) {
                RayHit hit{ true, true, sample.cell, verticalPoint,
                    verticalDist };
                hit.block = sample.block;
                hit.face = verticalHitFace(m_player, ray);
                hit.row = sample.row;
                hit.column = sample.column;
                return hit;
            }

            nextVerticalIntersection(
                map, verticalPoint, ray, m_player.tan(ray), verticalPoint);
            verticalActive = isInsideMap(map, verticalPoint);
        } else {
            const auto sample
                = sampleHorizontalCell(map, m_player, horizontalPoint, ray);
            if (isSolidHit(map, sample)) {
                RayHit hit{ true, false, sample.cell, horizontalPoint,
                    horizontalDist };
                hit.block = sample.block;
                hit.face = horizontalHitFace(m_player, ray);
                hit.row = sample.row;
                hit.column = sample.column;
                return hit;
            }

            nextHorizontalIntersection(map, horizontalPoint, ray,
                m_player.invtan(ray), horizontalPoint);
            horizontalActive = isInsideMap(map, horizontalPoint);
        }
    }

    return {};
}
