#include "ColumnDepthBuffer.h"

#include <gtest/gtest.h>

TEST(ColumnDepthBufferTests, ResizeInitializesColumnsToFarDepth)
{
    ColumnDepthBuffer depthBuffer;

    depthBuffer.resize(3);

    EXPECT_EQ(3u, depthBuffer.width());
    EXPECT_EQ(ColumnDepthBuffer::farDepth(), depthBuffer.depth(0));
    EXPECT_EQ(ColumnDepthBuffer::farDepth(), depthBuffer.depth(1));
    EXPECT_EQ(ColumnDepthBuffer::farDepth(), depthBuffer.depth(2));
}

TEST(ColumnDepthBufferTests, SetNearestDepthKeepsNearestValue)
{
    ColumnDepthBuffer depthBuffer;
    depthBuffer.resize(1);

    depthBuffer.setNearestDepth(0, 100.0);
    depthBuffer.setNearestDepth(0, 150.0);
    depthBuffer.setNearestDepth(0, 25.0);

    EXPECT_EQ(25.0, depthBuffer.depth(0));
}

TEST(ColumnDepthBufferTests, OutOfRangeAccessIsSafe)
{
    ColumnDepthBuffer depthBuffer;
    depthBuffer.resize(1);

    depthBuffer.setDepth(5, 10.0);

    EXPECT_EQ(ColumnDepthBuffer::farDepth(), depthBuffer.depth(5));
}
