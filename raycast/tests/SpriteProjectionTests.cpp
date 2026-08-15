#include "SpriteProjection.h"

#include <gtest/gtest.h>

namespace {
SpriteProjectionInput defaultInput()
{
    SpriteProjectionInput input;
    input.screenWidth = 320;
    input.screenHeight = 200;
    input.projectionScaleRows = 200.0;
    input.worldCellHeight = 1.0;
    input.projectionCenter = 0.5;
    return input;
}
}

TEST(SpriteProjectionTests, ProjectsCenteredSpriteInFrontOfCamera)
{
    Sprite sprite;
    sprite.x = 10.0;
    sprite.y = 0.0;

    const auto projection = SpriteProjector::project(sprite, defaultInput());

    EXPECT_TRUE(projection.visible);
    EXPECT_DOUBLE_EQ(10.0, projection.depth);
    EXPECT_EQ(160, projection.centerColumn);
    EXPECT_LT(projection.leftColumn, projection.centerColumn);
    EXPECT_GT(projection.rightColumn, projection.centerColumn);
}

TEST(SpriteProjectionTests, RejectsSpriteBehindCamera)
{
    Sprite sprite;
    sprite.x = -10.0;
    sprite.y = 0.0;

    const auto projection = SpriteProjector::project(sprite, defaultInput());

    EXPECT_FALSE(projection.visible);
}

TEST(SpriteProjectionTests, UsesCameraFacingWhenProjecting)
{
    Sprite sprite;
    sprite.x = 0.0;
    sprite.y = 10.0;

    auto input = defaultInput();
    input.cameraFacingRadians = 3.14159265358979323846 / 2.0;

    const auto projection = SpriteProjector::project(sprite, input);

    EXPECT_TRUE(projection.visible);
    EXPECT_NEAR(10.0, projection.depth, 0.000001);
    EXPECT_EQ(160, projection.centerColumn);
}

TEST(SpriteProjectionTests, AppliesVerticalCameraOffset)
{
    Sprite sprite;
    sprite.x = 10.0;
    sprite.y = 0.0;

    auto input = defaultInput();
    const auto baseline = SpriteProjector::project(sprite, input);

    input.verticalCenterOffsetRows = 20.0;
    const auto shifted = SpriteProjector::project(sprite, input);

    EXPECT_TRUE(shifted.visible);
    EXPECT_EQ(baseline.topRow + 20, shifted.topRow);
    EXPECT_EQ(baseline.bottomRow + 20, shifted.bottomRow);
}

TEST(SpriteProjectionTests, ScaledSpriteStaysFloorAnchored)
{
    Sprite fullHeight;
    fullHeight.x = 10.0;
    fullHeight.y = 0.0;
    fullHeight.scale = 1.0;

    Sprite small = fullHeight;
    small.scale = 0.5;

    const auto fullProjection =
        SpriteProjector::project(fullHeight, defaultInput());
    const auto smallProjection =
        SpriteProjector::project(small, defaultInput());

    EXPECT_TRUE(fullProjection.visible);
    EXPECT_TRUE(smallProjection.visible);
    EXPECT_EQ(fullProjection.bottomRow, smallProjection.bottomRow);
    EXPECT_GT(smallProjection.topRow, fullProjection.topRow);
}

TEST(SpriteProjectionTests, PositiveVerticalOffsetLiftsSpriteAboveFloor)
{
    Sprite sprite;
    sprite.x = 10.0;
    sprite.y = 0.0;

    auto lifted = sprite;
    lifted.verticalOffset = 0.25;

    const auto baselineProjection =
        SpriteProjector::project(sprite, defaultInput());
    const auto liftedProjection =
        SpriteProjector::project(lifted, defaultInput());

    EXPECT_TRUE(baselineProjection.visible);
    EXPECT_TRUE(liftedProjection.visible);
    EXPECT_LT(liftedProjection.topRow, baselineProjection.topRow);
    EXPECT_LT(liftedProjection.bottomRow, baselineProjection.bottomRow);
}

TEST(SpriteProjectionTests, UsesRaycastProjectionScaleAndWorldCellHeight)
{
    Sprite sprite;
    sprite.x = 100.0;
    sprite.y = 0.0;
    sprite.scale = 256.0;

    auto input = defaultInput();
    input.projectionScaleRows = 250000.0;
    input.worldCellHeight = 512.0;

    const auto projection = SpriteProjector::project(sprite, input);

    EXPECT_TRUE(projection.visible);
    EXPECT_EQ(100.0, projection.depth);
    EXPECT_EQ(1250, projection.bottomRow - projection.topRow);
}

TEST(SpriteProjectionTests, UsesProjectionCenterForFloorAnchor)
{
    Sprite sprite;
    sprite.x = 10.0;
    sprite.y = 0.0;
    sprite.scale = 1.0;

    auto lowAnchorInput = defaultInput();
    lowAnchorInput.projectionCenter = 0.1;

    auto highAnchorInput = defaultInput();
    highAnchorInput.projectionCenter = 0.9;

    const auto lowAnchorProjection =
        SpriteProjector::project(sprite, lowAnchorInput);
    const auto highAnchorProjection =
        SpriteProjector::project(sprite, highAnchorInput);

    EXPECT_TRUE(lowAnchorProjection.visible);
    EXPECT_TRUE(highAnchorProjection.visible);
    EXPECT_GT(
        highAnchorProjection.bottomRow,
        lowAnchorProjection.bottomRow);
}

TEST(SpriteProjectionTests, KeepsSpriteVisibleWhileProjectedBoundsOverlapScreen)
{
    Sprite sprite;
    sprite.x = 10.0;
    sprite.y = 6.2;
    sprite.scale = 2.0;

    auto input = defaultInput();
    const auto projection = SpriteProjector::project(sprite, input);

    EXPECT_GT(projection.horizontalOffset, 1.0);
    EXPECT_GE(projection.leftColumn, 0);
    EXPECT_LT(projection.leftColumn, static_cast<int>(input.screenWidth));
    EXPECT_GT(projection.rightColumn, static_cast<int>(input.screenWidth));
    EXPECT_TRUE(projection.visible);
}

TEST(SpriteProjectionTests, DetectsVisibleColumnsAgainstDepthBuffer)
{
    Sprite sprite;
    sprite.x = 10.0;
    sprite.y = 0.0;

    const auto projection = SpriteProjector::project(sprite, defaultInput());

    ColumnDepthBuffer depthBuffer;
    depthBuffer.resize(320);
    depthBuffer.setDepth(160, 20.0);

    EXPECT_TRUE(SpriteProjector::hasVisibleColumn(projection, depthBuffer));

    for (uint32_t column = 0; column < depthBuffer.width(); ++column) {
        depthBuffer.setDepth(column, 5.0);
    }

    EXPECT_FALSE(SpriteProjector::hasVisibleColumn(projection, depthBuffer));
}
