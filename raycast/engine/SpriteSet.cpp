// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "SpriteSet.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace {
double normalizeDegrees(double angle) noexcept
{
    while (angle < 0.0) {
        angle += 360.0;
    }

    while (angle >= 360.0) {
        angle -= 360.0;
    }

    return angle;
}

double angularDistance(double lhs, double rhs) noexcept
{
    const auto delta = std::fabs(normalizeDegrees(lhs) - normalizeDegrees(rhs));
    return std::min(delta, 360.0 - delta);
}
} // namespace

void SpriteSet::setName(std::string name) { m_name = std::move(name); }

void SpriteSet::setFormat(std::string format) { m_format = std::move(format); }

void SpriteSet::setTransparentColor(Color color) noexcept
{
    m_transparentColor = color;
}

void SpriteSet::setSupportedResolutions(std::vector<uint32_t> resolutions)
{
    std::sort(resolutions.begin(), resolutions.end());
    resolutions.erase(
        std::unique(resolutions.begin(), resolutions.end()), resolutions.end());
    m_supportedResolutions = std::move(resolutions);
}

void SpriteSet::setDefaultResolution(uint32_t resolution) noexcept
{
    m_defaultResolution = resolution;
}

void SpriteSet::setMaxResolution(uint32_t resolution) noexcept
{
    m_maxResolution = resolution;
}

void SpriteSet::setDirections(std::vector<SpriteDirectionDefinition> directions)
{
    m_directions = std::move(directions);
}

void SpriteSet::setAnimations(std::vector<SpriteAnimationDefinition> animations)
{
    m_animations = std::move(animations);
}

void SpriteSet::setLodRules(std::vector<SpriteLodRule> lodRules)
{
    std::sort(lodRules.begin(), lodRules.end(),
        [](const SpriteLodRule& lhs, const SpriteLodRule& rhs) {
            return lhs.maxDistanceCells < rhs.maxDistanceCells;
        });
    m_lodRules = std::move(lodRules);
}

const std::string& SpriteSet::name() const noexcept { return m_name; }

const std::string& SpriteSet::format() const noexcept { return m_format; }

Color SpriteSet::transparentColor() const noexcept
{
    return m_transparentColor;
}

uint32_t SpriteSet::defaultResolution() const noexcept
{
    return m_defaultResolution;
}

uint32_t SpriteSet::maxResolution() const noexcept { return m_maxResolution; }

const std::vector<uint32_t>& SpriteSet::supportedResolutions() const noexcept
{
    return m_supportedResolutions;
}

const std::vector<SpriteDirectionDefinition>&
SpriteSet::directions() const noexcept
{
    return m_directions;
}

const std::vector<SpriteAnimationDefinition>&
SpriteSet::animations() const noexcept
{
    return m_animations;
}

const SpriteAnimationDefinition* SpriteSet::animation(
    const std::string& name) const noexcept
{
    for (const auto& animation : m_animations) {
        if (animation.name == name) {
            return &animation;
        }
    }

    return nullptr;
}

const std::vector<SpriteLodRule>& SpriteSet::lodRules() const noexcept
{
    return m_lodRules;
}

const SpriteDirectionDefinition* SpriteSet::directionForAngle(
    double angleDegrees) const noexcept
{
    const SpriteDirectionDefinition* bestDirection = nullptr;
    auto bestDistance = std::numeric_limits<double>::infinity();

    for (const auto& direction : m_directions) {
        const auto distance
            = angularDistance(angleDegrees, direction.angleDegrees);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestDirection = &direction;
        }
    }

    return bestDirection;
}

uint32_t SpriteSet::resolutionForDistance(double distanceCells) const noexcept
{
    for (const auto& lodRule : m_lodRules) {
        if (distanceCells <= lodRule.maxDistanceCells) {
            return lodRule.resolution;
        }
    }

    if (!m_lodRules.empty()) {
        return m_lodRules.back().resolution;
    }

    return m_defaultResolution;
}

uint32_t SpriteSet::closestAvailableResolution(
    const SpriteDirectionDefinition& direction,
    uint32_t preferredResolution) const noexcept
{
    const auto preferred
        = direction.filesByResolution.find(preferredResolution);
    if (preferred != direction.filesByResolution.end()) {
        return preferred->first;
    }

    uint32_t bestLower = 0;
    uint32_t bestHigher = 0;

    for (const auto& item : direction.filesByResolution) {
        if (item.first < preferredResolution && item.first > bestLower) {
            bestLower = item.first;
        }

        if (item.first > preferredResolution
            && (bestHigher == 0 || item.first < bestHigher)) {
            bestHigher = item.first;
        }
    }

    return bestLower != 0 ? bestLower : bestHigher;
}

const std::string* SpriteSet::fileFor(
    const SpriteDirectionDefinition& direction,
    uint32_t preferredResolution) const noexcept
{
    const auto resolution
        = closestAvailableResolution(direction, preferredResolution);
    if (resolution == 0) {
        return nullptr;
    }

    const auto file = direction.filesByResolution.find(resolution);
    return file == direction.filesByResolution.end() ? nullptr : &file->second;
}
