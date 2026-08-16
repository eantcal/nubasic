#include "RayCaster.h"

#include <gtest/gtest.h>

namespace {
constexpr Cell kEmpty = 0x0000000000ULL;
constexpr Cell kWall = 0x0000000001ULL;

WorldMap makeBoxMap()
{
    const Cell cells[] = {
        kWall, kWall, kWall,
        kWall, kEmpty, kWall,
        kWall, kWall, kWall,
    };

    WorldMap map;
    EXPECT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);
    return map;
}

Player makeCenteredPlayer()
{
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 96.0, 96.0 });
    return player;
}
}

TEST(RayCasterTests, CastsNearestVerticalWall)
{
    auto map = makeBoxMap();
    auto player = makeCenteredPlayer();
    map.setPlayerPos(player.getX(), player.getY());

    const auto hit = RayCaster(player).castSolidWallRay(map, 0);

    EXPECT_TRUE(hit.found);
    EXPECT_TRUE(hit.vertical);
    EXPECT_EQ(nu::rcade::WallFace::West, hit.face);
    EXPECT_EQ(kWall, hit.cell);
    EXPECT_DOUBLE_EQ(128.0, hit.point.first);
    EXPECT_DOUBLE_EQ(96.0, hit.point.second);
    EXPECT_DOUBLE_EQ(32.0, hit.distance);
}

TEST(RayCasterTests, CastsNearestHorizontalWall)
{
    auto map = makeBoxMap();
    auto player = makeCenteredPlayer();
    map.setPlayerPos(player.getX(), player.getY());

    const auto hit = RayCaster(player).castSolidWallRay(map, player.deg90());

    EXPECT_TRUE(hit.found);
    EXPECT_FALSE(hit.vertical);
    EXPECT_EQ(nu::rcade::WallFace::North, hit.face);
    EXPECT_EQ(kWall, hit.cell);
    EXPECT_NEAR(96.0, hit.point.first, 0.25);
    EXPECT_DOUBLE_EQ(128.0, hit.point.second);
    EXPECT_NEAR(32.0, hit.distance, 0.001);
}

TEST(RayCasterTests, ReportsNoHitWhenRayLeavesOpenMap)
{
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
    };

    WorldMap map;
    ASSERT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);

    auto player = makeCenteredPlayer();
    map.setPlayerPos(player.getX(), player.getY());

    const auto hit = RayCaster(player).castSolidWallRay(map, 0);

    EXPECT_FALSE(hit.found);
}

TEST(RayCasterTests, CastsStructuredBlockWithOnlyElevatedSolidSpan)
{
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
    };

    WorldMap map;
    ASSERT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);

    nu::rcade::BlockDefinition elevatedBlock;
    elevatedBlock.hasAnySolidSpan = true;
    elevatedBlock.walls.push_back({
        1,
        64,
        128,
        nu::rcade::WallSpanKind::Solid,
        true
    });
    map.setBlockDefinition(1, elevatedBlock);
    map.setBlockId(1, 2, 1);

    auto player = makeCenteredPlayer();
    map.setPlayerPos(player.getX(), player.getY());

    const auto hit = RayCaster(player).castSolidWallRay(map, 0);

    EXPECT_TRUE(hit.found);
    EXPECT_TRUE(hit.vertical);
    ASSERT_NE(nullptr, hit.block);
    ASSERT_EQ(1u, hit.block->walls.size());
    EXPECT_EQ(64, hit.block->walls[0].bottom);
    EXPECT_EQ(128, hit.block->walls[0].top);
}

TEST(RayCasterTests, LegacyTransparentUpperWallDoesNotBlockSolidRay)
{
    const Cell transparentUpperCell =
        (static_cast<Cell>(0x05) << 32)
        | (static_cast<Cell>(0x03) << 24)
        | (static_cast<Cell>(0x09) << 16)
        | (static_cast<Cell>(0x05) << 8);

    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty, kWall,
        kEmpty, kEmpty, transparentUpperCell, kWall,
        kEmpty, kEmpty, kEmpty, kWall,
    };

    WorldMap map;
    ASSERT_TRUE(map.setCells(cells, 3, 4));
    map.resizeCell(64, 64);

    nu::rcade::BlockDefinition transparentUpperBlock;
    transparentUpperBlock.hasAnySolidSpan = true;
    transparentUpperBlock.hasAnyTransparentSpan = true;
    transparentUpperBlock.walls.push_back({
        0x05,
        64,
        128,
        nu::rcade::WallSpanKind::Solid,
        true
    });
    transparentUpperBlock.walls.push_back({
        0x03,
        0,
        64,
        nu::rcade::WallSpanKind::Transparent,
        false
    });
    map.setBlockDefinition(1, transparentUpperBlock);
    map.setBlockId(1, 2, 1);

    auto player = makeCenteredPlayer();
    map.setPlayerPos(player.getX(), player.getY());

    const auto hit = RayCaster(player).castSolidWallRay(map, 0);

    EXPECT_TRUE(hit.found);
    EXPECT_EQ(kWall, hit.cell);
    EXPECT_DOUBLE_EQ(192.0, hit.point.first);
    EXPECT_FALSE(map.isSolidAtWorld(64.0 * 2.5, 64.0 * 1.5));
}

TEST(RayCasterTests, PlayerMovementCollidesWithStructuredBlocks)
{
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
    };

    WorldMap map;
    ASSERT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);

    nu::rcade::BlockDefinition wallBlock;
    wallBlock.hasAnySolidSpan = true;
    wallBlock.walls.push_back({
        1,
        0,
        64,
        nu::rcade::WallSpanKind::Solid,
        true
    });
    map.setBlockDefinition(1, wallBlock);
    map.setBlockId(1, 2, 1);

    auto player = makeCenteredPlayer();
    map.setPlayerPos(player.getX(), player.getY());

    const auto blockedCell = player.moveTo(40, map);

    EXPECT_EQ(kEmpty, blockedCell);
    EXPECT_DOUBLE_EQ(96.0, player.getX());
    EXPECT_DOUBLE_EQ(96.0, player.getY());
}

TEST(RayCasterTests, StructuredSolidPassableBlockRendersButDoesNotBlockMovement)
{
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
    };

    WorldMap map;
    ASSERT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);

    nu::rcade::BlockDefinition visualWallBlock;
    visualWallBlock.walls.push_back({
        1,
        0,
        64,
        nu::rcade::WallSpanKind::Solid,
        false
    });
    map.setBlockDefinition(1, visualWallBlock);
    map.setBlockId(1, 2, 1);
    map.set(1, 2, 1);

    auto player = makeCenteredPlayer();
    map.setPlayerPos(player.getX(), player.getY());

    const auto hit = RayCaster(player).castSolidWallRay(map, 0);

    EXPECT_TRUE(hit.found);
    EXPECT_FALSE(map.isSolidAtWorld(64.0 * 2.5, 64.0 * 1.5));
}

TEST(RayCasterTests, StructuredTransparentCollidingBlockBlocksMovementButNotSolidRay)
{
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
    };

    WorldMap map;
    ASSERT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);

    nu::rcade::BlockDefinition transparentGateBlock;
    transparentGateBlock.walls.push_back({
        3,
        0,
        64,
        nu::rcade::WallSpanKind::Transparent,
        true
    });
    map.setBlockDefinition(1, transparentGateBlock);
    map.setBlockId(1, 2, 1);
    map.set(1, 2, static_cast<Cell>(3) << 24);

    auto player = makeCenteredPlayer();
    map.setPlayerPos(player.getX(), player.getY());

    const auto hit = RayCaster(player).castSolidWallRay(map, 0);

    EXPECT_FALSE(hit.found);
    EXPECT_TRUE(map.isSolidAtWorld(64.0 * 2.5, 64.0 * 1.5));
}

TEST(RayCasterTests, DoorBlockOpensAndBecomesPassableForMovementAndRays)
{
    const Cell doorCell =
        (static_cast<Cell>(0x01) << 32)
        | (static_cast<Cell>(0x01) << 24);
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty, kWall,
        kEmpty, kEmpty, doorCell, kWall,
        kEmpty, kEmpty, kEmpty, kWall,
    };

    WorldMap map;
    ASSERT_TRUE(map.setCells(cells, 3, 4));
    map.resizeCell(64, 64);

    nu::rcade::BlockDefinition doorBlock;
    doorBlock.walls.push_back({
        1,
        64,
        128,
        nu::rcade::WallSpanKind::Solid,
        true
    });
    doorBlock.walls.push_back({
        1,
        0,
        64,
        nu::rcade::WallSpanKind::Transparent,
        true
    });
    doorBlock.door.enabled = true;
    doorBlock.door.blocksWhenClosed = true;
    doorBlock.door.triggerDistanceCells = 2.0;
    doorBlock.door.openTimeSeconds = 0.25;
    doorBlock.door.animationTextureKeys = { 1, 2, 3 };
    map.setBlockDefinition(1, doorBlock);
    map.setBlockId(1, 2, 1);

    auto player = makeCenteredPlayer();
    map.setPlayerPos(player.getX(), player.getY());

    EXPECT_TRUE(map.isSolidAtWorld(64.0 * 2.5, 64.0 * 1.5));
    EXPECT_EQ(1, map.transparentWallTextureAt(
        1,
        2,
        MapCell::transparentWallTexture(doorCell),
        nu::rcade::WallFace::West));

    auto closedHit = RayCaster(player).castSolidWallRay(map, 0);
    ASSERT_TRUE(closedHit.found);
    EXPECT_EQ(kWall, closedHit.cell);
    EXPECT_EQ(3, closedHit.column);
    EXPECT_DOUBLE_EQ(192.0, closedHit.point.first);

    std::vector<WorldMap::DoorEvent> doorEvents;
    map.updateDoors(
        player.getX(),
        player.getY(),
        {},
        0.5,
        &doorEvents);

    ASSERT_EQ(1u, doorEvents.size());
    EXPECT_EQ(WorldMap::DoorEvent::Type::OpeningStarted, doorEvents.front().type);
    EXPECT_EQ(1, doorEvents.front().row);
    EXPECT_EQ(2, doorEvents.front().column);
    EXPECT_EQ(1, doorEvents.front().blockId);

    EXPECT_TRUE(map.isDoorOpenAt(1, 2));
    EXPECT_FALSE(map.isSolidAtWorld(64.0 * 2.5, 64.0 * 1.5));
    EXPECT_EQ(3, map.transparentWallTextureAt(
        1,
        2,
        MapCell::transparentWallTexture(doorCell),
        nu::rcade::WallFace::West));

    auto openHit = RayCaster(player).castSolidWallRay(map, 0);
    ASSERT_TRUE(openHit.found);
    EXPECT_EQ(kWall, openHit.cell);
    EXPECT_EQ(3, openHit.column);
    EXPECT_DOUBLE_EQ(192.0, openHit.point.first);
}

TEST(RayCasterTests, PlayerMovementKeepsSmallDistanceFromSolidWalls)
{
    auto map = makeBoxMap();
    auto player = makeCenteredPlayer();
    map.setPlayerPos(player.getX(), player.getY());

    const auto blockedCell = player.moveTo(34, map);

    EXPECT_EQ(kEmpty, blockedCell);
    EXPECT_DOUBLE_EQ(96.0, player.getX());
    EXPECT_DOUBLE_EQ(96.0, player.getY());
}

TEST(RayCasterTests, OutsideMapIsSolidForMovementCollision)
{
    auto map = makeBoxMap();

    EXPECT_TRUE(map.isSolidAtWorld(-1.0, 96.0));
    EXPECT_TRUE(map.isSolidAtWorld(96.0, -1.0));
    EXPECT_TRUE(map.isSolidAtWorld(map.getMaxX(), 96.0));
    EXPECT_TRUE(map.isSolidAtWorld(96.0, map.getMaxY()));
}
