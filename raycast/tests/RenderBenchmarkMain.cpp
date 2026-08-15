// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Licensed under the MIT License. See COPYING file in the project root.
//
// Offscreen rendering benchmark. Renders a fixed, deterministic scene
// (see BenchmarkScene.h) over a camera path entirely into the engine's
// FrameBuffer — no window, no DirectDraw — and reports a per-phase timing
// breakdown plus the frame hash used by the regression guard.
//
// Run (Release recommended for meaningful numbers):
//   out\build\vs2026-x64\Release\WinRayCastBenchmark.exe [frames]

#include "BenchmarkScene.h"
#include "RaycastEngine.h"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

struct PhaseAverages {
    double sky = 0.0;
    double wallsFloorCeiling = 0.0;
    double sprites = 0.0;
    double transparentWalls = 0.0;
    double viewWeapon = 0.0;
    double total = 0.0;
};

void printRow(const char* name, double ms, double totalMs)
{
    const double pct = totalMs > 0.0 ? (100.0 * ms / totalMs) : 0.0;
    std::printf("  %-26s %9.4f ms   %5.1f%%\n", name, ms, pct);
}

} // namespace

int main(int argc, char** argv)
{
    int benchFrames = 600;
    if (argc > 1) {
        const int requested = std::atoi(argv[1]);
        if (requested > 0) {
            benchFrames = requested;
        }
    }
    constexpr int warmupFrames = 30;

    auto scene = bench::buildScene();
    Player player = bench::makePlayer();
    RaycastEngine engine(player, bench::kScale);
    for (const auto& sprite : scene.sprites) {
        engine.addSprite(sprite);
    }

    const auto warmupPath = bench::cameraPath(engine.player(), warmupFrames);
    const auto benchPath = bench::cameraPath(engine.player(), benchFrames);

    // Warmup (page in textures/buffers, let the CPU clock settle).
    for (const auto& sample : warmupPath) {
        engine.player().setPos({ sample.x, sample.y });
        engine.player().setAlpha(sample.alpha);
        engine.renderToFrameBuffer(scene.map, bench::kProjResX, bench::kProjResY);
    }

    PhaseAverages sum;
    double minTotal = 1e30;
    double maxTotal = 0.0;

    const auto wallStart = std::chrono::steady_clock::now();
    for (const auto& sample : benchPath) {
        engine.player().setPos({ sample.x, sample.y });
        engine.player().setAlpha(sample.alpha);
        engine.renderToFrameBuffer(scene.map, bench::kProjResX, bench::kProjResY);

        const auto& p = engine.lastRenderProfile();
        sum.sky += p.skyMs;
        sum.wallsFloorCeiling += p.wallsFloorCeilingMs;
        sum.sprites += p.spritesMs;
        sum.transparentWalls += p.transparentWallsMs;
        sum.viewWeapon += p.viewWeaponMs;
        sum.total += p.totalMs;
        if (p.totalMs < minTotal) minTotal = p.totalMs;
        if (p.totalMs > maxTotal) maxTotal = p.totalMs;
    }
    const auto wallEnd = std::chrono::steady_clock::now();

    const double n = static_cast<double>(benchFrames);
    const double avgTotal = sum.total / n;
    const double wallMs =
        std::chrono::duration<double, std::milli>(wallEnd - wallStart).count();

    std::printf("WinRayCast offscreen render benchmark\n");
    std::printf("  resolution      : %dx%d (projection), cell %d, FOV %d deg\n",
        bench::kProjResX, bench::kProjResY, bench::kCellSize, bench::kVisualDeg);
    std::printf("  scale           : %.0f\n", bench::kScale);
    std::printf("  frames          : %d (after %d warmup)\n",
        benchFrames, warmupFrames);
    std::printf("\n");
    std::printf("Per-frame averages (engine instrumentation):\n");
    printRow("sky fill", sum.sky / n, avgTotal);
    printRow("walls + floor + ceiling", sum.wallsFloorCeiling / n, avgTotal);
    printRow("sprites", sum.sprites / n, avgTotal);
    printRow("transparent walls (x2)", sum.transparentWalls / n, avgTotal);
    printRow("view weapon", sum.viewWeapon / n, avgTotal);
    const double accounted = sum.sky + sum.wallsFloorCeiling + sum.sprites
        + sum.transparentWalls + sum.viewWeapon;
    printRow("setup / depth clear", (sum.total - accounted) / n, avgTotal);
    std::printf("  %-26s %9.4f ms\n", "TOTAL (avg)", avgTotal);
    std::printf("\n");
    std::printf("Frame time: avg %.4f ms  min %.4f ms  max %.4f ms  => %.1f FPS\n",
        avgTotal, minTotal, maxTotal, avgTotal > 0.0 ? 1000.0 / avgTotal : 0.0);
    std::printf("Wall-clock loop: %.2f ms total, %.4f ms/frame\n",
        wallMs, wallMs / n);

    // Regression hash over the exact frames the regression test checks.
    RaycastEngine hashEngine(player, bench::kScale);
    for (const auto& sprite : scene.sprites) {
        hashEngine.addSprite(sprite);
    }
    const auto hashPath =
        bench::cameraPath(hashEngine.player(), bench::kRegressionFrames);
    const uint64_t hash =
        bench::renderPathAndHash(hashEngine, scene.map, hashPath);
    std::printf("\nRegression frame hash (%d frames): 0x%016llxULL\n",
        bench::kRegressionFrames, static_cast<unsigned long long>(hash));

    return 0;
}
