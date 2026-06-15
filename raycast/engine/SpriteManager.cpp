// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "SpriteManager.h"

#include <utility>

bool SpriteManager::addSpriteSet(SpriteSet spriteSet)
{
    const auto name = spriteSet.name();
    if (name.empty()) {
        return false;
    }

    return m_spriteSets.emplace(name, std::move(spriteSet)).second;
}

const SpriteSet* SpriteManager::spriteSet(
    const std::string& name) const noexcept
{
    const auto item = m_spriteSets.find(name);
    return item == m_spriteSets.end() ? nullptr : &item->second;
}

SpriteManager::Selection SpriteManager::selectFrame(
    const std::string& spriteSetName, double relativeAngleDegrees,
    double distanceCells) const noexcept
{
    Selection selection;

    selection.spriteSet = spriteSet(spriteSetName);
    if (!selection.spriteSet) {
        return selection;
    }

    selection.direction
        = selection.spriteSet->directionForAngle(relativeAngleDegrees);
    if (!selection.direction) {
        return selection;
    }

    const auto preferredResolution
        = selection.spriteSet->resolutionForDistance(distanceCells);
    selection.resolution = selection.spriteSet->closestAvailableResolution(
        *selection.direction, preferredResolution);
    selection.filePath = selection.spriteSet->fileFor(
        *selection.direction, preferredResolution);

    return selection;
}
