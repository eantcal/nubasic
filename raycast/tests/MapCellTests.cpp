#include "MapCell.h"

#include <gtest/gtest.h>

TEST(MapCellTests, ExtractsTextureKeysFromPackedCell)
{
    const Cell cell = 0x0a00090c0eULL;

    EXPECT_EQ(0x0e, MapCell::solidWallTexture(cell));
    EXPECT_EQ(0x0c, MapCell::ceilingTexture(cell));
    EXPECT_EQ(0x09, MapCell::floorTexture(cell));
    EXPECT_EQ(0x00, MapCell::transparentWallTexture(cell));
    EXPECT_EQ(0x0a, MapCell::upperWallTexture(cell));
}

TEST(MapCellTests, DetectsSolidAndTransparentWalls)
{
    EXPECT_FALSE(MapCell::hasAnyWall(0x0000000000ULL));
    EXPECT_TRUE(MapCell::hasAnyWall(0x0000000001ULL));
    EXPECT_TRUE(MapCell::hasAnyWall(0x0005000000ULL));

    EXPECT_TRUE(MapCell::hasSolidWall(0x0000000001ULL));
    EXPECT_FALSE(MapCell::hasSolidWall(0x0005000000ULL));

    EXPECT_FALSE(MapCell::hasTransparentWall(0x0000000001ULL));
    EXPECT_TRUE(MapCell::hasTransparentWall(0x0005000000ULL));
}

TEST(MapCellTests, DetectsTransparentTextureKey)
{
    EXPECT_TRUE(MapCell::isTransparentTexture(0xff));
    EXPECT_FALSE(MapCell::isTransparentTexture(0x01));
}
