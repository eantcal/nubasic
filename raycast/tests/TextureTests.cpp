#include "Texture.h"

#include <gtest/gtest.h>

TEST(TextureTests, FillBufferWrapsSourceIntoDestination)
{
    Texture texture(2, 2);
    texture.setPixel(0, 0, 0x00000001);
    texture.setPixel(1, 0, 0x00000002);
    texture.setPixel(0, 1, 0x00000003);
    texture.setPixel(1, 1, 0x00000004);

    Texture::Pixel dest[9] = {};

    texture.fillBuffer(dest, 3, 3, 0, 2);

    EXPECT_EQ(0x00000001u, dest[0]);
    EXPECT_EQ(0x00000002u, dest[1]);
    EXPECT_EQ(0x00000001u, dest[2]);
    EXPECT_EQ(0x00000003u, dest[3]);
    EXPECT_EQ(0x00000004u, dest[4]);
    EXPECT_EQ(0x00000003u, dest[5]);
    EXPECT_EQ(0x00000001u, dest[6]);
    EXPECT_EQ(0x00000002u, dest[7]);
    EXPECT_EQ(0x00000001u, dest[8]);
}

TEST(TextureTests, FillBufferAppliesHorizontalOffset)
{
    Texture texture(3, 1);
    texture.setPixel(0, 0, 0x00000001);
    texture.setPixel(1, 0, 0x00000002);
    texture.setPixel(2, 0, 0x00000003);

    Texture::Pixel dest[3] = {};

    texture.fillBuffer(dest, 3, 1, 1, 3);

    EXPECT_EQ(0x00000002u, dest[0]);
    EXPECT_EQ(0x00000003u, dest[1]);
    EXPECT_EQ(0x00000001u, dest[2]);
}
