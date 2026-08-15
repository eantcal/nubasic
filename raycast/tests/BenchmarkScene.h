// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Licensed under the MIT License. See COPYING file in the project root.
//
// Shared, deterministic offscreen scene used by both the rendering benchmark
// (WinRayCastBenchmark) and the rendering regression test
// (RenderRegressionTests). The scene mirrors the real app's projection
// resolution (512x512), field of view (60 deg), cell size (512) and engine
// scale (250000) so the per-phase timings are representative, while staying
// fully synthetic (no Win32 / no PNG decode) so it runs inside the engine-only
// test target on any machine and produces a reproducible frame hash.

#pragma once

#include "RaycastEngine.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace bench {

// Mirror the shipped demo configuration (see src/app/WinRayCast.cpp).
constexpr int kProjResX = 512;
constexpr int kProjResY = 512;
constexpr int kVisualDeg = 60;
constexpr double kScale = 250000.0;
constexpr int kCellSize = 512;
constexpr int kMapDim = 16;

// Number of frames the regression guard hashes. Kept modest so ctest stays
// fast; the benchmark times many more frames on top of this.
constexpr int kRegressionFrames = 16;

// Texture keys. Wall/floor/ceiling/transparent live in the packed-cell range;
// the sprite key is deliberately above it (sprites may use extended keys).
constexpr MapCell::TextureResourceKey kWallKey = 0x01;
constexpr MapCell::TextureResourceKey kCeilKey = 0x02;
constexpr MapCell::TextureResourceKey kFloorKey = 0x03;
constexpr MapCell::TextureResourceKey kTranspKey = 0x04;
constexpr MapCell::TextureResourceKey kSpriteKey = 0x105;

inline std::shared_ptr<Texture> makeCheckerTexture(
    uint32_t size, Color a, Color b, uint32_t tile = 32)
{
    auto texture = std::make_shared<Texture>(size, size);
    for (uint32_t y = 0; y < size; ++y) {
        for (uint32_t x = 0; x < size; ++x) {
            const bool on = (((x / tile) + (y / tile)) & 1u) != 0u;
            texture->setPixel(x, y, on ? a : b);
        }
    }
    return texture;
}

inline std::shared_ptr<Texture> makeGradientTexture(uint32_t size)
{
    auto texture = std::make_shared<Texture>(size, size);
    for (uint32_t y = 0; y < size; ++y) {
        for (uint32_t x = 0; x < size; ++x) {
            texture->setPixel(
                x, y, makeColor((x * 255) / size, (y * 255) / size, 128));
        }
    }
    return texture;
}

// Mostly-opaque pane with a regular pattern of color-keyed (black) holes, so
// the transparent-wall pass has real compositing work to do.
inline std::shared_ptr<Texture> makeTransparentTexture(uint32_t size)
{
    auto texture = std::make_shared<Texture>(size, size);
    for (uint32_t y = 0; y < size; ++y) {
        for (uint32_t x = 0; x < size; ++x) {
            const bool hole = (((x / 32) + (y / 32)) & 1u) == 0u;
            texture->setPixel(
                x, y, hole ? makeColor(0, 0, 0) : makeColor(40, 80, 200));
        }
    }
    return texture;
}

inline std::shared_ptr<Texture> makeSpriteTexture(uint32_t size)
{
    auto texture = std::make_shared<Texture>(size, size);
    // Black is the color key; leave the border transparent and draw a blob.
    for (uint32_t y = size / 4; y < (3 * size) / 4; ++y) {
        for (uint32_t x = size / 3; x < (2 * size) / 3; ++x) {
            texture->setPixel(x, y, makeColor(220, 40, 40));
        }
    }
    return texture;
}

struct Scene {
    WorldMap map;
    std::vector<Sprite> sprites;
};

// A 16x16 room: solid wall border, a few interior pillars, one transparent
// wall, everything else open floor/ceiling — so a 360-degree sweep exercises
// the wall, floor, ceiling, sprite and transparent-wall passes.
inline Scene buildScene()
{
    Scene scene;

    const int dim = kMapDim;
    std::vector<Cell> cells(static_cast<size_t>(dim) * dim, 0);

    const Cell openCell = (static_cast<Cell>(kCeilKey) << 8)
        | (static_cast<Cell>(kFloorKey) << 16);
    const Cell wallCell = static_cast<Cell>(kWallKey);

    for (int r = 0; r < dim; ++r) {
        for (int c = 0; c < dim; ++c) {
            const bool border = (r == 0 || c == 0 || r == dim - 1 || c == dim - 1);
            const bool pillar = (!border && (r % 5) == 0 && (c % 5) == 0
                && !(r == dim / 2 && c == dim / 2));
            cells[static_cast<size_t>(r) * dim + c] = (border || pillar)
                ? wallCell
                : openCell;
        }
    }

    // A transparent wall in an open interior cell (visible from the centre).
    cells[static_cast<size_t>(8) * dim + 4] =
        openCell | (static_cast<Cell>(kTranspKey) << 24);

    scene.map.setCells(cells.data(), dim, dim);
    scene.map.resizeCell(kCellSize, kCellSize);

    scene.map.applyTexture(kWallKey,
        makeCheckerTexture(kCellSize, makeColor(180, 160, 120), makeColor(110, 90, 60)));
    scene.map.applyTexture(kCeilKey,
        makeCheckerTexture(kCellSize, makeColor(60, 60, 90), makeColor(40, 40, 70)));
    scene.map.applyTexture(kFloorKey, makeGradientTexture(kCellSize));
    scene.map.applyTexture(kTranspKey, makeTransparentTexture(kCellSize));
    scene.map.applyTexture(MapCell::TRANSPARENT_TEXTURE_KEY,
        makeCheckerTexture(kProjResX, makeColor(20, 24, 40), makeColor(28, 34, 55), 64));
    scene.map.applyTexture(kSpriteKey, makeSpriteTexture(256));

    auto makeSprite = [](double cx, double cy) {
        Sprite sprite;
        sprite.x = cx;
        sprite.y = cy;
        sprite.scale = 300.0;
        sprite.collisionRadius = 64.0;
        sprite.transparentColor = makeColor(0, 0, 0);
        sprite.frames = DirectionalSpriteFrames({ { kSpriteKey } });
        return sprite;
    };

    scene.sprites.push_back(makeSprite(7.5 * kCellSize, 9.5 * kCellSize));
    scene.sprites.push_back(makeSprite(9.5 * kCellSize, 7.5 * kCellSize));
    scene.sprites.push_back(makeSprite(10.5 * kCellSize, 10.5 * kCellSize));

    return scene;
}

inline Player makePlayer()
{
    Player player(0, 0, kVisualDeg, kProjResX, kProjResY);
    player.setPos({ 8.5 * kCellSize, 8.5 * kCellSize });
    player.setCenterProj(0.5);
    player.setSlope(0);
    return player;
}

struct CameraSample {
    double x = 0.0;
    double y = 0.0;
    int alpha = 0;
};

// A full 360-degree sweep with a small circular drift around the centre, so
// the path samples near and far walls, floor, ceiling and sprites. Fully
// deterministic (no time / no randomness).
inline std::vector<CameraSample> cameraPath(const Player& player, int frames)
{
    std::vector<CameraSample> path;
    if (frames <= 0) {
        return path;
    }

    path.reserve(static_cast<size_t>(frames));

    const double centre = 8.5 * kCellSize;
    const int deg360 = player.deg360();
    const double pi = 3.14159265358979323846;

    for (int i = 0; i < frames; ++i) {
        CameraSample sample;
        sample.alpha = static_cast<int>(
            (static_cast<long long>(deg360) * i) / frames) % deg360;
        const double angle = (2.0 * pi * i) / frames;
        sample.x = centre + (kCellSize * 0.75) * std::sin(angle);
        sample.y = centre + (kCellSize * 0.75) * std::cos(angle);
        path.push_back(sample);
    }

    return path;
}

// Render the whole path and fold every framebuffer pixel into an FNV-1a 64-bit
// hash. Identical output ⇒ identical hash, so this is the quality guard used by
// the regression test and reported by the benchmark.
inline uint64_t renderPathAndHash(
    RaycastEngine& engine,
    WorldMap& map,
    const std::vector<CameraSample>& path)
{
    uint64_t hash = 1469598103934665603ull; // FNV offset basis
    for (const auto& sample : path) {
        engine.player().setPos({ sample.x, sample.y });
        engine.player().setAlpha(sample.alpha);
        engine.renderToFrameBuffer(map, kProjResX, kProjResY);

        const auto& fb = engine.frameBuffer();
        const auto* pixels = fb.pixels();
        const size_t count = static_cast<size_t>(fb.width()) * fb.height();
        for (size_t i = 0; i < count; ++i) {
            hash ^= pixels[i];
            hash *= 1099511628211ull; // FNV prime
        }
    }
    return hash;
}

} // namespace bench
