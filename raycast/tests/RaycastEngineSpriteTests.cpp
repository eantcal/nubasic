#include "RaycastEngine.h"
#include "SpriteProjection.h"

#include <gtest/gtest.h>

#include <memory>
#include <utility>
#include <vector>

namespace {
constexpr Cell kEmpty = 0x0000000000ULL;
constexpr Cell kWall = 0x0000000001ULL;
constexpr MapCell::TextureResourceKey kSpriteTexture = 0x180;

std::shared_ptr<Texture> makeSpriteTexture()
{
    auto texture = std::make_shared<Texture>(8, 8);

    for (uint32_t y = 1; y < 7; ++y) {
        for (uint32_t x = 2; x < 6; ++x) {
            texture->setPixel(x, y, makeColor(220, 32, 32));
        }
    }

    return texture;
}

std::shared_ptr<Texture> makeFilledTexture(Color color, uint32_t width = 8, uint32_t height = 8)
{
    auto texture = std::make_shared<Texture>(width, height);
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            texture->setPixel(x, y, color);
        }
    }

    return texture;
}

std::shared_ptr<Texture> makeHorizontalGradientTexture(uint32_t width = 64, uint32_t height = 64)
{
    auto texture = std::make_shared<Texture>(width, height);
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            texture->setPixel(x, y, makeColor(x, x, x));
        }
    }

    return texture;
}

std::shared_ptr<Texture> makeAlphaByteColorKeySpriteTexture()
{
    auto texture = std::make_shared<Texture>(8, 8);
    const auto transparentMagenta =
        static_cast<Color>(0xff000000u) | makeColor(255, 0, 255);

    for (uint32_t y = 0; y < 8; ++y) {
        for (uint32_t x = 0; x < 8; ++x) {
            texture->setPixel(x, y, transparentMagenta);
        }
    }

    for (uint32_t y = 2; y < 6; ++y) {
        for (uint32_t x = 2; x < 6; ++x) {
            texture->setPixel(x, y, makeColor(220, 32, 32));
        }
    }

    return texture;
}

WorldMap makeOpenMap()
{
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
    };

    WorldMap map;
    EXPECT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);
    map.applyTexture(kSpriteTexture, makeSpriteTexture());
    return map;
}

WorldMap makeOpenMapWithAlphaByteColorKeySprite()
{
    auto map = makeOpenMap();
    map.applyTexture(kSpriteTexture, makeAlphaByteColorKeySpriteTexture());
    return map;
}

WorldMap makeBlockedMap()
{
    const Cell cells[] = {
        kEmpty, kEmpty, kWall,
        kEmpty, kEmpty, kWall,
        kEmpty, kEmpty, kWall,
    };

    WorldMap map;
    EXPECT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);
    map.applyTexture(kSpriteTexture, makeSpriteTexture());
    return map;
}

WorldMap makePerFaceWallMap()
{
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kWall,
        kEmpty, kEmpty, kEmpty,
    };

    WorldMap map;
    EXPECT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);
    map.applyTexture(1, makeFilledTexture(makeColor(220, 0, 0), 64, 64));
    map.applyTexture(2, makeFilledTexture(makeColor(0, 220, 0), 64, 64));

    nu::rcade::BlockDefinition block;
    nu::rcade::WallSpan span;
    span.textureKey = 1;
    span.faceTextureKeys[static_cast<size_t>(nu::rcade::WallFace::West)] = 2;
    span.bottom = 0;
    span.top = 64;
    span.kind = nu::rcade::WallSpanKind::Solid;
    span.collision = true;
    block.walls.push_back(span);
    map.setBlockDefinition(1, block);
    map.setBlockId(1, 2, 1);
    return map;
}

Sprite makeTestSprite()
{
    Sprite sprite;
    sprite.x = 96.0;
    sprite.y = 96.0;
    sprite.scale = 32.0;
    sprite.collisionRadius = 8.0;
    sprite.frames = DirectionalSpriteFrames({ { kSpriteTexture } });
    return sprite;
}

Sprite makeAnimatedSprite(
    MapCell::TextureResourceKey firstFrame,
    MapCell::TextureResourceKey secondFrame)
{
    auto sprite = makeTestSprite();
    sprite.animations = {
        SpriteAnimationClip(
            "walk",
            DirectionalSpriteFrames({ { firstFrame } }),
            {
                DirectionalSpriteFrames({ { firstFrame } }),
                DirectionalSpriteFrames({ { secondFrame } }),
            },
            100.0,
            true),
    };
    sprite.setAnimation("walk");
    return sprite;
}
}

TEST(RaycastEngineSpriteTests, RendersDistantSpriteWithDepthShading)
{
    auto map = makeOpenMap();

    Player player(0, 0, 60, 60, 40);
    player.setPos({ 96.0, 96.0 });

    RaycastEngine engine(player, 250000);

    auto sprite = makeTestSprite();
    sprite.x = 4096.0;

    engine.addSprite(sprite);
    engine.renderToFrameBuffer(map, 60, 40);

    bool foundShadedSpritePixel = false;
    for (uint32_t y = 0; y < engine.frameBuffer().height(); ++y) {
        for (uint32_t x = 0; x < engine.frameBuffer().width(); ++x) {
            const auto pixel = engine.frameBuffer().pixel(x, y);
            if (colorRed(pixel) > colorGreen(pixel)
                && colorRed(pixel) > 0
                && colorRed(pixel) < 220) {
                foundShadedSpritePixel = true;
                break;
            }
        }
    }

    EXPECT_TRUE(foundShadedSpritePixel);
}

TEST(RaycastEngineSpriteTests, LayerBrightnessScalesDistantSpriteColor)
{
    auto map = makeOpenMap();
    auto sprite = makeTestSprite();
    sprite.x = 4096.0;

    auto maxRenderedRed = [&](double brightness) {
        Player player(0, 0, 60, 60, 40);
        player.setPos({ 96.0, 96.0 });
        RaycastEngine engine(player, 250000);
        engine.setBrightness(brightness);
        engine.addSprite(sprite);
        engine.renderToFrameBuffer(map, 60, 40);

        uint8_t maxRed = 0;
        for (uint32_t y = 0; y < engine.frameBuffer().height(); ++y) {
            for (uint32_t x = 0; x < engine.frameBuffer().width(); ++x) {
                maxRed = std::max(maxRed, colorRed(engine.frameBuffer().pixel(x, y)));
            }
        }
        return maxRed;
    };

    const auto neutralRed = maxRenderedRed(1.0);
    const auto dimmedRed = maxRenderedRed(0.5);
    EXPECT_GT(neutralRed, 0);
    EXPECT_LT(dimmedRed, neutralRed);
}

TEST(RaycastEngineSpriteTests, VisibleSpriteKeepsOpaqueColumnDepthUnchanged)
{
    auto map = makeOpenMap();

    Player player(0, 0, 60, 60, 40);
    player.setPos({ 96.0, 96.0 });

    RaycastEngine engine(player, 250000);

    auto sprite = makeTestSprite();
    sprite.x = 4096.0;

    engine.addSprite(sprite);
    engine.renderToFrameBuffer(map, 60, 40);

    bool foundOpaqueDepth = false;
    for (uint32_t column = 0; column < engine.columnDepthBuffer().width(); ++column) {
        if (engine.columnDepthBuffer().depth(column) != ColumnDepthBuffer::farDepth()) {
            foundOpaqueDepth = true;
            break;
        }
    }

    EXPECT_FALSE(foundOpaqueDepth);
}

TEST(RaycastEngineSpriteTests, StructuredWallUsesFaceTextureForHitSide)
{
    auto map = makePerFaceWallMap();

    Player player(0, 0, 60, 60, 40);
    player.setPos({ 96.0, 96.0 });

    RaycastEngine engine(player, 4096);
    engine.setDepthShadingLevel(1.0);
    engine.renderToFrameBuffer(map, 60, 40);

    bool foundWestFacePixel = false;
    bool foundDefaultFacePixel = false;
    for (uint32_t y = 0; y < engine.frameBuffer().height(); ++y) {
        for (uint32_t x = 0; x < engine.frameBuffer().width(); ++x) {
            const auto pixel = engine.frameBuffer().pixel(x, y);
            foundWestFacePixel = foundWestFacePixel
                || (colorGreen(pixel) > colorRed(pixel)
                    && colorGreen(pixel) > colorBlue(pixel));
            foundDefaultFacePixel = foundDefaultFacePixel
                || (colorRed(pixel) > colorGreen(pixel)
                    && colorRed(pixel) > colorBlue(pixel));
        }
    }

    EXPECT_TRUE(foundWestFacePixel);
    EXPECT_FALSE(foundDefaultFacePixel);
}

TEST(RaycastEngineSpriteTests, TransparentWallFacingOutOfMapIsNotProjected)
{
    constexpr Cell transparentWall = static_cast<Cell>(1) << 24;
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, transparentWall,
        kEmpty, kEmpty, kEmpty,
    };

    WorldMap map;
    ASSERT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);
    const auto warningColor = makeColor(240, 20, 20);
    map.applyTexture(1, makeFilledTexture(warningColor, 64, 64));
    nu::rcade::BlockDefinition block;
    nu::rcade::WallSpan outwardSpan;
    outwardSpan.textureKey = 1;
    outwardSpan.bottom = 0;
    outwardSpan.top = 64;
    outwardSpan.kind = nu::rcade::WallSpanKind::Transparent;
    outwardSpan.facesEnabled.fill(false);
    outwardSpan.facesEnabled[static_cast<size_t>(nu::rcade::WallFace::East)] = true;
    block.walls.push_back(outwardSpan);
    block.hasAnyTransparentSpan = true;
    map.setBlockDefinition(1, block);
    map.setBlockId(1, 2, 1);

    Player player(0, 0, 60, 80, 50);
    player.setPos({ 96.0, 96.0 });
    player.setAlpha(player.deg360() - player.degHalfVisual());

    RaycastEngine engine(player, 4096);
    engine.setDepthShadingLevel(1.0);
    engine.renderToFrameBuffer(map, 80, 50);

    int warningPixels = 0;
    for (uint32_t y = 0; y < engine.frameBuffer().height(); ++y) {
        for (uint32_t x = 0; x < engine.frameBuffer().width(); ++x) {
            warningPixels += engine.frameBuffer().pixel(x, y) == warningColor ? 1 : 0;
        }
    }
    EXPECT_EQ(0, warningPixels);
}

TEST(RaycastEngineSpriteTests, SpriteColorKeyIgnoresUnusedAlphaByte)
{
    auto map = makeOpenMapWithAlphaByteColorKeySprite();

    Player player(0, 0, 60, 60, 40);
    player.setPos({ 96.0, 96.0 });

    RaycastEngine engine(player, 250000);

    auto sprite = makeTestSprite();
    sprite.x = 4096.0;
    sprite.transparentColor = makeColor(255, 0, 255);

    engine.addSprite(sprite);
    engine.renderToFrameBuffer(map, 60, 40);

    bool foundSpritePixel = false;
    bool foundColorKeyPixel = false;
    for (uint32_t y = 0; y < engine.frameBuffer().height(); ++y) {
        for (uint32_t x = 0; x < engine.frameBuffer().width(); ++x) {
            const auto pixel = engine.frameBuffer().pixel(x, y);
            foundSpritePixel = foundSpritePixel
                || (colorRed(pixel) > colorGreen(pixel)
                    && colorGreen(pixel) > 0
                    && colorRed(pixel) < 220);
            foundColorKeyPixel = foundColorKeyPixel
                || (colorRed(pixel) > 0
                    && colorGreen(pixel) == 0
                    && colorBlue(pixel) > 0);
        }
    }

    EXPECT_TRUE(foundSpritePixel);
    EXPECT_FALSE(foundColorKeyPixel);
}

TEST(RaycastEngineSpriteTests, MovesSpriteWhenTargetCellIsOpen)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    RaycastEngine engine(player, 250000);

    engine.addSprite(makeTestSprite());

    EXPECT_TRUE(engine.moveSprite(0, 16.0, 0.0, map));
    ASSERT_EQ(1u, engine.sprites().size());
    EXPECT_DOUBLE_EQ(112.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
}

TEST(RaycastEngineSpriteTests, BlocksSpriteMovementIntoSolidWall)
{
    auto map = makeBlockedMap();
    Player player(0, 0, 60, 60, 40);
    RaycastEngine engine(player, 250000);

    engine.addSprite(makeTestSprite());

    EXPECT_FALSE(engine.moveSprite(0, 40.0, 0.0, map));
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
}

TEST(RaycastEngineSpriteTests, BlocksSpriteCornerClippingNearOpenDoor)
{
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kWall,
    };

    WorldMap map;
    ASSERT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);
    map.applyTexture(kSpriteTexture, makeSpriteTexture());

    Sprite sprite = makeTestSprite();
    sprite.x = 96.0;
    sprite.y = 96.0;
    sprite.collisionRadius = 18.0;

    Player player(0, 0, 60, 60, 40);
    RaycastEngine engine(player, 250000);
    engine.addSprite(sprite);

    EXPECT_FALSE(engine.moveSprite(0, 28.0, 28.0, map));
    ASSERT_EQ(1u, engine.sprites().size());
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
}

TEST(RaycastEngineSpriteTests, CanMoveSpriteThroughWallsWhenCollisionIsDisabled)
{
    auto map = makeBlockedMap();
    Player player(0, 0, 60, 60, 40);
    RaycastEngine engine(player, 250000);

    engine.addSprite(makeTestSprite());

    EXPECT_TRUE(engine.moveSprite(
        0,
        40.0,
        0.0,
        map,
        RaycastEngine::SpriteCollisionMode::PassThroughWalls));
    EXPECT_DOUBLE_EQ(136.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
}

TEST(RaycastEngineSpriteTests, RejectsInvalidSpriteIndex)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    RaycastEngine engine(player, 250000);

    EXPECT_FALSE(engine.moveSprite(0, 1.0, 0.0, map));
}

TEST(RaycastEngineSpriteTests, SpriteTexturesCanUseKeysBeyondPackedCellRange)
{
    WorldMap map;
    map.applyTexture(0x01, makeFilledTexture(makeColor(10, 20, 30)));
    map.applyTexture(0x101, makeFilledTexture(makeColor(220, 32, 32)));

    const auto* wallTexture = map.getTexture(0x01);
    ASSERT_NE(nullptr, wallTexture);
    EXPECT_EQ(makeColor(10, 20, 30), wallTexture->getPixel(0, 0));

    const auto* spriteTexture = map.getTexture(0x101);
    ASSERT_NE(nullptr, spriteTexture);
    EXPECT_EQ(makeColor(220, 32, 32), spriteTexture->getPixel(0, 0));
}

TEST(RaycastEngineSpriteTests, AdvancesSpriteAnimationsExceptExcludedActors)
{
    Player player(0, 0, 60, 60, 40);
    RaycastEngine engine(player, 250000);

    engine.addSprite(makeAnimatedSprite(10, 20));
    engine.addSprite(makeAnimatedSprite(30, 40));

    engine.advanceSpriteAnimations(0.11, std::vector<size_t>{ 1 });

    ASSERT_EQ(2u, engine.sprites().size());
    EXPECT_EQ(1u, engine.sprites()[0].animationFrameIndex);
    EXPECT_EQ(0u, engine.sprites()[1].animationFrameIndex);

    ASSERT_NE(nullptr, engine.sprites()[0].activeFrames().frameForView(0));
    EXPECT_EQ(20u, engine.sprites()[0].activeFrames().frameForView(0)->textureKey);

    ASSERT_NE(nullptr, engine.sprites()[1].activeFrames().frameForView(0));
    EXPECT_EQ(30u, engine.sprites()[1].activeFrames().frameForView(0)->textureKey);
}

TEST(RaycastEngineSpriteTests, PlayerSlopeIsClampedToAvoidExtremeTilt)
{
    Player player(0, 0, 60, 60, 80);

    player.setSlope(1000);
    EXPECT_EQ(26, player.getSlope());

    player.setSlope(-1000);
    EXPECT_EQ(-26, player.getSlope());
}

TEST(RaycastEngineSpriteTests, SpriteProjectionKeepsFeetAnchoredWhenPlayerHeightChanges)
{
    auto sprite = makeTestSprite();
    sprite.x = 128.0;
    sprite.y = 0.0;
    sprite.scale = 64.0;

    SpriteProjectionInput input;
    input.cameraX = 0.0;
    input.cameraY = 0.0;
    input.cameraFacingRadians = 0.0;
    input.fieldOfViewRadians = 3.14159265358979323846 / 3.0;
    input.screenWidth = 80;
    input.screenHeight = 50;
    input.verticalCenterOffsetRows = 0.0;
    input.projectionScaleRows = 1600.0;
    input.worldCellHeight = 64.0;

    input.projectionCenter = 0.25;
    const auto crouched = SpriteProjector::project(sprite, input);

    input.projectionCenter = 0.75;
    const auto tiptoe = SpriteProjector::project(sprite, input);

    ASSERT_TRUE(crouched.visible);
    ASSERT_TRUE(tiptoe.visible);
    EXPECT_GT(tiptoe.bottomRow, crouched.bottomRow);
    EXPECT_GT(tiptoe.topRow, crouched.topRow);
}

TEST(RaycastEngineSpriteTests, FloorAndCeilingProjectionFollowPlayerHeight)
{
    constexpr Cell floorAndCeiling =
        (static_cast<Cell>(3) << 8)
        | (static_cast<Cell>(2) << 16);
    constexpr Cell wall = 1;

    const Cell cells[] = {
        floorAndCeiling, floorAndCeiling, floorAndCeiling, wall,
        floorAndCeiling, floorAndCeiling, floorAndCeiling, wall,
        floorAndCeiling, floorAndCeiling, floorAndCeiling, wall,
        floorAndCeiling, floorAndCeiling, floorAndCeiling, wall,
    };

    WorldMap map;
    ASSERT_TRUE(map.setCells(cells, 4, 4));
    map.resizeCell(64, 64);
    map.applyTexture(1, makeFilledTexture(makeColor(80, 80, 80), 64, 64));
    map.applyTexture(2, makeHorizontalGradientTexture());
    map.applyTexture(3, makeHorizontalGradientTexture());

    Player player(0, 0, 60, 80, 50);
    player.setPos({ 96.0, 96.0 });
    player.setAlpha(player.deg360() - player.degHalfVisual());
    player.setCenterProj(0.75);

    RaycastEngine engine(player, 1600);
    engine.setDepthShadingLevel(1.0);
    engine.renderToFrameBuffer(map, 80, 50);

    // Center ray faces east. At screen y=40 the floor sample distance must use
    // camera height above the floor: 1600 * 0.75 / (25 - 10) = 80, x = 176.
    EXPECT_EQ(48, colorRed(engine.frameBuffer().pixel(40, 40)));

    // At screen y=10 the ceiling distance must use the remaining room height:
    // 1600 * 0.25 / (25 - 10) = 26.6, x = 122.
    EXPECT_EQ(58, colorRed(engine.frameBuffer().pixel(40, 10)));

    // The wall top must use the same ceiling-side projection. With the old
    // floor-side offset, this row was already covered by the wall.
    EXPECT_EQ(34, colorRed(engine.frameBuffer().pixel(40, 19)));
    EXPECT_EQ(80, colorRed(engine.frameBuffer().pixel(40, 22)));
}

TEST(RaycastEngineSpriteTests, ViewWeaponUsesProjectionViewportForPlacement)
{
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty,
    };

    WorldMap map;
    ASSERT_TRUE(map.setCells(cells, 3, 3));
    map.resizeCell(64, 64);

    Player player(0, 0, 60, 80, 50);
    player.setPos({ 96.0, 96.0 });

    RaycastEngine engine(player, 250000);

    ViewWeapon weapon;
    ViewWeapon::Animation idle;
    idle.name = "idle";
    idle.frames = { makeFilledTexture(makeColor(22, 44, 66)) };
    idle.frameDurationMs = 100.0;
    weapon.addAnimation(idle);
    weapon.setScreenHeightFraction(0.2);
    weapon.setAnchor(0.5, 1.0);
    ASSERT_TRUE(weapon.setAnimation("idle"));

    engine.setViewWeapon(std::move(weapon));
    engine.renderToFrameBuffer(map, 160, 100);

    EXPECT_EQ(makeColor(22, 44, 66), engine.frameBuffer().pixel(40, 45));
    EXPECT_NE(makeColor(22, 44, 66), engine.frameBuffer().pixel(80, 95));
}
