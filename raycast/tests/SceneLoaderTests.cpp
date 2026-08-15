#include "SceneLoader.h"

#include <gtest/gtest.h>

#include <direct.h>

#include <fstream>
#include <string>

namespace {
const char* kFixtureDir = "scene_loader_fixtures";

std::string fixturePath(const std::string& name)
{
    return std::string(kFixtureDir) + "/" + name;
}

void writeFile(const std::string& path, const std::string& content)
{
    std::ofstream file(path, std::ios::binary);
    file << content;
}

std::string validProjectJson()
{
    return R"json(
        {
          "project": "demo_world",
          "worldFile": "world.world.json",
          "textureRoot": ".",
          "playerWeapon": {
            "file": "weapons/super_shotgun/super_shotgun.weapon.json",
            "visible": true,
            "screenHeightFraction": 0.34
          },
          "backgroundMusic": {
            "file": "audio/holst_mars_15s_no_fadein.ogg",
            "enabled": true,
            "loop": true,
            "volumePercent": 65
          },
          "playerStart": {
            "xCell": 2.5,
            "yCell": 3.5,
            "facingDegrees": 45.0
          },
          "playerStats": {
            "maxHealth": 125.0,
            "health": 115.0
          },
          "spriteSets": [
            "sprites/monster.sprite.json"
          ],
          "spriteInstances": [
            {
              "name": "guard_01",
              "spriteSet": "monster",
              "xCell": 5.5,
              "yCell": 4.5,
              "facingDegrees": 90.0,
              "scaleCells": 1.25,
              "collisionRadiusCells": 0.3,
              "visible": true,
              "passThroughWalls": false,
              "chasePlayer": true,
              "speedCellsPerSecond": 0.75,
              "detectionRadiusCells": 8.0,
              "patrolRadiusCells": 5.0,
              "engagementHysteresisCells": 0.75,
              "patrolCircuit": true,
              "stoppingDistanceCells": 0.65,
              "maxHealth": 80.0,
              "health": 65.0,
              "attackDamage": 12.0,
              "rangedAttack": true,
              "attackRangeCells": 6.0,
              "attackCooldownSeconds": 1.25,
              "attackFovDegrees": 80.0,
              "attackBurstShots": 4,
              "attackBurstPauseSeconds": 1.75,
              "damageResponse": {
                "type": "break",
                "hitPoints": 18.0,
                "effectSpriteSet": "vase_break_256",
                "effectAnimation": "break",
                "effectScaleCells": 0.72,
                "destroyedSpriteSet": "broken_vase_256",
                "destroyedScaleCells": 0.58,
                "sound": "effects/breaking/can_crush_0.wav",
                "radiusCells": 0.0,
                "damage": 0.0
              }
            },
            {
              "name": "ghost",
              "spriteSet": "monster",
              "xCell": 1.5,
              "yCell": 2.5,
              "passThroughWalls": true,
              "pickupHealth": 35.0,
              "unlocksMap": true,
              "savePoint": true,
              "pickupWeapon": "weapons/pistol/pistol.weapon.json"
            }
          ]
        }
    )json";
}
}

TEST(SceneLoader, LoadsValidProjectFile)
{
    _mkdir(kFixtureDir);
    const auto path = fixturePath("valid_project.json");
    writeFile(path, validProjectJson());

    SceneLoader loader;
    const auto result = loader.loadFromFile(path);

    ASSERT_TRUE(result.success) << (result.errors.empty() ? "" : result.errors.front());
    EXPECT_EQ(result.scene.projectName, "demo_world");
    EXPECT_EQ(result.scene.worldFile, "world.world.json");
    EXPECT_EQ(result.scene.textureRoot, ".");
    EXPECT_EQ(
        result.scene.playerWeapon.file,
        "weapons/super_shotgun/super_shotgun.weapon.json");
    EXPECT_TRUE(result.scene.playerWeapon.visible);
    EXPECT_DOUBLE_EQ(result.scene.playerWeapon.screenHeightFraction, 0.34);
    ASSERT_EQ(result.scene.playerWeapons.size(), 1u);
    EXPECT_EQ(
        result.scene.playerWeapons.front().file,
        "weapons/super_shotgun/super_shotgun.weapon.json");
    EXPECT_EQ(result.scene.backgroundMusic.file, "audio/holst_mars_15s_no_fadein.ogg");
    EXPECT_TRUE(result.scene.backgroundMusic.enabled);
    EXPECT_TRUE(result.scene.backgroundMusic.loop);
    EXPECT_EQ(result.scene.backgroundMusic.volumePercent, 65);
    EXPECT_TRUE(result.scene.hasPlayerStart);
    EXPECT_DOUBLE_EQ(result.scene.playerStart.xCell, 2.5);
    EXPECT_DOUBLE_EQ(result.scene.playerStart.yCell, 3.5);
    EXPECT_DOUBLE_EQ(result.scene.playerStart.facingDegrees, 45.0);
    EXPECT_DOUBLE_EQ(result.scene.playerStats.maxHealth, 125.0);
    EXPECT_DOUBLE_EQ(result.scene.playerStats.health, 115.0);
    ASSERT_EQ(result.scene.spriteSets.size(), 1u);
    EXPECT_EQ(result.scene.spriteSets.front(), "sprites/monster.sprite.json");
    ASSERT_EQ(result.scene.spriteInstances.size(), 2u);

    const auto& guard = result.scene.spriteInstances.front();
    EXPECT_EQ(guard.name, "guard_01");
    EXPECT_EQ(guard.spriteSet, "monster");
    EXPECT_DOUBLE_EQ(guard.xCell, 5.5);
    EXPECT_DOUBLE_EQ(guard.yCell, 4.5);
    EXPECT_DOUBLE_EQ(guard.facingDegrees, 90.0);
    EXPECT_DOUBLE_EQ(guard.scaleCells, 1.25);
    EXPECT_DOUBLE_EQ(guard.collisionRadiusCells, 0.3);
    EXPECT_TRUE(guard.visible);
    EXPECT_FALSE(guard.passThroughWalls);
    EXPECT_TRUE(guard.chasePlayer);
    EXPECT_DOUBLE_EQ(guard.speedCellsPerSecond, 0.75);
    EXPECT_DOUBLE_EQ(guard.detectionRadiusCells, 8.0);
    EXPECT_DOUBLE_EQ(guard.patrolRadiusCells, 5.0);
    EXPECT_DOUBLE_EQ(guard.engagementHysteresisCells, 0.75);
    EXPECT_TRUE(guard.patrolCircuit);
    EXPECT_DOUBLE_EQ(guard.stoppingDistanceCells, 0.65);
    EXPECT_DOUBLE_EQ(guard.maxHealth, 80.0);
    EXPECT_DOUBLE_EQ(guard.health, 65.0);
    EXPECT_DOUBLE_EQ(guard.attackDamage, 12.0);
    EXPECT_TRUE(guard.rangedAttack);
    EXPECT_DOUBLE_EQ(guard.attackRangeCells, 6.0);
    EXPECT_DOUBLE_EQ(guard.attackCooldownSeconds, 1.25);
    EXPECT_DOUBLE_EQ(guard.attackFovDegrees, 80.0);
    EXPECT_EQ(guard.attackBurstShots, 4);
    EXPECT_DOUBLE_EQ(guard.attackBurstPauseSeconds, 1.75);
    EXPECT_EQ(guard.damageResponseType, "break");
    EXPECT_DOUBLE_EQ(guard.damageResponseHitPoints, 18.0);
    EXPECT_EQ(guard.damageResponseEffectSpriteSet, "vase_break_256");
    EXPECT_EQ(guard.damageResponseEffectAnimation, "break");
    EXPECT_DOUBLE_EQ(guard.damageResponseEffectScaleCells, 0.72);
    EXPECT_EQ(guard.damageResponseDestroyedSpriteSet, "broken_vase_256");
    EXPECT_DOUBLE_EQ(guard.damageResponseDestroyedScaleCells, 0.58);
    EXPECT_EQ(guard.damageResponseSound, "effects/breaking/can_crush_0.wav");
    EXPECT_DOUBLE_EQ(guard.damageResponseRadiusCells, 0.0);
    EXPECT_DOUBLE_EQ(guard.damageResponseDamage, 0.0);

    const auto& ghost = result.scene.spriteInstances.back();
    EXPECT_EQ(ghost.name, "ghost");
    EXPECT_TRUE(ghost.passThroughWalls);
    EXPECT_DOUBLE_EQ(ghost.scaleCells, 1.0);
    EXPECT_TRUE(ghost.visible);
    EXPECT_DOUBLE_EQ(ghost.pickupHealth, 35.0);
    EXPECT_TRUE(ghost.unlocksMap);
    EXPECT_TRUE(ghost.savePoint);
    EXPECT_EQ(ghost.pickupWeapon, "weapons/pistol/pistol.weapon.json");
}

TEST(SceneLoader, LoadsPlayerWeaponInventory)
{
    _mkdir(kFixtureDir);
    const auto path = fixturePath("player_weapon_inventory.json");
    writeFile(path, R"json(
        {
          "project": "demo_world",
          "worldFile": "world.world.json",
          "playerWeapon": {
            "file": "weapons/pistol/pistol.weapon.json",
            "visible": true,
            "unlocked": false,
            "screenHeightFraction": 0.48
          },
          "playerWeapons": [
            {
              "file": "weapons/pistol/pistol.weapon.json",
              "visible": true,
              "unlocked": false,
              "screenHeightFraction": 0.48
            },
            {
              "file": "weapons/super_shotgun/super_shotgun.weapon.json",
              "visible": true,
              "unlocked": false,
              "screenHeightFraction": 0.34
            },
            {
              "file": "weapons/submachine_gun/submachine_gun.weapon.json",
              "visible": true,
              "unlocked": true,
              "screenHeightFraction": 0.31
            }
          ]
        }
    )json");

    SceneLoader loader;
    const auto result = loader.loadFromFile(path);

    ASSERT_TRUE(result.success) << (result.errors.empty() ? "" : result.errors.front());
    EXPECT_EQ(
        result.scene.playerWeapon.file,
        "weapons/pistol/pistol.weapon.json");
    ASSERT_EQ(result.scene.playerWeapons.size(), 3u);
    EXPECT_EQ(
        result.scene.playerWeapons[0].file,
        "weapons/pistol/pistol.weapon.json");
    EXPECT_DOUBLE_EQ(result.scene.playerWeapons[0].screenHeightFraction, 0.48);
    EXPECT_FALSE(result.scene.playerWeapons[0].unlocked);
    EXPECT_EQ(
        result.scene.playerWeapons[1].file,
        "weapons/super_shotgun/super_shotgun.weapon.json");
    EXPECT_FALSE(result.scene.playerWeapons[1].unlocked);
    EXPECT_EQ(
        result.scene.playerWeapons[2].file,
        "weapons/submachine_gun/submachine_gun.weapon.json");
    EXPECT_TRUE(result.scene.playerWeapons[2].unlocked);
}

TEST(SceneLoader, MergesSelectedLayerSceneData)
{
    _mkdir(kFixtureDir);
    const auto path = fixturePath("layered_world.json");
    writeFile(path, R"json(
        {
          "format": "winraycast.world",
          "version": 2,
          "project": "layered",
          "brightness": 0.95,
          "depthShading": 120.0,
          "playerStart": {
            "xCell": 1.5,
            "yCell": 1.5,
            "facingDegrees": 0.0
          },
          "spriteSets": [ "sprites/shared.sprite.json" ],
          "backgroundMusic": "audio/shared_theme.ogg",
          "spriteInstances": [
            { "name": "shared", "spriteSet": "shared", "xCell": 2.5, "yCell": 2.5 }
          ],
          "layers": [
            {
              "id": "level_1",
              "brightness": 0.7,
              "depthShading": 240.0,
              "backgroundMusic": {
                "file": "audio/level_1.ogg",
                "enabled": true,
                "loop": false,
                "volumePercent": 45
              },
              "playerStart": { "xCell": 3.5, "yCell": 4.5, "facingDegrees": 90.0 },
              "spriteInstances": [
                { "name": "level_guard", "spriteSet": "shared", "xCell": 5.5, "yCell": 6.5 }
              ]
            },
            {
              "id": "level_2",
              "playerStart": { "xCell": 7.5, "yCell": 8.5, "facingDegrees": 180.0 }
            }
          ]
        }
    )json");

    SceneLoader loader;
    const auto result = loader.loadFromFile(path, "level_1");

    ASSERT_TRUE(result.success) << (result.errors.empty() ? "" : result.errors.front());
    EXPECT_EQ("level_1", result.scene.activeLayerId);
    EXPECT_DOUBLE_EQ(0.7, result.scene.brightness);
    EXPECT_DOUBLE_EQ(240.0, result.scene.depthShading);
    ASSERT_TRUE(result.scene.hasPlayerStart);
    EXPECT_EQ("audio/level_1.ogg", result.scene.backgroundMusic.file);
    EXPECT_TRUE(result.scene.backgroundMusic.enabled);
    EXPECT_FALSE(result.scene.backgroundMusic.loop);
    EXPECT_EQ(45, result.scene.backgroundMusic.volumePercent);
    EXPECT_DOUBLE_EQ(3.5, result.scene.playerStart.xCell);
    EXPECT_DOUBLE_EQ(4.5, result.scene.playerStart.yCell);
    EXPECT_DOUBLE_EQ(90.0, result.scene.playerStart.facingDegrees);
    ASSERT_EQ(2u, result.scene.spriteInstances.size());
    EXPECT_EQ("shared", result.scene.spriteInstances[0].name);
    EXPECT_TRUE(result.scene.spriteInstances[0].layerId.empty());
    EXPECT_EQ("level_guard", result.scene.spriteInstances[1].name);
    EXPECT_EQ("level_1", result.scene.spriteInstances[1].layerId);
}

TEST(SceneLoader, ReportsMissingFile)
{
    SceneLoader loader;
    const auto result = loader.loadFromFile("nonexistent_project.json");

    EXPECT_FALSE(result.success);
    ASSERT_FALSE(result.errors.empty());
    EXPECT_NE(result.errors.front().find("Cannot open"), std::string::npos);
}

TEST(SceneLoader, ReportsInvalidJson)
{
    _mkdir(kFixtureDir);
    const auto path = fixturePath("broken_project.json");
    writeFile(path, "{ not valid json");

    SceneLoader loader;
    const auto result = loader.loadFromFile(path);

    EXPECT_FALSE(result.success);
    ASSERT_FALSE(result.errors.empty());
    EXPECT_NE(result.errors.front().find("Invalid JSON"), std::string::npos);
}

TEST(SceneLoader, ReportsLegacyWorldFile)
{
    _mkdir(kFixtureDir);
    const auto path = fixturePath("legacy_project.json");
    writeFile(path, R"json(
        {
          "project": "legacy",
          "worldFile": "world.ini"
        }
    )json");

    SceneLoader loader;
    const auto result = loader.loadFromFile(path);

    EXPECT_FALSE(result.success);
    ASSERT_FALSE(result.errors.empty());
    EXPECT_NE(result.errors.front().find("legacy INI maps"), std::string::npos);
}

TEST(SceneLoader, AppliesDefaultsForMissingSpriteFields)
{
    _mkdir(kFixtureDir);
    const auto path = fixturePath("minimal_project.json");
    writeFile(path, R"json(
        {
          "project": "minimal",
          "spriteInstances": [
            { "name": "guard", "spriteSet": "doom" }
          ]
        }
    )json");

    SceneLoader loader;
    const auto result = loader.loadFromFile(path);

    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.scene.textureRoot, ".");
    ASSERT_EQ(result.scene.spriteInstances.size(), 1u);
    const auto& guard = result.scene.spriteInstances.front();
    EXPECT_DOUBLE_EQ(guard.scaleCells, 1.0);
    EXPECT_DOUBLE_EQ(guard.collisionRadiusCells, 0.0);
    EXPECT_TRUE(guard.visible);
    EXPECT_FALSE(guard.passThroughWalls);
    EXPECT_FALSE(guard.chasePlayer);
    EXPECT_DOUBLE_EQ(guard.speedCellsPerSecond, 0.0);
    EXPECT_DOUBLE_EQ(guard.detectionRadiusCells, 0.0);
    EXPECT_DOUBLE_EQ(guard.patrolRadiusCells, 0.0);
    EXPECT_DOUBLE_EQ(guard.engagementHysteresisCells, 0.5);
    EXPECT_FALSE(guard.patrolCircuit);
    EXPECT_DOUBLE_EQ(guard.stoppingDistanceCells, 0.0);
    EXPECT_DOUBLE_EQ(guard.maxHealth, 0.0);
    EXPECT_DOUBLE_EQ(guard.health, 0.0);
    EXPECT_DOUBLE_EQ(guard.attackDamage, 0.0);
    EXPECT_FALSE(guard.rangedAttack);
    EXPECT_DOUBLE_EQ(guard.attackRangeCells, 0.0);
    EXPECT_DOUBLE_EQ(guard.attackCooldownSeconds, 1.0);
    EXPECT_DOUBLE_EQ(guard.attackFovDegrees, 70.0);
    EXPECT_EQ(guard.attackBurstShots, 3);
    EXPECT_DOUBLE_EQ(guard.attackBurstPauseSeconds, 1.2);
}
