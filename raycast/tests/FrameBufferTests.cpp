#include "FrameBuffer.h"

#include <gtest/gtest.h>

TEST(FrameBufferTests, ResizeSetsDimensionsAndPitch)
{
    FrameBuffer frameBuffer;

    frameBuffer.resize(320, 200);

    EXPECT_FALSE(frameBuffer.empty());
    EXPECT_EQ(320u, frameBuffer.width());
    EXPECT_EQ(200u, frameBuffer.height());
    EXPECT_EQ(320u * sizeof(FrameBuffer::Pixel), frameBuffer.pitchBytes());
    EXPECT_EQ(320u * 200u * sizeof(FrameBuffer::Pixel), frameBuffer.byteSize());
}

TEST(FrameBufferTests, SetPixelClipsOutsideBounds)
{
    FrameBuffer frameBuffer(2, 2);

    frameBuffer.setPixel(1, 1, 0x00aabbcc);
    frameBuffer.setPixel(2, 1, 0x000000ff);
    frameBuffer.setPixel(1, 2, 0x000000ff);

    EXPECT_EQ(0x00aabbccu, frameBuffer.pixel(1, 1));
    EXPECT_EQ(0u, frameBuffer.pixel(2, 1));
    EXPECT_EQ(0u, frameBuffer.pixel(1, 2));
}
