#include "SpriteMetadataLoader.h"

#include <gtest/gtest.h>

#include <direct.h>

#include <fstream>
#include <string>

namespace {
const char* kDirectionNames[] = {
    "front",
    "front_right",
    "right",
    "back_right",
    "back",
    "back_left",
    "left",
    "front_left",
};

const int kDirectionAngles[] = {
    0, 45, 90, 135, 180, 225, 270, 315,
};

std::string fixtureDir()
{
    return "sprite_metadata_loader_fixtures";
}

void writeFile(const std::string& path, const std::string& content)
{
    std::ofstream file(path, std::ios::binary);
    file << content;
}

void ensureFixtureFiles()
{
    _mkdir(fixtureDir().c_str());

    for (const auto* directionName : kDirectionNames) {
        writeFile(fixtureDir() + "/" + directionName + "_64.bmp", "bmp");
        writeFile(fixtureDir() + "/" + directionName + "_128.bmp", "bmp");
        writeFile(fixtureDir() + "/" + directionName + "_64.png", "png");
        writeFile(fixtureDir() + "/" + directionName + "_128.png", "png");
    }
}

std::string makeMetadata(
    const std::string& format = "BMP",
    bool includeMissingFile = false,
    bool invalidAngle = false)
{
    std::string json =
        "{\n"
        "  \"spriteSet\": \"doom_style_monster\",\n"
        "  \"format\": \"" + format + "\",\n"
        "  \"transparentColor\": [0, 0, 0],\n"
        "  \"supportedResolutions\": [64, 128],\n"
        "  \"defaultResolution\": 128,\n"
        "  \"maxResolution\": 128,\n"
        "  \"directions\": [\n";

    for (int i = 0; i < 8; ++i) {
        const auto angle = invalidAngle && i == 0 ? 90 : kDirectionAngles[i];
        const auto extension = format == "PNG" ? ".png" : ".bmp";
        const auto file64 = includeMissingFile && i == 0
            ? std::string("missing_64") + extension
            : std::string(kDirectionNames[i]) + "_64" + extension;

        json +=
            "    {\n"
            "      \"name\": \"" + std::string(kDirectionNames[i]) + "\",\n"
            "      \"angle\": " + std::to_string(angle) + ",\n"
            "      \"files\": {\n"
            "        \"64\": \"" + file64 + "\",\n"
            "        \"128\": \"" + std::string(kDirectionNames[i]) + "_128" + extension + "\"\n"
            "      }\n"
            "    }";

        if (i != 7) {
            json += ",";
        }

        json += "\n";
    }

    json +=
        "  ],\n"
        "  \"lod\": [\n"
        "    { \"maxDistance\": 2.0, \"resolution\": 128 },\n"
        "    { \"maxDistance\": 9999.0, \"resolution\": 64 }\n"
        "  ]\n"
        "}\n";

    return json;
}

std::string makeDirectionsJson(const std::string& format)
{
    std::string json = "[\n";

    for (int i = 0; i < 8; ++i) {
        const auto extension = format == "PNG" ? ".png" : ".bmp";
        json +=
            "    {\n"
            "      \"name\": \"" + std::string(kDirectionNames[i]) + "\",\n"
            "      \"angle\": " + std::to_string(kDirectionAngles[i]) + ",\n"
            "      \"files\": {\n"
            "        \"64\": \"" + std::string(kDirectionNames[i]) + "_64" + extension + "\",\n"
            "        \"128\": \"" + std::string(kDirectionNames[i]) + "_128" + extension + "\"\n"
            "      }\n"
            "    }";

        if (i != 7) {
            json += ",";
        }

        json += "\n";
    }

    json += "  ]";
    return json;
}

std::string makeAnimatedMetadata()
{
    const auto idleDirections = makeDirectionsJson("PNG");
    const auto deathDirections = makeDirectionsJson("PNG");

    return
        "{\n"
        "  \"spriteSet\": \"animated_monster\",\n"
        "  \"format\": \"PNG\",\n"
        "  \"transparentColor\": [0, 0, 0],\n"
        "  \"supportedResolutions\": [64, 128],\n"
        "  \"defaultResolution\": 128,\n"
        "  \"maxResolution\": 128,\n"
        "  \"animations\": {\n"
        "    \"idle\": {\n"
        "      \"frameDurationMs\": 160,\n"
        "      \"loop\": true,\n"
        "      \"directions\": " + idleDirections + "\n"
        "    },\n"
        "    \"death\": {\n"
        "      \"frameDurationMs\": 140,\n"
        "      \"loop\": false,\n"
        "      \"directions\": " + deathDirections + "\n"
        "    }\n"
        "  },\n"
        "  \"lod\": [\n"
        "    { \"maxDistance\": 2.0, \"resolution\": 128 },\n"
        "    { \"maxDistance\": 9999.0, \"resolution\": 64 }\n"
        "  ]\n"
        "}\n";
}

std::string makeMultiFrameAnimatedMetadata()
{
    const auto idleDirections = makeDirectionsJson("PNG");
    const auto walkFrame0 = makeDirectionsJson("PNG");
    const auto walkFrame1 = makeDirectionsJson("PNG");

    return
        "{\n"
        "  \"spriteSet\": \"animated_monster\",\n"
        "  \"format\": \"PNG\",\n"
        "  \"transparentColor\": [0, 0, 0],\n"
        "  \"supportedResolutions\": [64, 128],\n"
        "  \"defaultResolution\": 128,\n"
        "  \"maxResolution\": 128,\n"
        "  \"animations\": {\n"
        "    \"idle\": {\n"
        "      \"frameDurationMs\": 160,\n"
        "      \"loop\": true,\n"
        "      \"directions\": " + idleDirections + "\n"
        "    },\n"
        "    \"walk\": {\n"
        "      \"frameDurationMs\": 120,\n"
        "      \"loop\": true,\n"
        "      \"frames\": [\n"
        "        { \"directions\": " + walkFrame0 + " },\n"
        "        { \"directions\": " + walkFrame1 + " }\n"
        "      ]\n"
        "    }\n"
        "  },\n"
        "  \"lod\": [\n"
        "    { \"maxDistance\": 2.0, \"resolution\": 128 },\n"
        "    { \"maxDistance\": 9999.0, \"resolution\": 64 }\n"
        "  ]\n"
        "}\n";
}

std::string writeMetadata(const std::string& fileName, const std::string& content)
{
    ensureFixtureFiles();
    const auto path = fixtureDir() + "/" + fileName;
    writeFile(path, content);
    return path;
}

bool hasErrorContaining(
    const std::vector<std::string>& errors,
    const std::string& text)
{
    for (const auto& error : errors) {
        if (error.find(text) != std::string::npos) {
            return true;
        }
    }

    return false;
}
}

TEST(SpriteMetadataLoaderTests, LoadsValidSpriteMetadata)
{
    const auto path = writeMetadata("valid.json", makeMetadata());

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    ASSERT_TRUE(result.success);
    EXPECT_EQ("doom_style_monster", result.spriteSet.name());
    EXPECT_EQ("BMP", result.spriteSet.format());
    EXPECT_EQ(makeColor(0, 0, 0), result.spriteSet.transparentColor());
    EXPECT_EQ(8u, result.spriteSet.directions().size());
    ASSERT_EQ(1u, result.spriteSet.animations().size());
    EXPECT_EQ("idle", result.spriteSet.animations()[0].name);
    EXPECT_EQ(2u, result.spriteSet.lodRules().size());
}

TEST(SpriteMetadataLoaderTests, LoadsNamedAnimationMetadata)
{
    const auto path = writeMetadata("animated.json", makeAnimatedMetadata());

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    ASSERT_TRUE(result.success)
        << (result.errors.empty() ? "" : result.errors.front());
    EXPECT_EQ("animated_monster", result.spriteSet.name());
    ASSERT_EQ(2u, result.spriteSet.animations().size());

    const auto* idle = result.spriteSet.animation("idle");
    ASSERT_NE(nullptr, idle);
    EXPECT_TRUE(idle->loop);
    EXPECT_DOUBLE_EQ(160.0, idle->frameDurationMs);
    EXPECT_EQ(8u, idle->directions.size());
    EXPECT_EQ(8u, result.spriteSet.directions().size());

    const auto* death = result.spriteSet.animation("death");
    ASSERT_NE(nullptr, death);
    EXPECT_FALSE(death->loop);
    EXPECT_DOUBLE_EQ(140.0, death->frameDurationMs);
    EXPECT_EQ(8u, death->directions.size());
    ASSERT_EQ(1u, death->frames.size());
    EXPECT_EQ(8u, death->frames[0].size());
}

TEST(SpriteMetadataLoaderTests, LoadsMultiFrameAnimationMetadata)
{
    const auto path = writeMetadata(
        "animated_multiframe.json",
        makeMultiFrameAnimatedMetadata());

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    ASSERT_TRUE(result.success)
        << (result.errors.empty() ? "" : result.errors.front());

    const auto* walk = result.spriteSet.animation("walk");
    ASSERT_NE(nullptr, walk);
    EXPECT_TRUE(walk->loop);
    EXPECT_DOUBLE_EQ(120.0, walk->frameDurationMs);
    ASSERT_EQ(2u, walk->frames.size());
    EXPECT_EQ(8u, walk->frames[0].size());
    EXPECT_EQ(8u, walk->frames[1].size());
    EXPECT_EQ(8u, walk->directions.size());
}

TEST(SpriteMetadataLoaderTests, RejectsAnimationsWithoutIdleClip)
{
    auto metadata = makeAnimatedMetadata();
    const std::string from = "\"idle\"";
    metadata.replace(metadata.find(from), from.size(), "\"walk\"");
    const auto path = writeMetadata("animated_missing_idle.json", metadata);

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    EXPECT_FALSE(result.success);
    EXPECT_TRUE(hasErrorContaining(result.errors, "idle clip"));
}

TEST(SpriteMetadataLoaderTests, ReportsUnsupportedFormat)
{
    const auto path = writeMetadata("unsupported_format.json", makeMetadata("GIF"));

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    EXPECT_FALSE(result.success);
    EXPECT_TRUE(hasErrorContaining(result.errors, "Unsupported sprite format"));
}

TEST(SpriteMetadataLoaderTests, LoadsPngSpriteMetadata)
{
    const auto path = writeMetadata("png_format.json", makeMetadata("PNG"));

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    ASSERT_TRUE(result.success);
    EXPECT_EQ("PNG", result.spriteSet.format());
    EXPECT_EQ(8u, result.spriteSet.directions().size());
}

TEST(SpriteMetadataLoaderTests, AcceptsResolutionUpTo1024)
{
    ensureFixtureFiles();
    for (const auto* directionName : kDirectionNames) {
        writeFile(fixtureDir() + "/" + directionName + "_1024.png", "png");
    }

    auto metadata = makeMetadata("PNG");
    const std::string from = "\"supportedResolutions\": [64, 128]";
    metadata.replace(metadata.find(from), from.size(), "\"supportedResolutions\": [64, 128, 1024]");
    const std::string maxFrom = "\"maxResolution\": 128";
    metadata.replace(metadata.find(maxFrom), maxFrom.size(), "\"maxResolution\": 1024");

    const auto path = writeMetadata("png_1024.json", metadata);
    const auto result = SpriteMetadataLoader().loadFromFile(path);

    EXPECT_TRUE(result.success) << (result.errors.empty() ? "" : result.errors.front());
}

TEST(SpriteMetadataLoaderTests, ReportsMissingReferencedFile)
{
    const auto path = writeMetadata("missing_file.json", makeMetadata("BMP", true));

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    EXPECT_FALSE(result.success);
    EXPECT_TRUE(hasErrorContaining(result.errors, "Missing sprite image file"));
}

TEST(SpriteMetadataLoaderTests, RejectsInvalidDirectionAngle)
{
    const auto path = writeMetadata("invalid_angle.json", makeMetadata("BMP", false, true));

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    EXPECT_FALSE(result.success);
    EXPECT_TRUE(hasErrorContaining(result.errors, "invalid angle"));
}

TEST(SpriteMetadataLoaderTests, LoadsShippedSheetBruteWalkAnimation)
{
    const auto path = std::string(NURAYCAST_DEMO_WORLD_DIR)
        + "/sprites/sheet_brute/sheet_brute.sprite.json";

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    ASSERT_TRUE(result.success)
        << (result.errors.empty() ? "" : result.errors.front());

    const auto* walk = result.spriteSet.animation("walk");
    ASSERT_NE(nullptr, walk);
    EXPECT_TRUE(walk->loop);
    EXPECT_DOUBLE_EQ(130.0, walk->frameDurationMs);
    ASSERT_EQ(4u, walk->frames.size());
    EXPECT_EQ(8u, walk->frames[0].size());
    EXPECT_EQ(8u, walk->frames[1].size());
}

TEST(SpriteMetadataLoaderTests, LoadsShippedMissileBruteAttackAndDeathAnimations)
{
    const auto path = std::string(NURAYCAST_DEMO_WORLD_DIR)
        + "/sprites/missile_brute/missile_brute.sprite.json";

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    ASSERT_TRUE(result.success)
        << (result.errors.empty() ? "" : result.errors.front());

    const auto* attack = result.spriteSet.animation("attack");
    ASSERT_NE(nullptr, attack);
    EXPECT_TRUE(attack->loop);
    EXPECT_DOUBLE_EQ(95.0, attack->frameDurationMs);
    ASSERT_EQ(3u, attack->frames.size());
    EXPECT_EQ(8u, attack->frames[0].size());
    EXPECT_EQ(8u, attack->frames[1].size());
    EXPECT_EQ(8u, attack->frames[2].size());

    const auto* death = result.spriteSet.animation("death");
    ASSERT_NE(nullptr, death);
    EXPECT_FALSE(death->loop);
    EXPECT_DOUBLE_EQ(120.0, death->frameDurationMs);
    ASSERT_EQ(9u, death->frames.size());
    EXPECT_EQ(8u, death->frames.front().size());
    EXPECT_EQ(8u, death->frames.back().size());
}

TEST(SpriteMetadataLoaderTests, LoadsShippedSoldierWalkAttackAndDeathAnimations)
{
    const auto path = std::string(NURAYCAST_DEMO_WORLD_DIR)
        + "/sprites/soldier/soldier.sprite.json";

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    ASSERT_TRUE(result.success)
        << (result.errors.empty() ? "" : result.errors.front());

    const auto* idle = result.spriteSet.animation("idle");
    ASSERT_NE(nullptr, idle);
    EXPECT_EQ(8u, idle->directions.size());

    const auto* walk = result.spriteSet.animation("walk");
    ASSERT_NE(nullptr, walk);
    EXPECT_TRUE(walk->loop);
    EXPECT_DOUBLE_EQ(125.0, walk->frameDurationMs);
    ASSERT_EQ(4u, walk->frames.size());
    EXPECT_EQ(8u, walk->frames.front().size());
    EXPECT_EQ(8u, walk->frames.back().size());

    const auto* attack = result.spriteSet.animation("attack");
    ASSERT_NE(nullptr, attack);
    EXPECT_TRUE(attack->loop);
    EXPECT_DOUBLE_EQ(90.0, attack->frameDurationMs);
    ASSERT_EQ(5u, attack->frames.size());
    EXPECT_EQ(8u, attack->frames.front().size());
    EXPECT_EQ(8u, attack->frames.back().size());

    const auto* death = result.spriteSet.animation("death");
    ASSERT_NE(nullptr, death);
    EXPECT_FALSE(death->loop);
    EXPECT_DOUBLE_EQ(115.0, death->frameDurationMs);
    ASSERT_EQ(14u, death->frames.size());
    EXPECT_EQ(8u, death->frames.front().size());
    EXPECT_EQ(8u, death->frames.back().size());
}

TEST(SpriteMetadataLoaderTests, LoadsShippedHoverSentinelDirectionalAnimations)
{
    const auto path = std::string(NURAYCAST_DEMO_WORLD_DIR)
        + "/sprites/hover_sentinel/hover_sentinel.sprite.json";

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    ASSERT_TRUE(result.success)
        << (result.errors.empty() ? "" : result.errors.front());

    const auto* idle = result.spriteSet.animation("idle");
    ASSERT_NE(nullptr, idle);
    ASSERT_EQ(1u, idle->frames.size());
    EXPECT_EQ(8u, idle->frames.front().size());

    const auto* attack = result.spriteSet.animation("attack");
    ASSERT_NE(nullptr, attack);
    EXPECT_TRUE(attack->loop);
    EXPECT_DOUBLE_EQ(90.0, attack->frameDurationMs);
    ASSERT_EQ(2u, attack->frames.size());
    EXPECT_EQ(8u, attack->frames.front().size());
    EXPECT_EQ(8u, attack->frames.back().size());
}

TEST(SpriteMetadataLoaderTests, LoadsShippedGuideRobotDirectionalAnimations)
{
    const auto path = std::string(NURAYCAST_DEMO_WORLD_DIR)
        + "/sprites/guide_robot/guide_robot.sprite.json";

    const auto result = SpriteMetadataLoader().loadFromFile(path);

    ASSERT_TRUE(result.success)
        << (result.errors.empty() ? "" : result.errors.front());

    const auto* idle = result.spriteSet.animation("idle");
    ASSERT_NE(nullptr, idle);
    ASSERT_EQ(1u, idle->frames.size());
    EXPECT_EQ(8u, idle->frames.front().size());

    const auto* walk = result.spriteSet.animation("walk");
    ASSERT_NE(nullptr, walk);
    EXPECT_TRUE(walk->loop);
    ASSERT_EQ(1u, walk->frames.size());
    EXPECT_EQ(8u, walk->frames.front().size());
    EXPECT_EQ(nullptr, result.spriteSet.animation("attack"));
}
