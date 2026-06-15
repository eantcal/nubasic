// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "ActorSystem.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace {
constexpr double kPi = 3.14159265358979323846;

struct PatrolDirection {
    double x = 0.0;
    double y = 0.0;
};

constexpr PatrolDirection kPatrolDirections[] = {
    { 1.0, 0.0 },
    { 0.0, 1.0 },
    { -1.0, 0.0 },
    { 0.0, -1.0 },
};

double averageCellSize(const WorldMap& map) noexcept
{
    return (static_cast<double>(map.getCellDx())
               + static_cast<double>(map.getCellDy()))
        * 0.5;
}

int normalizePatrolDirection(int direction) noexcept
{
    direction %= 4;
    return direction < 0 ? direction + 4 : direction;
}

int patrolDirectionFromFacing(double radians) noexcept
{
    while (radians < 0.0) {
        radians += kPi * 2.0;
    }

    while (radians >= kPi * 2.0) {
        radians -= kPi * 2.0;
    }

    return normalizePatrolDirection(
        static_cast<int>(std::floor((radians / (kPi * 0.5)) + 0.5)));
}

bool hasTransparentWallAtWorld(const WorldMap& map, double x, double y) noexcept
{
    if (x < 0.0 || y < 0.0 || x >= map.getMaxX() || y >= map.getMaxY()) {
        return true;
    }

    return MapCell::hasTransparentWall(map.cellAtWorld(x, y));
}

bool isInsidePatrolRadius(
    const SpriteActor& actor, double x, double y, double patrolRadius) noexcept
{
    if (patrolRadius <= 0.0 || !actor.hasHomePosition) {
        return true;
    }

    const auto dx = x - actor.homeX;
    const auto dy = y - actor.homeY;
    return std::sqrt(dx * dx + dy * dy) <= patrolRadius;
}

bool canOccupyPatrolPosition(const Sprite& sprite, const WorldMap& map,
    const SpriteActor& actor, double x, double y, double patrolRadius) noexcept
{
    const auto radius = std::max(0.0, sprite.collisionRadius);
    const std::pair<double, double> samples[] = {
        { x, y },
        { x - radius, y },
        { x + radius, y },
        { x, y - radius },
        { x, y + radius },
    };

    for (const auto& sample : samples) {
        if (!isInsidePatrolRadius(
                actor, sample.first, sample.second, patrolRadius)
            || map.isSolidAtWorld(sample.first, sample.second)
            || hasTransparentWallAtWorld(map, sample.first, sample.second)) {
            return false;
        }
    }

    return true;
}

void setSpriteFacingFromPatrolDirection(Sprite& sprite, int direction) noexcept
{
    const auto& vector = kPatrolDirections[normalizePatrolDirection(direction)];
    sprite.facingRadians = std::atan2(vector.y, vector.x);
    if (sprite.facingRadians < 0.0) {
        sprite.facingRadians += kPi * 2.0;
    }
}

void advanceDeadActor(
    SpriteActor& actor, Sprite& sprite, double deltaSeconds) noexcept
{
    actor.dead = true;
    actor.state = ActorState::Idle;

    if (!actor.deathAnimationStarted) {
        if (!sprite.setAnimation("death")) {
            sprite.visible = false;
            return;
        }

        actor.deathAnimationStarted = true;
    }

    sprite.advanceAnimation(deltaSeconds);
}
} // namespace

void ActorSystem::update(RaycastEngine& engine, const WorldMap& map,
    std::vector<SpriteActor>& actors, double deltaSeconds) const noexcept
{
    if (deltaSeconds <= 0.0) {
        return;
    }

    for (auto& actor : actors) {
        actor.attackCooldownRemaining
            = std::max(0.0, actor.attackCooldownRemaining - deltaSeconds);
        actor.attackHoldSecondsRemaining
            = std::max(0.0, actor.attackHoldSecondsRemaining - deltaSeconds);
        actor.noiseAlertSecondsRemaining
            = std::max(0.0, actor.noiseAlertSecondsRemaining - deltaSeconds);
        updateChasingActor(engine, map, actor, deltaSeconds);
    }
}

void ActorSystem::updateChasingActor(RaycastEngine& engine, const WorldMap& map,
    SpriteActor& actor, double deltaSeconds) const noexcept
{
    auto* sprite = engine.sprite(actor.spriteIndex);
    if (sprite == nullptr || !sprite->visible) {
        actor.state = ActorState::Idle;
        return;
    }

    if (actor.dead || (actor.maxHealth > 0.0 && actor.health <= 0.0)) {
        advanceDeadActor(actor, *sprite, deltaSeconds);
        return;
    }

    if (!actor.hasHomePosition) {
        actor.homeX = sprite->x;
        actor.homeY = sprite->y;
        actor.hasHomePosition = true;
        actor.patrolDirection
            = patrolDirectionFromFacing(sprite->facingRadians);
    }

    const auto playerX = static_cast<double>(engine.player().getX());
    const auto playerY = static_cast<double>(engine.player().getY());
    const auto cellSize = averageCellSize(map);
    const auto detectionRadius
        = std::max(0.0, actor.detectionRadiusCells) * cellSize;
    const auto patrolRadius = std::max(0.0, actor.patrolRadiusCells) * cellSize;
    const auto hysteresis
        = std::max(0.0, actor.engagementHysteresisCells) * cellSize;
    const auto stoppingDistance
        = std::max(0.0, actor.stoppingDistanceCells) * cellSize;
    const auto dx = playerX - sprite->x;
    const auto dy = playerY - sprite->y;
    const auto distance = std::sqrt(dx * dx + dy * dy);
    const auto noiseAlertRadius
        = std::max(0.0, actor.noiseAlertRadiusCells) * cellSize;
    const auto noiseAlertActive = actor.noiseAlertSecondsRemaining > 0.0
        && (noiseAlertRadius <= 0.0 || distance <= noiseAlertRadius);

    const auto playerHomeDx = playerX - actor.homeX;
    const auto playerHomeDy = playerY - actor.homeY;
    const auto playerHomeDistance
        = std::sqrt(playerHomeDx * playerHomeDx + playerHomeDy * playerHomeDy);
    const auto spriteHomeDx = sprite->x - actor.homeX;
    const auto spriteHomeDy = sprite->y - actor.homeY;
    const auto spriteHomeDistance
        = std::sqrt(spriteHomeDx * spriteHomeDx + spriteHomeDy * spriteHomeDy);

    const auto actionEnterRadius
        = patrolRadius > 0.0 ? patrolRadius : detectionRadius;
    const auto actionExitRadius = actionEnterRadius + hysteresis;
    const auto playerCanStartChase
        = actionEnterRadius <= 0.0 || playerHomeDistance <= actionEnterRadius;
    const auto playerCanKeepChase
        = actionExitRadius <= 0.0 || playerHomeDistance <= actionExitRadius;
    const auto actorOutsidePatrolRadius
        = patrolRadius > 0.0 && spriteHomeDistance > patrolRadius + hysteresis;

    if (actorOutsidePatrolRadius && !noiseAlertActive) {
        actor.state = ActorState::Returning;
        if (!moveActorToward(engine, map, actor, actor.homeX, actor.homeY,
                cellSize * 0.05, deltaSeconds)) {
            actor.state = ActorState::Idle;
            sprite->setAnimationOrFallback("idle", "");
            sprite->advanceAnimation(deltaSeconds);
        }
        return;
    }

    if (!actor.chasePlayer) {
        if (actor.patrolCircuit
            && updatePatrolActor(engine, map, actor, deltaSeconds)) {
            return;
        }

        actor.state = ActorState::Idle;
        sprite->setAnimationOrFallback("idle", "");
        sprite->advanceAnimation(deltaSeconds);
        return;
    }

    if (distance <= 0.000001) {
        actor.state = ActorState::Idle;
        sprite->setAnimationOrFallback("idle", "");
        sprite->advanceAnimation(deltaSeconds);
        return;
    }

    if (actor.rangedAttack
        && (actor.attackBurstShotsRemaining > 0
            || actor.attackHoldSecondsRemaining > 0.0)) {
        actor.state = ActorState::Attacking;
        sprite->facingRadians = std::atan2(dy, dx);
        if (sprite->facingRadians < 0.0) {
            sprite->facingRadians += kPi * 2.0;
        }

        sprite->setAnimationOrFallback("attack", "idle");
        sprite->advanceAnimation(deltaSeconds);
        return;
    }

    const auto engageRadius = detectionRadius;
    const auto disengageRadius = engageRadius + hysteresis;
    const auto shouldStartChase = noiseAlertActive
        || (playerCanStartChase
            && (engageRadius <= 0.0 || distance <= engageRadius));
    const auto shouldKeepNoiseChase = noiseAlertActive && playerCanKeepChase;
    const auto shouldKeepChase = shouldKeepNoiseChase
        || ((actor.state == ActorState::Chasing
                || actor.state == ActorState::Attacking)
            && playerCanKeepChase
            && (engageRadius <= 0.0 || distance <= disengageRadius));

    if (!shouldStartChase && !shouldKeepChase) {
        if (actor.patrolCircuit
            && updatePatrolActor(engine, map, actor, deltaSeconds)) {
            return;
        }

        actor.state = ActorState::Idle;
        sprite->setAnimationOrFallback("idle", "");
        sprite->advanceAnimation(deltaSeconds);
        return;
    }

    if (stoppingDistance > 0.0 && distance <= stoppingDistance) {
        actor.state = ActorState::Idle;
        sprite->facingRadians = std::atan2(dy, dx);
        if (sprite->facingRadians < 0.0) {
            sprite->facingRadians += kPi * 2.0;
        }
        sprite->setAnimationOrFallback("attack", "idle");
        sprite->advanceAnimation(deltaSeconds);
        return;
    }

    actor.state = ActorState::Chasing;
    if (!moveActorToward(engine, map, actor, playerX, playerY, stoppingDistance,
            deltaSeconds)) {
        actor.state = ActorState::Idle;
        sprite->setAnimationOrFallback("idle", "");
    }
}

bool ActorSystem::updatePatrolActor(RaycastEngine& engine, const WorldMap& map,
    SpriteActor& actor, double deltaSeconds) const noexcept
{
    auto* sprite = engine.sprite(actor.spriteIndex);
    if (sprite == nullptr || !sprite->visible) {
        return false;
    }

    const auto cellSize = averageCellSize(map);
    const auto patrolRadius = std::max(0.0, actor.patrolRadiusCells) * cellSize;
    const auto step
        = std::max(0.0, actor.speedCellsPerSecond) * cellSize * deltaSeconds;
    if (step <= 0.0) {
        return false;
    }

    const auto lookAhead
        = step + std::max(sprite->collisionRadius, cellSize * 0.28);
    const auto currentDirection
        = normalizePatrolDirection(actor.patrolDirection);
    const int candidateDirections[] = {
        currentDirection,
        normalizePatrolDirection(currentDirection + 1),
        normalizePatrolDirection(currentDirection - 1),
        normalizePatrolDirection(currentDirection + 2),
    };

    for (const auto direction : candidateDirections) {
        const auto& vector = kPatrolDirections[direction];
        const auto probeX = sprite->x + vector.x * lookAhead;
        const auto probeY = sprite->y + vector.y * lookAhead;
        if (!canOccupyPatrolPosition(
                *sprite, map, actor, probeX, probeY, patrolRadius)) {
            continue;
        }

        const auto targetX = sprite->x + vector.x * step;
        const auto targetY = sprite->y + vector.y * step;
        if (!canOccupyPatrolPosition(
                *sprite, map, actor, targetX, targetY, patrolRadius)) {
            continue;
        }

        if (!engine.moveSprite(actor.spriteIndex, vector.x * step,
                vector.y * step, map,
                RaycastEngine::SpriteCollisionMode::BlockSolidWalls)) {
            continue;
        }

        actor.patrolDirection = direction;
        actor.state = ActorState::Patrolling;
        setSpriteFacingFromPatrolDirection(*sprite, direction);
        sprite->setAnimationOrFallback("walk", "idle");
        sprite->advanceAnimation(deltaSeconds);
        return true;
    }

    actor.state = ActorState::Idle;
    sprite->setAnimationOrFallback("idle", "");
    sprite->advanceAnimation(deltaSeconds);
    return false;
}

bool ActorSystem::moveActorToward(RaycastEngine& engine, const WorldMap& map,
    SpriteActor& actor, double targetX, double targetY, double stoppingDistance,
    double deltaSeconds) const noexcept
{
    auto* sprite = engine.sprite(actor.spriteIndex);
    if (sprite == nullptr || !sprite->visible) {
        return false;
    }

    const auto dx = targetX - sprite->x;
    const auto dy = targetY - sprite->y;
    const auto distance = std::sqrt(dx * dx + dy * dy);
    if (distance <= std::max(0.000001, stoppingDistance)) {
        return false;
    }

    sprite->facingRadians = std::atan2(dy, dx);
    if (sprite->facingRadians < 0.0) {
        sprite->facingRadians += kPi * 2.0;
    }

    const auto cellSize = averageCellSize(map);
    const auto maxStep
        = std::max(0.0, actor.speedCellsPerSecond) * cellSize * deltaSeconds;
    if (maxStep <= 0.0) {
        return false;
    }

    const auto step = std::min(maxStep, distance - stoppingDistance);
    const auto moveX = dx / distance * step;
    const auto moveY = dy / distance * step;
    const auto collisionMode = actor.collidesWithWorld
        ? RaycastEngine::SpriteCollisionMode::BlockSolidWalls
        : RaycastEngine::SpriteCollisionMode::PassThroughWalls;

    sprite->setAnimationOrFallback("walk", "idle");
    sprite->advanceAnimation(deltaSeconds);
    if (engine.moveSprite(
            actor.spriteIndex, moveX, moveY, map, collisionMode)) {
        return true;
    }

    auto moved = false;
    if (moveX != 0.0) {
        moved = engine.moveSprite(
                    actor.spriteIndex, moveX, 0.0, map, collisionMode)
            || moved;
    }

    if (moveY != 0.0) {
        moved = engine.moveSprite(
                    actor.spriteIndex, 0.0, moveY, map, collisionMode)
            || moved;
    }

    if (!moved) {
        sprite->setAnimationOrFallback("idle", "");
    }

    return moved;
}
