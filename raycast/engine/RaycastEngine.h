// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.


/* -------------------------------------------------------------------------- */

#pragma once

#include "Color.h"
#include "ColumnDepthBuffer.h"
#include "FrameBuffer.h"
#include "MapCell.h"
#include "Player.h"
#include "RayCaster.h"
#include "Sprite.h"
#include "ViewWeapon.h"
#include "WorldMap.h"

#pragma warning(disable : 4786)
#include <math.h>
#include <memory>
#include <vector>


/* -------------------------------------------------------------------------- */

class WorldMap;


/* -------------------------------------------------------------------------- */

using Point2d = std::pair<double, double>;


/* -------------------------------------------------------------------------- */

class RaycastEngine {
public:
    enum class SpriteCollisionMode { PassThroughWalls, BlockSolidWalls };

    RaycastEngine(Player& player, double scale)
        : m_scale(scale)
        , m_player(player)
    {
        m_ceilFloorShadingPar = m_scale / m_depthShadingPar;
    }

    ~RaycastEngine() = default;

    void setShadingBrighter() noexcept
    {
        m_depthShadingPar /= 1.1;

        if (m_depthShadingPar < 1.0) {
            m_depthShadingPar = 1.0;
        }

        m_ceilFloorShadingPar = m_scale / m_depthShadingPar;
    }

    void setShadingDarker() noexcept
    {
        m_depthShadingPar *= 1.1;
        m_ceilFloorShadingPar = m_scale / m_depthShadingPar;
    }

    double getDepthShadingLevel() const noexcept { return m_depthShadingPar; }

    void setDepthShadingLevel(double level) noexcept
    {
        m_depthShadingPar = level;

        if (m_depthShadingPar < 1.0) {
            m_depthShadingPar = 1.0;
        }

        m_ceilFloorShadingPar = m_scale / m_depthShadingPar;
    }

    // Per-phase timing of the last renderToFrameBuffer call, in milliseconds.
    // Populated every frame; the cost is a handful of clock reads, so it is
    // always on and adds no per-pixel work. Used by the offscreen benchmark to
    // produce a baseline breakdown before/after optimizations.
    struct RenderProfile {
        double skyMs = 0.0;
        double wallsFloorCeilingMs = 0.0;
        double spritesMs = 0.0;
        double transparentWallsMs = 0.0;
        double viewWeaponMs = 0.0;
        double totalMs = 0.0;
    };

    void renderToFrameBuffer(WorldMap& aMap, uint32_t width, uint32_t height);

    const RenderProfile& lastRenderProfile() const noexcept
    {
        return m_lastProfile;
    }

    const FrameBuffer& frameBuffer() const noexcept { return m_frameBuffer; }

    const ColumnDepthBuffer& columnDepthBuffer() const noexcept
    {
        return m_columnDepthBuffer;
    }

    void addSprite(Sprite sprite) { m_sprites.push_back(std::move(sprite)); }

    void clearSprites() noexcept { m_sprites.clear(); }

    const std::vector<Sprite>& sprites() const noexcept { return m_sprites; }

    void setViewWeapon(ViewWeapon weapon);
    void clearViewWeapon() noexcept;
    ViewWeapon* viewWeapon() noexcept;
    const ViewWeapon* viewWeapon() const noexcept;
    void advanceViewWeapon(double deltaSeconds, bool playerIsMoving) noexcept;

    Sprite* sprite(size_t spriteIndex) noexcept
    {
        return spriteIndex < m_sprites.size() ? &m_sprites[spriteIndex]
                                              : nullptr;
    }

    const Sprite* sprite(size_t spriteIndex) const noexcept
    {
        return spriteIndex < m_sprites.size() ? &m_sprites[spriteIndex]
                                              : nullptr;
    }

    void advanceSpriteAnimations(double deltaSeconds) noexcept;

    void advanceSpriteAnimations(double deltaSeconds,
        const std::vector<size_t>& excludedSpriteIndices) noexcept;

    bool moveSprite(size_t spriteIndex, double dx, double dy,
        const WorldMap& map,
        SpriteCollisionMode collisionMode
        = SpriteCollisionMode::BlockSolidWalls) noexcept;

    Player& player() { return m_player; }

    const Player& player() const noexcept { return m_player; }

private:
    void drawPixel32(unsigned int x, unsigned int y, Color color_value) noexcept
    {
        m_frameBuffer.setPixel(x, y, color_value);
    }

    void drawAlphaPixel32(
        unsigned int x, unsigned int y, Color color_value) noexcept
    {
        const auto alpha = colorAlpha(color_value);
        if (alpha == 0) {
            return;
        }

        if (alpha == 255) {
            drawPixel32(x, y, color_value);
            return;
        }

        const auto dest = m_frameBuffer.pixel(x, y);
        const auto invAlpha = 255 - alpha;
        drawPixel32(x, y,
            makeColor(
                (colorRed(color_value) * alpha + colorRed(dest) * invAlpha)
                    / 255,
                (colorGreen(color_value) * alpha + colorGreen(dest) * invAlpha)
                    / 255,
                (colorBlue(color_value) * alpha + colorBlue(dest) * invAlpha)
                    / 255));
    }

    void renderTranspWall(WorldMap& aMap, bool render_internal_wall);

    void renderSprites(WorldMap& map);
    void renderViewWeapon();

    double rayToRadians(int ray) const noexcept;

    bool canPlaceSprite(const Sprite& sprite, double x, double y,
        const WorldMap& map, SpriteCollisionMode collisionMode) const noexcept;

    Player m_player;
    FrameBuffer m_frameBuffer;
    RenderProfile m_lastProfile;
    ColumnDepthBuffer m_columnDepthBuffer;
    std::vector<double> m_opaquePixelDepthBuffer;
    std::vector<double> m_spritePixelDepthBuffer;
    std::vector<Sprite> m_sprites;
    std::unique_ptr<ViewWeapon> m_viewWeapon;

    double getM(int ray) noexcept { return (m_player.tan(ray)); }

    double getM1(int ray) noexcept { return (m_player.invtan(ray)); }

    void vertint1st(
        WorldMap& map, int ray, const double& M, Point2d& point) const noexcept;

    void horzint1st(WorldMap& map, int ray, const double& M1,
        Point2d& point) const noexcept;

    void vertint(WorldMap& map, const Point2d& firstInt, int ray,
        const double& M, Point2d& point) const noexcept;

    void horzint(WorldMap& map, const Point2d& firstInt, int ray,
        const double& M1, Point2d& point) const noexcept;

    double horzDist(const Point2d& h_inter, int ray) const noexcept
    {
        return (h_inter.second - m_player.getY()) * m_player.invsin(ray);
    }

    double vertDist(const Point2d& v_inter, int ray) const noexcept
    {
        return (v_inter.first - m_player.getX()) * m_player.invcos(ray);
    }

    bool isInClientRect(
        const WorldMap& map, const Point2d& point) const noexcept
    {
        return point.first >= 0 && point.first <= map.getMaxX()
            && point.second >= 0 && point.second <= map.getMaxY();
    }

    Cell horzWall(WorldMap& map, const Point2d& point, int ray) const noexcept;
    Cell vertWall(WorldMap& map, const Point2d& point, int ray) const noexcept;

    Cell horzIntWall(
        WorldMap& map, const Point2d& point, int ray) const noexcept;
    Cell vertIntWall(
        WorldMap& map, const Point2d& point, int ray) const noexcept;

    void shadingStretchBtl(int xDest, int yDest, int heightDest, int xSrc,
        int ySrc, int height_source, int widthSrc, int maxVisibleY,
        double depthPar, const Texture* texture, double wallDepth,
        const Texture* overlayTexture = nullptr);

    void transpShadingStretchBtl(int xDest, int yDest, int heightDest, int xSrc,
        int ySrc, int height_source, int widthSrc, int maxVisibleY,
        double depthPar, const Texture* texture, int transpC, double wallDepth,
        const Texture* overlayTexture = nullptr);

private:
    double m_scale = 0;
    double m_depthShadingPar = 100.0;
    double m_ceilFloorShadingPar = 0;

    int m_fps = 0;
};
