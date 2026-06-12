// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "SpriteMetadataLoader.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

namespace {
const std::map<std::string, double> kExpectedDirections = {
    { "front", 0.0 },
    { "front_right", 45.0 },
    { "right", 90.0 },
    { "back_right", 135.0 },
    { "back", 180.0 },
    { "back_left", 225.0 },
    { "left", 270.0 },
    { "front_left", 315.0 },
};

const std::set<std::string> kSupportedFormats = {
    "BMP",
    "PNG",
};

constexpr uint32_t kMaxSupportedSpriteResolution = 1024;

bool fileExists(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    return file.good();
}

std::string directoryOf(const std::string& path)
{
    const auto slash = path.find_last_of("/\\");
    if (slash == std::string::npos) {
        return {};
    }

    return path.substr(0, slash + 1);
}

std::string joinPath(const std::string& base, const std::string& relative)
{
    if (relative.size() > 1 && relative[1] == ':') {
        return relative;
    }

    if (!relative.empty() && (relative[0] == '/' || relative[0] == '\\')) {
        return relative;
    }

    return base + relative;
}

void addError(std::vector<std::string>& errors, const std::string& error)
{
    errors.push_back(error);
}

bool isIntegerColor(int value) noexcept { return value >= 0 && value <= 255; }

std::string toString(uint32_t value)
{
    std::ostringstream stream;
    stream << value;
    return stream.str();
}

std::vector<SpriteDirectionDefinition> parseDirections(
    const nlohmann::json& directionsJson, const std::string& ownerName,
    const std::set<uint32_t>& supportedResolutionSet,
    const std::string& metadataDirectory, std::vector<std::string>& errors)
{
    std::vector<SpriteDirectionDefinition> directions;
    std::set<std::string> directionNames;

    if (!directionsJson.is_array()) {
        addError(errors, ownerName + " directions must be an array.");
        return directions;
    }

    for (const auto& directionJson : directionsJson) {
        if (!directionJson.is_object()) {
            addError(errors,
                "Each direction in " + ownerName + " must be an object.");
            continue;
        }

        SpriteDirectionDefinition direction;

        if (!directionJson.contains("name")
            || !directionJson["name"].is_string()) {
            addError(errors,
                "Direction in " + ownerName + " is missing a valid name.");
            continue;
        }

        direction.name = directionJson["name"].get<std::string>();
        const auto expectedDirection = kExpectedDirections.find(direction.name);
        if (expectedDirection == kExpectedDirections.end()) {
            addError(errors, "Unsupported direction name: " + direction.name);
            continue;
        }

        if (!directionNames.insert(direction.name).second) {
            addError(errors, "Duplicate direction name: " + direction.name);
            continue;
        }

        if (!directionJson.contains("angle")
            || !directionJson["angle"].is_number()) {
            addError(errors,
                "Direction " + direction.name + " is missing a numeric angle.");
            continue;
        }

        direction.angleDegrees = directionJson["angle"].get<double>();
        if (direction.angleDegrees != expectedDirection->second) {
            addError(errors,
                "Direction " + direction.name + " has an invalid angle.");
            continue;
        }

        if (!directionJson.contains("files")
            || !directionJson["files"].is_object()) {
            addError(
                errors, "Direction " + direction.name + " is missing files.");
            continue;
        }

        for (const auto& fileItem : directionJson["files"].items()) {
            uint32_t resolution = 0;
            try {
                resolution = static_cast<uint32_t>(std::stoul(fileItem.key()));
            } catch (...) {
                addError(errors,
                    "Invalid resolution key in direction " + direction.name);
                continue;
            }

            if (supportedResolutionSet.find(resolution)
                == supportedResolutionSet.end()) {
                addError(errors,
                    "Direction " + direction.name
                        + " references unsupported resolution "
                        + toString(resolution));
                continue;
            }

            if (!fileItem.value().is_string()) {
                addError(errors,
                    "File entry must be a string in direction "
                        + direction.name);
                continue;
            }

            const auto filePath = fileItem.value().get<std::string>();
            const auto fullPath = joinPath(metadataDirectory, filePath);
            if (!fileExists(fullPath)) {
                addError(errors, "Missing sprite image file: " + fullPath);
                continue;
            }

            direction.filesByResolution[resolution] = filePath;
        }

        if (direction.filesByResolution.empty()) {
            addError(
                errors, "Direction " + direction.name + " has no valid files.");
            continue;
        }

        directions.push_back(std::move(direction));
    }

    if (directionNames.size() != kExpectedDirections.size()) {
        addError(
            errors, ownerName + " must define all 8 supported directions.");
    }

    return directions;
}
} // namespace

SpriteMetadataLoader::Result SpriteMetadataLoader::loadFromFile(
    const std::string& metadataPath) const
{
    Result result;

    std::ifstream input(metadataPath);
    if (!input.is_open()) {
        addError(
            result.errors, "Cannot open sprite metadata file: " + metadataPath);
        return result;
    }

    nlohmann::json document;
    try {
        input >> document;
    } catch (const std::exception& error) {
        addError(result.errors, std::string("Invalid JSON: ") + error.what());
        return result;
    }

    if (!document.is_object()) {
        addError(result.errors, "Sprite metadata root must be a JSON object.");
        return result;
    }

    SpriteSet spriteSet;

    if (!document.contains("spriteSet") || !document["spriteSet"].is_string()) {
        addError(result.errors, "Missing or invalid string field: spriteSet.");
    } else {
        spriteSet.setName(document["spriteSet"].get<std::string>());
    }

    if (!document.contains("format") || !document["format"].is_string()) {
        addError(result.errors, "Missing or invalid string field: format.");
    } else {
        const auto format = document["format"].get<std::string>();
        spriteSet.setFormat(format);
        if (kSupportedFormats.find(format) == kSupportedFormats.end()) {
            addError(result.errors, "Unsupported sprite format: " + format);
        }
    }

    if (!document.contains("transparentColor")
        || !document["transparentColor"].is_array()
        || document["transparentColor"].size() != 3) {
        addError(result.errors, "transparentColor must be an RGB array.");
    } else {
        const auto& color = document["transparentColor"];
        if (!color[0].is_number_integer() || !color[1].is_number_integer()
            || !color[2].is_number_integer()) {
            addError(
                result.errors, "transparentColor values must be integers.");
        } else {
            const auto red = color[0].get<int>();
            const auto green = color[1].get<int>();
            const auto blue = color[2].get<int>();

            if (!isIntegerColor(red) || !isIntegerColor(green)
                || !isIntegerColor(blue)) {
                addError(result.errors,
                    "transparentColor values must be in 0..255.");
            } else {
                spriteSet.setTransparentColor(makeColor(red, green, blue));
            }
        }
    }

    std::set<uint32_t> supportedResolutionSet;
    std::vector<uint32_t> supportedResolutions;
    if (!document.contains("supportedResolutions")
        || !document["supportedResolutions"].is_array()) {
        addError(result.errors, "supportedResolutions must be an array.");
    } else {
        for (const auto& resolution : document["supportedResolutions"]) {
            if (!resolution.is_number_unsigned()) {
                addError(result.errors,
                    "supportedResolutions must contain positive integers.");
                continue;
            }

            const auto value = resolution.get<uint32_t>();
            if (value == 0 || value > kMaxSupportedSpriteResolution) {
                addError(result.errors,
                    "Supported sprite resolution out of range: "
                        + toString(value));
                continue;
            }

            supportedResolutionSet.insert(value);
            supportedResolutions.push_back(value);
        }
        spriteSet.setSupportedResolutions(supportedResolutions);
    }

    if (document.contains("defaultResolution")
        && document["defaultResolution"].is_number_unsigned()) {
        spriteSet.setDefaultResolution(
            document["defaultResolution"].get<uint32_t>());
    } else {
        addError(result.errors, "Missing or invalid defaultResolution.");
    }

    if (document.contains("maxResolution")
        && document["maxResolution"].is_number_unsigned()) {
        const auto maxResolution = document["maxResolution"].get<uint32_t>();
        spriteSet.setMaxResolution(maxResolution);
        if (maxResolution > kMaxSupportedSpriteResolution) {
            addError(result.errors,
                "maxResolution exceeds current engine limit of "
                    + toString(kMaxSupportedSpriteResolution) + ".");
        }
    } else {
        addError(result.errors, "Missing or invalid maxResolution.");
    }

    const auto metadataDirectory = directoryOf(metadataPath);
    std::vector<SpriteAnimationDefinition> animations;

    if (document.contains("animations")) {
        if (!document["animations"].is_object()) {
            addError(result.errors, "animations must be an object.");
        } else {
            for (const auto& animationItem : document["animations"].items()) {
                if (!animationItem.value().is_object()) {
                    addError(result.errors,
                        "Animation " + animationItem.key()
                            + " must be an object.");
                    continue;
                }

                const auto& animationJson = animationItem.value();
                SpriteAnimationDefinition animation;
                animation.name = animationItem.key();

                if (animation.name.empty()) {
                    addError(result.errors, "Animation name cannot be empty.");
                    continue;
                }

                if (animationJson.contains("frameDurationMs")) {
                    if (!animationJson["frameDurationMs"].is_number()) {
                        addError(result.errors,
                            "Animation " + animation.name
                                + " has an invalid frameDurationMs.");
                        continue;
                    }

                    animation.frameDurationMs
                        = animationJson["frameDurationMs"].get<double>();
                    if (animation.frameDurationMs <= 0.0) {
                        addError(result.errors,
                            "Animation " + animation.name
                                + " frameDurationMs must be positive.");
                        continue;
                    }
                }

                if (animationJson.contains("loop")) {
                    if (!animationJson["loop"].is_boolean()) {
                        addError(result.errors,
                            "Animation " + animation.name
                                + " has an invalid loop flag.");
                        continue;
                    }

                    animation.loop = animationJson["loop"].get<bool>();
                }

                if (animationJson.contains("frames")) {
                    if (!animationJson["frames"].is_array()) {
                        addError(result.errors,
                            "Animation " + animation.name
                                + " frames must be an array.");
                        continue;
                    }

                    for (const auto& frameJson : animationJson["frames"]) {
                        if (!frameJson.is_object()
                            || !frameJson.contains("directions")) {
                            addError(result.errors,
                                "Each frame in animation " + animation.name
                                    + " must contain directions.");
                            continue;
                        }

                        auto frameDirections
                            = parseDirections(frameJson["directions"],
                                "Animation " + animation.name + " frame",
                                supportedResolutionSet, metadataDirectory,
                                result.errors);
                        if (!frameDirections.empty()) {
                            animation.frames.push_back(
                                std::move(frameDirections));
                        }
                    }

                    if (animation.frames.empty()) {
                        addError(result.errors,
                            "Animation " + animation.name
                                + " has no valid frames.");
                        continue;
                    }

                    animation.directions = animation.frames.front();
                } else {
                    if (!animationJson.contains("directions")) {
                        addError(result.errors,
                            "Animation " + animation.name
                                + " is missing directions.");
                        continue;
                    }

                    animation.directions = parseDirections(
                        animationJson["directions"],
                        "Animation " + animation.name, supportedResolutionSet,
                        metadataDirectory, result.errors);

                    if (!animation.directions.empty()) {
                        animation.frames.push_back(animation.directions);
                    }
                }

                if (!animation.directions.empty()) {
                    animations.push_back(std::move(animation));
                }
            }
        }

        const auto idle = std::find_if(animations.begin(), animations.end(),
            [](const SpriteAnimationDefinition& animation) {
                return animation.name == "idle";
            });
        if (idle == animations.end()) {
            addError(result.errors, "animations must define an idle clip.");
        } else {
            spriteSet.setDirections(idle->directions);
        }
    } else if (!document.contains("directions")) {
        addError(result.errors, "directions must be an array.");
    } else {
        auto directions
            = parseDirections(document["directions"], "Sprite metadata",
                supportedResolutionSet, metadataDirectory, result.errors);
        spriteSet.setDirections(directions);

        if (!directions.empty()) {
            SpriteAnimationDefinition idleAnimation;
            idleAnimation.name = "idle";
            idleAnimation.frameDurationMs = 0.0;
            idleAnimation.loop = true;
            idleAnimation.directions = std::move(directions);
            idleAnimation.frames.push_back(idleAnimation.directions);
            animations.push_back(std::move(idleAnimation));
        }
    }

    spriteSet.setAnimations(std::move(animations));

    std::vector<SpriteLodRule> lodRules;
    if (!document.contains("lod") || !document["lod"].is_array()) {
        addError(result.errors, "lod must be an array.");
    } else {
        for (const auto& lodJson : document["lod"]) {
            if (!lodJson.is_object() || !lodJson.contains("maxDistance")
                || !lodJson.contains("resolution")
                || !lodJson["maxDistance"].is_number()
                || !lodJson["resolution"].is_number_unsigned()) {
                addError(result.errors,
                    "Each lod entry must contain maxDistance and resolution.");
                continue;
            }

            SpriteLodRule rule;
            rule.maxDistanceCells = lodJson["maxDistance"].get<double>();
            rule.resolution = lodJson["resolution"].get<uint32_t>();

            if (rule.maxDistanceCells <= 0.0) {
                addError(result.errors, "LOD maxDistance must be positive.");
                continue;
            }

            if (supportedResolutionSet.find(rule.resolution)
                == supportedResolutionSet.end()) {
                addError(result.errors,
                    "LOD references unsupported resolution "
                        + toString(rule.resolution));
                continue;
            }

            lodRules.push_back(rule);
        }

        if (lodRules.empty()) {
            addError(result.errors, "At least one valid LOD rule is required.");
        }

        spriteSet.setLodRules(std::move(lodRules));
    }

    result.success = result.errors.empty();
    result.spriteSet = std::move(spriteSet);
    return result;
}
