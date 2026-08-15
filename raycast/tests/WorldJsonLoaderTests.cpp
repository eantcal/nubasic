#include "WorldJsonLoader.h"
#include "WorldMap.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

TEST(WorldJsonLoaderTests, PopulatesBlocksAndPackedCellsFromV2Document)
{
    const std::string json = R"({
        "format": "winraycast.world",
        "version": 2,
        "name": "demo",
        "grid": { "columns": 2, "rows": 1, "cellWidth": 256, "cellDepth": 512, "defaultWallHeight": 512 },
        "playerStart": { "xCell": 1.25, "yCell": 0.5, "facingDegrees": 90 },
        "defaultHorizonImage": "textures/custom_sky.png",
        "textures": {
            "01": { "name": "brick", "file": "textures/brick.bmp" },
            "0a": { "name": "ceil", "file": "ceil.bmp" }
        },
        "blocks": {
            "00": { "name": "empty" },
            "01": {
                "name": "stack",
                "ceiling": { "texture": "0a", "height": 512 },
                "walls": [
                    { "kind": "solid", "texture": "01", "bottom": 0, "top": 512 },
                    { "kind": "solid", "texture": "01", "bottom": 512, "top": 1280 }
                ]
            }
        },
        "cells": [
            [ "00", "01" ]
        ]
    })";

    WorldMap map;
    WorldJsonLoader loader;
    const auto result = loader.loadFromString(json, map);

    ASSERT_TRUE(result.success) << (result.errors.empty() ? std::string("no errors") : result.errors.front());

    EXPECT_EQ(1, map.getRowCount());
    EXPECT_EQ(2, map.getColCount());
    EXPECT_EQ(256u, map.getCellDx());
    EXPECT_EQ(512u, map.getCellDy());
    EXPECT_TRUE(map.hasPlayerStart());
    const auto sky = map.getTextureList().find("ff");
    ASSERT_NE(map.getTextureList().end(), sky);
    EXPECT_EQ("textures/custom_sky.png", sky->second);
    EXPECT_DOUBLE_EQ(1.25, map.getPlayerCellPos().first);
    EXPECT_DOUBLE_EQ(0.5, map.getPlayerCellPos().second);
    EXPECT_DOUBLE_EQ(90.0, map.getPlayerFacingDegrees());

    EXPECT_TRUE(map.usesBlockLayout());
    EXPECT_EQ(0, map.blockIdAt(0, 0));
    EXPECT_EQ(1, map.blockIdAt(0, 1));

    ASSERT_NE(nullptr, map.blockAtCell(0, 1));
    const auto* stack = map.blockAtCell(0, 1);
    ASSERT_EQ(2u, stack->walls.size());
    EXPECT_EQ(0, stack->walls[0].bottom);
    EXPECT_EQ(512, stack->walls[0].top);
    EXPECT_EQ(512, stack->walls[1].bottom);
    EXPECT_EQ(1280, stack->walls[1].top);

    // Packed cell back-compat: first solid span -> solid wall, second one above default height -> upper wall.
    const auto packed = map[0][1];
    EXPECT_EQ(0x01, MapCell::solidWallTexture(packed));
    EXPECT_EQ(0x01, MapCell::upperWallTexture(packed));
    EXPECT_EQ(0x0a, MapCell::ceilingTexture(packed));
}

TEST(WorldJsonLoaderTests, SelectsRequestedWorldLayer)
{
    const std::string json = R"({
        "format": "winraycast.world",
        "version": 2,
        "grid": { "columns": 2, "rows": 1, "cellWidth": 128, "cellDepth": 128, "defaultWallHeight": 128 },
        "activeLayer": "upper",
        "textures": {
            "01": { "name": "wall", "file": "wall.png" },
            "02": { "name": "other", "file": "other.png" }
        },
        "blocks": {
            "00": { "name": "empty" },
            "01": { "name": "wall", "walls": [ { "kind": "solid", "texture": "01", "bottom": 0, "top": 128 } ] },
            "02": { "name": "other", "walls": [ { "kind": "solid", "texture": "02", "bottom": 0, "top": 128 } ] }
        },
        "cells": [ [ "00", "01" ] ],
        "layers": [
            {
                "id": "upper",
                "playerStart": { "xCell": 0.5, "yCell": 0.5, "facingDegrees": 0 },
                "cells": [ [ "00", "01" ] ]
            },
            {
                "id": "lower",
                "playerStart": { "xCell": 1.5, "yCell": 0.5, "facingDegrees": 180 },
                "cells": [ [ "02", "00" ] ]
            }
        ]
    })";

    WorldMap map;
    WorldJsonLoader loader;
    const auto result = loader.loadFromString(json, map, "lower");

    ASSERT_TRUE(result.success) << (result.errors.empty() ? std::string("no errors") : result.errors.front());
    EXPECT_EQ("lower", result.activeLayerId);
    ASSERT_EQ(1, map.getRowCount());
    ASSERT_EQ(2, map.getColCount());
    EXPECT_EQ(2, map.blockIdAt(0, 0));
    EXPECT_EQ(0, map.blockIdAt(0, 1));
    EXPECT_DOUBLE_EQ(1.5, map.getPlayerCellPos().first);
    EXPECT_DOUBLE_EQ(0.5, map.getPlayerCellPos().second);
    EXPECT_DOUBLE_EQ(180.0, map.getPlayerFacingDegrees());
}

TEST(WorldJsonLoaderTests, ReportsInvalidWallSpan)
{
    const std::string json = R"({
        "format": "winraycast.world",
        "version": 2,
        "grid": { "columns": 1, "rows": 1, "cellWidth": 512, "cellDepth": 512, "defaultWallHeight": 512 },
        "textures": { "01": { "name": "wall", "file": "wall.bmp" } },
        "blocks": {
            "00": { "name": "empty" },
            "01": {
                "name": "broken",
                "walls": [ { "kind": "solid", "texture": "01", "bottom": 512, "top": 128 } ]
            }
        },
        "cells": [ [ "01" ] ]
    })";

    WorldMap map;
    WorldJsonLoader loader;
    const auto result = loader.loadFromString(json, map);

    EXPECT_FALSE(result.success);
    bool sawInvalidSpan = false;
    for (const auto& message : result.errors) {
        if (message.find("invalid wall span") != std::string::npos) {
            sawInvalidSpan = true;
            break;
        }
    }

    EXPECT_TRUE(sawInvalidSpan);
}

TEST(WorldJsonLoaderTests, PassableWallSpanControlsCollisionIndependentlyFromKind)
{
    const std::string json = R"({
        "format": "winraycast.world",
        "version": 2,
        "grid": { "columns": 3, "rows": 1, "cellWidth": 64, "cellDepth": 64, "defaultWallHeight": 64 },
        "textures": {
            "01": { "name": "solid", "file": "solid.png" },
            "03": { "name": "transparent", "file": "transparent.png" }
        },
        "blocks": {
            "00": { "name": "empty" },
            "01": {
                "name": "visual_only_wall",
                "walls": [
                    { "kind": "solid", "texture": "01", "bottom": 0, "top": 64, "passable": true }
                ]
            },
            "02": {
                "name": "blocking_glass",
                "walls": [
                    { "kind": "transparent", "texture": "03", "bottom": 0, "top": 64, "passable": false }
                ]
            }
        },
        "cells": [
            [ "00", "01", "02" ]
        ]
    })";

    WorldMap map;
    WorldJsonLoader loader;
    const auto result = loader.loadFromString(json, map);

    ASSERT_TRUE(result.success) << (result.errors.empty() ? std::string("no errors") : result.errors.front());

    const auto* visualOnly = map.blockAtCell(0, 1);
    ASSERT_NE(nullptr, visualOnly);
    ASSERT_EQ(1u, visualOnly->walls.size());
    EXPECT_EQ(WinRayCast::WallSpanKind::Solid, visualOnly->walls[0].kind);
    EXPECT_FALSE(visualOnly->walls[0].collision);
    EXPECT_TRUE(visualOnly->hasAnySolidSpan);
    EXPECT_FALSE(visualOnly->hasAnyCollidingSpan);
    EXPECT_FALSE(map.isSolidAtWorld(64.0 * 1.5, 32.0));

    const auto* blockingGlass = map.blockAtCell(0, 2);
    ASSERT_NE(nullptr, blockingGlass);
    ASSERT_EQ(1u, blockingGlass->walls.size());
    EXPECT_EQ(WinRayCast::WallSpanKind::Transparent, blockingGlass->walls[0].kind);
    EXPECT_TRUE(blockingGlass->walls[0].collision);
    EXPECT_TRUE(blockingGlass->hasAnyTransparentSpan);
    EXPECT_TRUE(blockingGlass->hasAnyCollidingSpan);
    EXPECT_TRUE(map.isSolidAtWorld(64.0 * 2.5, 32.0));
}

TEST(WorldJsonLoaderTests, WallSpanCanOverrideTexturePerFace)
{
    const std::string json = R"({
        "format": "winraycast.world",
        "version": 2,
        "grid": { "columns": 2, "rows": 1, "cellWidth": 64, "cellDepth": 64, "defaultWallHeight": 64 },
        "textures": {
            "01": { "name": "default", "file": "default.png" },
            "02": { "name": "north", "file": "north.png" },
            "03": { "name": "east", "file": "east.png" },
            "04": { "name": "south", "file": "south.png" },
            "05": { "name": "west", "file": "west.png" }
        },
        "blocks": {
            "00": { "name": "empty" },
            "01": {
                "name": "four_sided_wall",
                "walls": [
                    {
                        "kind": "solid",
                        "texture": "01",
                        "faceTextures": {
                            "north": "02",
                            "east": "03",
                            "south": "04",
                            "west": "05"
                        },
                        "bottom": 0,
                        "top": 64
                    }
                ]
            }
        },
        "cells": [
            [ "00", "01" ]
        ]
    })";

    WorldMap map;
    WorldJsonLoader loader;
    const auto result = loader.loadFromString(json, map);

    ASSERT_TRUE(result.success) << (result.errors.empty() ? std::string("no errors") : result.errors.front());
    const auto* block = map.blockAtCell(0, 1);
    ASSERT_NE(nullptr, block);
    ASSERT_EQ(1u, block->walls.size());
    const auto& wall = block->walls[0];
    EXPECT_EQ(0x01, wall.textureKey);
    // JSON labels and runtime WallFace values are both geometric sides of the block.
    EXPECT_EQ(0x02, wall.textureForFace(WinRayCast::WallFace::North));
    EXPECT_EQ(0x03, wall.textureForFace(WinRayCast::WallFace::East));
    EXPECT_EQ(0x04, wall.textureForFace(WinRayCast::WallFace::South));
    EXPECT_EQ(0x05, wall.textureForFace(WinRayCast::WallFace::West));
}

TEST(WorldJsonLoaderTests, WallSpanCanDeclareAnimatedTextureOverlay)
{
    const std::string json = R"({
        "format": "winraycast.world",
        "version": 2,
        "grid": { "columns": 1, "rows": 1, "cellWidth": 64, "cellDepth": 64, "defaultWallHeight": 64 },
        "textures": {
            "01": { "name": "base", "file": "base.png" },
            "c0": { "name": "tvcc_0", "file": "tvcc_0.png" },
            "c1": { "name": "tvcc_1", "file": "tvcc_1.png" }
        },
        "blocks": {
            "01": {
                "name": "monitor_wall",
                "walls": [
                    { "kind": "solid", "texture": "01", "bottom": 0, "top": 64 }
                ],
                "animations": [
                    {
                        "name": "tvcc_cycle",
                        "target": "wallOverlay",
                        "wallIndex": 0,
                        "face": "north",
                        "frameDurationMs": 2500,
                        "loop": true,
                        "frames": [ "c0", "c1" ]
                    }
                ]
            }
        },
        "cells": [
            [ "01" ]
        ]
    })";

    WorldMap map;
    WorldJsonLoader loader;
    const auto result = loader.loadFromString(json, map);

    ASSERT_TRUE(result.success) << (result.errors.empty() ? std::string("no errors") : result.errors.front());
    const auto* block = map.blockAtCell(0, 0);
    ASSERT_NE(nullptr, block);
    ASSERT_EQ(1u, block->walls.size());
    const auto& wall = block->walls[0];
    ASSERT_EQ(1u, wall.overlayAnimations.size());
    EXPECT_TRUE(wall.baseAnimations.empty());

    const auto& animation = wall.overlayAnimations[0];
    EXPECT_EQ(static_cast<int>(WinRayCast::WallFace::North), animation.face);
    EXPECT_DOUBLE_EQ(2.5, animation.frameDurationSeconds);
    EXPECT_TRUE(animation.loop);
    ASSERT_EQ(2u, animation.textureKeys.size());
    EXPECT_EQ(0xc0, animation.textureKeys[0]);
    EXPECT_EQ(0xc1, animation.textureKeys[1]);
}

TEST(WorldJsonLoaderTests, LoadsDoorOpeningSoundMetadata)
{
    const std::string json = R"({
        "format": "winraycast.world",
        "version": 2,
        "grid": { "columns": 1, "rows": 1, "cellWidth": 64, "cellDepth": 64, "defaultWallHeight": 64 },
        "textures": {
            "01": { "name": "door", "file": "door.png" }
        },
        "blocks": {
            "00": { "name": "empty" },
            "01": {
                "name": "lift_door",
                "walls": [
                    { "kind": "transparent", "texture": "01", "bottom": 0, "top": 64 }
                ],
                "door": {
                    "enabled": true,
                    "openSound": "effects/Elevator_Opening_Sequence.mp3",
                    "openSoundVolumePercent": 100,
                    "frames": [ "01" ]
                }
            }
        },
        "cells": [
            [ "01" ]
        ]
    })";

    WorldMap map;
    WorldJsonLoader loader;
    const auto result = loader.loadFromString(json, map);

    ASSERT_TRUE(result.success) << (result.errors.empty() ? std::string("no errors") : result.errors.front());
    const auto* block = map.blockAtCell(0, 0);
    ASSERT_NE(nullptr, block);
    EXPECT_EQ("effects/Elevator_Opening_Sequence.mp3", block->door.openSound);
    EXPECT_EQ(100, block->door.openSoundVolumePercent);
}

TEST(WorldJsonLoaderTests, WallSpanCanUseInteriorTextureForInternalRendering)
{
    const std::string json = R"({
        "format": "winraycast.world",
        "version": 2,
        "grid": { "columns": 2, "rows": 1, "cellWidth": 64, "cellDepth": 64, "defaultWallHeight": 64 },
        "textures": {
            "01": { "name": "outside", "file": "outside.png" },
            "04": { "name": "inside", "file": "inside.png" }
        },
        "blocks": {
            "00": { "name": "empty" },
            "01": {
                "name": "interior_wall",
                "walls": [
                    {
                        "kind": "solid",
                        "texture": "01",
                        "interiorTexture": "04",
                        "bottom": 0,
                        "top": 64
                    }
                ]
            }
        },
        "cells": [
            [ "00", "01" ]
        ]
    })";

    WorldMap map;
    WorldJsonLoader loader;
    const auto result = loader.loadFromString(json, map);

    ASSERT_TRUE(result.success) << (result.errors.empty() ? std::string("no errors") : result.errors.front());
    const auto* block = map.blockAtCell(0, 1);
    ASSERT_NE(nullptr, block);
    ASSERT_EQ(1u, block->walls.size());
    const auto& wall = block->walls[0];

    EXPECT_EQ(0x01, map.textureForWallSpanAt(0, 1, wall, WinRayCast::WallFace::West, false));
    EXPECT_EQ(0x04, map.textureForWallSpanAt(0, 1, wall, WinRayCast::WallFace::West, true));
}

TEST(WorldJsonLoaderTests, DoorMetadataBuildsAnimatedRuntimeDoor)
{
    const std::string json = R"({
        "format": "winraycast.world",
        "version": 2,
        "grid": { "columns": 2, "rows": 1, "cellWidth": 64, "cellDepth": 64, "defaultWallHeight": 64 },
        "textures": {
            "01": { "name": "door_closed", "file": "door_closed.png" },
            "02": { "name": "door_opening", "file": "door_opening.png" },
            "03": { "name": "door_open", "file": "door_open.png" },
            "04": { "name": "green_key_overlay", "file": "green_key_overlay.png" }
        },
        "blocks": {
            "00": { "name": "empty" },
            "01": {
                "name": "sliding_door",
                "walls": [
                    { "kind": "transparent", "texture": "01", "bottom": 0, "top": 64, "collision": true }
                ],
                "door": {
                    "enabled": true,
                    "blocksWhenClosed": true,
                    "requiredKey": "green",
                    "triggerDistanceCells": 2.0,
                    "openTimeSeconds": 0.25,
                    "closeDelaySeconds": 0.5,
                    "frames": [ "01", "02", "03" ],
                    "lockedOverlays": { "green": "04" }
                }
            }
        },
        "cells": [
            [ "00", "01" ]
        ]
    })";

    WorldMap map;
    WorldJsonLoader loader;
    const auto result = loader.loadFromString(json, map);

    ASSERT_TRUE(result.success) << (result.errors.empty() ? std::string("no errors") : result.errors.front());
    const auto* block = map.blockAtCell(0, 1);
    ASSERT_NE(nullptr, block);
    EXPECT_TRUE(block->door.enabled);
    EXPECT_TRUE(block->door.blocksWhenClosed);
    EXPECT_EQ("green", block->door.requiredKey);
    EXPECT_DOUBLE_EQ(2.0, block->door.triggerDistanceCells);
    EXPECT_DOUBLE_EQ(0.25, block->door.openTimeSeconds);
    EXPECT_DOUBLE_EQ(0.5, block->door.closeDelaySeconds);
    ASSERT_EQ(3u, block->door.animationTextureKeys.size());
    EXPECT_EQ(0x01, block->door.animationTextureKeys[0]);
    EXPECT_EQ(0x03, block->door.animationTextureKeys[2]);
    ASSERT_EQ(1u, block->door.lockedOverlayTextureKeysByKey.size());
    EXPECT_EQ(0x04, block->door.lockedOverlayTextureKeysByKey.at("green"));
    EXPECT_TRUE(map.isSolidAtWorld(64.0 * 1.5, 32.0));
    const auto lockedLayer = map.transparentWallTextureLayerAt(
        0,
        1,
        0x01,
        WinRayCast::WallFace::West);
    EXPECT_EQ(0x01, lockedLayer.base);
    EXPECT_EQ(0x04, lockedLayer.overlay);

    map.updateDoors(32.0, 32.0, {}, 0.5);
    EXPECT_FALSE(map.isDoorOpenAt(0, 1));
    EXPECT_TRUE(map.isSolidAtWorld(64.0 * 1.5, 32.0));

    map.setDoorKeyring({ "green" });
    map.updateDoors(32.0, 32.0, {}, 0.5);

    EXPECT_TRUE(map.isDoorOpenAt(0, 1));
    EXPECT_FALSE(map.isSolidAtWorld(64.0 * 1.5, 32.0));
    const auto unlockedLayer = map.transparentWallTextureLayerAt(
        0,
        1,
        0x01,
        WinRayCast::WallFace::West);
    EXPECT_EQ(0x03, unlockedLayer.base);
    EXPECT_EQ(0, unlockedLayer.overlay);
}

TEST(WorldJsonLoaderTests, RejectsUnsupportedVersion)
{
    const std::string json = R"({
        "format": "winraycast.world",
        "version": 1,
        "grid": { "columns": 1, "rows": 1, "cellWidth": 512, "cellDepth": 512, "defaultWallHeight": 512 },
        "textures": {},
        "cells": []
    })";

    WorldMap map;
    WorldJsonLoader loader;
    const auto result = loader.loadFromString(json, map);

    EXPECT_FALSE(result.success);
}

TEST(WorldJsonLoaderTests, ParsesShippedDemoWorldWithVariableHeights)
{
    const std::string path =
        std::string(NURAYCAST_DEMO_WORLD_DIR) + "/demo.world.json";
    std::ifstream file(path);
    ASSERT_TRUE(file.is_open()) << "Cannot open " << path;

    std::ostringstream buffer;
    buffer << file.rdbuf();

    WorldMap map;
    WorldJsonLoader loader;
    const auto result = loader.loadFromString(buffer.str(), map);
    ASSERT_TRUE(result.success)
        << (result.errors.empty() ? std::string("no errors") : result.errors.front());

    EXPECT_GT(map.getRowCount(), 0);
    EXPECT_GT(map.getColCount(), 0);

    bool foundTallWall = false;
    for (int blockId = 0; blockId <= 0xff && !foundTallWall; ++blockId) {
        const auto* block = map.blockDefinition(
            static_cast<WinRayCast::BlockId>(blockId));
        if (block == nullptr) {
            continue;
        }

        for (const auto& span : block->walls) {
            if (span.top > static_cast<int>(map.getCellDy())) {
                foundTallWall = true;
                break;
            }
        }
    }

    EXPECT_TRUE(foundTallWall)
        << "demo.world.json should ship with at least one >1-block-high wall span";
}

TEST(WorldJsonLoaderTests, ShippedDemoWorldLoadsWithExpectedDimensions)
{
    const std::string jsonPath =
        std::string(NURAYCAST_DEMO_WORLD_DIR) + "/demo.world.json";

    WorldMap jsonMap;
    WorldJsonLoader loader;
    const auto result = loader.loadFromFile(jsonPath, jsonMap);
    ASSERT_TRUE(result.success)
        << (result.errors.empty() ? std::string("no errors") : result.errors.front());

    ASSERT_EQ(16, jsonMap.getRowCount());
    ASSERT_EQ(16, jsonMap.getColCount());
}

TEST(WorldJsonLoaderTests, ShippedDemoWorldDeclaresSelfContainedSpriteAssets)
{
    const std::string jsonPath =
        std::string(NURAYCAST_DEMO_WORLD_DIR) + "/demo.world.json";
    std::ifstream file(jsonPath);
    ASSERT_TRUE(file.is_open()) << "Cannot open " << jsonPath;

    nlohmann::json document;
    file >> document;

    ASSERT_EQ(7u, document["layers"].size());
    ASSERT_EQ(42u, document["layerTransitions"].size());

    size_t toolboxCount = 0;
    const auto expectUniformToolboxScale = [&toolboxCount](const auto& sprites) {
        for (const auto& sprite : sprites) {
            if (sprite.value("spriteSet", std::string()) != "item_toolbox") {
                continue;
            }

            ++toolboxCount;
            EXPECT_DOUBLE_EQ(0.32, sprite["scaleCells"].get<double>());
        }
    };
    expectUniformToolboxScale(document["spriteInstances"]);
    for (const auto& layer : document["layers"]) {
        expectUniformToolboxScale(layer["spriteInstances"]);
    }
    EXPECT_EQ(11u, toolboxCount);

    EXPECT_EQ(
        "textures/final_level/metallic_floor.png",
        document["textures"]["c4"]["file"].get<std::string>());
    EXPECT_EQ(
        "textures/final_level/white_tiles.png",
        document["textures"]["c5"]["file"].get<std::string>());
    EXPECT_EQ("c4", document["blocks"]["9e"]["floor"]["texture"].get<std::string>());
    EXPECT_EQ("c5", document["blocks"]["9e"]["ceiling"]["texture"].get<std::string>());
    EXPECT_EQ("c6", document["blocks"]["88"]["walls"][0]["texture"].get<std::string>());
    EXPECT_EQ("c7", document["blocks"]["92"]["walls"][0]["texture"].get<std::string>());
    EXPECT_EQ("c8", document["blocks"]["80"]["walls"][0]["texture"].get<std::string>());
    EXPECT_EQ("c9", document["blocks"]["21"]["walls"][0]["texture"].get<std::string>());

    ASSERT_TRUE(document.contains("spriteSets"));
    ASSERT_TRUE(document["spriteSets"].is_array());
    const char* expectedSpriteSets[] = {
        "sprites/sheet_brute/sheet_brute.sprite.json",
        "sprites/missile_brute/missile_brute.sprite.json",
        "sprites/soldier/soldier.sprite.json",
        "sprites/hover_sentinel/hover_sentinel.sprite.json",
        "sprites/guide_robot/guide_robot.sprite.json",
        "sprites/items/item_supply_crate/item_supply_crate.sprite.json",
        "sprites/items/item_ammo_box/item_ammo_box.sprite.json",
        "sprites/items/item_medikit/item_medikit.sprite.json",
        "sprites/items/item_oxygen_tank/item_oxygen_tank.sprite.json",
        "sprites/items/item_toolbox/item_toolbox.sprite.json",
        "sprites/items/item_computer/item_computer.sprite.json",
        "sprites/items/item_wire_spool/item_wire_spool.sprite.json",
        "sprites/items/item_office_copier/item_office_copier.sprite.json",
        "sprites/items/item_potted_plant/item_potted_plant.sprite.json",
        "sprites/items/item_hazard_barrel/item_hazard_barrel.sprite.json",
        "sprites/items/item_key_green/item_key_green.sprite.json",
        "sprites/items/item_key_blue/item_key_blue.sprite.json",
        "sprites/items/item_key_red/item_key_red.sprite.json",
        "sprites/items/item_weapon_pistol/item_weapon_pistol.sprite.json",
        "sprites/items/item_weapon_super_shotgun/item_weapon_super_shotgun.sprite.json",
        "sprites/items/item_weapon_submachine_gun/item_weapon_submachine_gun.sprite.json",
        "sprites/effects/explosion_512/explosion_512.sprite.json",
        "sprites/effects/vase_break_256/vase_break_256.sprite.json",
        "sprites/effects/broken_vase_256/broken_vase_256.sprite.json",
        "sprites/effects/ash_pile/ash_pile.sprite.json"
    };
    ASSERT_EQ(
        sizeof(expectedSpriteSets) / sizeof(expectedSpriteSets[0]),
        document["spriteSets"].size());
    for (size_t index = 0; index < document["spriteSets"].size(); ++index) {
        EXPECT_EQ(
            expectedSpriteSets[index],
            document["spriteSets"][index].get<std::string>());
    }

    ASSERT_TRUE(document.contains("spriteInstances"));
    ASSERT_TRUE(document["spriteInstances"].is_array());
    ASSERT_EQ(4u, document["spriteInstances"].size());
    EXPECT_EQ(
        "sheet_brute",
        document["spriteInstances"][0]["spriteSet"].get<std::string>());
    EXPECT_TRUE(document["spriteInstances"][0]["chasePlayer"].get<bool>());
    EXPECT_TRUE(document["spriteInstances"][0]["patrolCircuit"].get<bool>());
    EXPECT_EQ(
        "missile_brute",
        document["spriteInstances"][1]["spriteSet"].get<std::string>());
    EXPECT_TRUE(document["spriteInstances"][1]["chasePlayer"].get<bool>());
    EXPECT_TRUE(document["spriteInstances"][1]["patrolCircuit"].get<bool>());
    EXPECT_EQ(
        "missile_brute",
        document["spriteInstances"][2]["spriteSet"].get<std::string>());
    EXPECT_TRUE(document["spriteInstances"][2]["chasePlayer"].get<bool>());
    EXPECT_DOUBLE_EQ(
        0.55,
        document["spriteInstances"][2]["speedCellsPerSecond"].get<double>());
    EXPECT_DOUBLE_EQ(
        2.25,
        document["spriteInstances"][2]["detectionRadiusCells"].get<double>());
    EXPECT_DOUBLE_EQ(
        3.5,
        document["spriteInstances"][2]["patrolRadiusCells"].get<double>());
    EXPECT_DOUBLE_EQ(
        0.75,
        document["spriteInstances"][2]["engagementHysteresisCells"].get<double>());
    EXPECT_TRUE(document["spriteInstances"][2]["patrolCircuit"].get<bool>());
    EXPECT_DOUBLE_EQ(
        0.7,
        document["spriteInstances"][2]["stoppingDistanceCells"].get<double>());
    EXPECT_EQ(
        "soldier",
        document["spriteInstances"][3]["spriteSet"].get<std::string>());
    EXPECT_TRUE(document["spriteInstances"][3]["chasePlayer"].get<bool>());
    EXPECT_TRUE(document["spriteInstances"][3]["patrolCircuit"].get<bool>());
    EXPECT_DOUBLE_EQ(
        0.7,
        document["spriteInstances"][3]["speedCellsPerSecond"].get<double>());
    EXPECT_DOUBLE_EQ(
        2.8,
        document["spriteInstances"][3]["detectionRadiusCells"].get<double>());

    const auto& levelZero = *std::find_if(
        document["layers"].begin(),
        document["layers"].end(),
        [](const auto& layer) { return layer["id"] == "level_0"; });
    EXPECT_DOUBLE_EQ(0.82, levelZero["brightness"].get<double>());
    EXPECT_DOUBLE_EQ(180.0, levelZero["depthShading"].get<double>());
    EXPECT_EQ("Laboratory", levelZero["name"].get<std::string>());

    const auto& levelOne = *std::find_if(
        document["layers"].begin(),
        document["layers"].end(),
        [](const auto& layer) { return layer["id"] == "level_1"; });
    EXPECT_EQ("Armory & Infirmary", levelOne["name"].get<std::string>());

    const auto& levelTwo = *std::find_if(
        document["layers"].begin(),
        document["layers"].end(),
        [](const auto& layer) { return layer["id"] == "level_2"; });
    EXPECT_EQ("Library", levelTwo["name"].get<std::string>());

    const char* cloneIds[] = { "level_3", "level_4", "level_5" };
    const char* cloneNames[] = {
        "Industrial Complex",
        "Legal Office",
        "Forbidden Archives"
    };
    for (size_t index = 0; index < 3; ++index) {
        const auto clone = std::find_if(
            document["layers"].begin(),
            document["layers"].end(),
            [=](const auto& layer) { return layer["id"] == cloneIds[index]; });
        ASSERT_NE(document["layers"].end(), clone);
        EXPECT_EQ(cloneNames[index], (*clone)["name"].get<std::string>());
        EXPECT_EQ("e1", (*clone)["cells"][14][2].get<std::string>());
    }
    const auto guideRobot = std::find_if(
        levelZero["spriteInstances"].begin(),
        levelZero["spriteInstances"].end(),
        [](const auto& sprite) { return sprite["name"] == "level0_guide_robot"; });
    ASSERT_NE(levelZero["spriteInstances"].end(), guideRobot);
    EXPECT_EQ("guide_robot", (*guideRobot)["spriteSet"].get<std::string>());
    EXPECT_TRUE((*guideRobot)["patrolCircuit"].get<bool>());
    EXPECT_GT((*guideRobot)["speedCellsPerSecond"].get<double>(), 0.0);
    EXPECT_FALSE((*guideRobot)["chasePlayer"].get<bool>());
    EXPECT_FALSE((*guideRobot)["rangedAttack"].get<bool>());
    EXPECT_DOUBLE_EQ(0.0, (*guideRobot)["attackDamage"].get<double>());
    EXPECT_DOUBLE_EQ(100.0, (*guideRobot)["maxHealth"].get<double>());
    EXPECT_TRUE((*guideRobot)["explosive"].get<bool>());
    EXPECT_DOUBLE_EQ(1.75, (*guideRobot)["explosionRadiusCells"].get<double>());
    EXPECT_DOUBLE_EQ(55.0, (*guideRobot)["explosionDamage"].get<double>());
    EXPECT_EQ("explosion_512", (*guideRobot)["explosionSpriteSet"].get<std::string>());

    const auto& finalLayer = *std::find_if(
        document["layers"].begin(),
        document["layers"].end(),
        [](const auto& layer) { return layer["id"] == "level_final"; });
    EXPECT_DOUBLE_EQ(0.72, finalLayer["brightness"].get<double>());
    EXPECT_DOUBLE_EQ(220.0, finalLayer["depthShading"].get<double>());
    const auto hoverSentinel = std::find_if(
        finalLayer["spriteInstances"].begin(),
        finalLayer["spriteInstances"].end(),
        [](const auto& sprite) { return sprite["name"] == "final_hover_sentinel"; });
    ASSERT_NE(finalLayer["spriteInstances"].end(), hoverSentinel);
    EXPECT_EQ("hover_sentinel", (*hoverSentinel)["spriteSet"].get<std::string>());
    EXPECT_DOUBLE_EQ(0.0, (*hoverSentinel)["verticalOffsetCells"].get<double>());
    EXPECT_TRUE((*hoverSentinel)["rangedAttack"].get<bool>());
    EXPECT_EQ(5, (*hoverSentinel)["attackBurstShots"].get<int>());
    EXPECT_TRUE((*hoverSentinel)["explosive"].get<bool>());
    EXPECT_DOUBLE_EQ(2.25, (*hoverSentinel)["explosionRadiusCells"].get<double>());
    EXPECT_DOUBLE_EQ(85.0, (*hoverSentinel)["explosionDamage"].get<double>());
    EXPECT_EQ(
        "explosion_512",
        (*hoverSentinel)["explosionSpriteSet"].get<std::string>());
}
