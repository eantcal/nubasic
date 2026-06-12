// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include <string>
#include <vector>

class SceneLoader {
public:
    struct SpriteInstance {
        std::string name;
        std::string spriteSet;
        double xCell = 0.0;
        double yCell = 0.0;
        double facingDegrees = 0.0;
        double scaleCells = 1.0;
        double collisionRadiusCells = 0.0;
        bool visible = true;
        bool passThroughWalls = false;
        bool chasePlayer = false;
        double speedCellsPerSecond = 0.0;
        double detectionRadiusCells = 0.0;
        double patrolRadiusCells = 0.0;
        double engagementHysteresisCells = 0.5;
        bool patrolCircuit = false;
        double stoppingDistanceCells = 0.0;
        double maxHealth = 0.0;
        double health = 0.0;
        double attackDamage = 0.0;
        bool rangedAttack = false;
        double attackRangeCells = 0.0;
        double attackCooldownSeconds = 1.0;
        double attackFovDegrees = 70.0;
        int attackBurstShots = 3;
        double attackBurstPauseSeconds = 1.2;
        double pickupHealth = 0.0;
        bool unlocksMap = false;
        std::string pickupWeapon;
        bool explosive = false;
        double explosiveHitPoints = 45.0;
        double explosionRadiusCells = 0.0;
        double explosionDamage = 0.0;
        double explosionScaleCells = 1.5;
        std::string explosionSpriteSet;
        std::string destroyedSpriteSet;
        double destroyedScaleCells = 0.55;
        std::string damageResponseType;
        double damageResponseHitPoints = 0.0;
        std::string damageResponseEffectSpriteSet;
        std::string damageResponseEffectAnimation;
        double damageResponseEffectScaleCells = 1.5;
        std::string damageResponseDestroyedSpriteSet;
        double damageResponseDestroyedScaleCells = 0.55;
        std::string damageResponseSound;
        double damageResponseRadiusCells = 0.0;
        double damageResponseDamage = 0.0;
    };

    struct PlayerStart {
        double xCell = 1.5;
        double yCell = 1.5;
        double facingDegrees = 0.0;
    };

    struct CombatStats {
        double maxHealth = 100.0;
        double health = 100.0;
    };

    struct PlayerWeapon {
        std::string file;
        bool visible = true;
        bool unlocked = true;
        double screenHeightFraction = 0.0;
    };

    struct BackgroundMusic {
        std::string file;
        bool enabled = true;
        bool loop = true;
        int volumePercent = 80;
    };

    struct Scene {
        std::string projectName;
        std::string worldFile;
        std::string activeLayerId;
        std::string textureRoot = ".";
        bool hasPlayerStart = false;
        PlayerStart playerStart;
        CombatStats playerStats;
        PlayerWeapon playerWeapon;
        std::vector<PlayerWeapon> playerWeapons;
        BackgroundMusic backgroundMusic;
        std::vector<std::string> spriteSets;
        std::vector<SpriteInstance> spriteInstances;
    };

    struct Result {
        bool success = false;
        Scene scene;
        std::vector<std::string> errors;
    };

    Result loadFromFile(const std::string& projectPath,
        const std::string& layerId = std::string()) const;
};
