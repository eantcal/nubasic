// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "SceneLoader.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>

namespace {
double readDouble(const nlohmann::json& node, const char* name, double fallback)
{
    if (!node.contains(name) || !node[name].is_number()) {
        return fallback;
    }

    return node[name].get<double>();
}

bool readBool(const nlohmann::json& node, const char* name, bool fallback)
{
    if (!node.contains(name) || !node[name].is_boolean()) {
        return fallback;
    }

    return node[name].get<bool>();
}

std::string readString(
    const nlohmann::json& node, const char* name, const std::string& fallback)
{
    if (!node.contains(name) || !node[name].is_string()) {
        return fallback;
    }

    return node[name].get<std::string>();
}

int clampPercent(double value) noexcept
{
    if (value < 0.0) {
        return 0;
    }

    if (value > 100.0) {
        return 100;
    }

    return static_cast<int>(value);
}

bool endsWithIgnoreCase(const std::string& value, const std::string& suffix)
{
    if (suffix.size() > value.size()) {
        return false;
    }

    return std::equal(
        suffix.rbegin(), suffix.rend(), value.rbegin(), [](char lhs, char rhs) {
            return std::tolower(static_cast<unsigned char>(lhs))
                == std::tolower(static_cast<unsigned char>(rhs));
        });
}

const nlohmann::json* selectLayer(const nlohmann::json& document,
    const std::string& requestedLayerId, std::string& activeLayerId,
    std::vector<std::string>& errors)
{
    activeLayerId.clear();
    if (!document.contains("layers")) {
        return nullptr;
    }

    if (!document["layers"].is_array()) {
        errors.push_back("World layers must be an array.");
        return nullptr;
    }

    const auto& layers = document["layers"];
    if (layers.empty()) {
        errors.push_back("World layers array must not be empty.");
        return nullptr;
    }

    auto selectedLayerId = requestedLayerId;
    if (selectedLayerId.empty()) {
        selectedLayerId = readString(document, "activeLayer", std::string());
    }

    if (selectedLayerId.empty()) {
        selectedLayerId = readString(document, "startLayer", std::string());
    }

    const nlohmann::json* fallbackLayer = nullptr;
    std::string fallbackLayerId;
    for (const auto& layer : layers) {
        if (!layer.is_object()) {
            errors.push_back("Each world layer must be an object.");
            continue;
        }

        const auto id = readString(layer, "id", std::string());
        if (fallbackLayer == nullptr) {
            fallbackLayer = &layer;
            fallbackLayerId = id;
        }

        if (!selectedLayerId.empty() && id == selectedLayerId) {
            activeLayerId = id;
            return &layer;
        }
    }

    if (!selectedLayerId.empty()) {
        errors.push_back(
            "World does not contain layer '" + selectedLayerId + "'.");
        return nullptr;
    }

    activeLayerId = fallbackLayerId;
    return fallbackLayer;
}

void readSpriteInstances(const nlohmann::json& node,
    std::vector<SceneLoader::SpriteInstance>& spriteInstances,
    std::vector<std::string>& errors)
{
    if (!node.contains("spriteInstances")) {
        return;
    }

    if (!node["spriteInstances"].is_array()) {
        errors.push_back("spriteInstances must be an array.");
        return;
    }

    for (const auto& entry : node["spriteInstances"]) {
        if (!entry.is_object()) {
            errors.push_back("Each sprite instance must be a JSON object.");
            continue;
        }

        SceneLoader::SpriteInstance sprite;
        sprite.name = readString(entry, "name", std::string());
        sprite.spriteSet = readString(entry, "spriteSet", std::string());
        sprite.xCell = readDouble(entry, "xCell", 0.0);
        sprite.yCell = readDouble(entry, "yCell", 0.0);
        sprite.facingDegrees = readDouble(entry, "facingDegrees", 0.0);
        sprite.scaleCells = readDouble(entry, "scaleCells", 1.0);
        sprite.collisionRadiusCells
            = readDouble(entry, "collisionRadiusCells", 0.0);
        sprite.visible = readBool(entry, "visible", true);
        sprite.passThroughWalls = readBool(entry, "passThroughWalls", false);
        sprite.chasePlayer = readBool(entry, "chasePlayer", false);
        sprite.speedCellsPerSecond
            = readDouble(entry, "speedCellsPerSecond", 0.0);
        sprite.detectionRadiusCells
            = readDouble(entry, "detectionRadiusCells", 0.0);
        sprite.patrolRadiusCells = readDouble(entry, "patrolRadiusCells", 0.0);
        sprite.engagementHysteresisCells
            = readDouble(entry, "engagementHysteresisCells", 0.5);
        sprite.patrolCircuit = readBool(entry, "patrolCircuit", false);
        sprite.stoppingDistanceCells
            = readDouble(entry, "stoppingDistanceCells", 0.0);
        sprite.maxHealth = readDouble(entry, "maxHealth", 0.0);
        sprite.health = readDouble(entry, "health", sprite.maxHealth);
        sprite.attackDamage = readDouble(entry, "attackDamage", 0.0);
        sprite.rangedAttack = readBool(entry, "rangedAttack", false);
        sprite.attackRangeCells = readDouble(entry, "attackRangeCells", 0.0);
        sprite.attackCooldownSeconds
            = readDouble(entry, "attackCooldownSeconds", 1.0);
        sprite.attackFovDegrees = readDouble(entry, "attackFovDegrees", 70.0);
        sprite.attackBurstShots = static_cast<int>(
            std::max(1.0, readDouble(entry, "attackBurstShots", 3.0)));
        sprite.attackBurstPauseSeconds
            = readDouble(entry, "attackBurstPauseSeconds", 1.2);
        sprite.pickupHealth = readDouble(entry, "pickupHealth", 0.0);
        sprite.unlocksMap = readBool(entry, "unlocksMap", false);
        sprite.pickupWeapon = readString(entry, "pickupWeapon",
            readString(entry, "pickupWeaponFile", std::string()));
        sprite.explosive = readBool(entry, "explosive", false);
        sprite.explosiveHitPoints
            = readDouble(entry, "explosiveHitPoints", 45.0);
        sprite.explosionRadiusCells
            = readDouble(entry, "explosionRadiusCells", 0.0);
        sprite.explosionDamage = readDouble(entry, "explosionDamage", 0.0);
        sprite.explosionScaleCells
            = readDouble(entry, "explosionScaleCells", 1.5);
        sprite.explosionSpriteSet
            = readString(entry, "explosionSpriteSet", std::string());
        sprite.destroyedSpriteSet
            = readString(entry, "destroyedSpriteSet", std::string());
        sprite.destroyedScaleCells
            = readDouble(entry, "destroyedScaleCells", 0.55);
        if (entry.contains("damageResponse")) {
            const auto& response = entry["damageResponse"];
            if (!response.is_object()) {
                errors.push_back(
                    "spriteInstances damageResponse must be an object.");
            } else {
                sprite.damageResponseType
                    = readString(response, "type", std::string());
                sprite.damageResponseHitPoints = readDouble(
                    response, "hitPoints", sprite.explosiveHitPoints);
                sprite.damageResponseEffectSpriteSet = readString(response,
                    "effectSpriteSet",
                    readString(response, "explosionSpriteSet", std::string()));
                sprite.damageResponseEffectAnimation
                    = readString(response, "effectAnimation", std::string());
                sprite.damageResponseEffectScaleCells
                    = readDouble(response, "effectScaleCells",
                        readDouble(response, "explosionScaleCells", 1.5));
                sprite.damageResponseDestroyedSpriteSet
                    = readString(response, "destroyedSpriteSet", std::string());
                sprite.damageResponseDestroyedScaleCells
                    = readDouble(response, "destroyedScaleCells", 0.55);
                sprite.damageResponseSound
                    = readString(response, "sound", std::string());
                sprite.damageResponseRadiusCells
                    = readDouble(response, "radiusCells",
                        readDouble(response, "explosionRadiusCells", 0.0));
                sprite.damageResponseDamage = readDouble(response, "damage",
                    readDouble(response, "explosionDamage", 0.0));
            }
        }
        spriteInstances.push_back(std::move(sprite));
    }
}

SceneLoader::BackgroundMusic readBackgroundMusic(const nlohmann::json& node,
    const SceneLoader::BackgroundMusic& fallback,
    std::vector<std::string>& errors)
{
    auto music = fallback;
    if (!node.contains("backgroundMusic")) {
        return music;
    }

    const auto& value = node["backgroundMusic"];
    if (value.is_string()) {
        music.file = value.get<std::string>();
        music.enabled = !music.file.empty();
        music.loop = true;
        return music;
    }

    if (!value.is_object()) {
        errors.push_back("backgroundMusic must be a string or object.");
        return music;
    }

    music.file = readString(value, "file", music.file);
    music.enabled = readBool(value, "enabled", music.enabled);
    music.loop = readBool(value, "loop", music.loop);
    music.volumePercent = clampPercent(readDouble(value, "volumePercent",
        readDouble(value, "volume", music.volumePercent)));
    return music;
}

bool readPlayerWeapon(const nlohmann::json& value,
    SceneLoader::PlayerWeapon& playerWeapon, std::vector<std::string>& errors,
    const std::string& fieldName)
{
    if (value.is_string()) {
        playerWeapon.file = value.get<std::string>();
        return true;
    }

    if (value.is_object()) {
        playerWeapon.file = readString(value, "file", std::string());
        playerWeapon.visible = readBool(value, "visible", true);
        playerWeapon.unlocked = readBool(value, "unlocked", true);
        playerWeapon.screenHeightFraction
            = readDouble(value, "screenHeightFraction", 0.0);
        return true;
    }

    errors.push_back(fieldName + " must be a string or object.");
    return false;
}

void appendPlayerWeaponIfUsable(
    std::vector<SceneLoader::PlayerWeapon>& playerWeapons,
    const SceneLoader::PlayerWeapon& playerWeapon)
{
    if (playerWeapon.file.empty()) {
        return;
    }

    const auto exists = std::find_if(playerWeapons.begin(), playerWeapons.end(),
        [&playerWeapon](const SceneLoader::PlayerWeapon& item) {
            return item.file == playerWeapon.file;
        });

    if (exists == playerWeapons.end()) {
        playerWeapons.push_back(playerWeapon);
    }
}
} // namespace

SceneLoader::Result SceneLoader::loadFromFile(
    const std::string& projectPath, const std::string& layerId) const
{
    Result result;

    std::ifstream input(projectPath);
    if (!input.is_open()) {
        result.errors.push_back("Cannot open project file: " + projectPath);
        return result;
    }

    nlohmann::json document;
    try {
        input >> document;
    } catch (const std::exception& error) {
        result.errors.push_back(std::string("Invalid JSON: ") + error.what());
        return result;
    }

    if (!document.is_object()) {
        result.errors.push_back("Project root must be a JSON object.");
        return result;
    }

    result.scene.projectName = readString(document, "project", std::string());
    result.scene.worldFile = readString(document, "worldFile", std::string());
    if (!result.scene.worldFile.empty()
        && !endsWithIgnoreCase(result.scene.worldFile, ".json")) {
        result.errors.push_back(
            "Project worldFile must point to a JSON world file; legacy INI "
            "maps are no longer supported.");
    }

    result.scene.textureRoot = readString(document, "textureRoot", ".");

    std::string activeLayerId;
    const auto* layer
        = selectLayer(document, layerId, activeLayerId, result.errors);
    if (layer != nullptr) {
        result.scene.activeLayerId = activeLayerId;
    }

    if (document.contains("playerWeapon")) {
        readPlayerWeapon(document["playerWeapon"], result.scene.playerWeapon,
            result.errors, "playerWeapon");
    }

    if (document.contains("playerWeapons")) {
        if (!document["playerWeapons"].is_array()) {
            result.errors.push_back("playerWeapons must be an array.");
        } else {
            for (const auto& entry : document["playerWeapons"]) {
                SceneLoader::PlayerWeapon playerWeapon;
                if (readPlayerWeapon(entry, playerWeapon, result.errors,
                        "playerWeapons entry")) {
                    appendPlayerWeaponIfUsable(
                        result.scene.playerWeapons, playerWeapon);
                }
            }
        }
    }

    if (result.scene.playerWeapons.empty()) {
        appendPlayerWeaponIfUsable(
            result.scene.playerWeapons, result.scene.playerWeapon);
    } else if (result.scene.playerWeapon.file.empty()) {
        result.scene.playerWeapon = result.scene.playerWeapons.front();
    } else {
        const auto activeWeapon = result.scene.playerWeapon;
        const auto existingActive
            = std::find_if(result.scene.playerWeapons.begin(),
                result.scene.playerWeapons.end(),
                [&activeWeapon](const SceneLoader::PlayerWeapon& item) {
                    return item.file == activeWeapon.file;
                });
        if (existingActive == result.scene.playerWeapons.end()) {
            result.scene.playerWeapons.insert(
                result.scene.playerWeapons.begin(), activeWeapon);
        }
    }

    result.scene.backgroundMusic = readBackgroundMusic(
        document, result.scene.backgroundMusic, result.errors);
    if (layer != nullptr) {
        result.scene.backgroundMusic = readBackgroundMusic(
            *layer, result.scene.backgroundMusic, result.errors);
    }

    const auto* playerStartNode = &document;
    if (layer != nullptr && layer->contains("playerStart")
        && (*layer)["playerStart"].is_object()) {
        playerStartNode = layer;
    }

    if (playerStartNode->contains("playerStart")
        && (*playerStartNode)["playerStart"].is_object()) {
        const auto& playerStart = (*playerStartNode)["playerStart"];
        result.scene.playerStart.xCell = readDouble(playerStart, "xCell", 1.5);
        result.scene.playerStart.yCell = readDouble(playerStart, "yCell", 1.5);
        result.scene.playerStart.facingDegrees
            = readDouble(playerStart, "facingDegrees", 0.0);
        result.scene.hasPlayerStart = true;
    }

    if (document.contains("playerStats")
        && document["playerStats"].is_object()) {
        const auto& playerStats = document["playerStats"];
        result.scene.playerStats.maxHealth = readDouble(
            playerStats, "maxHealth", result.scene.playerStats.maxHealth);
        result.scene.playerStats.health = readDouble(
            playerStats, "health", result.scene.playerStats.maxHealth);
    }

    if (document.contains("spriteSets") && document["spriteSets"].is_array()) {
        for (const auto& entry : document["spriteSets"]) {
            if (entry.is_string()) {
                auto value = entry.get<std::string>();
                if (!value.empty()) {
                    result.scene.spriteSets.push_back(std::move(value));
                }
            }
        }
    }

    readSpriteInstances(document, result.scene.spriteInstances, result.errors);
    if (layer != nullptr) {
        readSpriteInstances(
            *layer, result.scene.spriteInstances, result.errors);
    }

    result.success = result.errors.empty();
    return result;
}
