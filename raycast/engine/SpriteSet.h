// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include "Color.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct SpriteDirectionDefinition {
    std::string name;
    double angleDegrees = 0.0;
    std::map<uint32_t, std::string> filesByResolution;
};

struct SpriteLodRule {
    double maxDistanceCells = 0.0;
    uint32_t resolution = 0;
};

struct SpriteAnimationDefinition {
    std::string name;
    double frameDurationMs = 0.0;
    bool loop = true;
    std::vector<SpriteDirectionDefinition> directions;
    std::vector<std::vector<SpriteDirectionDefinition>> frames;
};

class SpriteSet {
public:
    void setName(std::string name);
    void setFormat(std::string format);
    void setTransparentColor(Color color) noexcept;
    void setSupportedResolutions(std::vector<uint32_t> resolutions);
    void setDefaultResolution(uint32_t resolution) noexcept;
    void setMaxResolution(uint32_t resolution) noexcept;
    void setDirections(std::vector<SpriteDirectionDefinition> directions);
    void setAnimations(std::vector<SpriteAnimationDefinition> animations);
    void setLodRules(std::vector<SpriteLodRule> lodRules);

    const std::string& name() const noexcept;
    const std::string& format() const noexcept;
    Color transparentColor() const noexcept;
    uint32_t defaultResolution() const noexcept;
    uint32_t maxResolution() const noexcept;
    const std::vector<uint32_t>& supportedResolutions() const noexcept;
    const std::vector<SpriteDirectionDefinition>& directions() const noexcept;
    const std::vector<SpriteAnimationDefinition>& animations() const noexcept;
    const SpriteAnimationDefinition* animation(
        const std::string& name) const noexcept;
    const std::vector<SpriteLodRule>& lodRules() const noexcept;

    const SpriteDirectionDefinition* directionForAngle(
        double angleDegrees) const noexcept;
    uint32_t resolutionForDistance(double distanceCells) const noexcept;
    uint32_t closestAvailableResolution(
        const SpriteDirectionDefinition& direction,
        uint32_t preferredResolution) const noexcept;
    const std::string* fileFor(const SpriteDirectionDefinition& direction,
        uint32_t preferredResolution) const noexcept;

private:
    std::string m_name;
    std::string m_format;
    Color m_transparentColor = makeColor(0, 0, 0);
    std::vector<uint32_t> m_supportedResolutions;
    uint32_t m_defaultResolution = 0;
    uint32_t m_maxResolution = 0;
    std::vector<SpriteDirectionDefinition> m_directions;
    std::vector<SpriteAnimationDefinition> m_animations;
    std::vector<SpriteLodRule> m_lodRules;
};
