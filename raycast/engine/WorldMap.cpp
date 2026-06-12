// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.


/* -------------------------------------------------------------------------- */

#include "WorldMap.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>


/* -------------------------------------------------------------------------- */

namespace {

std::string normalizeDoorKey(std::string key)
{
    std::string normalized;
    normalized.reserve(key.size());
    for (char ch : key) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            normalized.push_back(static_cast<char>(
                std::tolower(static_cast<unsigned char>(ch))));
        }
    }

    return normalized;
}

bool doorKeyringContains(
    const std::vector<std::string>& keyring, const std::string& requiredKey)
{
    const auto normalizedRequired = normalizeDoorKey(requiredKey);
    if (normalizedRequired.empty()) {
        return true;
    }

    return std::find_if(keyring.begin(), keyring.end(),
               [&normalizedRequired](const std::string& key) {
                   const auto normalizedKey = normalizeDoorKey(key);
                   if (normalizedKey.empty()) {
                       return false;
                   }

                   return normalizedKey == normalizedRequired
                       || normalizedKey.find(normalizedRequired)
                       != std::string::npos
                       || normalizedRequired.find(normalizedKey)
                       != std::string::npos;
               })
        != keyring.end();
}

MapCell::TextureKey lockedDoorOverlayTexture(
    const WinRayCast::DoorDefinition& door,
    const std::vector<std::string>& keyring) noexcept
{
    if (!door.enabled || door.requiredKey.empty()
        || doorKeyringContains(keyring, door.requiredKey)) {
        return 0;
    }

    const auto normalizedRequired = normalizeDoorKey(door.requiredKey);
    if (normalizedRequired.empty()) {
        return 0;
    }

    const auto exact
        = door.lockedOverlayTextureKeysByKey.find(normalizedRequired);
    if (exact != door.lockedOverlayTextureKeysByKey.end()) {
        return exact->second;
    }

    const auto fallback = door.lockedOverlayTextureKeysByKey.find("default");
    return fallback != door.lockedOverlayTextureKeysByKey.end()
        ? fallback->second
        : 0;
}

MapCell::TextureKey textureAnimationFrame(
    const WinRayCast::WallTextureAnimation& animation,
    double timeSeconds) noexcept
{
    if (animation.textureKeys.empty()
        || animation.frameDurationSeconds <= 0.0) {
        return 0;
    }

    auto frameIndex = static_cast<size_t>(std::floor(
        std::max(0.0, timeSeconds) / animation.frameDurationSeconds));
    if (animation.loop) {
        frameIndex %= animation.textureKeys.size();
    } else if (frameIndex >= animation.textureKeys.size()) {
        frameIndex = animation.textureKeys.size() - 1;
    }

    return animation.textureKeys[frameIndex];
}

MapCell::TextureKey animatedTextureForFace(
    const std::vector<WinRayCast::WallTextureAnimation>& animations,
    WinRayCast::WallFace face, double timeSeconds) noexcept
{
    const auto faceIndex = static_cast<int>(face);
    const WinRayCast::WallTextureAnimation* allFaces = nullptr;
    for (const auto& animation : animations) {
        if (animation.textureKeys.empty()) {
            continue;
        }

        if (animation.face == faceIndex) {
            return textureAnimationFrame(animation, timeSeconds);
        }

        if (animation.face < 0 && allFaces == nullptr) {
            allFaces = &animation;
        }
    }

    return allFaces != nullptr ? textureAnimationFrame(*allFaces, timeSeconds)
                               : 0;
}

} // namespace


/* -------------------------------------------------------------------------- */

bool WorldMap::setMapInfo(const Cell* array, uint32_t rows, uint32_t cols)
{
    if (rows <= 0 || cols <= 0) {
        return false;
    }

    m_map.resize(rows);

    int i = 0;

    for (uint32_t r = 0; r < rows; ++r) {
        m_map[r].resize(cols);

        for (uint32_t c = 0; c < cols; ++c) {
            m_map[r][c] = array[i++];
        }
    }

    m_maxX = getCellDx() * getColCount();
    m_maxY = getCellDy() * getRowCount();
    m_doorStates.assign(rows, std::vector<DoorRuntimeState>(cols));

    return true; // success
}


/* -------------------------------------------------------------------------- */

bool WorldMap::isSolidCellHit(int row, int column, Cell cell) const noexcept
{
    if (row < 0 || column < 0 || row >= getRowCount()
        || column >= getColCount()) {
        return true;
    }

    if (MapCell::hasAnyWall(cell)) {
        return MapCell::hasSolidWall(cell);
    }

    const auto* block = blockAtCell(row, column);
    if (block != nullptr) {
        return block->hasAnySolidSpan;
    }

    return false;
}


/* -------------------------------------------------------------------------- */

bool WorldMap::isDoorOpenAt(int row, int column) const noexcept
{
    return doorOpenAmountAt(row, column) >= 0.98;
}


/* -------------------------------------------------------------------------- */

double WorldMap::doorOpenAmountAt(int row, int column) const noexcept
{
    if (row < 0 || column < 0 || row >= static_cast<int>(m_doorStates.size())
        || column >= static_cast<int>(m_doorStates[row].size())) {
        return 0.0;
    }

    return m_doorStates[row][column].openAmount;
}


/* -------------------------------------------------------------------------- */

WorldMap::TextureLayer WorldMap::textureLayerForWallSpanAt(int row, int column,
    const WinRayCast::WallSpan& span, WinRayCast::WallFace face,
    bool internalWall) const noexcept
{
    TextureLayer layer;

    const auto faceIndex = static_cast<std::size_t>(face);
    if (faceIndex < span.facesEnabled.size() && !span.facesEnabled[faceIndex]) {
        return layer;
    }

    if (internalWall && span.interiorTextureKey != 0) {
        layer.base = span.interiorTextureKey;
        return layer;
    }

    const auto faceOverride = faceIndex < span.faceTextureKeys.size()
        ? span.faceTextureKeys[faceIndex]
        : MapCell::TextureKey(0);
    const auto* block = blockAtCell(row, column);
    const auto animatedOverlay = animatedTextureForFace(
        span.overlayAnimations, face, m_dynamicTextureTimeSeconds);
    if (block != nullptr) {
        layer.overlay = animatedOverlay != 0
            ? animatedOverlay
            : lockedDoorOverlayTexture(block->door, m_doorKeyring);
    } else {
        layer.overlay = animatedOverlay;
    }

    const auto animatedBase = animatedTextureForFace(
        span.baseAnimations, face, m_dynamicTextureTimeSeconds);
    if (animatedBase != 0) {
        layer.base = animatedBase;
        return layer;
    }

    // A per-face texture override always wins so the visual is identical
    // from any side. To keep a face animated by the door, simply leave its
    // override unset.
    if (faceOverride != 0) {
        layer.base = faceOverride;
        return layer;
    }

    if (block != nullptr && block->door.enabled
        && !block->door.animationTextureKeys.empty()) {
        const auto amount
            = std::max(0.0, std::min(1.0, doorOpenAmountAt(row, column)));
        const auto frameCount = block->door.animationTextureKeys.size();
        auto frameIndex = static_cast<size_t>(
            std::round(amount * static_cast<double>(frameCount - 1)));
        if (frameIndex >= frameCount) {
            frameIndex = frameCount - 1;
        }

        const auto textureKey = block->door.animationTextureKeys[frameIndex];
        if (textureKey != 0) {
            layer.base = textureKey;
            return layer;
        }
    }

    layer.base = span.textureKey;
    return layer;
}


/* -------------------------------------------------------------------------- */

void WorldMap::advanceDynamicTextures(double deltaSeconds) noexcept
{
    if (deltaSeconds <= 0.0) {
        return;
    }

    m_dynamicTextureTimeSeconds += deltaSeconds;
    if (m_dynamicTextureTimeSeconds > 86400.0) {
        m_dynamicTextureTimeSeconds
            = std::fmod(m_dynamicTextureTimeSeconds, 86400.0);
    }
}


/* -------------------------------------------------------------------------- */

MapCell::TextureKey WorldMap::textureForWallSpanAt(int row, int column,
    const WinRayCast::WallSpan& span, WinRayCast::WallFace face,
    bool internalWall) const noexcept
{
    return textureLayerForWallSpanAt(row, column, span, face, internalWall)
        .base;
}


/* -------------------------------------------------------------------------- */

WorldMap::TextureLayer WorldMap::transparentWallTextureLayerAt(int row,
    int column, MapCell::TextureKey fallback, WinRayCast::WallFace face,
    bool internalWall) const noexcept
{
    const auto* block = blockAtCell(row, column);
    if (block == nullptr) {
        return { fallback, 0 };
    }

    for (const auto& span : block->walls) {
        if (span.kind == WinRayCast::WallSpanKind::Transparent) {
            return textureLayerForWallSpanAt(
                row, column, span, face, internalWall);
        }
    }

    return { fallback, 0 };
}


/* -------------------------------------------------------------------------- */

MapCell::TextureKey WorldMap::transparentWallTextureAt(int row, int column,
    MapCell::TextureKey fallback, WinRayCast::WallFace face,
    bool internalWall) const noexcept
{
    return transparentWallTextureLayerAt(
        row, column, fallback, face, internalWall)
        .base;
}


/* -------------------------------------------------------------------------- */

void WorldMap::updateDoors(double playerX, double playerY,
    const std::vector<Point2d>& actorPositions, double deltaSeconds,
    std::vector<DoorEvent>* events) noexcept
{
    if (!m_hasBlockIds || deltaSeconds <= 0.0) {
        return;
    }

    const auto cellSize
        = (static_cast<double>(getCellDx()) + static_cast<double>(getCellDy()))
        * 0.5;

    for (int row = 0; row < getRowCount(); ++row) {
        for (int column = 0; column < getColCount(); ++column) {
            const auto* block = blockAtCell(row, column);
            if (block == nullptr || !block->door.enabled) {
                continue;
            }

            if (row >= static_cast<int>(m_doorStates.size())
                || column >= static_cast<int>(m_doorStates[row].size())) {
                continue;
            }

            const auto triggerDistance
                = std::max(0.0, block->door.triggerDistanceCells) * cellSize;
            const auto doorX
                = (static_cast<double>(column) + 0.5) * getCellDx();
            const auto doorY = (static_cast<double>(row) + 0.5) * getCellDy();

            const auto checkNear = [&](double x, double y) noexcept {
                const auto dx = x - doorX;
                const auto dy = y - doorY;
                return std::sqrt(dx * dx + dy * dy) <= triggerDistance;
            };
            const auto playerIsNear = checkNear(playerX, playerY);
            auto isNear = false;

            if (!block->door.requiredKey.empty()) {
                isNear = playerIsNear
                    && doorKeyringContains(
                        m_doorKeyring, block->door.requiredKey);
            } else {
                isNear = playerIsNear;
                for (const auto& actor : actorPositions) {
                    isNear = isNear || checkNear(actor.first, actor.second);
                    if (isNear) {
                        break;
                    }
                }
            }

            auto& state = m_doorStates[row][column];
            const auto openTime = std::max(0.01, block->door.openTimeSeconds);
            const auto previousOpenAmount = state.openAmount;
            if (isNear) {
                state.closeDelay = std::max(0.0, block->door.closeDelaySeconds);
                state.openAmount
                    = std::min(1.0, state.openAmount + deltaSeconds / openTime);
                if (events != nullptr && previousOpenAmount <= 0.0
                    && state.openAmount > 0.0) {
                    events->push_back({ DoorEvent::Type::OpeningStarted, row,
                        column, blockIdAt(row, column) });
                }
            } else if (state.closeDelay > 0.0) {
                state.closeDelay
                    = std::max(0.0, state.closeDelay - deltaSeconds);
            } else {
                state.openAmount
                    = std::max(0.0, state.openAmount - deltaSeconds / openTime);
            }
        }
    }
}


/* -------------------------------------------------------------------------- */

void WorldMap::forceDoorClosingAt(
    int row, int column, double deltaSeconds) noexcept
{
    if (!m_hasBlockIds || deltaSeconds <= 0.0) {
        return;
    }

    if (row < 0 || column < 0 || row >= static_cast<int>(m_doorStates.size())
        || column >= static_cast<int>(m_doorStates[row].size())) {
        return;
    }

    const auto* block = blockAtCell(row, column);
    if (block == nullptr || !block->door.enabled) {
        return;
    }

    const auto openTime = std::max(0.01, block->door.openTimeSeconds);
    auto& state = m_doorStates[row][column];
    state.closeDelay = 0.0;
    state.openAmount
        = std::max(0.0, state.openAmount - 2.0 * deltaSeconds / openTime);
}


/* -------------------------------------------------------------------------- */

void WorldMap::forceDoorOpenAt(int row, int column) noexcept
{
    if (!m_hasBlockIds) {
        return;
    }

    if (row < 0 || column < 0 || row >= static_cast<int>(m_doorStates.size())
        || column >= static_cast<int>(m_doorStates[row].size())) {
        return;
    }

    const auto* block = blockAtCell(row, column);
    if (block == nullptr || !block->door.enabled) {
        return;
    }

    auto& state = m_doorStates[row][column];
    state.openAmount = 1.0;
    state.closeDelay = std::max(0.0, block->door.closeDelaySeconds);
}
