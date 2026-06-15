// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.


/* -------------------------------------------------------------------------- */

#include "Player.h"
#include "WorldMap.h"

#include <algorithm>


/* -------------------------------------------------------------------------- */

namespace {
double playerCollisionRadius(const WorldMap& wMap) noexcept
{
    return std::max(4.0, std::min(wMap.getCellDx(), wMap.getCellDy()) / 8.0);
}

bool canOccupy(WorldMap& wMap, double x, double y) noexcept
{
    const auto radius = playerCollisionRadius(wMap);
    return !wMap.isSolidAtWorld(x, y) && !wMap.isSolidAtWorld(x - radius, y)
        && !wMap.isSolidAtWorld(x + radius, y)
        && !wMap.isSolidAtWorld(x, y - radius)
        && !wMap.isSolidAtWorld(x, y + radius);
}
} // namespace

/* -------------------------------------------------------------------------- */

Player::Cell Player::moveTo(int offset, WorldMap& wMap, int deg)
{
    Cell retVal = 0;

    try {
        const int alpha = normalizeDeg(m_alpha + degHalfVisual() + deg);

        double x = double(offset) * m_cosTbl[alpha];
        double y = double(offset) * m_sinTbl[alpha];
        const auto nextX = m_x + x;
        const auto nextY = m_y + y;

        if (nextX < 0.0 || nextY < 0.0 || nextX >= wMap.getMaxX()
            || nextY >= wMap.getMaxY()) {
            return retVal;
        }

        const int c = static_cast<int>(nextX) / wMap.getCellDx();
        const int r = static_cast<int>(nextY) / wMap.getCellDy();

        if (r < 0 || c < 0 || r >= wMap.getRowCount()
            || c >= wMap.getColCount()) {
            return retVal;
        }

        retVal = wMap[r][c];

        if (canOccupy(wMap, nextX, nextY)) {
            m_x = nextX;
            m_y = nextY;
        }
    } catch (...) {
    }

    return retVal;
}

/* -------------------------------------------------------------------------- */

static const double POSITIVE_INFINITY = 1000000.0;
static const double SMALLEST_EPSILON = double(1.0) / POSITIVE_INFINITY;


/* -------------------------------------------------------------------------- */

Player::Player(int x, int y, int visualDeg, int xProjRes, int yProjRes,
    int slope, double projCenter) noexcept
    : m_x(x)
    , m_y(y)
    , m_alpha(0)
    , m_visualDeg(visualDeg)
    , m_xProjRes(xProjRes)
    , m_yProjRes(yProjRes)
    , m_slope(slope)
    , m_projCenter(projCenter)
{
    auto sign = [](double x) { return x == 0.0 ? 0.0 : (x > .0 ? 1. : -1.); };

    m_floorShadingPar = yProjRes / 16;

    const int vecSize = m_xProjRes * (360 / m_visualDeg);

    m_degVisual = (vecSize * m_visualDeg) / 360;
    m_degVisual2 = m_degVisual / 2;

    m_deg90 = vecSize / 4;
    m_deg180 = vecSize / 2;
    m_deg270 = (vecSize / 4) * 3;
    m_deg360 = vecSize;

    m_cosTbl.resize(vecSize);
    m_sinTbl.resize(vecSize);
    m_tanTbl.resize(vecSize);
    m_invSinTbl.resize(vecSize);
    m_invCosTbl.resize(vecSize);
    m_invTanTbl.resize(vecSize);

    for (int ray = 0; ray < vecSize; ++ray) {
        const double alpha = (double(ray * 360.0) / double(m_deg360))
            * (3.14159265359 / 180.0);

        m_cosTbl[ray] = ::cos(alpha);
        m_sinTbl[ray] = ::sin(alpha);
        m_tanTbl[ray] = ::tan(alpha);

        m_invCosTbl[ray] = fabs(m_cosTbl[ray]) <= SMALLEST_EPSILON
            ? sign(m_cosTbl[ray]) * POSITIVE_INFINITY
            : double(1.0) / m_cosTbl[ray];

        m_invSinTbl[ray] = fabs(m_sinTbl[ray]) <= SMALLEST_EPSILON
            ? sign(m_cosTbl[ray]) * POSITIVE_INFINITY
            : double(1.0) / m_sinTbl[ray];

        m_invTanTbl[ray] = fabs(m_tanTbl[ray]) <= SMALLEST_EPSILON
            ? sign(m_cosTbl[ray]) * POSITIVE_INFINITY
            : double(1.0) / m_tanTbl[ray];
    }
}

/* -------------------------------------------------------------------------- */
