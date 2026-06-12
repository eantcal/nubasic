// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include "RaycastEngine.h"
#include "WorldMap.h"

#include <cstddef>
#include <string>
#include <vector>

enum class ActorState { Idle, Patrolling, Chasing, Returning, Attacking };

struct SpriteActor {
    size_t spriteIndex = 0;
    std::string persistenceKey;
    ActorState state = ActorState::Idle;
    double homeX = 0.0;
    double homeY = 0.0;
    bool hasHomePosition = false;
    double speedCellsPerSecond = 0.0;
    double detectionRadiusCells = 0.0;
    double patrolRadiusCells = 0.0;
    double engagementHysteresisCells = 0.5;
    bool patrolCircuit = false;
    bool chasePlayer = true;
    int patrolDirection = 0;
    double stoppingDistanceCells = 0.0;
    bool collidesWithWorld = true;
    double maxHealth = 0.0;
    double health = 0.0;
    double attackDamage = 0.0;
    bool rangedAttack = false;
    double attackRangeCells = 0.0;
    double attackCooldownSeconds = 1.0;
    double attackFovDegrees = 70.0;
    double attackCooldownRemaining = 0.0;
    int attackBurstShots = 3;
    int attackBurstShotsRemaining = 0;
    double attackBurstPauseSeconds = 1.2;
    double attackHoldSecondsRemaining = 0.0;
    double noiseAlertSecondsRemaining = 0.0;
    double noiseAlertRadiusCells = 0.0;
    bool dead = false;
    bool deathAnimationStarted = false;
};

class ActorSystem {
public:
    void update(RaycastEngine& engine, const WorldMap& map,
        std::vector<SpriteActor>& actors, double deltaSeconds) const noexcept;

private:
    void updateChasingActor(RaycastEngine& engine, const WorldMap& map,
        SpriteActor& actor, double deltaSeconds) const noexcept;

    bool updatePatrolActor(RaycastEngine& engine, const WorldMap& map,
        SpriteActor& actor, double deltaSeconds) const noexcept;

    bool moveActorToward(RaycastEngine& engine, const WorldMap& map,
        SpriteActor& actor, double targetX, double targetY,
        double stoppingDistance, double deltaSeconds) const noexcept;
};
