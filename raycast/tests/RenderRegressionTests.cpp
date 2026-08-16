// This file is part of nuRCADE (New (nu) Raycasting Classic Arcade Development Engine).
// Licensed under the MIT License. See COPYING file in the project root.
//
// Visual-regression guard for the software renderer. Renders the shared
// deterministic benchmark scene over a fixed camera path and hashes every
// framebuffer pixel. Any optimization that is meant to preserve output must
// keep this hash unchanged; a changed hash means the rendered image changed.
//
// The golden hash is produced by nuRCADEBenchmark on this toolchain. If the
// renderer is intentionally changed (or the toolchain changes the
// floating-point result), re-run the benchmark and update kGoldenFrameHash.

#include "BenchmarkScene.h"
#include "RaycastEngine.h"

#include <gtest/gtest.h>

#include <cstdint>

namespace {

// Baseline produced by nuRCADEBenchmark (Release, MSVC x64).
constexpr uint64_t kGoldenFrameHash = 0x55062e8779065d7eULL;

uint64_t renderBenchmarkHash()
{
    auto scene = bench::buildScene();
    Player player = bench::makePlayer();
    RaycastEngine engine(player, bench::kScale);
    for (const auto& sprite : scene.sprites) {
        engine.addSprite(sprite);
    }

    const auto path = bench::cameraPath(engine.player(), bench::kRegressionFrames);
    return bench::renderPathAndHash(engine, scene.map, path);
}

} // namespace

TEST(RenderRegressionTests, BenchmarkSceneFrameHashIsStable)
{
    const uint64_t hash = renderBenchmarkHash();
    EXPECT_EQ(kGoldenFrameHash, hash)
        << "Rendered output changed. If this change is intentional, re-run "
           "nuRCADEBenchmark and update kGoldenFrameHash to 0x" << std::hex
        << hash << "ULL.";
}

TEST(RenderRegressionTests, BenchmarkSceneHashIsDeterministic)
{
    EXPECT_EQ(renderBenchmarkHash(), renderBenchmarkHash());
}

TEST(RenderRegressionTests, BenchmarkSceneProducesVariedOutput)
{
    auto scene = bench::buildScene();
    Player player = bench::makePlayer();
    RaycastEngine engine(player, bench::kScale);
    for (const auto& sprite : scene.sprites) {
        engine.addSprite(sprite);
    }

    engine.renderToFrameBuffer(scene.map, bench::kProjResX, bench::kProjResY);

    // A correctly rendered frame is neither blank nor a single flat color.
    const auto& fb = engine.frameBuffer();
    const auto first = fb.pixel(0, 0);
    bool foundDifferent = false;
    for (uint32_t y = 0; y < fb.height() && !foundDifferent; ++y) {
        for (uint32_t x = 0; x < fb.width(); ++x) {
            if (fb.pixel(x, y) != first) {
                foundDifferent = true;
                break;
            }
        }
    }
    EXPECT_TRUE(foundDifferent);

    const auto& profile = engine.lastRenderProfile();
    EXPECT_GE(profile.totalMs, 0.0);
}
