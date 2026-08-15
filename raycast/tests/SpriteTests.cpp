#include "Sprite.h"

#include <gtest/gtest.h>

TEST(SpriteTests, SelectsDirectionalViewFacingCamera)
{
    DirectionalSpriteFrames frames({
        { 1 }, { 2 }, { 3 }, { 4 },
        { 5 }, { 6 }, { 7 }, { 8 },
    });

    EXPECT_EQ(0u, frames.selectViewIndex(10.0, 0.0, 0.0, 0.0, 0.0));
    EXPECT_EQ(2u, frames.selectViewIndex(0.0, 10.0, 0.0, 0.0, 0.0));
    EXPECT_EQ(4u, frames.selectViewIndex(-10.0, 0.0, 0.0, 0.0, 0.0));
    EXPECT_EQ(6u, frames.selectViewIndex(0.0, -10.0, 0.0, 0.0, 0.0));
}

TEST(SpriteTests, SpriteFacingRotatesDirectionalView)
{
    DirectionalSpriteFrames frames({
        { 1 }, { 2 }, { 3 }, { 4 },
        { 5 }, { 6 }, { 7 }, { 8 },
    });

    const auto quarterTurn = 3.14159265358979323846 / 2.0;

    EXPECT_EQ(6u, frames.selectViewIndex(10.0, 0.0, 0.0, 0.0, quarterTurn));
}

TEST(SpriteTests, EmptyFrameSetReturnsNullFrame)
{
    DirectionalSpriteFrames frames;

    EXPECT_TRUE(frames.empty());
    EXPECT_EQ(nullptr, frames.frameForView(0));
    EXPECT_EQ(0u, frames.selectViewIndex(1.0, 0.0, 0.0, 0.0, 0.0));
}

TEST(SpriteTests, ActiveAnimationFallsBackToIdleThenLegacyFrames)
{
    Sprite sprite;
    sprite.frames = DirectionalSpriteFrames({ { 1 } });
    sprite.animations = {
        SpriteAnimationClip("idle", DirectionalSpriteFrames({ { 2 } }), {}, 160.0, true),
        SpriteAnimationClip("walk", DirectionalSpriteFrames({ { 3 } }), {}, 120.0, true),
    };

    EXPECT_TRUE(sprite.setAnimation("walk"));
    ASSERT_NE(nullptr, sprite.activeFrames().frameForView(0));
    EXPECT_EQ(3u, sprite.activeFrames().frameForView(0)->textureKey);

    sprite.activeAnimation = "missing";
    ASSERT_NE(nullptr, sprite.activeFrames().frameForView(0));
    EXPECT_EQ(2u, sprite.activeFrames().frameForView(0)->textureKey);

    sprite.animations.clear();
    ASSERT_NE(nullptr, sprite.activeFrames().frameForView(0));
    EXPECT_EQ(1u, sprite.activeFrames().frameForView(0)->textureKey);
}

TEST(SpriteTests, SwitchingAnimationResetsRuntimeClock)
{
    Sprite sprite;
    sprite.animations = {
        SpriteAnimationClip("idle", DirectionalSpriteFrames({ { 1 } }), {}, 160.0, true),
        SpriteAnimationClip("walk", DirectionalSpriteFrames({ { 2 } }), {}, 120.0, true),
    };

    sprite.setAnimation("idle");
    sprite.advanceAnimation(0.5);
    EXPECT_GT(sprite.animationTimeSeconds, 0.0);

    EXPECT_TRUE(sprite.setAnimation("walk"));
    EXPECT_DOUBLE_EQ(0.0, sprite.animationTimeSeconds);
    EXPECT_EQ(0u, sprite.animationFrameIndex);
}

TEST(SpriteTests, LoopingAnimationAdvancesFrameSets)
{
    Sprite sprite;
    sprite.animations = {
        SpriteAnimationClip(
            "walk",
            DirectionalSpriteFrames({ { 1 } }),
            {
                DirectionalSpriteFrames({ { 10 } }),
                DirectionalSpriteFrames({ { 20 } }),
            },
            100.0,
            true),
    };

    sprite.setAnimation("walk");
    sprite.advanceAnimation(0.11);
    ASSERT_NE(nullptr, sprite.activeFrames().frameForView(0));
    EXPECT_EQ(20u, sprite.activeFrames().frameForView(0)->textureKey);

    sprite.advanceAnimation(0.10);
    ASSERT_NE(nullptr, sprite.activeFrames().frameForView(0));
    EXPECT_EQ(10u, sprite.activeFrames().frameForView(0)->textureKey);
}

TEST(SpriteTests, NonLoopingAnimationClampsToLastFrameSet)
{
    Sprite sprite;
    sprite.animations = {
        SpriteAnimationClip(
            "death",
            DirectionalSpriteFrames({ { 1 } }),
            {
                DirectionalSpriteFrames({ { 10 } }),
                DirectionalSpriteFrames({ { 20 } }),
            },
            100.0,
            false),
    };

    sprite.setAnimation("death");
    sprite.advanceAnimation(1.0);
    ASSERT_NE(nullptr, sprite.activeFrames().frameForView(0));
    EXPECT_EQ(20u, sprite.activeFrames().frameForView(0)->textureKey);
    EXPECT_EQ(1u, sprite.animationFrameIndex);
}
