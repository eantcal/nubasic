#include "SpriteManager.h"
#include "SpriteSet.h"

#include <gtest/gtest.h>

namespace {
SpriteSet makeSpriteSet()
{
    SpriteDirectionDefinition front;
    front.name = "front";
    front.angleDegrees = 0.0;
    front.filesByResolution[64] = "front_64.bmp";
    front.filesByResolution[256] = "front_256.bmp";

    SpriteDirectionDefinition right;
    right.name = "right";
    right.angleDegrees = 90.0;
    right.filesByResolution[128] = "right_128.bmp";

    SpriteSet spriteSet;
    spriteSet.setName("test");
    spriteSet.setFormat("BMP");
    spriteSet.setTransparentColor(makeColor(0, 0, 0));
    spriteSet.setSupportedResolutions({ 64, 128, 256 });
    spriteSet.setDefaultResolution(128);
    spriteSet.setMaxResolution(256);
    spriteSet.setDirections({ front, right });
    spriteSet.setAnimations({
        { "idle", 0.0, true, { front, right }, { { front, right } } },
        { "death", 140.0, false, { front }, { { front } } },
    });
    spriteSet.setLodRules({
        { 2.0, 256 },
        { 8.0, 128 },
        { 9999.0, 64 },
    });
    return spriteSet;
}

TEST(SpriteSetTests, ResolvesNamedAnimation)
{
    const auto spriteSet = makeSpriteSet();

    const auto* idle = spriteSet.animation("idle");
    ASSERT_NE(nullptr, idle);
    EXPECT_TRUE(idle->loop);
    EXPECT_EQ(2u, idle->directions.size());

    const auto* death = spriteSet.animation("death");
    ASSERT_NE(nullptr, death);
    EXPECT_FALSE(death->loop);
    EXPECT_DOUBLE_EQ(140.0, death->frameDurationMs);

    EXPECT_EQ(nullptr, spriteSet.animation("walk"));
}
}

TEST(SpriteSetTests, SelectsResolutionFromLodDistanceInCells)
{
    const auto spriteSet = makeSpriteSet();

    EXPECT_EQ(256u, spriteSet.resolutionForDistance(1.5));
    EXPECT_EQ(128u, spriteSet.resolutionForDistance(4.0));
    EXPECT_EQ(64u, spriteSet.resolutionForDistance(20.0));
}

TEST(SpriteSetTests, SelectsNearestDirectionByAngle)
{
    const auto spriteSet = makeSpriteSet();

    const auto* direction = spriteSet.directionForAngle(80.0);

    ASSERT_NE(nullptr, direction);
    EXPECT_EQ("right", direction->name);
}

TEST(SpriteSetTests, FallsBackToLowerResolutionThenHigher)
{
    const auto spriteSet = makeSpriteSet();
    const auto* front = spriteSet.directionForAngle(0.0);
    ASSERT_NE(nullptr, front);

    EXPECT_EQ(64u, spriteSet.closestAvailableResolution(*front, 128));
    EXPECT_EQ(256u, spriteSet.closestAvailableResolution(*front, 512));
    EXPECT_EQ(64u, spriteSet.closestAvailableResolution(*front, 32));
}

TEST(SpriteSetTests, SpriteManagerResolvesFrameSelection)
{
    SpriteManager manager;
    EXPECT_TRUE(manager.addSpriteSet(makeSpriteSet()));

    const auto selection = manager.selectFrame("test", 0.0, 1.0);

    ASSERT_NE(nullptr, selection.spriteSet);
    ASSERT_NE(nullptr, selection.direction);
    ASSERT_NE(nullptr, selection.filePath);
    EXPECT_EQ("front", selection.direction->name);
    EXPECT_EQ(256u, selection.resolution);
    EXPECT_EQ("front_256.bmp", *selection.filePath);
}
