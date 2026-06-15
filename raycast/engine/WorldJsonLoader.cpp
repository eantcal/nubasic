// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "WorldJsonLoader.h"

#include "MapCell.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

using WinRayCast::BlockDefinition;
using WinRayCast::BlockId;
using WinRayCast::BlockSurface;
using WinRayCast::DoorDefinition;
using WinRayCast::WallFace;
using WinRayCast::WallSpan;
using WinRayCast::WallSpanKind;

std::string readString(
    const nlohmann::json& node, const char* name, const std::string& fallback)
{
    if (!node.contains(name) || !node[name].is_string()) {
        return fallback;
    }

    return node[name].get<std::string>();
}

int readInt(const nlohmann::json& node, const char* name, int fallback)
{
    if (!node.contains(name) || !node[name].is_number()) {
        return fallback;
    }

    return node[name].get<int>();
}

int clampInt(int value, int minValue, int maxValue) noexcept
{
    return (std::min)(maxValue, (std::max)(minValue, value));
}

bool readBool(const nlohmann::json& node, const char* name, bool fallback)
{
    if (!node.contains(name) || !node[name].is_boolean()) {
        return fallback;
    }

    return node[name].get<bool>();
}

double readDouble(const nlohmann::json& node, const char* name, double fallback)
{
    if (!node.contains(name) || !node[name].is_number()) {
        return fallback;
    }

    return node[name].get<double>();
}

std::string normalizeDoorKey(std::string key)
{
    std::string normalized;
    normalized.reserve(key.size());
    for (char ch : key) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            normalized.push_back(static_cast<char>(
                std::tolower(static_cast<unsigned char>(ch))));
        }
    }

    return normalized;
}

bool endsWithIgnoreCase(const std::string& value, const std::string& suffix)
{
    if (suffix.size() > value.size()) {
        return false;
    }

    return std::equal(
        suffix.rbegin(), suffix.rend(), value.rbegin(), [](char lhs, char rhs) {
            return std::tolower(static_cast<unsigned char>(lhs))
                == std::tolower(static_cast<unsigned char>(rhs));
        });
}

std::string normalizeTextureResource(std::string resource)
{
    const auto bmpSuffix = std::string(".bmp");
    if (endsWithIgnoreCase(resource, bmpSuffix)) {
        resource.resize(resource.size() - bmpSuffix.size());
    }

    return resource;
}

bool tryParseHexByte(const std::string& text, BlockId& out)
{
    if (text.empty() || text.size() > 2) {
        return false;
    }

    for (char c : text) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    try {
        const auto value = std::stoul(text, nullptr, 16);
        if (value > 0xff) {
            return false;
        }

        out = static_cast<BlockId>(value);
        return true;
    } catch (...) {
        return false;
    }
}

bool tryParsePackedCell(const std::string& text, Cell& out)
{
    if (text.empty() || text.size() > 16) {
        return false;
    }

    for (char c : text) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    try {
        out = static_cast<Cell>(std::stoull(text, nullptr, 16));
        return true;
    } catch (...) {
        return false;
    }
}

bool tryParseWallFace(const std::string& text, WallFace& out) noexcept
{
    std::string lower;
    lower.reserve(text.size());
    for (char c : text) {
        lower.push_back(
            static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }

    // JSON uses geometric face labels: "north" means the north side of the
    // block, visible from the cell immediately north of it. Raycast hits use
    // the same geometric face, so keep the mapping direct.
    if (lower == "north") {
        out = WallFace::North;
        return true;
    }

    if (lower == "east") {
        out = WallFace::East;
        return true;
    }

    if (lower == "south") {
        out = WallFace::South;
        return true;
    }

    if (lower == "west") {
        out = WallFace::West;
        return true;
    }

    return false;
}

bool tryParseAnimationFace(const std::string& text, int& out) noexcept
{
    std::string lower;
    lower.reserve(text.size());
    for (char c : text) {
        lower.push_back(
            static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }

    if (lower.empty() || lower == "all") {
        out = -1;
        return true;
    }

    WallFace face = WallFace::North;
    if (!tryParseWallFace(lower, face)) {
        return false;
    }

    out = static_cast<int>(face);
    return true;
}

BlockSurface buildSurface(const nlohmann::json& node, const char* key)
{
    BlockSurface surface;
    if (!node.contains(key) || !node[key].is_object()) {
        return surface;
    }

    BlockId textureKey = 0;
    const auto textureText = readString(node[key], "texture", std::string());
    if (!textureText.empty() && tryParseHexByte(textureText, textureKey)) {
        surface.textureKey = textureKey;
        surface.present = true;
    }

    surface.height = readInt(node[key], "height", 0);
    return surface;
}

WallSpan buildSpan(const nlohmann::json& node, int defaultWallHeight)
{
    WallSpan span;
    BlockId textureKey = 0;
    const auto textureText = readString(node, "texture", std::string());
    if (!textureText.empty() && tryParseHexByte(textureText, textureKey)) {
        span.textureKey = textureKey;
    }

    if (node.contains("faceTextures") && node["faceTextures"].is_object()) {
        for (const auto& faceEntry : node["faceTextures"].items()) {
            WallFace face = WallFace::North;
            BlockId faceTextureKey = 0;
            if (tryParseWallFace(faceEntry.key(), face)
                && faceEntry.value().is_string()
                && tryParseHexByte(
                    faceEntry.value().get<std::string>(), faceTextureKey)) {
                span.faceTextureKeys[static_cast<size_t>(face)]
                    = faceTextureKey;
            }
        }
    }

    if (node.contains("facesEnabled") && node["facesEnabled"].is_object()) {
        for (const auto& faceEntry : node["facesEnabled"].items()) {
            WallFace face = WallFace::North;
            if (tryParseWallFace(faceEntry.key(), face)
                && faceEntry.value().is_boolean()) {
                span.facesEnabled[static_cast<size_t>(face)]
                    = faceEntry.value().get<bool>();
            }
        }
    }

    const auto interiorTextureText
        = readString(node, "interiorTexture", std::string());
    if (!interiorTextureText.empty()) {
        BlockId interiorKey = 0;
        if (tryParseHexByte(interiorTextureText, interiorKey)) {
            span.interiorTextureKey = interiorKey;
        }
    }

    span.bottom = readInt(node, "bottom", 0);
    span.top = readInt(node, "top", defaultWallHeight);
    if (node.contains("passable") && node["passable"].is_boolean()) {
        span.collision = !readBool(node, "passable", false);
    } else {
        span.collision = readBool(node, "collision", true);
    }

    const auto kindText = readString(node, "kind", std::string("solid"));
    std::string lower;
    lower.reserve(kindText.size());
    for (char c : kindText) {
        lower.push_back(
            static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }

    if (lower == "transparent") {
        span.kind = WallSpanKind::Transparent;
    } else {
        span.kind = WallSpanKind::Solid;
    }

    return span;
}

DoorDefinition buildDoor(const nlohmann::json& blockNode,
    const std::string& blockKey, WorldJsonLoader::Result& result)
{
    DoorDefinition door;
    if (!blockNode.contains("door")) {
        return door;
    }

    if (!blockNode["door"].is_object()) {
        result.errors.push_back(
            "Block '" + blockKey + "' has an invalid door object.");
        return door;
    }

    const auto& doorNode = blockNode["door"];
    door.enabled = readBool(doorNode, "enabled", true);
    door.blocksWhenClosed = readBool(doorNode, "blocksWhenClosed", true);
    door.requiredKey = readString(
        doorNode, "requiredKey", readString(doorNode, "key", door.requiredKey));
    door.triggerDistanceCells = readDouble(
        doorNode, "triggerDistanceCells", door.triggerDistanceCells);
    door.openTimeSeconds
        = readDouble(doorNode, "openTimeSeconds", door.openTimeSeconds);
    door.closeDelaySeconds
        = readDouble(doorNode, "closeDelaySeconds", door.closeDelaySeconds);
    door.openSound = readString(doorNode, "openSound",
        readString(doorNode, "openingSound", door.openSound));
    door.openSoundVolumePercent = readInt(
        doorNode, "openSoundVolumePercent", door.openSoundVolumePercent);
    if (door.openSoundVolumePercent < 0) {
        door.openSoundVolumePercent = 0;
    } else if (door.openSoundVolumePercent > 100) {
        door.openSoundVolumePercent = 100;
    }

    if (doorNode.contains("frames") && doorNode["frames"].is_array()) {
        for (const auto& frameNode : doorNode["frames"]) {
            if (!frameNode.is_string()) {
                result.errors.push_back("Block '" + blockKey
                    + "' has a door frame that is not a texture id.");
                continue;
            }

            BlockId frameKey = 0;
            const auto frameText = frameNode.get<std::string>();
            if (!tryParseHexByte(frameText, frameKey)) {
                result.errors.push_back("Block '" + blockKey
                    + "' has an invalid door frame texture id '" + frameText
                    + "'.");
                continue;
            }

            door.animationTextureKeys.push_back(frameKey);
        }
    }

    const auto readLockedOverlays = [&](const char* propertyName) {
        if (!doorNode.contains(propertyName)) {
            return;
        }

        if (!doorNode[propertyName].is_object()) {
            result.errors.push_back("Block '" + blockKey
                + "' has an invalid door " + std::string(propertyName)
                + " object.");
            return;
        }

        for (const auto& overlayEntry : doorNode[propertyName].items()) {
            const auto normalizedKey = normalizeDoorKey(overlayEntry.key());
            if (normalizedKey.empty()) {
                result.errors.push_back(
                    "Block '" + blockKey + "' has an empty door overlay key.");
                continue;
            }

            if (!overlayEntry.value().is_string()) {
                result.errors.push_back("Block '" + blockKey
                    + "' has a door overlay that is not a texture id.");
                continue;
            }

            BlockId overlayKey = 0;
            const auto overlayText = overlayEntry.value().get<std::string>();
            if (!tryParseHexByte(overlayText, overlayKey)) {
                result.errors.push_back("Block '" + blockKey
                    + "' has an invalid door overlay texture id '" + overlayText
                    + "'.");
                continue;
            }

            door.lockedOverlayTextureKeysByKey[normalizedKey] = overlayKey;
        }
    };

    readLockedOverlays("lockedOverlays");
    readLockedOverlays("overlayByKey");

    if (door.openTimeSeconds <= 0.0) {
        result.errors.push_back("Block '" + blockKey
            + "' has a door openTimeSeconds value that must be positive.");
        door.openTimeSeconds = 0.45;
    }

    if (door.triggerDistanceCells < 0.0) {
        result.errors.push_back("Block '" + blockKey
            + "' has a negative door triggerDistanceCells value.");
        door.triggerDistanceCells = 0.0;
    }

    if (door.closeDelaySeconds < 0.0) {
        result.errors.push_back("Block '" + blockKey
            + "' has a negative door closeDelaySeconds value.");
        door.closeDelaySeconds = 0.0;
    }

    return door;
}

void buildBlockAnimations(const nlohmann::json& blockNode,
    const std::string& blockKey, BlockDefinition& block,
    WorldJsonLoader::Result& result)
{
    if (!blockNode.contains("animations")) {
        return;
    }

    if (!blockNode["animations"].is_array()) {
        result.errors.push_back(
            "Block '" + blockKey + "' has an invalid animations array.");
        return;
    }

    for (const auto& animationNode : blockNode["animations"]) {
        if (!animationNode.is_object()) {
            result.errors.push_back("Block '" + blockKey
                + "' has an animation that is not an object.");
            continue;
        }

        auto target = readString(animationNode, "target", "wall");
        std::transform(
            target.begin(), target.end(), target.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });

        const auto isWallBase = target == "wall";
        const auto isWallOverlay
            = target == "walloverlay" || target == "overlay";
        if (!isWallBase && !isWallOverlay) {
            continue;
        }

        const auto wallIndex = readInt(animationNode, "wallIndex", -1);
        if (wallIndex < 0
            || wallIndex >= static_cast<int>(block.walls.size())) {
            result.errors.push_back("Block '" + blockKey
                + "' has an animation with an invalid wall index.");
            continue;
        }

        int face = -1;
        if (!tryParseAnimationFace(
                readString(animationNode, "face", "all"), face)) {
            result.errors.push_back("Block '" + blockKey
                + "' has an animation with an invalid face.");
            continue;
        }

        WinRayCast::WallTextureAnimation animation;
        animation.face = face;
        animation.frameDurationSeconds = std::max(0.001,
            readDouble(animationNode, "frameDurationMs", 120.0) / 1000.0);
        animation.loop = readBool(animationNode, "loop", true);

        if (!animationNode.contains("frames")
            || !animationNode["frames"].is_array()) {
            result.errors.push_back("Block '" + blockKey
                + "' has a wall animation without frames.");
            continue;
        }

        for (const auto& frameNode : animationNode["frames"]) {
            if (!frameNode.is_string()) {
                result.errors.push_back("Block '" + blockKey
                    + "' has an animation frame that is not a texture id.");
                continue;
            }

            BlockId frameKey = 0;
            const auto frameText = frameNode.get<std::string>();
            if (!tryParseHexByte(frameText, frameKey)) {
                result.errors.push_back("Block '" + blockKey
                    + "' has an invalid animation frame texture id '"
                    + frameText + "'.");
                continue;
            }

            animation.textureKeys.push_back(frameKey);
        }

        if (animation.textureKeys.empty()) {
            result.errors.push_back("Block '" + blockKey
                + "' has a wall animation with no valid frames.");
            continue;
        }

        auto& wall = block.walls[static_cast<size_t>(wallIndex)];
        if (isWallOverlay) {
            wall.overlayAnimations.push_back(std::move(animation));
        } else {
            wall.baseAnimations.push_back(std::move(animation));
        }
    }
}

const nlohmann::json* selectWorldLayer(const nlohmann::json& document,
    const std::string& requestedLayerId, std::string& activeLayerId,
    WorldJsonLoader::Result& result)
{
    activeLayerId.clear();
    if (!document.contains("layers")) {
        return &document;
    }

    if (!document["layers"].is_array()) {
        result.errors.push_back("World JSON layers must be an array.");
        return nullptr;
    }

    const auto& layers = document["layers"];
    if (layers.empty()) {
        result.errors.push_back("World JSON layers array must not be empty.");
        return nullptr;
    }

    auto selectedLayerId = requestedLayerId;
    if (selectedLayerId.empty()) {
        selectedLayerId = readString(document, "activeLayer", std::string());
    }

    if (selectedLayerId.empty()) {
        selectedLayerId = readString(document, "startLayer", std::string());
    }

    const nlohmann::json* fallbackLayer = nullptr;
    std::string fallbackLayerId;
    for (const auto& layer : layers) {
        if (!layer.is_object()) {
            result.errors.push_back("Each world layer must be a JSON object.");
            continue;
        }

        const auto id = readString(layer, "id", std::string());
        if (fallbackLayer == nullptr) {
            fallbackLayer = &layer;
            fallbackLayerId = id;
        }

        if (!selectedLayerId.empty() && id == selectedLayerId) {
            activeLayerId = id;
            return &layer;
        }
    }

    if (!selectedLayerId.empty()) {
        result.errors.push_back(
            "World JSON does not contain layer '" + selectedLayerId + "'.");
        return nullptr;
    }

    activeLayerId = fallbackLayerId;
    return fallbackLayer;
}

Cell packCellFromBlock(const BlockDefinition& block, int defaultWallHeight)
{
    Cell packed = 0;

    if (block.floor.present) {
        packed |= static_cast<Cell>(block.floor.textureKey) << 16;
    }

    if (block.ceiling.present) {
        packed |= static_cast<Cell>(block.ceiling.textureKey) << 8;
    }

    for (const auto& wall : block.walls) {
        const auto textureKey = wall.textureForFace(WallFace::North);
        if (wall.kind == WallSpanKind::Transparent) {
            packed |= static_cast<Cell>(textureKey) << 24;
        } else if (wall.bottom >= defaultWallHeight) {
            packed |= static_cast<Cell>(textureKey) << 32;
        } else if ((packed & 0xff) == 0) {
            packed |= static_cast<Cell>(textureKey);
        }
    }

    return packed;
}

} // namespace

WorldJsonLoader::Result WorldJsonLoader::loadFromFile(
    const std::string& path, WorldMap& target, const std::string& layerId) const
{
    Result result;

    std::ifstream input(path);
    if (!input.is_open()) {
        result.errors.push_back("Cannot open world JSON file: " + path);
        return result;
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    return loadFromString(buffer.str(), target, layerId);
}

WorldJsonLoader::Result WorldJsonLoader::loadFromString(
    const std::string& jsonText, WorldMap& target,
    const std::string& layerId) const
{
    Result result;

    nlohmann::json document;
    try {
        document = nlohmann::json::parse(jsonText);
    } catch (const std::exception& error) {
        result.errors.push_back(
            std::string("Invalid world JSON: ") + error.what());
        return result;
    }

    if (!document.is_object()) {
        result.errors.push_back("World JSON root must be an object.");
        return result;
    }

    const auto format = readString(document, "format", std::string());
    if (format != "winraycast.world") {
        result.errors.push_back("Unsupported world format: " + format);
        return result;
    }

    const auto version = readInt(document, "version", 0);
    if (version != 2) {
        result.errors.push_back(
            "Unsupported world version: " + std::to_string(version));
        return result;
    }

    const auto* layer
        = selectWorldLayer(document, layerId, result.activeLayerId, result);
    if (layer == nullptr) {
        return result;
    }

    const auto& worldNode = *layer;
    const auto* gridNode = &document;
    if (worldNode.contains("grid") && worldNode["grid"].is_object()) {
        gridNode = &worldNode;
    }

    if (!gridNode->contains("grid") || !(*gridNode)["grid"].is_object()) {
        result.errors.push_back("World JSON missing grid object.");
        return result;
    }

    const auto& grid = (*gridNode)["grid"];
    const auto columns = readInt(grid, "columns", 0);
    const auto rows = readInt(grid, "rows", 0);
    const auto cellWidth = readInt(grid, "cellWidth", 0);
    const auto cellDepth = readInt(grid, "cellDepth", 0);
    const auto defaultWallHeight
        = readInt(grid, "defaultWallHeight", cellDepth);
    if (columns <= 0 || rows <= 0 || cellWidth <= 0 || cellDepth <= 0) {
        result.errors.push_back("Invalid grid dimensions.");
        return result;
    }

    const auto* playerStartNode = &document;
    if (worldNode.contains("playerStart")
        && worldNode["playerStart"].is_object()) {
        playerStartNode = &worldNode;
    }

    if (playerStartNode->contains("playerStart")
        && (*playerStartNode)["playerStart"].is_object()) {
        const auto& playerStart = (*playerStartNode)["playerStart"];
        target.setPlayerStartCell(readDouble(playerStart, "xCell", 1.5),
            readDouble(playerStart, "yCell", 1.5),
            readDouble(playerStart, "facingDegrees", 0.0));
    }

    const auto* playerTurnNode = &document;
    if (worldNode.contains("playerTurn")
        && worldNode["playerTurn"].is_object()) {
        playerTurnNode = &worldNode;
    }

    if (playerTurnNode->contains("playerTurn")
        && (*playerTurnNode)["playerTurn"].is_object()) {
        const auto& playerTurn = (*playerTurnNode)["playerTurn"];
        // Non-positive (or missing) values leave the WorldMap defaults in
        // place.
        target.setPlayerTurnConfig(
            readDouble(playerTurn, "baseDegreesPerSecond", 0.0),
            readDouble(playerTurn, "maxDegreesPerSecond", 0.0),
            readDouble(playerTurn, "accelerationDegreesPerSecondSquared", 0.0));
    }

    const auto* messageLogNode = &document;
    if (worldNode.contains("messageLog")
        && worldNode["messageLog"].is_object()) {
        messageLogNode = &worldNode;
    }

    if (messageLogNode->contains("messageLog")
        && (*messageLogNode)["messageLog"].is_object()) {
        const auto& messageLog = (*messageLogNode)["messageLog"];
        auto& log = target.messageLog();
        log.enabled = readBool(messageLog, "enabled", log.enabled);
        log.maxLines
            = clampInt(readInt(messageLog, "maxLines", log.maxLines), 1, 4);
        log.keyPickup = readString(messageLog, "keyPickup", log.keyPickup);
        log.ammoPickup = readString(messageLog, "ammoPickup", log.ammoPickup);
        log.healthPickup
            = readString(messageLog, "healthPickup", log.healthPickup);
        log.mapUnlocked
            = readString(messageLog, "mapUnlocked", log.mapUnlocked);
        log.mapActorsUnlocked = readString(
            messageLog, "mapActorsUnlocked", log.mapActorsUnlocked);
        log.weaponReload
            = readString(messageLog, "weaponReload", log.weaponReload);
        log.itemPickup = readString(messageLog, "itemPickup", log.itemPickup);
    }

    // Texture mapping (textureKey hex -> file name without .bmp)
    auto& textureList = target.getTextureList();
    textureList.clear();
    if (document.contains("textures") && document["textures"].is_object()) {
        for (const auto& entry : document["textures"].items()) {
            const auto& key = entry.key();
            if (!entry.value().is_object()) {
                continue;
            }

            const auto file = readString(entry.value(), "file", std::string());
            const auto name = readString(entry.value(), "name", std::string());
            std::string resolved = file.empty() ? name : file;
            textureList[key] = normalizeTextureResource(std::move(resolved));
        }
    }

    auto defaultHorizonImage
        = readString(document, "defaultHorizonImage", std::string());
    defaultHorizonImage
        = readString(worldNode, "defaultHorizonImage", defaultHorizonImage);
    if (!defaultHorizonImage.empty()) {
        textureList["ff"] = normalizeTextureResource(defaultHorizonImage);
    }

    if (!document.contains("blocks") || !document["blocks"].is_object()) {
        result.errors.push_back("World JSON missing blocks object.");
        return result;
    }

    BlockDefinition parsedBlocks[256]{};
    bool parsedPresent[256]{};
    Cell packedCellOverrides[256]{};
    bool hasPackedCellOverride[256]{};

    for (const auto& entry : document["blocks"].items()) {
        BlockId blockId = 0;
        if (!tryParseHexByte(entry.key(), blockId)) {
            result.errors.push_back("Invalid block id: " + entry.key());
            continue;
        }

        if (!entry.value().is_object()) {
            result.errors.push_back(
                "Block '" + entry.key() + "' must be an object.");
            continue;
        }

        BlockDefinition block;
        block.name = readString(entry.value(), "name", std::string());
        block.horizonImage
            = readString(entry.value(), "horizonImage", std::string());

        const auto legacyPackedCell
            = readString(entry.value(), "legacyPackedCell", std::string());
        if (!legacyPackedCell.empty()) {
            Cell packedCell = 0;
            if (!tryParsePackedCell(legacyPackedCell, packedCell)) {
                result.errors.push_back("Block '" + entry.key()
                    + "' has an invalid legacyPackedCell.");
                continue;
            }

            packedCellOverrides[blockId] = packedCell;
            hasPackedCellOverride[blockId] = true;
        }

        block.floor = buildSurface(entry.value(), "floor");
        block.ceiling = buildSurface(entry.value(), "ceiling");

        if (entry.value().contains("walls")
            && entry.value()["walls"].is_array()) {
            for (const auto& wallNode : entry.value()["walls"]) {
                if (!wallNode.is_object()) {
                    continue;
                }

                auto span = buildSpan(wallNode, defaultWallHeight);
                if (span.top <= span.bottom) {
                    result.errors.push_back("Block '" + entry.key()
                        + "' has an invalid wall span.");
                    continue;
                }

                if (span.kind == WallSpanKind::Solid) {
                    block.hasAnySolidSpan = true;
                } else {
                    block.hasAnyTransparentSpan = true;
                }

                if (span.collision && span.bottom < cellDepth) {
                    block.hasAnyCollidingSpan = true;
                }

                block.walls.push_back(span);
            }
        }

        block.door = buildDoor(entry.value(), entry.key(), result);
        if (block.door.enabled && block.door.animationTextureKeys.empty()
            && !block.walls.empty()) {
            block.door.animationTextureKeys.push_back(
                block.walls.front().textureKey);
        }
        buildBlockAnimations(entry.value(), entry.key(), block, result);

        parsedBlocks[blockId] = std::move(block);
        parsedPresent[blockId] = true;
    }

    if (!worldNode.contains("cells") || !worldNode["cells"].is_array()) {
        result.errors.push_back("World JSON missing cells array.");
        return result;
    }

    const auto& cellRows = worldNode["cells"];
    if (static_cast<int>(cellRows.size()) != rows) {
        result.errors.push_back("Cell row count does not match grid.rows.");
        return result;
    }

    std::vector<Cell> packedCells;
    packedCells.reserve(
        static_cast<size_t>(rows) * static_cast<size_t>(columns));
    std::vector<std::vector<BlockId>> blockIds(
        rows, std::vector<BlockId>(columns, 0));

    for (int row = 0; row < rows; ++row) {
        if (!cellRows[row].is_array()
            || static_cast<int>(cellRows[row].size()) != columns) {
            result.errors.push_back("Cell row " + std::to_string(row)
                + " does not match grid.columns.");
            return result;
        }

        for (int column = 0; column < columns; ++column) {
            const auto& cellEntry = cellRows[row][column];
            if (!cellEntry.is_string()) {
                result.errors.push_back("Cell (" + std::to_string(row) + ","
                    + std::to_string(column) + ") must be a block id string.");
                return result;
            }

            BlockId blockId = 0;
            if (!tryParseHexByte(cellEntry.get<std::string>(), blockId)) {
                result.errors.push_back("Cell (" + std::to_string(row) + ","
                    + std::to_string(column) + ") has an invalid block id '"
                    + cellEntry.get<std::string>() + "'.");
                return result;
            }

            if (!parsedPresent[blockId]) {
                result.errors.push_back("Cell (" + std::to_string(row) + ","
                    + std::to_string(column) + ") references unknown block id '"
                    + cellEntry.get<std::string>() + "'.");
                return result;
            }

            blockIds[row][column] = blockId;
            packedCells.push_back(hasPackedCellOverride[blockId]
                    ? packedCellOverrides[blockId]
                    : packCellFromBlock(
                          parsedBlocks[blockId], defaultWallHeight));
        }
    }

    if (!target.setCells(packedCells.data(), rows, columns)) {
        result.errors.push_back("Failed to populate world map matrix.");
        return result;
    }

    target.resizeCell(cellWidth, cellDepth);

    for (int blockId = 0; blockId < 256; ++blockId) {
        if (parsedPresent[blockId]) {
            target.setBlockDefinition(static_cast<BlockId>(blockId),
                std::move(parsedBlocks[blockId]));
        }
    }

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            target.setBlockId(row, column, blockIds[row][column]);
        }
    }

    result.success = result.errors.empty();
    return result;
}
