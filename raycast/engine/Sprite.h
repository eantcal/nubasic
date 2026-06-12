// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include "Color.h"
#include "MapCell.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct SpriteFrame {
    MapCell::TextureResourceKey textureKey = 0;
};

class DirectionalSpriteFrames {
public:
    explicit DirectionalSpriteFrames(std::vector<SpriteFrame> views = {})
        : m_views(std::move(views))
    {
    }

    bool empty() const noexcept { return m_views.empty(); }

    size_t viewCount() const noexcept { return m_views.size(); }

    const SpriteFrame* frameForView(size_t viewIndex) const noexcept
    {
        if (m_views.empty()) {
            return nullptr;
        }

        return &m_views[viewIndex % m_views.size()];
    }

    size_t selectViewIndex(double cameraX, double cameraY, double spriteX,
        double spriteY, double spriteFacingRadians) const noexcept
    {
        if (m_views.empty()) {
            return 0;
        }

        const auto toCamera = std::atan2(cameraY - spriteY, cameraX - spriteX);
        const auto relative = normalizeRadians(toCamera - spriteFacingRadians);
        const auto normalized = relative / twoPi();
        const auto rawIndex = static_cast<size_t>(
            std::floor(normalized * m_views.size() + 0.5));

        return rawIndex % m_views.size();
    }

private:
    static constexpr double pi() noexcept { return 3.14159265358979323846; }

    static constexpr double twoPi() noexcept { return pi() * 2.0; }

    static double normalizeRadians(double angle) noexcept
    {
        while (angle < 0.0) {
            angle += twoPi();
        }

        while (angle >= twoPi()) {
            angle -= twoPi();
        }

        return angle;
    }

    std::vector<SpriteFrame> m_views;
};

struct SpriteAnimationClip {
    SpriteAnimationClip() = default;

    SpriteAnimationClip(std::string clipName,
        DirectionalSpriteFrames clipFrames,
        std::vector<DirectionalSpriteFrames> clipFrameSets,
        double clipFrameDurationMs, bool clipLoop)
        : name(std::move(clipName))
        , frames(std::move(clipFrames))
        , frameSets(std::move(clipFrameSets))
        , frameDurationMs(clipFrameDurationMs)
        , loop(clipLoop)
    {
    }

    std::string name;
    DirectionalSpriteFrames frames;
    std::vector<DirectionalSpriteFrames> frameSets;
    double frameDurationMs = 0.0;
    bool loop = true;
};

struct Sprite {
    double x = 0.0;
    double y = 0.0;
    double facingRadians = 0.0;
    double scale = 1.0;
    double collisionRadius = 0.0;
    bool visible = true;
    Color transparentColor = makeColor(0, 0, 0);
    DirectionalSpriteFrames frames;
    std::vector<SpriteAnimationClip> animations;
    std::string activeAnimation = "idle";
    double animationTimeSeconds = 0.0;
    size_t animationFrameIndex = 0;

    const SpriteAnimationClip* animation(const std::string& name) const noexcept
    {
        for (const auto& clip : animations) {
            if (clip.name == name) {
                return &clip;
            }
        }

        return nullptr;
    }

    bool setAnimation(const std::string& name) noexcept
    {
        if (!animation(name)) {
            return false;
        }

        if (activeAnimation != name) {
            activeAnimation = name;
            animationTimeSeconds = 0.0;
            animationFrameIndex = 0;
        }

        return true;
    }

    bool setAnimationOrFallback(
        const std::string& preferred, const std::string& fallback) noexcept
    {
        return setAnimation(preferred) || setAnimation(fallback);
    }

    const DirectionalSpriteFrames& activeFrames() const noexcept
    {
        const auto* clip = animation(activeAnimation);
        if (clip != nullptr && !clip->frameSets.empty()
            && !clip->frameSets[animationFrameIndex % clip->frameSets.size()]
                .empty()) {
            return clip
                ->frameSets[animationFrameIndex % clip->frameSets.size()];
        }

        if (clip != nullptr && !clip->frames.empty()) {
            return clip->frames;
        }

        const auto* idle = animation("idle");
        if (idle != nullptr && !idle->frameSets.empty()
            && !idle->frameSets[animationFrameIndex % idle->frameSets.size()]
                .empty()) {
            return idle
                ->frameSets[animationFrameIndex % idle->frameSets.size()];
        }

        if (idle != nullptr && !idle->frames.empty()) {
            return idle->frames;
        }

        return frames;
    }

    void advanceAnimation(double deltaSeconds) noexcept
    {
        if (deltaSeconds <= 0.0) {
            return;
        }

        animationTimeSeconds += deltaSeconds;

        const auto* clip = animation(activeAnimation);
        if (clip == nullptr || clip->frameDurationMs <= 0.0
            || clip->frameSets.empty()) {
            animationFrameIndex = 0;
            return;
        }

        const auto frameDurationSeconds = clip->frameDurationMs / 1000.0;
        if (frameDurationSeconds <= 0.0) {
            animationFrameIndex = 0;
            return;
        }

        const auto computedFrame
            = static_cast<size_t>(animationTimeSeconds / frameDurationSeconds);
        if (clip->loop) {
            animationFrameIndex = computedFrame % clip->frameSets.size();
        } else {
            animationFrameIndex = computedFrame >= clip->frameSets.size()
                ? clip->frameSets.size() - 1
                : computedFrame;
        }
    }
};
