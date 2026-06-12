// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.


/* -------------------------------------------------------------------------- */

#include "RaycastEngine.h"
#include "SpriteProjection.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <future>
#include <thread>
#include <utility>
#include <vector>

namespace {

// Run fn(begin, end) over a [0, total) column range, split into one chunk per
// hardware thread. Columns are rendered independently (each writes only its own
// framebuffer column and its own per-column depth indices), so the partition is
// race-free and the result is identical to the serial version regardless of
// scheduling. std::async(launch::async) reuses the platform thread pool on
// MSVC, avoiding raw per-frame thread creation.
template <typename Fn> void parallelColumns(int total, Fn&& fn)
{
    if (total <= 1) {
        fn(0, total);
        return;
    }

    unsigned hw = std::thread::hardware_concurrency();
    if (hw == 0) {
        hw = 1;
    }

    int chunks = static_cast<int>(
        std::min<unsigned>(hw, static_cast<unsigned>(total)));
    if (chunks <= 1) {
        fn(0, total);
        return;
    }

    const int per = (total + chunks - 1) / chunks;
    std::vector<std::future<void>> futures;
    futures.reserve(static_cast<size_t>(chunks - 1));

    // Launch every chunk except the first asynchronously, then run the first
    // chunk on the calling thread instead of leaving it blocked in get().
    for (int c = 1; c < chunks; ++c) {
        const int begin = c * per;
        if (begin >= total) {
            break;
        }
        const int end = std::min(total, begin + per);
        futures.push_back(std::async(
            std::launch::async, [&fn, begin, end]() { fn(begin, end); }));
    }

    fn(0, std::min(total, per));

    for (auto& future : futures) {
        future.get();
    }
}

} // namespace


/* -------------------------------------------------------------------------- */
// RAYCAST ENGINE
/* -------------------------------------------------------------------------- */

static const Color TRANSP_COLOR = makeColor(0, 0, 0);
static const double POSITIVE_INFINITY = 1000000.0;
static const double PI = 3.14159265358979323846;

static bool sameRgb(Color lhs, Color rhs) noexcept
{
    return colorRed(lhs) == colorRed(rhs) && colorGreen(lhs) == colorGreen(rhs)
        && colorBlue(lhs) == colorBlue(rhs);
}

static bool isTransparentTexturePixel(
    const Texture& texture, Color color, Color transparentColor) noexcept
{
    if (texture.hasAlpha()) {
        return colorAlpha(color) == 0;
    }

    return sameRgb(color, transparentColor);
}

static Color withAlpha(Color color, uint8_t alpha) noexcept
{
    return (color & 0x00ffffffu) | (static_cast<Color>(alpha) << 24);
}

static Color alphaBlendOver(Color base, Color overlay, uint8_t alpha) noexcept
{
    const auto inverseAlpha = 255 - static_cast<int>(alpha);
    return withAlpha(
        makeColor((static_cast<int>(colorRed(overlay)) * alpha
                      + static_cast<int>(colorRed(base)) * inverseAlpha)
                / 255,
            (static_cast<int>(colorGreen(overlay)) * alpha
                + static_cast<int>(colorGreen(base)) * inverseAlpha)
                / 255,
            (static_cast<int>(colorBlue(overlay)) * alpha
                + static_cast<int>(colorBlue(base)) * inverseAlpha)
                / 255),
        255);
}

static Color overlayWallPixel(Color base, const Texture* overlayTexture,
    int xSrc, int ySrc, bool baseIsTransparent = false,
    bool* becameOpaque = nullptr) noexcept
{
    if (becameOpaque != nullptr) {
        *becameOpaque = false;
    }

    if (overlayTexture == nullptr || overlayTexture->empty()
        || overlayTexture->width() == 0 || overlayTexture->height() == 0) {
        return base;
    }

    const auto overlayX
        = static_cast<uint32_t>(xSrc >= 0 ? xSrc : 0) % overlayTexture->width();
    const auto overlayY = static_cast<uint32_t>(ySrc >= 0 ? ySrc : 0)
        % overlayTexture->height();
    const auto overlay = overlayTexture->getPixel(overlayX, overlayY);
    const auto alpha
        = overlayTexture->hasAlpha() ? colorAlpha(overlay) : uint8_t(255);
    if (alpha == 0) {
        return base;
    }

    if (becameOpaque != nullptr) {
        *becameOpaque = true;
    }

    if (baseIsTransparent) {
        return withAlpha(makeColor(colorRed(overlay), colorGreen(overlay),
                             colorBlue(overlay)),
            alpha);
    }

    return alpha == 255
        ? withAlpha(makeColor(colorRed(overlay), colorGreen(overlay),
                        colorBlue(overlay)),
              255)
        : alphaBlendOver(base, overlay, alpha);
}

static WinRayCast::WallFace wallFaceForHit(
    const Player& player, int ray, bool vertical, bool internalWall) noexcept
{
    auto face = vertical ? (ray >= player.deg90() && ray < player.deg270()
                                   ? WinRayCast::WallFace::East
                                   : WinRayCast::WallFace::West)
                         : (ray >= player.deg180() && ray < player.deg360()
                                   ? WinRayCast::WallFace::South
                                   : WinRayCast::WallFace::North);

    if (!internalWall) {
        return face;
    }

    switch (face) {
    case WinRayCast::WallFace::North:
        return WinRayCast::WallFace::South;
    case WinRayCast::WallFace::South:
        return WinRayCast::WallFace::North;
    case WinRayCast::WallFace::East:
        return WinRayCast::WallFace::West;
    case WinRayCast::WallFace::West:
    default:
        return WinRayCast::WallFace::East;
    }
}

// The wall texture column is derived from the world X/Y of the hit, which runs
// in a fixed direction regardless of which side of the wall is being viewed.
// Without correction the two opposing faces sample the column in opposite
// screen order, so asymmetric textures (e.g. text) come out mirrored on half of
// the orientations. Mirror the column for the viewing direction that needs it,
// keyed purely on the ray direction so it is correct for inner faces too.
static bool shouldMirrorWallColumn(
    const Player& player, int relRay, bool vertical) noexcept
{
    if (vertical) {
        // Viewing an east-facing side (ray heading -X).
        return relRay >= player.deg90() && relRay < player.deg270();
    }

    // Viewing a north-facing side (ray heading +Y).
    return relRay < player.deg180();
}

void RaycastEngine::setViewWeapon(ViewWeapon weapon)
{
    m_viewWeapon.reset(new ViewWeapon(std::move(weapon)));
}

void RaycastEngine::clearViewWeapon() noexcept { m_viewWeapon.reset(); }

ViewWeapon* RaycastEngine::viewWeapon() noexcept { return m_viewWeapon.get(); }

const ViewWeapon* RaycastEngine::viewWeapon() const noexcept
{
    return m_viewWeapon.get();
}

void RaycastEngine::advanceViewWeapon(
    double deltaSeconds, bool playerIsMoving) noexcept
{
    if (m_viewWeapon) {
        m_viewWeapon->advance(deltaSeconds, playerIsMoving);
    }
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::advanceSpriteAnimations(double deltaSeconds) noexcept
{
    static const std::vector<size_t> noExcludedSpriteIndices;
    advanceSpriteAnimations(deltaSeconds, noExcludedSpriteIndices);
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::advanceSpriteAnimations(double deltaSeconds,
    const std::vector<size_t>& excludedSpriteIndices) noexcept
{
    if (deltaSeconds <= 0.0) {
        return;
    }

    for (size_t spriteIndex = 0; spriteIndex < m_sprites.size();
        ++spriteIndex) {
        if (std::find(excludedSpriteIndices.begin(),
                excludedSpriteIndices.end(), spriteIndex)
            != excludedSpriteIndices.end()) {
            continue;
        }

        m_sprites[spriteIndex].advanceAnimation(deltaSeconds);
    }
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::horzint1st(
    WorldMap& wMap, int ray, const double& M1, Point2d& point) const noexcept
{
    const double xp = m_player.getX();
    const double yp = m_player.getY();

    double yi = ray >= m_player.deg180() && ray < m_player.deg360()
        ? ((double)wMap.getPlayerCellPos().second) * wMap.getCellDy()
        : ((double)wMap.getPlayerCellPos().second + 1) * wMap.getCellDy();

    double xi = M1 * (yi - yp) + xp;

    point.first = xi;
    point.second = yi;
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::vertint1st(
    WorldMap& wMap, int ray, const double& M, Point2d& point) const noexcept
{
    const double xp = m_player.getX();
    const double yp = m_player.getY();

    const double xi = ray >= m_player.deg90() && ray < m_player.deg270()
        ? ((double)wMap.getPlayerCellPos().first) * wMap.getCellDx()
        : ((double)wMap.getPlayerCellPos().first + 1) * wMap.getCellDx();

    const double yi = M * (xi - xp) + yp;

    point.first = xi;
    point.second = yi;
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::vertint(WorldMap& wMap, const Point2d& firstInt, int ray,
    const double& M, Point2d& point) const noexcept
{
    double xi, yi;

    if (ray >= m_player.deg90() && ray < m_player.deg270()) {
        yi = firstInt.second - M * wMap.getCellDx();
        xi = firstInt.first - wMap.getCellDx();
    } else {
        yi = firstInt.second + M * wMap.getCellDx();
        xi = firstInt.first + wMap.getCellDx();
    }

    point.first = xi;
    point.second = yi;
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::horzint(WorldMap& wMap, const Point2d& firstInt, int ray,
    const double& M1, Point2d& point) const noexcept
{
    double xi, yi;

    if (ray >= m_player.deg180() && ray < m_player.deg360()) {
        xi = firstInt.first - M1 * wMap.getCellDy();
        yi = firstInt.second - wMap.getCellDy();
    } else {
        xi = firstInt.first + M1 * wMap.getCellDy();
        yi = firstInt.second + wMap.getCellDy();
    }

    point.first = xi;
    point.second = yi;
}


/* -------------------------------------------------------------------------- */

Cell RaycastEngine::horzWall(
    WorldMap& wMap, const Point2d& point, int ray) const noexcept
{
    int c = int(point.first / wMap.getCellDx());
    int r = int(point.second / wMap.getCellDy());

    if (ray >= m_player.deg180() && ray < m_player.deg360()) {
        --r;
    }

    if (c >= int(wMap.getColCount())) {
        c = wMap.getColCount() - 1;
    } else if (c < 0) {
        c = 0;
    }

    if (r >= int(wMap.getRowCount())) {
        r = wMap.getRowCount() - 1;
    } else if (r < 0) {
        r = 0;
    }

    return (wMap[r][c]);
}


/* -------------------------------------------------------------------------- */

Cell RaycastEngine::vertWall(
    WorldMap& wMap, const Point2d& point, int ray) const noexcept
{
    int c = int(point.first / wMap.getCellDx());
    int r = int(point.second / wMap.getCellDy());

    if (ray >= m_player.deg90() && ray < m_player.deg270()) {
        --c;
    }

    if (c >= int(wMap.getColCount())) {
        c = wMap.getColCount() - 1;
    } else if (c < 0) {
        c = 0;
    }

    if (r >= int(wMap.getRowCount())) {
        r = wMap.getRowCount() - 1;
    } else if (r < 0) {
        r = 0;
    }

    return (wMap[r][c]);
}


/* -------------------------------------------------------------------------- */

Cell RaycastEngine::horzIntWall(
    WorldMap& wMap, const Point2d& point, int ray) const noexcept
{
    int c = int(point.first / wMap.getCellDx());
    int r = int(point.second / wMap.getCellDy());

    if (!(ray >= m_player.deg180() && ray < m_player.deg360())) {
        --r;
    }

    if (c >= int(wMap.getColCount())) {
        c = wMap.getColCount() - 1;
    } else if (c < 0) {
        c = 0;
    }

    if (r >= int(wMap.getRowCount())) {
        r = wMap.getRowCount() - 1;
    } else if (r < 0) {
        r = 0;
    }

    return (wMap[r][c]);
}


/* -------------------------------------------------------------------------- */

Cell RaycastEngine::vertIntWall(
    WorldMap& wMap, const Point2d& point, int ray) const noexcept
{
    int c = int(point.first / wMap.getCellDx());
    int r = int(point.second / wMap.getCellDy());

    if (!(ray >= m_player.deg90() && ray < m_player.deg270())) {
        --c;
    }

    if (c >= int(wMap.getColCount())) {
        c = wMap.getColCount() - 1;
    } else if (c < 0) {
        c = 0;
    }

    if (r >= int(wMap.getRowCount())) {
        r = wMap.getRowCount() - 1;
    } else if (r < 0) {
        r = 0;
    }

    return (wMap[r][c]);
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::shadingStretchBtl(int xDest, int yDest, int heightDest,
    int xSrc, int ySrc, int height_source, int widthSrc, int maxVisibleY,
    double depthPar, const Texture* texture, double wallDepth,
    const Texture* overlayTexture)
{
    if (!texture) {
        return;
    }

    heightDest += 2;

    double step = double(height_source) / double(heightDest);

    double ys = double(ySrc);
    int yd = yDest - 1;
    int max_yd = std::min(maxVisibleY, heightDest + yDest);

    double Rcomp, Gcomp, Bcomp;

    if (yd < 0) {
        ys += (-yd) * step;
        yd = 0;
    }

    while (yd < max_yd && ys < height_source) {
        Color c = texture->getPixel(xSrc % widthSrc, int(ys) % height_source);
        c = overlayWallPixel(c, overlayTexture, xSrc, int(ys) % height_source);
        const auto depthIndex = static_cast<size_t>(yd)
                * static_cast<size_t>(m_player.getXProjRes())
            + static_cast<size_t>(xDest);
        if (depthIndex < m_opaquePixelDepthBuffer.size()) {
            m_opaquePixelDepthBuffer[depthIndex]
                = std::min(m_opaquePixelDepthBuffer[depthIndex], wallDepth);
        }

        if (depthPar < double(1.0)) {
            Rcomp = depthPar * colorRed(c);
            Gcomp = depthPar * colorGreen(c);
            Bcomp = depthPar * colorBlue(c);

            drawPixel32(xDest, yd, makeColor(Rcomp, Gcomp, Bcomp));
        } // if
        else {
            drawPixel32(xDest, yd, c);
        }

        ++yd;
        ys += step;
    }
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::transpShadingStretchBtl(int xDest, int yDest,
    int heightDest, int xSrc, int ySrc, int height_source, int widthSrc,
    int maxVisibleY, double depthPar, const Texture* texture, int transpC,
    double wallDepth, const Texture* overlayTexture)
{
    if (!texture) {
        return;
    }

    heightDest += 2;

    double step = double(height_source) / double(heightDest);

    double ys = double(ySrc);

    int yd = yDest - 1;
    int max_yd = std::min(maxVisibleY, heightDest + yDest);

    double Rcomp, Gcomp, Bcomp;

    if (yd < 0) {
        ys += (-yd) * step;
        yd = 0;
    }

    while (yd < max_yd && ys < height_source) {
        Color c
            = texture->getPixel(xSrc % widthSrc, int(ys) /*% height_source*/);
        bool becameOpaque = false;
        const auto baseIsTransparent = isTransparentTexturePixel(
            *texture, c, static_cast<Color>(transpC));
        c = overlayWallPixel(
            c, overlayTexture, xSrc, int(ys), baseIsTransparent, &becameOpaque);

        if (!baseIsTransparent || becameOpaque) {
            const auto depthIndex = static_cast<size_t>(yd)
                    * static_cast<size_t>(m_player.getXProjRes())
                + static_cast<size_t>(xDest);
            if (depthIndex < m_spritePixelDepthBuffer.size()
                && wallDepth >= m_spritePixelDepthBuffer[depthIndex]) {
                ++yd;
                ys += step;
                continue;
            }

            if (depthIndex < m_opaquePixelDepthBuffer.size()
                && wallDepth >= m_opaquePixelDepthBuffer[depthIndex]) {
                ++yd;
                ys += step;
                continue;
            }

            const auto sampleHasAlpha = texture->hasAlpha()
                || (becameOpaque && colorAlpha(c) > 0 && colorAlpha(c) < 255);
            if (sampleHasAlpha && colorAlpha(c) < 255) {
                drawAlphaPixel32(xDest, yd, c);
            } else if (depthPar < double(1.0)) {
                Rcomp = depthPar * colorRed(c);
                Gcomp = depthPar * colorGreen(c);
                Bcomp = depthPar * colorBlue(c);

                drawPixel32(xDest, yd, makeColor(Rcomp, Gcomp, Bcomp));
            } // if
            else {
                drawPixel32(xDest, yd, c);
            }

            if (depthIndex < m_opaquePixelDepthBuffer.size()) {
                m_opaquePixelDepthBuffer[depthIndex]
                    = std::min(m_opaquePixelDepthBuffer[depthIndex], wallDepth);
            }
        }

        ++yd;
        ys += step;
    }
}


/* -------------------------------------------------------------------------- */

double RaycastEngine::rayToRadians(int ray) const noexcept
{
    while (ray < 0) {
        ray += m_player.deg360();
    }

    while (ray >= m_player.deg360()) {
        ray -= m_player.deg360();
    }

    return (static_cast<double>(ray) / static_cast<double>(m_player.deg360()))
        * PI * 2.0;
}


/* -------------------------------------------------------------------------- */

bool RaycastEngine::canPlaceSprite(const Sprite& sprite, double x, double y,
    const WorldMap& map, SpriteCollisionMode collisionMode) const noexcept
{
    if (collisionMode == SpriteCollisionMode::PassThroughWalls) {
        return true;
    }

    const auto radius = std::max(0.0, sprite.collisionRadius);

    return !map.isSolidAtWorld(x, y) && !map.isSolidAtWorld(x - radius, y)
        && !map.isSolidAtWorld(x + radius, y)
        && !map.isSolidAtWorld(x, y - radius)
        && !map.isSolidAtWorld(x, y + radius);
}


/* -------------------------------------------------------------------------- */

bool RaycastEngine::moveSprite(size_t spriteIndex, double dx, double dy,
    const WorldMap& map, SpriteCollisionMode collisionMode) noexcept
{
    if (spriteIndex >= m_sprites.size()) {
        return false;
    }

    auto& sprite = m_sprites[spriteIndex];
    const auto nextX = sprite.x + dx;
    const auto nextY = sprite.y + dy;

    if (!canPlaceSprite(sprite, nextX, nextY, map, collisionMode)) {
        return false;
    }

    sprite.x = nextX;
    sprite.y = nextY;
    return true;
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::renderSprites(WorldMap& wMap)
{
    struct VisibleSprite {
        const Sprite* sprite = nullptr;
        const SpriteFrame* frame = nullptr;
        const Texture* texture = nullptr;
        SpriteScreenProjection projection;
    };

    if (m_sprites.empty()) {
        return;
    }

    std::vector<VisibleSprite> visibleSprites;

    int cameraFacingRay = m_player.getAlpha() + m_player.degHalfVisual();
    if (cameraFacingRay >= m_player.deg360()) {
        cameraFacingRay -= m_player.deg360();
    }

    SpriteProjectionInput input;
    input.cameraX = m_player.getX();
    input.cameraY = m_player.getY();
    input.cameraFacingRadians = rayToRadians(cameraFacingRay);
    input.fieldOfViewRadians = rayToRadians(m_player.degHalfVisual() * 2);
    input.screenWidth = static_cast<uint32_t>(m_player.getXProjRes());
    input.screenHeight = static_cast<uint32_t>(m_player.getYProjRes());
    input.verticalCenterOffsetRows
        = static_cast<double>(m_player.getSlope()) * 0.5;
    input.projectionScaleRows = m_scale;
    input.worldCellHeight = static_cast<double>(wMap.getCellDy());
    input.projectionCenter = m_player.getCenterProj();

    for (const auto& sprite : m_sprites) {
        const auto projection = SpriteProjector::project(sprite, input);
        if (!SpriteProjector::hasVisibleColumn(
                projection, m_columnDepthBuffer)) {
            continue;
        }

        const auto& frames = sprite.activeFrames();
        const auto viewIndex = frames.selectViewIndex(input.cameraX,
            input.cameraY, sprite.x, sprite.y, sprite.facingRadians);

        const auto* frame = frames.frameForView(viewIndex);
        if (!frame) {
            continue;
        }

        const auto* texture = wMap.getTexture(frame->textureKey);
        if (!texture || texture->empty()) {
            continue;
        }

        visibleSprites.push_back({ &sprite, frame, texture, projection });
    }

    std::sort(visibleSprites.begin(), visibleSprites.end(),
        [](const VisibleSprite& lhs, const VisibleSprite& rhs) {
            return lhs.projection.depth > rhs.projection.depth;
        });

    for (const auto& visible : visibleSprites) {
        const auto& projection = visible.projection;
        const auto* texture = visible.texture;

        const int spriteWidth = projection.rightColumn - projection.leftColumn;
        const int spriteHeight = projection.bottomRow - projection.topRow;

        if (spriteWidth <= 0 || spriteHeight <= 0) {
            continue;
        }

        const int firstColumn = std::max(0, projection.leftColumn);
        const int lastColumn
            = std::min(m_player.getXProjRes() - 1, projection.rightColumn);

        for (int column = firstColumn; column <= lastColumn; ++column) {
            if (projection.depth >= m_columnDepthBuffer.depth(column)) {
                continue;
            }

            const auto textureX = static_cast<uint32_t>(
                ((column - projection.leftColumn)
                    * static_cast<int>(texture->width()))
                / spriteWidth);

            const int firstRow = std::max(0, projection.topRow);
            const int lastRow
                = std::min(m_player.getYProjRes() - 1, projection.bottomRow);

            for (int row = firstRow; row <= lastRow; ++row) {
                const auto textureY = static_cast<uint32_t>(
                    ((row - projection.topRow)
                        * static_cast<int>(texture->height()))
                    / spriteHeight);

                const auto color = texture->getPixel(textureX, textureY);
                if (isTransparentTexturePixel(
                        *texture, color, visible.sprite->transparentColor)) {
                    continue;
                }

                if (texture->hasAlpha() && colorAlpha(color) < 255) {
                    drawAlphaPixel32(column, row, color);
                } else {
                    drawPixel32(column, row, color);
                }
                const auto depthIndex = static_cast<size_t>(row)
                        * static_cast<size_t>(m_player.getXProjRes())
                    + static_cast<size_t>(column);
                if (depthIndex < m_spritePixelDepthBuffer.size()) {
                    m_spritePixelDepthBuffer[depthIndex] = std::min(
                        m_spritePixelDepthBuffer[depthIndex], projection.depth);
                }
            }
        }
    }
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::renderViewWeapon()
{
    if (!m_viewWeapon || m_frameBuffer.empty()) {
        return;
    }

    const auto* texture = m_viewWeapon->currentFrame();
    if (texture == nullptr || texture->empty()) {
        return;
    }

    const auto screenWidth = std::min(
        static_cast<int>(m_frameBuffer.width()), m_player.getXProjRes());
    const auto screenHeight = std::min(
        static_cast<int>(m_frameBuffer.height()), m_player.getYProjRes());
    if (screenWidth <= 0 || screenHeight <= 0) {
        return;
    }

    const auto destHeight = std::max(1,
        static_cast<int>(std::round(static_cast<double>(screenHeight)
            * m_viewWeapon->screenHeightFraction())));
    const auto destWidth = std::max(1,
        static_cast<int>(std::round(static_cast<double>(destHeight)
            * static_cast<double>(texture->width())
            / static_cast<double>(std::max<uint32_t>(1, texture->height())))));

    const auto originX = static_cast<int>(
        std::round(static_cast<double>(screenWidth) * m_viewWeapon->anchorX()
            - static_cast<double>(destWidth) * m_viewWeapon->anchorX()
            + m_viewWeapon->baseOffsetX() + m_viewWeapon->bobOffsetX()));
    const auto originY = static_cast<int>(
        std::round(static_cast<double>(screenHeight) * m_viewWeapon->anchorY()
            - static_cast<double>(destHeight) + m_viewWeapon->baseOffsetY()
            + m_viewWeapon->bobOffsetY()));

    const auto firstX = std::max(0, originX);
    const auto lastX = std::min(screenWidth - 1, originX + destWidth - 1);
    const auto firstY = std::max(0, originY);
    const auto lastY = std::min(screenHeight - 1, originY + destHeight - 1);

    for (int y = firstY; y <= lastY; ++y) {
        const auto textureY = static_cast<uint32_t>(
            ((y - originY) * static_cast<int>(texture->height())) / destHeight);

        for (int x = firstX; x <= lastX; ++x) {
            const auto textureX = static_cast<uint32_t>(
                ((x - originX) * static_cast<int>(texture->width()))
                / destWidth);
            const auto color = texture->getPixel(textureX, textureY);
            if (texture->hasAlpha()) {
                drawAlphaPixel32(x, y, color);
            } else if (color != TRANSP_COLOR) {
                drawPixel32(x, y, color);
            }
        }
    }
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::renderTranspWall(WorldMap& wMap, bool render_internal_wall)
{
    struct TransparentWallHit {
        Cell mapKey = 0;
        Point2d point{};
        double distance = 0.0;
        bool vertical = true;
        std::pair<int, int> cell{ 0, 0 };
    };

    int cameraRayOffset = m_player.getAlpha();
    const auto cameraCell
        = std::pair<int, int>{ m_player.getY() / wMap.getCellDy(),
              m_player.getX() / wMap.getCellDx() };

    // main casting loop (for each pixel of projection x coord...). Kept serial:
    // the transparent pass is cheap (few hits) and parallel-dispatch overhead
    // would outweigh the work; the heavy lifting is the opaque pass above.
    for (int ray = 0; ray < m_player.getXProjRes(); ++ray) {
        int relRay = ray + cameraRayOffset;

        if (relRay < 0)
            relRay += m_player.deg360();
        else if (relRay >= m_player.deg360())
            relRay -= m_player.deg360();

        double M = getM(relRay);
        double M1 = getM1(relRay);

        // Search first intersection with the grid (WorldMap)
        Point2d pv;
        Point2d ph;

        horzint1st(wMap, relRay, M1, ph);
        vertint1st(wMap, relRay, M, pv);

        Cell hCellVal = 0;
        Cell vCellVal = 0;

        Cell mapKey = 0;

        bool v_not_found = false;
        bool h_not_found = false;
        // At most 8 transparent hits per column; a fixed stack buffer avoids a
        // heap allocation on every column of every frame (two passes).
        std::array<TransparentWallHit, 8> hits;
        size_t hitCount = 0;

        while ((!v_not_found || !h_not_found) && hitCount < hits.size()) {
            if (render_internal_wall) {
                while (!v_not_found
                    && (vCellVal = vertIntWall(wMap, pv, relRay),
                        !MapCell::hasAnyWall(vCellVal))) {
                    vertint(wMap, pv, relRay, M, pv);
                    if (!isInClientRect(wMap, pv)) {
                        v_not_found = true;
                        break;
                    }
                }

                while (!h_not_found
                    && (hCellVal = horzIntWall(wMap, ph, relRay),
                        !MapCell::hasAnyWall(hCellVal))) {
                    horzint(wMap, ph, relRay, M1, ph);
                    if (!isInClientRect(wMap, ph)) {
                        h_not_found = true;
                        break;
                    }
                }
            } else {
                while (!v_not_found
                    && (vCellVal = vertWall(wMap, pv, relRay),
                        !MapCell::hasAnyWall(vCellVal))) {
                    vertint(wMap, pv, relRay, M, pv);
                    if (!isInClientRect(wMap, pv)) {
                        v_not_found = true;
                        break;
                    }
                }

                while (!h_not_found
                    && (hCellVal = horzWall(wMap, ph, relRay),
                        !MapCell::hasAnyWall(hCellVal))) {
                    horzint(wMap, ph, relRay, M1, ph);
                    if (!isInClientRect(wMap, ph)) {
                        h_not_found = true;
                        break;
                    }
                }
            }

            if (v_not_found && h_not_found) {
                break;
            }

            // Compute the distance with intersections
            const double dh
                = h_not_found ? POSITIVE_INFINITY : horzDist(ph, relRay);
            const double dv
                = v_not_found ? POSITIVE_INFINITY : vertDist(pv, relRay);

            bool vert = true;

            // What's the nearest to the player ?
            if (dh < dv) {
                mapKey = hCellVal;
                vert = false;
            } else {
                mapKey = vCellVal;
            }

            if (render_internal_wall && MapCell::hasSolidWall(mapKey)) {
                break;
            }

            const auto wallKey = MapCell::transparentWallTexture(mapKey);

            if (!wallKey) {
                break;
            }

            const auto hitPoint = vert ? pv : ph;
            const auto hitCell = [&]() {
                int c = int(hitPoint.first / wMap.getCellDx());
                int r = int(hitPoint.second / wMap.getCellDy());

                if (render_internal_wall) {
                    if (vert) {
                        if (!(relRay >= m_player.deg90()
                                && relRay < m_player.deg270())) {
                            --c;
                        }
                    } else if (!(relRay >= m_player.deg180()
                                   && relRay < m_player.deg360())) {
                        --r;
                    }
                } else {
                    if (vert) {
                        if (relRay >= m_player.deg90()
                            && relRay < m_player.deg270()) {
                            --c;
                        }
                    } else if (relRay >= m_player.deg180()
                        && relRay < m_player.deg360()) {
                        --r;
                    }
                }

                if (c >= int(wMap.getColCount())) {
                    c = wMap.getColCount() - 1;
                } else if (c < 0) {
                    c = 0;
                }

                if (r >= int(wMap.getRowCount())) {
                    r = wMap.getRowCount() - 1;
                } else if (r < 0) {
                    r = 0;
                }

                return std::pair<int, int>{ r, c };
            }();

            hits[hitCount++]
                = { mapKey, hitPoint, vert ? dv : dh, vert, hitCell };

            if (vert) {
                vertint(wMap, pv, relRay, M, pv);
                v_not_found = !isInClientRect(wMap, pv);
            } else {
                horzint(wMap, ph, relRay, M1, ph);
                h_not_found = !isInClientRect(wMap, ph);
            }
        }

        for (size_t hi = hitCount; hi-- > 0;) {
            const TransparentWallHit* hitIt = &hits[hi];
            const auto mapKey = hitIt->mapKey;
            const auto& hitPoint = hitIt->point;
            const auto d = hitIt->distance;
            const auto vert = hitIt->vertical;
            const bool hitOnPlayerCell = (hitIt->cell == cameraCell);
            const bool effectiveInternalWall
                = render_internal_wall || hitOnPlayerCell;
            const auto face
                = wallFaceForHit(m_player, relRay, vert, effectiveInternalWall);
            const auto wallLayer
                = wMap.transparentWallTextureLayerAt(hitIt->cell.first,
                    hitIt->cell.second, MapCell::transparentWallTexture(mapKey),
                    face, effectiveInternalWall);
            const auto wallKey = wallLayer.base;
            const auto wallHeight = MapCell::upperWallTexture(mapKey);
            const bool hitCameraCell = hitIt->cell == cameraCell;

            // Compute the view distort LTU
            int distortDeg = ray - m_player.degHalfVisual();

            if (distortDeg >= m_player.deg360()) {
                distortDeg -= m_player.deg360();
            } else if (distortDeg < 0) {
                distortDeg += m_player.deg360();
            }

            double viewDistortLut = m_player.cos(distortDeg);
            double scaledDistortLut = m_scale / viewDistortLut;
            const double correctedDepth = d * viewDistortLut;

            int k = 0;
            int centerProj = 0;
            int ceilingProj = 0;

            // Prevent division by zero
            if (d > double(0.0)) {
                k = int(scaledDistortLut / d);
                centerProj = int(k * m_player.getCenterProj());
                ceilingProj = k - centerProj;
            }

            if (unsigned(k) < POSITIVE_INFINITY) {
                ////////////////////
                // Walls rendering

                int cellBound = 0;
                int currentCellRay = 0;

                if (vert) {
                    cellBound = wMap.getCellDy();
                    currentCellRay = int(hitPoint.second) % cellBound;
                } else {
                    cellBound = wMap.getCellDx();
                    currentCellRay = int(hitPoint.first) % cellBound;
                }

                if (shouldMirrorWallColumn(m_player, relRay, vert)) {
                    currentCellRay = (cellBound - 1) - currentCellRay;
                }

                if (currentCellRay >= 0 && currentCellRay < cellBound) {
                    int x_coord_source = currentCellRay;

                    double shadingAttr = double(k) / double(m_depthShadingPar);

                    if (wallHeight && !hitCameraCell) {
                        transpShadingStretchBtl(ray,
                            ((m_player.getSlope() + m_player.getYProjRes())
                                >> 1)
                                - ceilingProj - k,
                            k, x_coord_source, 0,
                            wMap.getCellDy(), // height
                            wMap.getCellDx(), // width (do not invert it)
                            m_player.getYProjRes(), shadingAttr,
                            wMap.getTexture(wallHeight), TRANSP_COLOR,
                            correctedDepth);
                    }

                    transpShadingStretchBtl(ray,
                        ((m_player.getSlope() + m_player.getYProjRes()) >> 1)
                            - ceilingProj,
                        k, x_coord_source, 0,
                        wMap.getCellDy(), // height
                        wMap.getCellDx(), // width (do not invert it)
                        m_player.getYProjRes(), shadingAttr,
                        wMap.getTexture(wallKey), TRANSP_COLOR, correctedDepth,
                        wallLayer.overlay ? wMap.getTexture(wallLayer.overlay)
                                          : nullptr);
                } // if current_cell...
            } // if k...
        }

    } // for ray
}


/* -------------------------------------------------------------------------- */

void RaycastEngine::renderToFrameBuffer(
    WorldMap& wMap, uint32_t width, uint32_t height)
{
    const auto profileStart = std::chrono::steady_clock::now();

    if (m_frameBuffer.width() != width || m_frameBuffer.height() != height) {
        m_frameBuffer.resize(width, height);
    }

    if (m_columnDepthBuffer.width()
        != static_cast<uint32_t>(m_player.getXProjRes())) {
        m_columnDepthBuffer.resize(m_player.getXProjRes());
    } else {
        m_columnDepthBuffer.clear();
    }

    const auto spriteDepthSize = static_cast<size_t>(m_player.getXProjRes())
        * static_cast<size_t>(m_player.getYProjRes());
    m_spritePixelDepthBuffer.assign(
        spriteDepthSize, ColumnDepthBuffer::farDepth());
    m_opaquePixelDepthBuffer.assign(
        spriteDepthSize, ColumnDepthBuffer::farDepth());

    const auto skyTexture = wMap.getTexture(MapCell::TRANSPARENT_TEXTURE_KEY);

    wMap.setPlayerPos(m_player.getX(), m_player.getY());

    const int cameraRayOffset = m_player.getAlpha();
    const int cameraXPos = m_player.getX();
    const int cameraYPos = m_player.getY();

    const int org_x_res = m_player.getXProjRes();

    const auto profileSky0 = std::chrono::steady_clock::now();

    if (skyTexture) {
        skyTexture->fillBuffer(m_frameBuffer.data(), m_frameBuffer.width(),
            m_frameBuffer.height(), cameraRayOffset /*+ m_fps/30*/, org_x_res);
    }

    const auto profileSky1 = std::chrono::steady_clock::now();

    // One caster for the whole frame; it only holds a reference to the player.
    const RayCaster caster(m_player);

    // main casting loop (for each pixel of projection x coord...). Columns are
    // independent, so the range is split across hardware threads; each column
    // writes only its own framebuffer column and per-column depth indices.
    auto renderColumnRange = [&](int rayBegin, int rayEnd) {
        for (int ray = rayBegin; ray < rayEnd; ++ray) {
            int relRay = ray + cameraRayOffset;

            if (relRay < 0)
                relRay += m_player.deg360();
            else if (relRay >= m_player.deg360())
                relRay -= m_player.deg360();

            const auto hit = caster.castSolidWallRay(wMap, relRay);

            if (!hit.found) {
                continue;
            }

            const auto wallKey = MapCell::solidWallTexture(hit.cell);
            const auto wallHeight = MapCell::upperWallTexture(hit.cell);

            // Compute the view distort LTU
            int distortDeg = ray - m_player.degHalfVisual();

            if (distortDeg >= m_player.deg360()) {
                distortDeg -= m_player.deg360();
            } else if (distortDeg < 0) {
                distortDeg += m_player.deg360();
            }

            const double viewDistortLut = m_player.cos(distortDeg);
            const double scaledDistortLut = m_scale / viewDistortLut;
            const double playerHeightRatio = m_player.getCenterProj();
            const double floorScaledDistortLut
                = scaledDistortLut * playerHeightRatio;
            const double ceilScaledDistortLut
                = scaledDistortLut * (1.0 - playerHeightRatio);
            const double minWallDistance = std::max(
                1.0, std::min(wMap.getCellDx(), wMap.getCellDy()) / 16.0);
            const double renderDistance
                = std::max(hit.distance, minWallDistance);

            if (hit.distance > 0.0 && wallKey
                && !MapCell::isTransparentTexture(wallKey)) {
                m_columnDepthBuffer.setNearestDepth(
                    ray, renderDistance * viewDistortLut);
            }
            const double correctedDepth = renderDistance * viewDistortLut;

            int k = 0;
            int centerProj = 0;
            int ceilingProj = 0;

            // Prevent division by zero
            if (renderDistance > 0.0) {
                k = int(scaledDistortLut / renderDistance);
                centerProj = int(k * m_player.getCenterProj());
                ceilingProj = k - centerProj;
            }

            if (unsigned(k) < POSITIVE_INFINITY) {
                // Ceil rendering
                int ceilBottom
                    = ((m_player.getYProjRes() + m_player.getSlope()) >> 1);

                // For each visible y-coord of screen
                for (int ceilRay = 0; ceilRay < (ceilBottom - ceilingProj);
                    ++ceilRay) {
                    const double deltaC = ceilBottom - ceilRay;

                    if (deltaC <= 0.0)
                        continue;

                    const double distToPtOnCeiling
                        = ceilScaledDistortLut / deltaC;

                    const int xPicture
                        = int(m_player.cos(relRay) * distToPtOnCeiling)
                        + cameraXPos;
                    const int yPicture
                        = int(m_player.sin(relRay) * distToPtOnCeiling)
                        + cameraYPos;

                    const int cellDx = wMap.getCellDx();
                    const int cellDy = wMap.getCellDy();

                    MapCell::TextureKey ceilKey = 0;

                    const int row = yPicture / cellDy;
                    const int col = xPicture / cellDx;

                    if (row < int(wMap.getRowCount())
                        && col < int(wMap.getColCount()) && col >= 0
                        && row >= 0) {
                        const Cell mapKey = wMap[row][col];

                        if (wMap.isSolidCellHit(row, col, mapKey))
                            continue;

                        ceilKey = MapCell::ceilingTexture(mapKey);
                    } else {
                        continue;
                    }

                    if (MapCell::isTransparentTexture(ceilKey)) {
                        continue;
                    }

                    const auto texture = wMap.getTexture(ceilKey);
                    if (!texture) {
                        continue;
                    }

                    const double shadingAttr
                        = m_ceilFloorShadingPar / double(distToPtOnCeiling);

                    const Color c = texture->getPixel(
                        xPicture % cellDx, yPicture % cellDy);
                    const auto depthIndex = static_cast<size_t>(ceilRay)
                            * static_cast<size_t>(m_player.getXProjRes())
                        + static_cast<size_t>(ray);
                    if (depthIndex < m_opaquePixelDepthBuffer.size()) {
                        m_opaquePixelDepthBuffer[depthIndex]
                            = std::min(m_opaquePixelDepthBuffer[depthIndex],
                                distToPtOnCeiling * viewDistortLut);
                    }

                    if (shadingAttr >= 1.0) {
                        drawPixel32(ray, ceilRay, c);
                    } else {
                        const double Rcomp = shadingAttr * colorRed(c);
                        const double Gcomp = shadingAttr * colorGreen(c);
                        const double Bcomp = shadingAttr * colorBlue(c);

                        drawPixel32(
                            ray, ceilRay, makeColor(Rcomp, Gcomp, Bcomp));
                    } // else
                }

                // Floor rendering
                for (int floorRay = m_player.getSlope();
                    floorRay < (ceilBottom + centerProj); ++floorRay) {
                    const double deltaC = ceilBottom - floorRay;
                    if (deltaC <= 0.0)
                        continue;

                    const double distToPtOnCeiling
                        = floorScaledDistortLut / deltaC;

                    const int xPicture
                        = int(m_player.cos(relRay) * distToPtOnCeiling)
                        + cameraXPos;
                    const int yPicture
                        = int(m_player.sin(relRay) * distToPtOnCeiling)
                        + cameraYPos;

                    const int cellDx = wMap.getCellDx();
                    const int cellDy = wMap.getCellDy();

                    MapCell::TextureKey floorKey = 0;

                    const int row = yPicture / cellDy;
                    const int col = xPicture / cellDx;

                    if ((row < int(wMap.getRowCount()))
                        && col < int(wMap.getColCount()) && row >= 0
                        && col >= 0) {
                        const Cell mapKey = wMap[row][col];
                        if (wMap.isSolidCellHit(row, col, mapKey))
                            continue;
                        floorKey = MapCell::floorTexture(mapKey);
                    } else {
                        continue;
                    }

                    if (MapCell::isTransparentTexture(floorKey)) {
                        continue;
                    }

                    const auto texture = wMap.getTexture(floorKey);
                    if (!texture) {
                        continue;
                    }

                    const double shadingAttr
                        = m_ceilFloorShadingPar / double(distToPtOnCeiling);
                    const Color c = texture->getPixel(
                        xPicture % cellDx, yPicture % cellDy);

                    const int y = m_player.getSlope() + m_player.getYProjRes()
                        - floorRay;
                    const auto depthIndex = static_cast<size_t>(y)
                            * static_cast<size_t>(m_player.getXProjRes())
                        + static_cast<size_t>(ray);
                    if (depthIndex < m_opaquePixelDepthBuffer.size()) {
                        m_opaquePixelDepthBuffer[depthIndex]
                            = std::min(m_opaquePixelDepthBuffer[depthIndex],
                                distToPtOnCeiling * viewDistortLut);
                    }

                    if (shadingAttr >= 1.0) {
                        drawPixel32(ray, y, c);
                    } else {
                        const double Rcomp = shadingAttr * colorRed(c);
                        const double Gcomp = shadingAttr * colorGreen(c);
                        const double Bcomp = shadingAttr * colorBlue(c);

                        drawPixel32(ray, y, makeColor(Rcomp, Gcomp, Bcomp));
                    } // else
                } // for


                ////////////////////
                // Walls rendering
                if (wallKey && !MapCell::isTransparentTexture(wallKey)) {
                    int cellBound = 0;
                    int currentCellRay = 0;

                    // Determine the cell bound
                    if (hit.vertical) {
                        cellBound = wMap.getCellDy();
                        currentCellRay = int(hit.point.second) % cellBound;
                    } else {
                        cellBound = wMap.getCellDx();
                        currentCellRay = int(hit.point.first) % cellBound;
                    }

                    if (shouldMirrorWallColumn(
                            m_player, relRay, hit.vertical)) {
                        currentCellRay = (cellBound - 1) - currentCellRay;
                    }

                    const double shadingAttr
                        = double(k) / double(m_depthShadingPar);
                    const int horizonY
                        = ((m_player.getSlope() + m_player.getYProjRes()) >> 1);
                    const int cellDy = wMap.getCellDy();

                    bool renderedAnyExtraSpan = false;
                    if (hit.block != nullptr) {
                        for (const auto& span : hit.block->walls) {
                            if (span.kind != WinRayCast::WallSpanKind::Solid) {
                                continue;
                            }

                            const auto spanTextureLayer
                                = wMap.textureLayerForWallSpanAt(
                                    hit.row, hit.column, span, hit.face);
                            const auto spanTextureKey = spanTextureLayer.base;
                            if (!spanTextureKey
                                || MapCell::isTransparentTexture(
                                    spanTextureKey)) {
                                continue;
                            }

                            const int spanTopWorld = span.top;
                            const int spanBottomWorld = span.bottom;
                            const int spanTopScreen = horizonY + centerProj
                                - int((double(k) * spanTopWorld) / cellDy);
                            const int spanBottomScreen = horizonY + centerProj
                                - int((double(k) * spanBottomWorld) / cellDy);
                            int spanHeight = spanBottomScreen - spanTopScreen;
                            if (spanHeight <= 0) {
                                continue;
                            }

                            shadingStretchBtl(ray, spanTopScreen, spanHeight,
                                currentCellRay, 0, cellDy, wMap.getCellDx(),
                                m_player.getYProjRes(), shadingAttr,
                                wMap.getTexture(spanTextureKey), correctedDepth,
                                spanTextureLayer.overlay
                                    ? wMap.getTexture(spanTextureLayer.overlay)
                                    : nullptr);

                            if (spanBottomWorld >= cellDy) {
                                // span starts above the floor: ceiling slice
                                // visible beneath top of span
                                const int ceilBottom = horizonY;
                                for (int ceilRay = 0;
                                    ceilRay < (ceilBottom - ceilingProj);
                                    ++ceilRay) {
                                    const double deltaC = ceilBottom - ceilRay;
                                    if (deltaC <= 0.0)
                                        continue;
                                    const double distToPtOnCeiling
                                        = ceilScaledDistortLut / deltaC;
                                    const int xPicture
                                        = int(m_player.cos(relRay)
                                              * distToPtOnCeiling)
                                        + cameraXPos;
                                    const int yPicture
                                        = int(m_player.sin(relRay)
                                              * distToPtOnCeiling)
                                        + cameraYPos;
                                    const int cellDx = wMap.getCellDx();
                                    const int row = yPicture / cellDy;
                                    const int col = xPicture / cellDx;
                                    if (row < 0 || col < 0
                                        || row >= int(wMap.getRowCount())
                                        || col >= int(wMap.getColCount())) {
                                        continue;
                                    }

                                    const Cell mapKey = wMap[row][col];
                                    if (wMap.isSolidCellHit(row, col, mapKey)) {
                                        continue;
                                    }

                                    const auto ceilKey
                                        = MapCell::ceilingTexture(mapKey);
                                    if (MapCell::isTransparentTexture(
                                            ceilKey)) {
                                        continue;
                                    }

                                    const auto texture
                                        = wMap.getTexture(ceilKey);
                                    if (!texture) {
                                        continue;
                                    }

                                    const double shadingC
                                        = m_ceilFloorShadingPar
                                        / double(distToPtOnCeiling);
                                    const Color c = texture->getPixel(
                                        xPicture % cellDx, yPicture % cellDy);
                                    const auto depthIndex
                                        = static_cast<size_t>(ceilRay)
                                            * static_cast<size_t>(
                                                m_player.getXProjRes())
                                        + static_cast<size_t>(ray);
                                    if (depthIndex
                                        < m_opaquePixelDepthBuffer.size()) {
                                        m_opaquePixelDepthBuffer[depthIndex]
                                            = std::min(m_opaquePixelDepthBuffer
                                                           [depthIndex],
                                                distToPtOnCeiling
                                                    * viewDistortLut);
                                    }

                                    if (shadingC >= 1.0) {
                                        drawPixel32(ray, ceilRay, c);
                                    } else {
                                        const double Rcomp
                                            = shadingC * colorRed(c);
                                        const double Gcomp
                                            = shadingC * colorGreen(c);
                                        const double Bcomp
                                            = shadingC * colorBlue(c);
                                        drawPixel32(ray, ceilRay,
                                            makeColor(Rcomp, Gcomp, Bcomp));
                                    }
                                }
                            }

                            renderedAnyExtraSpan = true;
                        }
                    }

                    if (!renderedAnyExtraSpan) {
                        const auto currentTexture = wMap.getTexture(wallKey);
                        if (wallHeight) {
                            shadingStretchBtl(ray, horizonY - ceilingProj - k,
                                k, currentCellRay, 0, cellDy, wMap.getCellDx(),
                                m_player.getYProjRes(), shadingAttr,
                                wMap.getTexture(wallHeight), correctedDepth);

                            // Ceil rendering above the upper wall
                            const int ceilBottom = horizonY;
                            for (int ceilRay = 0;
                                ceilRay < (ceilBottom - ceilingProj);
                                ++ceilRay) {
                                const double deltaC = ceilBottom - ceilRay;
                                if (deltaC <= 0.0)
                                    continue;
                                const double distToPtOnCeiling
                                    = ceilScaledDistortLut / deltaC;
                                const int xPicture = int(m_player.cos(relRay)
                                                         * distToPtOnCeiling)
                                    + cameraXPos;
                                const int yPicture = int(m_player.sin(relRay)
                                                         * distToPtOnCeiling)
                                    + cameraYPos;
                                const int cellDx = wMap.getCellDx();
                                const int row = yPicture / cellDy;
                                const int col = xPicture / cellDx;
                                if (row < 0 || col < 0
                                    || row >= int(wMap.getRowCount())
                                    || col >= int(wMap.getColCount())) {
                                    continue;
                                }

                                const Cell mapKey = wMap[row][col];
                                if (wMap.isSolidCellHit(row, col, mapKey))
                                    continue;
                                const auto ceilKey
                                    = MapCell::ceilingTexture(mapKey);
                                if (MapCell::isTransparentTexture(ceilKey))
                                    continue;
                                const auto texture = wMap.getTexture(ceilKey);
                                if (!texture)
                                    continue;

                                const double shadingC = m_ceilFloorShadingPar
                                    / double(distToPtOnCeiling);
                                const Color c = texture->getPixel(
                                    xPicture % cellDx, yPicture % cellDy);
                                const auto depthIndex
                                    = static_cast<size_t>(ceilRay)
                                        * static_cast<size_t>(
                                            m_player.getXProjRes())
                                    + static_cast<size_t>(ray);
                                if (depthIndex
                                    < m_opaquePixelDepthBuffer.size()) {
                                    m_opaquePixelDepthBuffer[depthIndex]
                                        = std::min(m_opaquePixelDepthBuffer
                                                       [depthIndex],
                                            distToPtOnCeiling * viewDistortLut);
                                }

                                if (shadingC >= 1.0) {
                                    drawPixel32(ray, ceilRay, c);
                                } else {
                                    const double Rcomp = shadingC * colorRed(c);
                                    const double Gcomp
                                        = shadingC * colorGreen(c);
                                    const double Bcomp
                                        = shadingC * colorBlue(c);
                                    drawPixel32(ray, ceilRay,
                                        makeColor(Rcomp, Gcomp, Bcomp));
                                }
                            }
                        }

                        shadingStretchBtl(ray, horizonY - ceilingProj, k,
                            currentCellRay, 0, cellDy, wMap.getCellDx(),
                            m_player.getYProjRes(), shadingAttr, currentTexture,
                            correctedDepth);
                    }
                }
            } // if k...
        } // for
    };

    parallelColumns(org_x_res, renderColumnRange);

    const auto profileWalls1 = std::chrono::steady_clock::now();

    renderSprites(wMap);

    const auto profileSprites1 = std::chrono::steady_clock::now();

    renderTranspWall(wMap, true); // internal
    renderTranspWall(wMap, false); // external

    const auto profileTransp1 = std::chrono::steady_clock::now();

    renderViewWeapon();

    const auto profileWeapon1 = std::chrono::steady_clock::now();

    using ProfileMs = std::chrono::duration<double, std::milli>;
    m_lastProfile.skyMs = ProfileMs(profileSky1 - profileSky0).count();
    m_lastProfile.wallsFloorCeilingMs
        = ProfileMs(profileWalls1 - profileSky1).count();
    m_lastProfile.spritesMs
        = ProfileMs(profileSprites1 - profileWalls1).count();
    m_lastProfile.transparentWallsMs
        = ProfileMs(profileTransp1 - profileSprites1).count();
    m_lastProfile.viewWeaponMs
        = ProfileMs(profileWeapon1 - profileTransp1).count();
    m_lastProfile.totalMs = ProfileMs(profileWeapon1 - profileStart).count();

    ++m_fps;
}


/* -------------------------------------------------------------------------- */
