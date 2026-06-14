//
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.
//

#include "nu_builtin_function_support.h"
#include "nu_builtin_registry.h"

#ifdef NUBASIC_HAS_RAYCAST
#include "ActorSystem.h"
#include "Player.h"
#include "RaycastEngine.h"
#include "SceneLoader.h"
#include "SpriteMetadataLoader.h"
#include "SpriteSet.h"
#include "WorldJsonLoader.h"
#include "WorldMap.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#ifdef _WIN32
#include "WinTextureLoader.h"
#include "nu_os_console.h"
#include "nu_os_gdi.h"
#include "nu_winconsole_api.h"

#include <mmsystem.h>
#include <windows.h>
#endif
#endif

namespace nu {
namespace {

#ifdef NUBASIC_HAS_RAYCAST
    constexpr int kDefaultVisualDegrees = 60;
    constexpr int kDefaultProjectionWidth = 320;
    constexpr int kDefaultProjectionHeight = 200;
    constexpr int kDefaultCellSize = 512;
    constexpr double kDefaultScale = 500000.0;
    constexpr double kPi = 3.14159265358979323846;

    struct loaded_sprite_set_t {
        DirectionalSpriteFrames frames;
        std::vector<SpriteAnimationClip> animations;
        Color transparent_color = makeColor(0, 0, 0);
    };

    struct raycast_session_t {
        struct RuntimeSpriteInfo {
            size_t sprite_index = 0;
            std::string persistence_key;
            std::string name;
            std::string sprite_set;
            std::string pickup_weapon;
            double pickup_health = 0.0;
            bool unlocks_map = false;
            bool explosive = false;
            double explosive_health = 0.0;
            double explosion_radius_cells = 0.0;
            double explosion_damage = 0.0;
            double explosion_scale_cells = 1.5;
            std::string explosion_sprite_set;
            std::string destroyed_sprite_set;
            double destroyed_scale_cells = 0.55;
            std::string damage_response_type;
            std::string damage_response_effect_sprite_set;
            std::string damage_response_effect_animation;
            double damage_response_effect_scale_cells = 1.5;
            std::string damage_response_sound;
            bool consumed = false;
            bool destroyed = false;
        };

        struct RuntimeEffect {
            size_t sprite_index = 0;
            double remaining_seconds = 0.0;
        };

        struct LayerTransition {
            std::string from_layer;
            std::string to_layer;
            int trigger_row = -1;
            int trigger_column = -1;
            double wait_seconds = 0.0;
            double target_x_cell = 1.5;
            double target_y_cell = 1.5;
            double target_facing_degrees = 0.0;
        };

        std::unique_ptr<WorldMap> world;
        std::unique_ptr<RaycastEngine> engine;
        ActorSystem actor_system;
        std::vector<SpriteActor> actors;
        std::vector<RuntimeSpriteInfo> runtime_sprites;
        std::vector<RuntimeEffect> runtime_effects;
        std::vector<std::string> keyring;
        std::vector<std::string> weapon_inventory;
        std::string active_weapon_path;
        std::vector<LayerTransition> transitions;
        std::map<std::string, loaded_sprite_set_t> loaded_sprite_sets;
        std::set<std::string> collected_item_keys;
        std::set<std::string> destroyed_object_keys;
        std::set<std::string> killed_enemy_keys;
        int total_completion_items = 0;
        int total_completion_enemies = 0;
        int total_damage_reactive_objects = 0;
        int projection_width = kDefaultProjectionWidth;
        int projection_height = kDefaultProjectionHeight;
        double pending_player_damage = 0.0;
        double pending_player_healing = 0.0;
        double transition_cooldown_seconds = 0.0;
        int map_unlock_count = 0;
        std::string background_music_alias;
        std::string base_dir;
        std::string project_path;
        std::string project_dir;
        std::string world_path;
        std::string active_layer_id;
        int pending_transition_index = -1;
        double pending_transition_seconds = 0.0;
    };

    raycast_session_t& raycast_session()
    {
        static raycast_session_t session;
        return session;
    }

    int normalize_ray(int ray, const Player& player) noexcept
    {
        ray %= player.deg360();
        return ray < 0 ? ray + player.deg360() : ray;
    }

    int player_alpha_for_facing_degrees(
        const Player& player, double facing_degrees) noexcept
    {
        auto normalized = std::fmod(facing_degrees, 360.0);
        if (normalized < 0.0) {
            normalized += 360.0;
        }

        const auto facing_ray = static_cast<int>(std::round(
            normalized * static_cast<double>(player.deg360()) / 360.0));
        return normalize_ray(facing_ray - player.degHalfVisual(), player);
    }

    double camera_facing_degrees(const Player& player) noexcept
    {
        const auto facing_ray
            = normalize_ray(player.getAlpha() + player.degHalfVisual(), player);
        return static_cast<double>(facing_ray) * 360.0
            / static_cast<double>(player.deg360());
    }

    std::string normalized_weapon_id(std::string path)
    {
        std::replace(path.begin(), path.end(), '\\', '/');
        std::transform(
            path.begin(), path.end(), path.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
        return path;
    }

    bool has_weapon_in_inventory(
        const raycast_session_t& session, const std::string& weapon_path)
    {
        const auto id = normalized_weapon_id(weapon_path);
        return std::find(session.weapon_inventory.begin(),
                   session.weapon_inventory.end(), id)
            != session.weapon_inventory.end();
    }

    void add_weapon_to_inventory(
        raycast_session_t& session, const std::string& weapon_path)
    {
        const auto id = normalized_weapon_id(weapon_path);
        if (id.empty() || has_weapon_in_inventory(session, id)) {
            return;
        }

        session.weapon_inventory.push_back(id);
    }

    std::string persistence_key_for(const std::string& layer_id,
        const std::string& name, const std::string& sprite_set, size_t index)
    {
        auto id
            = name.empty() ? sprite_set + "#" + std::to_string(index) : name;
        return layer_id + ":" + id;
    }

    std::vector<std::string> project_layer_ids(const std::string& project_path)
    {
        std::ifstream input(project_path);
        if (!input.is_open()) {
            return {};
        }

        nlohmann::json document;
        try {
            input >> document;
        } catch (...) {
            return {};
        }

        std::vector<std::string> ids;
        if (!document.is_object() || !document.contains("layers")
            || !document["layers"].is_array()) {
            return ids;
        }

        for (const auto& layer : document["layers"]) {
            if (!layer.is_object()) {
                continue;
            }

            if (layer.contains("id") && layer["id"].is_string()) {
                auto id = layer["id"].get<std::string>();
                if (!id.empty()) {
                    ids.push_back(std::move(id));
                }
            }
        }

        return ids;
    }

    std::string parent_path_string(const std::string& path)
    {
        return std::filesystem::path(path).parent_path().string();
    }

    std::string join_path(const std::string& base, const std::string& relative)
    {
        const std::filesystem::path rel(relative);
        if (rel.is_absolute()) {
            return rel.string();
        }

        return (std::filesystem::path(base) / rel).string();
    }

    std::string resolve_session_path(const std::string& path)
    {
        const std::filesystem::path fs_path(path);
        if (fs_path.is_absolute()) {
            return fs_path.string();
        }

        const auto& base_dir = raycast_session().base_dir;
        return base_dir.empty() ? path : join_path(base_dir, path);
    }

    std::string normalize_base_dir_path(const std::string& path)
    {
        std::filesystem::path base(path);
        std::error_code ec;
        auto canonical = std::filesystem::weakly_canonical(base, ec);
        if (!ec) {
            base = canonical;
        }

        std::error_code status_ec;
        const auto status = std::filesystem::status(base, status_ec);
        if (!status_ec && std::filesystem::is_regular_file(status)) {
            base = base.parent_path();
        } else if (!base.extension().empty()) {
            base = base.parent_path();
        }

        return base.empty() ? std::string(".") : base.string();
    }

    bool has_image_extension(std::string path)
    {
        auto extension = std::filesystem::path(path).extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
            [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
        return extension == ".bmp" || extension == ".png";
    }

    double normalize_degrees(double degrees) noexcept
    {
        while (degrees < 0.0) {
            degrees += 360.0;
        }

        while (degrees >= 360.0) {
            degrees -= 360.0;
        }

        return degrees;
    }

    double absolute_angle_delta_degrees(double lhs, double rhs) noexcept
    {
        const auto delta
            = std::fabs(normalize_degrees(lhs) - normalize_degrees(rhs));
        return delta > 180.0 ? 360.0 - delta : delta;
    }

    bool is_completion_enemy(const SpriteActor& actor) noexcept
    {
        return actor.maxHealth > 0.0 && actor.chasePlayer;
    }

    bool contains_ignore_case(std::string text, std::string needle)
    {
        std::transform(
            text.begin(), text.end(), text.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
        std::transform(
            needle.begin(), needle.end(), needle.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
        return text.find(needle) != std::string::npos;
    }

    bool is_pickup_item(const raycast_session_t::RuntimeSpriteInfo& info)
    {
        return info.pickup_health > 0.0 || !info.pickup_weapon.empty()
            || info.unlocks_map || contains_ignore_case(info.sprite_set, "ammo")
            || contains_ignore_case(info.sprite_set, "key")
            || contains_ignore_case(info.sprite_set, "med")
            || contains_ignore_case(info.name, "ammo")
            || contains_ignore_case(info.name, "key")
            || contains_ignore_case(info.name, "med");
    }

    bool is_damage_reactive(const raycast_session_t::RuntimeSpriteInfo& info)
    {
        return info.explosive || !info.damage_response_type.empty()
            || info.explosive_health > 0.0;
    }

    bool is_completion_item(const raycast_session_t::RuntimeSpriteInfo& info)
    {
        return is_pickup_item(info);
    }

    raycast_session_t::RuntimeSpriteInfo runtime_info_for_counting(
        const SceneLoader::SpriteInstance& instance)
    {
        raycast_session_t::RuntimeSpriteInfo info;
        info.name = instance.name;
        info.sprite_set = instance.spriteSet;
        info.pickup_weapon = instance.pickupWeapon;
        info.pickup_health = std::max(0.0, instance.pickupHealth);
        info.unlocks_map = instance.unlocksMap;
        info.explosive = instance.explosive;
        const auto has_damage_response = !instance.damageResponseType.empty()
            || !instance.damageResponseEffectSpriteSet.empty()
            || !instance.damageResponseDestroyedSpriteSet.empty()
            || !instance.damageResponseSound.empty()
            || instance.damageResponseHitPoints > 0.0;
        if (instance.explosive || has_damage_response) {
            info.explosive_health = std::max(1.0,
                instance.damageResponseHitPoints > 0.0
                    ? instance.damageResponseHitPoints
                    : instance.explosiveHitPoints);
        }
        info.damage_response_type = has_damage_response
                && instance.damageResponseType.empty()
                && (!instance.damageResponseEffectSpriteSet.empty()
                    || !instance.damageResponseDestroyedSpriteSet.empty()
                    || instance.damageResponseHitPoints > 0.0)
            ? std::string("break")
            : instance.damageResponseType;
        return info;
    }

    void refresh_project_progress_totals(
        raycast_session_t& session, const std::string& project_path)
    {
        session.total_completion_items = 0;
        session.total_completion_enemies = 0;
        session.total_damage_reactive_objects = 0;

        const auto layer_ids = project_layer_ids(project_path);
        if (layer_ids.empty()) {
            return;
        }

        SceneLoader loader;
        for (const auto& layer_id : layer_ids) {
            const auto result = loader.loadFromFile(project_path, layer_id);
            if (!result.success) {
                continue;
            }

            for (const auto& instance : result.scene.spriteInstances) {
                if (!instance.visible) {
                    continue;
                }

                auto info = runtime_info_for_counting(instance);
                if (is_completion_item(info)) {
                    ++session.total_completion_items;
                }

                if (is_damage_reactive(info)) {
                    ++session.total_damage_reactive_objects;
                }

                if (instance.maxHealth > 0.0 && instance.chasePlayer) {
                    ++session.total_completion_enemies;
                }
            }
        }
    }

    std::string inferred_key_id(
        const raycast_session_t::RuntimeSpriteInfo& info)
    {
        if (!contains_ignore_case(info.sprite_set, "key")
            && !contains_ignore_case(info.name, "key")) {
            return std::string();
        }

        if (contains_ignore_case(info.sprite_set, "red")
            || contains_ignore_case(info.name, "red")) {
            return "red";
        }

        if (contains_ignore_case(info.sprite_set, "blue")
            || contains_ignore_case(info.name, "blue")) {
            return "blue";
        }

        if (contains_ignore_case(info.sprite_set, "yellow")
            || contains_ignore_case(info.name, "yellow")) {
            return "yellow";
        }

        if (contains_ignore_case(info.sprite_set, "green")
            || contains_ignore_case(info.name, "green")) {
            return "green";
        }

        return "default";
    }

    void start_actor_death(SpriteActor& actor, Sprite* sprite) noexcept
    {
        actor.dead = true;
        actor.health = 0.0;
        actor.state = ActorState::Idle;
        actor.collidesWithWorld = false;

        if (sprite != nullptr && !actor.deathAnimationStarted) {
            if (sprite->setAnimation("death")) {
                actor.deathAnimationStarted = true;
            } else {
                sprite->visible = false;
            }
        }
    }

    std::string json_string(const nlohmann::json& object,
        const std::string& field, const std::string& fallback = std::string())
    {
        if (!object.is_object() || !object.contains(field)
            || !object[field].is_string()) {
            return fallback;
        }
        return object[field].get<std::string>();
    }

    double json_double(const nlohmann::json& object, const std::string& field,
        double fallback = 0.0)
    {
        if (!object.is_object() || !object.contains(field)
            || !object[field].is_number()) {
            return fallback;
        }
        return object[field].get<double>();
    }

    int json_int(const nlohmann::json& object, const std::string& field,
        int fallback = 0)
    {
        if (!object.is_object() || !object.contains(field)
            || !object[field].is_number_integer()) {
            return fallback;
        }
        return object[field].get<int>();
    }

    bool json_bool(const nlohmann::json& object, const std::string& field,
        bool fallback = false)
    {
        if (!object.is_object() || !object.contains(field)
            || !object[field].is_boolean()) {
            return fallback;
        }
        return object[field].get<bool>();
    }

    bool play_sound_file(const std::string& path)
    {
#ifdef _WIN32
        static unsigned sound_id = 0;
        const auto alias
            = "nubasic_raycast_sound_" + std::to_string(++sound_id);
        std::string open = "open \"" + path + "\" alias " + alias;
        if (mciSendStringA(open.c_str(), nullptr, 0, nullptr) != 0) {
            return false;
        }

        const auto play = "play " + alias + " from 0";
        return mciSendStringA(play.c_str(), nullptr, 0, nullptr) == 0;
#else
        (void)path;
        return false;
#endif
    }

    void stop_background_music(raycast_session_t& session)
    {
#ifdef _WIN32
        if (!session.background_music_alias.empty()) {
            const auto stop = "stop " + session.background_music_alias;
            const auto close = "close " + session.background_music_alias;
            mciSendStringA(stop.c_str(), nullptr, 0, nullptr);
            mciSendStringA(close.c_str(), nullptr, 0, nullptr);
            session.background_music_alias.clear();
        }
#else
        (void)session;
#endif
    }

    bool start_background_music_file(raycast_session_t& session,
        const std::string& path, bool loop, int volume_percent)
    {
#ifdef _WIN32
        static unsigned music_id = 0;
        stop_background_music(session);
        const auto alias
            = "nubasic_raycast_music_" + std::to_string(++music_id);
        const auto open = "open \"" + path + "\" alias " + alias;
        if (mciSendStringA(open.c_str(), nullptr, 0, nullptr) != 0) {
            return false;
        }

        const auto volume = std::clamp(volume_percent, 0, 100) * 10;
        const auto set_volume
            = "setaudio " + alias + " volume to " + std::to_string(volume);
        mciSendStringA(set_volume.c_str(), nullptr, 0, nullptr);

        const auto play = "play " + alias + (loop ? " repeat" : "");
        if (mciSendStringA(play.c_str(), nullptr, 0, nullptr) != 0) {
            const auto close = "close " + alias;
            mciSendStringA(close.c_str(), nullptr, 0, nullptr);
            return false;
        }

        session.background_music_alias = alias;
        return true;
#else
        (void)session;
        (void)path;
        (void)loop;
        (void)volume_percent;
        return false;
#endif
    }

    bool play_project_sound(const std::string& relative_path)
    {
        if (relative_path.empty()) {
            return false;
        }

        const auto& session = raycast_session();
        if (!session.project_dir.empty()) {
            return play_sound_file(
                join_path(session.project_dir, relative_path));
        }

        if (!session.world_path.empty()) {
            return play_sound_file(join_path(
                parent_path_string(session.world_path), relative_path));
        }

        return play_sound_file(resolve_session_path(relative_path));
    }

    bool play_pickup_sound(const raycast_session_t::RuntimeSpriteInfo& info)
    {
        if (!info.pickup_weapon.empty()) {
            return play_project_sound("effects/pickups/beep4.mp3");
        }

        if (info.pickup_health > 0.0
            || contains_ignore_case(info.sprite_set, "med")
            || contains_ignore_case(info.name, "med")) {
            return play_project_sound("effects/pickups/beep5.mp3");
        }

        if (info.unlocks_map || contains_ignore_case(info.sprite_set, "key")
            || contains_ignore_case(info.name, "key")) {
            return play_project_sound("effects/pickups/bling1.mp3");
        }

        return play_project_sound("effects/pickups/beep6.mp3");
    }

    std::vector<raycast_session_t::LayerTransition> load_layer_transitions(
        const std::string& project_path)
    {
        std::ifstream input(project_path);
        if (!input.is_open()) {
            return {};
        }

        nlohmann::json document;
        try {
            input >> document;
        } catch (...) {
            return {};
        }

        if (!document.is_object() || !document.contains("layerTransitions")
            || !document["layerTransitions"].is_array()) {
            return {};
        }

        std::vector<raycast_session_t::LayerTransition> transitions;
        for (const auto& entry : document["layerTransitions"]) {
            if (!entry.is_object() || !entry.contains("trigger")
                || !entry["trigger"].is_object()) {
                continue;
            }

            raycast_session_t::LayerTransition transition;
            transition.from_layer
                = json_string(entry, "fromLayer", std::string());
            transition.to_layer = json_string(entry, "toLayer", std::string());
            transition.wait_seconds
                = std::max(0.0, json_double(entry, "waitSeconds", 0.0));
            transition.trigger_row = json_int(entry["trigger"], "row", -1);
            transition.trigger_column
                = json_int(entry["trigger"], "column", -1);

            if (entry.contains("targetPlayerStart")
                && entry["targetPlayerStart"].is_object()) {
                const auto& target = entry["targetPlayerStart"];
                transition.target_x_cell = json_double(target, "xCell", 1.5);
                transition.target_y_cell = json_double(target, "yCell", 1.5);
                transition.target_facing_degrees
                    = json_double(target, "facingDegrees", 0.0);
            }

            if (!transition.from_layer.empty() && !transition.to_layer.empty()
                && transition.trigger_row >= 0
                && transition.trigger_column >= 0) {
                transitions.push_back(std::move(transition));
            }
        }

        return transitions;
    }

    void trigger_weapon_fire(RaycastEngine& engine)
    {
        auto* weapon = engine.viewWeapon();
        if (weapon == nullptr) {
            return;
        }

        weapon->restartAnimationOrFallback("fire", "idle");
        if (!weapon->fireSoundPath().empty() && weapon->fireSoundReady()) {
            play_sound_file(weapon->fireSoundPath());
            weapon->markFireSoundStarted();
        }
    }

    double animation_duration_seconds(
        const SpriteAnimationClip* animation, double fallback = 0.6) noexcept
    {
        if (animation == nullptr || animation->frameDurationMs <= 0.0
            || animation->frameSets.empty()) {
            return fallback;
        }

        return std::max(0.05,
            animation->frameDurationMs
                * static_cast<double>(animation->frameSets.size()) / 1000.0);
    }

    bool apply_loaded_sprite_set(
        Sprite& sprite, const loaded_sprite_set_t& sprite_set)
    {
        sprite.transparentColor = sprite_set.transparent_color;
        sprite.frames = sprite_set.frames;
        sprite.animations = sprite_set.animations;
        return !sprite.frames.empty() || !sprite.animations.empty();
    }

    void spawn_runtime_effect(raycast_session_t& session, double x, double y,
        double scale, const std::string& sprite_set_name,
        const std::string& animation_name)
    {
        if (!session.engine || sprite_set_name.empty()) {
            return;
        }

        const auto set_it = session.loaded_sprite_sets.find(sprite_set_name);
        if (set_it == session.loaded_sprite_sets.end()) {
            return;
        }

        Sprite effect;
        effect.x = x;
        effect.y = y;
        effect.scale = std::max(1.0, scale);
        effect.collisionRadius = 0.0;
        effect.visible = true;
        if (!apply_loaded_sprite_set(effect, set_it->second)) {
            return;
        }

        const auto animation
            = animation_name.empty() ? std::string("idle") : animation_name;
        effect.setAnimationOrFallback(animation, "idle");
        const auto duration = animation_duration_seconds(
            effect.animation(effect.activeAnimation));
        const auto sprite_index = session.engine->sprites().size();
        session.engine->addSprite(std::move(effect));
        session.runtime_effects.push_back({ sprite_index, duration });
    }

    void update_runtime_effects(raycast_session_t& session, double dt)
    {
        if (!session.engine) {
            return;
        }

        for (auto& effect : session.runtime_effects) {
            auto* sprite = session.engine->sprite(effect.sprite_index);
            if (sprite != nullptr && sprite->visible) {
                sprite->advanceAnimation(dt);
            }

            effect.remaining_seconds -= dt;
            if (effect.remaining_seconds <= 0.0 && sprite != nullptr) {
                sprite->visible = false;
            }
        }

        session.runtime_effects.erase(
            std::remove_if(session.runtime_effects.begin(),
                session.runtime_effects.end(),
                [](const raycast_session_t::RuntimeEffect& effect) {
                    return effect.remaining_seconds <= 0.0;
                }),
            session.runtime_effects.end());
    }

    bool has_clear_line_to_player(const WorldMap& world, const Sprite& sprite,
        double player_x, double player_y) noexcept
    {
        const auto dx = player_x - sprite.x;
        const auto dy = player_y - sprite.y;
        const auto distance = std::sqrt(dx * dx + dy * dy);
        if (distance <= 0.000001) {
            return true;
        }

        const auto cell_size
            = (static_cast<double>(world.getCellDx())
                  + static_cast<double>(world.getCellDy()))
            * 0.5;
        const auto step = std::max(8.0, cell_size / 8.0);
        const auto samples
            = std::max(1, static_cast<int>(std::ceil(distance / step)));

        // Skip the exact endpoints: the actor and the player are allowed to
        // occupy their own cells. Intermediate opaque geometry blocks attacks.
        for (int i = 1; i < samples; ++i) {
            const auto t = static_cast<double>(i) / static_cast<double>(samples);
            const auto x = sprite.x + dx * t;
            const auto y = sprite.y + dy * t;
            if (world.isSolidAtWorld(x, y)) {
                return false;
            }
        }

        return true;
    }

    bool actor_has_line_of_fire(const WorldMap& world, const SpriteActor& actor,
        const Sprite& sprite, double player_x, double player_y) noexcept
    {
        const auto dx = player_x - sprite.x;
        const auto dy = player_y - sprite.y;
        auto angle = std::atan2(dy, dx) * 180.0 / kPi;
        if (angle < 0.0) {
            angle += 360.0;
        }

        auto facing = sprite.facingRadians * 180.0 / kPi;
        if (facing < 0.0) {
            facing += 360.0;
        }

        return absolute_angle_delta_degrees(angle, facing)
            <= std::max(1.0, actor.attackFovDegrees) * 0.5
            && has_clear_line_to_player(world, sprite, player_x, player_y);
    }

    void update_enemy_attacks(raycast_session_t& session)
    {
        if (!session.engine || !session.world) {
            return;
        }

        const auto& player = session.engine->player();
        const auto player_x = static_cast<double>(player.getX());
        const auto player_y = static_cast<double>(player.getY());
        const auto cell_size
            = (static_cast<double>(session.world->getCellDx())
                  + static_cast<double>(session.world->getCellDy()))
            * 0.5;

        for (auto& actor : session.actors) {
            if (!is_completion_enemy(actor) || actor.dead || actor.health <= 0.0
                || actor.attackDamage <= 0.0
                || actor.attackCooldownRemaining > 0.0) {
                continue;
            }

            auto* sprite = session.engine->sprite(actor.spriteIndex);
            if (sprite == nullptr || !sprite->visible) {
                continue;
            }

            const auto dx = player_x - sprite->x;
            const auto dy = player_y - sprite->y;
            const auto distance = std::sqrt(dx * dx + dy * dy);
            const auto attack_range = actor.rangedAttack
                ? std::max(0.0, actor.attackRangeCells) * cell_size
                : std::max(cell_size * 0.75,
                      std::max(0.0, actor.stoppingDistanceCells) * cell_size);
            if (distance > attack_range || distance <= 0.000001) {
                continue;
            }

            if (!actor_has_line_of_fire(
                    *session.world, actor, *sprite, player_x, player_y)) {
                continue;
            }

            actor.state = ActorState::Attacking;
            actor.attackCooldownRemaining
                = std::max(0.1, actor.attackCooldownSeconds);
            actor.attackHoldSecondsRemaining = 0.25;
            sprite->setAnimationOrFallback("attack", "idle");
            session.pending_player_damage += actor.attackDamage;
            if (actor.rangedAttack) {
                play_project_sound("weapons/submachine_gun/sounds/"
                                   "machine_gun_burst_dry_close.wav");
            } else {
                play_project_sound("effects/enemies/punch1.mp3");
            }
        }
    }

    uint32_t pick_sprite_resolution(const SpriteSet& sprite_set)
    {
        if (sprite_set.defaultResolution() != 0) {
            return sprite_set.defaultResolution();
        }

        if (sprite_set.maxResolution() != 0) {
            return sprite_set.maxResolution();
        }

        if (!sprite_set.supportedResolutions().empty()) {
            return sprite_set.supportedResolutions().front();
        }

        return 64;
    }

    bool build_directional_frames(const SpriteSet& sprite_set,
        const std::vector<SpriteDirectionDefinition>& directions,
        uint32_t resolution, const std::string& sprite_set_dir, WorldMap& world,
        MapCell::TextureResourceKey& next_texture_key,
        DirectionalSpriteFrames& frames)
    {
        auto ordered = directions;
        std::sort(ordered.begin(), ordered.end(),
            [](const SpriteDirectionDefinition& lhs,
                const SpriteDirectionDefinition& rhs) {
                return lhs.angleDegrees < rhs.angleDegrees;
            });

        std::vector<SpriteFrame> direction_frames;
        direction_frames.reserve(ordered.size());
        for (const auto& direction : ordered) {
            const auto* file = sprite_set.fileFor(direction, resolution);
            if (file == nullptr || file->empty()) {
                return false;
            }

#ifdef _WIN32
            const auto texture_key = next_texture_key++;
            world.applyTexture(texture_key,
                loadTextureFromFile(join_path(sprite_set_dir, *file),
                    static_cast<int>(resolution),
                    static_cast<int>(resolution)));
            direction_frames.push_back({ texture_key });
#else
            return false;
#endif
        }

        if (direction_frames.empty()) {
            return false;
        }

        frames = DirectionalSpriteFrames(std::move(direction_frames));
        return true;
    }

    bool load_world_textures(WorldMap& world, const std::string& world_dir)
    {
#ifdef _WIN32
        const auto load_relative
            = [&](const std::string& image, int width = kDefaultCellSize,
                  int height = kDefaultCellSize,
                  const char* default_ext = ".bmp") {
                  auto path = join_path(world_dir, image);
                  if (!has_image_extension(path)) {
                      path += default_ext;
                  }

                  return loadTextureFromFile(path, width, height);
              };

        for (const auto& item : world.getTextureList()) {
            world.applyTextureToPanel(
                std::stoi(item.first, nullptr, 16), load_relative(item.second));
        }

        if (!world.getTexture(MapCell::TRANSPARENT_TEXTURE_KEY)) {
            world.applyTextureToPanel(MapCell::TRANSPARENT_TEXTURE_KEY,
                load_relative("clouds", kDefaultProjectionWidth,
                    kDefaultProjectionHeight));
        }

        return true;
#else
        (void)world;
        (void)world_dir;
        return false;
#endif
    }

    bool load_scene_sprites(raycast_session_t& session,
        const SceneLoader::Scene& scene, const std::string& project_dir)
    {
#ifdef _WIN32
        if (!session.engine || !session.world) {
            return false;
        }

        std::map<std::string, loaded_sprite_set_t> sprite_sets;
        auto next_texture_key = static_cast<MapCell::TextureResourceKey>(0x100);

        for (const auto& sprite_set_relative : scene.spriteSets) {
            const auto sprite_set_path
                = join_path(project_dir, sprite_set_relative);
            const auto sprite_set_dir = parent_path_string(sprite_set_path);

            SpriteMetadataLoader loader;
            const auto result = loader.loadFromFile(sprite_set_path);
            if (!result.success) {
                continue;
            }

            const auto& sprite_set = result.spriteSet;
            const auto resolution = pick_sprite_resolution(sprite_set);
            loaded_sprite_set_t loaded;
            if (!build_directional_frames(sprite_set, sprite_set.directions(),
                    resolution, sprite_set_dir, *session.world,
                    next_texture_key, loaded.frames)) {
                continue;
            }

            loaded.transparent_color = sprite_set.transparentColor();
            for (const auto& animation : sprite_set.animations()) {
                SpriteAnimationClip clip;
                clip.name = animation.name;
                clip.frameDurationMs = animation.frameDurationMs;
                clip.loop = animation.loop;
                build_directional_frames(sprite_set, animation.directions,
                    resolution, sprite_set_dir, *session.world,
                    next_texture_key, clip.frames);

                for (const auto& frame_directions : animation.frames) {
                    DirectionalSpriteFrames frame_set;
                    if (build_directional_frames(sprite_set, frame_directions,
                            resolution, sprite_set_dir, *session.world,
                            next_texture_key, frame_set)) {
                        clip.frameSets.push_back(std::move(frame_set));
                    }
                }

                loaded.animations.push_back(std::move(clip));
            }

            sprite_sets[sprite_set.name()] = std::move(loaded);
        }
        session.loaded_sprite_sets = sprite_sets;

        for (size_t instance_index = 0;
            instance_index < scene.spriteInstances.size(); ++instance_index) {
            const auto& instance = scene.spriteInstances[instance_index];
            if (!instance.visible) {
                continue;
            }

            const auto set_it = sprite_sets.find(instance.spriteSet);
            if (set_it == sprite_sets.end()) {
                continue;
            }

            Sprite sprite;
            sprite.x = kDefaultCellSize * instance.xCell;
            sprite.y = kDefaultCellSize * instance.yCell;
            sprite.scale = kDefaultCellSize * instance.scaleCells;
            sprite.facingRadians
                = instance.facingDegrees * 3.14159265358979323846 / 180.0;
            sprite.collisionRadius
                = kDefaultCellSize * instance.collisionRadiusCells;
            sprite.visible = instance.visible;
            sprite.transparentColor = set_it->second.transparent_color;
            sprite.frames = set_it->second.frames;
            sprite.animations = set_it->second.animations;
            sprite.setAnimationOrFallback("idle", "");

            const auto sprite_index = session.engine->sprites().size();
            session.engine->addSprite(std::move(sprite));

            raycast_session_t::RuntimeSpriteInfo runtime_info;
            runtime_info.sprite_index = sprite_index;
            runtime_info.persistence_key
                = persistence_key_for(scene.activeLayerId, instance.name,
                    instance.spriteSet, instance_index);
            runtime_info.name = instance.name;
            runtime_info.sprite_set = instance.spriteSet;
            runtime_info.pickup_weapon = instance.pickupWeapon;
            runtime_info.pickup_health = std::max(0.0, instance.pickupHealth);
            runtime_info.unlocks_map = instance.unlocksMap;
            runtime_info.explosive = instance.explosive;
            const auto has_damage_response
                = !instance.damageResponseType.empty()
                || !instance.damageResponseEffectSpriteSet.empty()
                || !instance.damageResponseDestroyedSpriteSet.empty()
                || !instance.damageResponseSound.empty()
                || instance.damageResponseHitPoints > 0.0;
            if (instance.explosive || has_damage_response) {
                runtime_info.explosive_health = std::max(1.0,
                    instance.damageResponseHitPoints > 0.0
                        ? instance.damageResponseHitPoints
                        : instance.explosiveHitPoints);
                runtime_info.explosion_radius_cells = std::max(0.0,
                    instance.damageResponseRadiusCells > 0.0
                        ? instance.damageResponseRadiusCells
                        : instance.explosionRadiusCells);
                runtime_info.explosion_damage = std::max(0.0,
                    instance.damageResponseDamage > 0.0
                        ? instance.damageResponseDamage
                        : instance.explosionDamage);
                runtime_info.explosion_scale_cells
                    = std::max(0.0, instance.explosionScaleCells);
                runtime_info.explosion_sprite_set = instance.explosionSpriteSet;
                runtime_info.destroyed_sprite_set
                    = !instance.damageResponseDestroyedSpriteSet.empty()
                    ? instance.damageResponseDestroyedSpriteSet
                    : instance.destroyedSpriteSet;
                runtime_info.destroyed_scale_cells = std::max(0.0,
                    instance.damageResponseDestroyedScaleCells > 0.0
                        ? instance.damageResponseDestroyedScaleCells
                        : instance.destroyedScaleCells);
            }
            runtime_info.damage_response_type = has_damage_response
                    && instance.damageResponseType.empty()
                    && (!instance.damageResponseEffectSpriteSet.empty()
                        || !instance.damageResponseDestroyedSpriteSet.empty()
                        || instance.damageResponseHitPoints > 0.0)
                ? std::string("break")
                : instance.damageResponseType;
            runtime_info.damage_response_sound = instance.damageResponseSound;
            runtime_info.damage_response_effect_sprite_set
                = instance.damageResponseEffectSpriteSet;
            runtime_info.damage_response_effect_animation
                = instance.damageResponseEffectAnimation;
            runtime_info.damage_response_effect_scale_cells
                = std::max(0.0, instance.damageResponseEffectScaleCells);

            auto* added_sprite = session.engine->sprite(sprite_index);
            if (session.collected_item_keys.count(runtime_info.persistence_key)
                > 0) {
                runtime_info.consumed = true;
                if (added_sprite != nullptr) {
                    added_sprite->visible = false;
                }
            }
            if (session.destroyed_object_keys.count(
                    runtime_info.persistence_key)
                > 0) {
                runtime_info.destroyed = true;
                if (added_sprite != nullptr) {
                    if (!runtime_info.destroyed_sprite_set.empty()) {
                        const auto destroyed_it
                            = session.loaded_sprite_sets.find(
                                runtime_info.destroyed_sprite_set);
                        if (destroyed_it != session.loaded_sprite_sets.end()
                            && apply_loaded_sprite_set(
                                *added_sprite, destroyed_it->second)) {
                            added_sprite->scale = kDefaultCellSize
                                * std::max(
                                    0.1, runtime_info.destroyed_scale_cells);
                            added_sprite->collisionRadius = 0.0;
                            added_sprite->visible = true;
                            added_sprite->setAnimationOrFallback("idle", "");
                        } else {
                            added_sprite->visible = false;
                        }
                    } else {
                        added_sprite->visible = false;
                    }
                }
            }
            session.runtime_sprites.push_back(std::move(runtime_info));

            if (instance.chasePlayer || instance.patrolCircuit
                || instance.maxHealth > 0.0) {
                SpriteActor actor;
                actor.spriteIndex = sprite_index;
                actor.persistenceKey = persistence_key_for(scene.activeLayerId,
                    instance.name, instance.spriteSet, instance_index);
                actor.homeX = kDefaultCellSize * instance.xCell;
                actor.homeY = kDefaultCellSize * instance.yCell;
                actor.hasHomePosition = true;
                actor.chasePlayer = instance.chasePlayer;
                actor.speedCellsPerSecond = instance.speedCellsPerSecond;
                actor.detectionRadiusCells = instance.detectionRadiusCells;
                actor.patrolRadiusCells = instance.patrolRadiusCells;
                actor.engagementHysteresisCells
                    = instance.engagementHysteresisCells;
                actor.patrolCircuit = instance.patrolCircuit;
                actor.stoppingDistanceCells = instance.stoppingDistanceCells;
                actor.collidesWithWorld = !instance.passThroughWalls;
                actor.maxHealth = std::max(0.0, instance.maxHealth);
                actor.health = actor.maxHealth > 0.0
                    ? std::clamp(instance.health, 0.0, actor.maxHealth)
                    : 0.0;
                actor.attackDamage = std::max(0.0, instance.attackDamage);
                actor.rangedAttack = instance.rangedAttack;
                actor.attackRangeCells
                    = std::max(0.0, instance.attackRangeCells);
                actor.attackCooldownSeconds
                    = std::max(0.1, instance.attackCooldownSeconds);
                actor.attackFovDegrees
                    = std::max(1.0, instance.attackFovDegrees);
                actor.attackBurstShots = std::max(1, instance.attackBurstShots);
                actor.attackBurstPauseSeconds
                    = std::max(0.1, instance.attackBurstPauseSeconds);
                if (session.killed_enemy_keys.count(actor.persistenceKey) > 0) {
                    actor.dead = true;
                    actor.health = 0.0;
                    if (added_sprite != nullptr) {
                        added_sprite->visible = false;
                    }
                }
                session.actors.push_back(actor);
            }
        }

        return true;
#else
        (void)session;
        (void)scene;
        (void)project_dir;
        return false;
#endif
    }

    std::unique_ptr<ViewWeapon> load_view_weapon_from_metadata(
        const std::string& metadata_path);

    bool load_session_weapon(raycast_session_t& session,
        const std::string& requested_weapon, rt_prog_ctx_t* ctx = nullptr)
    {
        if (!session.engine) {
            return false;
        }

        if (!has_weapon_in_inventory(session, requested_weapon)) {
            return false;
        }

        const auto weapon_path = session.project_dir.empty()
            ? resolve_session_path(requested_weapon)
            : join_path(session.project_dir, requested_weapon);
        auto weapon = load_view_weapon_from_metadata(weapon_path);
        if (weapon == nullptr) {
            if (ctx != nullptr) {
                ctx->set_errno(EINVAL);
            }
            return false;
        }

        session.engine->setViewWeapon(std::move(*weapon));
        session.active_weapon_path = normalized_weapon_id(requested_weapon);
        return true;
    }

    int update_player_pickups(raycast_session_t& session)
    {
        if (!session.engine || !session.world) {
            return 0;
        }

        auto& player = session.engine->player();
        const auto player_x = static_cast<double>(player.getX());
        const auto player_y = static_cast<double>(player.getY());
        const auto pickup_radius
            = (static_cast<double>(session.world->getCellDx())
                  + static_cast<double>(session.world->getCellDy()))
            * 0.35;
        int collected = 0;

        for (auto& info : session.runtime_sprites) {
            if (info.consumed || !is_pickup_item(info)) {
                continue;
            }

            auto* sprite = session.engine->sprite(info.sprite_index);
            if (sprite == nullptr || !sprite->visible) {
                continue;
            }

            const auto dx = sprite->x - player_x;
            const auto dy = sprite->y - player_y;
            if (std::sqrt(dx * dx + dy * dy) > pickup_radius) {
                continue;
            }

            info.consumed = true;
            sprite->visible = false;
            if (is_completion_item(info)) {
                session.collected_item_keys.insert(info.persistence_key);
            }
            ++collected;

            if (info.pickup_health > 0.0) {
                session.pending_player_healing += info.pickup_health;
            }

            if (info.unlocks_map) {
                ++session.map_unlock_count;
            }

            const auto key_id = inferred_key_id(info);
            if (!key_id.empty()) {
                session.keyring.push_back(key_id);
                session.world->setDoorKeyring(session.keyring);
            }

            if (!info.pickup_weapon.empty()) {
                add_weapon_to_inventory(session, info.pickup_weapon);
                load_session_weapon(session, info.pickup_weapon);
            }

            play_pickup_sound(info);
        }

        return collected;
    }

    std::unique_ptr<ViewWeapon> load_view_weapon_from_metadata(
        const std::string& metadata_path)
    {
#ifdef _WIN32
        std::ifstream input(metadata_path);
        if (!input.is_open()) {
            return {};
        }

        nlohmann::json document;
        try {
            input >> document;
        } catch (...) {
            return {};
        }

        if (!document.is_object() || !document.contains("animations")
            || !document["animations"].is_object()) {
            return {};
        }

        const auto metadata_dir = parent_path_string(metadata_path);
        const auto frame_width = json_int(document, "frameWidth", 320);
        const auto frame_height = json_int(document, "frameHeight", 220);
        if (frame_width <= 0 || frame_height <= 0) {
            return {};
        }

        auto weapon = std::make_unique<ViewWeapon>();
        weapon->setName(json_string(document, "weapon", "view_weapon"));
        weapon->setScreenHeightFraction(
            json_double(document, "screenHeightFraction", 0.45));
        weapon->setDamage(json_double(document, "damage", 0.0));
        weapon->setRangeCells(json_double(document, "rangeCells", 8.0));

        if (document.contains("fireBehavior")
            && document["fireBehavior"].is_object()) {
            const auto& fire_behavior = document["fireBehavior"];
            weapon->setFireBehavior(
                json_bool(fire_behavior, "automatic", false),
                json_double(fire_behavior, "intervalMs", 0.0),
                json_double(fire_behavior, "soundIntervalMs", 0.0));
        } else {
            weapon->setFireBehavior(json_bool(document, "automaticFire", false),
                json_double(document, "fireIntervalMs", 0.0));
        }

        if (document.contains("sounds") && document["sounds"].is_object()) {
            const auto fire_sound
                = json_string(document["sounds"], "fire", std::string());
            if (!fire_sound.empty()) {
                weapon->setFireSoundPath(join_path(metadata_dir, fire_sound));
            }
        }

        if (document.contains("ammo") && document["ammo"].is_object()) {
            const auto& ammo = document["ammo"];
            weapon->setAmmo(json_int(ammo, "magazineSize", 0),
                json_int(ammo, "maxAmmo", 0),
                json_int(ammo, "initialAmmo", -1));
        }

        if (document.contains("anchor") && document["anchor"].is_object()) {
            weapon->setAnchor(json_double(document["anchor"], "x", 0.5),
                json_double(document["anchor"], "y", 1.0));
        }

        if (document.contains("baseOffset")
            && document["baseOffset"].is_object()) {
            weapon->setBaseOffset(json_double(document["baseOffset"], "x", 0.0),
                json_double(document["baseOffset"], "y", 0.0));
        }

        if (document.contains("bob") && document["bob"].is_object()) {
            weapon->setBob(json_bool(document["bob"], "enabled", true),
                json_double(document["bob"], "amount", 1.0),
                json_double(document["bob"], "amplitudeX", 6.0),
                json_double(document["bob"], "amplitudeY", 4.0),
                json_double(document["bob"], "frequencyHz", 3.0));
        }

        for (const auto& animation_item : document["animations"].items()) {
            if (!animation_item.value().is_object()
                || !animation_item.value().contains("files")
                || !animation_item.value()["files"].is_array()) {
                continue;
            }

            ViewWeapon::Animation animation;
            animation.name = animation_item.key();
            animation.frameDurationMs
                = json_double(animation_item.value(), "frameDurationMs", 100.0);
            animation.loop = json_bool(animation_item.value(), "loop", true);

            for (const auto& file_entry : animation_item.value()["files"]) {
                if (!file_entry.is_string()) {
                    continue;
                }

                auto texture = loadTextureFromFile(
                    join_path(metadata_dir, file_entry.get<std::string>()),
                    frame_width, frame_height);
                if (texture) {
                    texture->setHasAlpha(true);
                    animation.frames.push_back(std::move(texture));
                }
            }

            weapon->addAnimation(std::move(animation));
        }

        if (!weapon->setAnimation("idle")) {
            return {};
        }

        return weapon;
#else
        (void)metadata_path;
        return {};
#endif
    }

    bool ensure_session_initialized()
    {
        auto& session = raycast_session();
        if (session.world && session.engine) {
            return true;
        }

        auto world = std::make_unique<WorldMap>();
        world->resizeCell(kDefaultCellSize, kDefaultCellSize);

        Player player(0, 0, kDefaultVisualDegrees, session.projection_width,
            session.projection_height);
        player.setPos({ kDefaultCellSize * 1.5, kDefaultCellSize * 1.5 });

        session.world = std::move(world);
        session.engine = std::make_unique<RaycastEngine>(player, kDefaultScale);
        session.actors.clear();
        session.runtime_sprites.clear();
        session.runtime_effects.clear();
        session.keyring.clear();
        session.weapon_inventory.clear();
        session.active_weapon_path.clear();
        session.collected_item_keys.clear();
        session.destroyed_object_keys.clear();
        session.killed_enemy_keys.clear();
        session.total_completion_items = 0;
        session.total_completion_enemies = 0;
        session.total_damage_reactive_objects = 0;
        session.transitions.clear();
        session.loaded_sprite_sets.clear();
        session.pending_player_damage = 0.0;
        session.pending_player_healing = 0.0;
        session.transition_cooldown_seconds = 0.0;
        session.map_unlock_count = 0;
        stop_background_music(session);
        session.project_path.clear();
        session.project_dir.clear();
        session.world_path.clear();
        session.active_layer_id.clear();
        session.pending_transition_index = -1;
        session.pending_transition_seconds = 0.0;
        return true;
    }

    bool load_world_into_session(const std::string& world_path,
        const std::string& layer_id, const SceneLoader::Scene* scene)
    {
        auto world = std::make_unique<WorldMap>();
        WorldJsonLoader loader;
        const auto result = loader.loadFromFile(world_path, *world, layer_id);
        if (!result.success) {
            return false;
        }

        world->resizeCell(kDefaultCellSize, kDefaultCellSize);

        if (scene != nullptr && scene->hasPlayerStart) {
            world->setPlayerStartCell(scene->playerStart.xCell,
                scene->playerStart.yCell, scene->playerStart.facingDegrees);
        }

        Player player(0, 0, kDefaultVisualDegrees,
            raycast_session().projection_width,
            raycast_session().projection_height);
        if (world->hasPlayerStart()) {
            const auto& start = world->getPlayerCellPos();
            player.setPos(
                { start.first * static_cast<double>(world->getCellDx()),
                    start.second * static_cast<double>(world->getCellDy()) });
            player.setAlpha(player_alpha_for_facing_degrees(
                player, world->getPlayerFacingDegrees()));
        } else {
            player.setPos({ kDefaultCellSize * 1.5, kDefaultCellSize * 1.5 });
        }

        auto& session = raycast_session();
        const auto world_dir = parent_path_string(world_path);
        load_world_textures(*world, world_dir);
        session.world = std::move(world);
        session.engine = std::make_unique<RaycastEngine>(player, kDefaultScale);
        session.actors.clear();
        session.runtime_sprites.clear();
        session.runtime_effects.clear();
        session.keyring.clear();
        session.weapon_inventory.clear();
        session.active_weapon_path.clear();
        session.collected_item_keys.clear();
        session.destroyed_object_keys.clear();
        session.killed_enemy_keys.clear();
        session.total_completion_items = 0;
        session.total_completion_enemies = 0;
        session.total_damage_reactive_objects = 0;
        session.loaded_sprite_sets.clear();
        session.pending_player_damage = 0.0;
        session.pending_player_healing = 0.0;
        session.transition_cooldown_seconds = 0.0;
        session.map_unlock_count = 0;
        session.world_path = world_path;
        session.active_layer_id = result.activeLayerId;
        session.pending_transition_index = -1;
        session.pending_transition_seconds = 0.0;
        return true;
    }

    bool load_project_layer_into_session(const std::string& project_path,
        const std::string& layer_id,
        const raycast_session_t::LayerTransition* transition = nullptr)
    {
        const auto project_dir = parent_path_string(project_path);
        SceneLoader loader;
        const auto result = loader.loadFromFile(project_path, layer_id);
        if (!result.success) {
            return false;
        }

        const auto world_path = result.scene.worldFile.empty()
            ? project_path
            : join_path(project_dir, result.scene.worldFile);

        auto& session = raycast_session();
        auto keyring = session.keyring;
        auto weapon_inventory = session.weapon_inventory;
        auto active_weapon_path = session.active_weapon_path;
        auto collected_item_keys = session.collected_item_keys;
        auto destroyed_object_keys = session.destroyed_object_keys;
        auto killed_enemy_keys = session.killed_enemy_keys;
        const auto total_completion_items = session.total_completion_items;
        const auto total_completion_enemies = session.total_completion_enemies;
        const auto total_damage_reactive_objects
            = session.total_damage_reactive_objects;
        auto transitions = session.transitions;
        const auto map_unlock_count = session.map_unlock_count;
        const auto had_active_weapon = !session.active_weapon_path.empty();
        const auto stored_project_path = session.project_path.empty()
            ? project_path
            : session.project_path;

        stop_background_music(session);

        if (!load_world_into_session(
                world_path, result.scene.activeLayerId, &result.scene)) {
            return false;
        }

        session.project_path = stored_project_path;
        session.project_dir = project_dir;
        session.transitions = std::move(transitions);
        session.keyring = std::move(keyring);
        session.weapon_inventory = std::move(weapon_inventory);
        session.active_weapon_path = std::move(active_weapon_path);
        session.collected_item_keys = std::move(collected_item_keys);
        session.destroyed_object_keys = std::move(destroyed_object_keys);
        session.killed_enemy_keys = std::move(killed_enemy_keys);
        session.total_completion_items = total_completion_items;
        session.total_completion_enemies = total_completion_enemies;
        session.total_damage_reactive_objects = total_damage_reactive_objects;
        session.map_unlock_count = map_unlock_count;
        if (session.world) {
            session.world->setDoorKeyring(session.keyring);
        }

        load_scene_sprites(session, result.scene, project_dir);
        if (!result.scene.playerWeapon.file.empty() && session.engine) {
            add_weapon_to_inventory(session, result.scene.playerWeapon.file);
            auto weapon = load_view_weapon_from_metadata(
                join_path(project_dir, result.scene.playerWeapon.file));
            if (weapon && result.scene.playerWeapon.visible
                && session.active_weapon_path.empty()) {
                if (result.scene.playerWeapon.screenHeightFraction > 0.0) {
                    weapon->setScreenHeightFraction(
                        result.scene.playerWeapon.screenHeightFraction);
                }
                session.engine->setViewWeapon(std::move(*weapon));
                session.active_weapon_path
                    = normalized_weapon_id(result.scene.playerWeapon.file);
            }
        }

        if (had_active_weapon && !session.active_weapon_path.empty()) {
            load_session_weapon(session, session.active_weapon_path);
        }

        if (result.scene.backgroundMusic.enabled
            && !result.scene.backgroundMusic.file.empty()) {
            const auto music_path
                = join_path(project_dir, result.scene.backgroundMusic.file);
            const auto music_volume
                = std::min(result.scene.backgroundMusic.volumePercent, 12);
            if (!start_background_music_file(session, music_path,
                    result.scene.backgroundMusic.loop, music_volume)) {
                start_background_music_file(session,
                    join_path(project_dir, "audio/demo_theme.mp3"),
                    result.scene.backgroundMusic.loop, music_volume);
            }
        }

        if (transition != nullptr && session.engine) {
            auto& player = session.engine->player();
            player.setPos({ transition->target_x_cell
                    * static_cast<double>(session.world->getCellDx()),
                transition->target_y_cell
                    * static_cast<double>(session.world->getCellDy()) });
            player.setAlpha(player_alpha_for_facing_degrees(
                player, transition->target_facing_degrees));
        }

        return true;
    }

    bool update_layer_transition(raycast_session_t& session, double dt)
    {
        if (!session.engine || !session.world || session.project_path.empty()
            || session.active_layer_id.empty() || session.transitions.empty()) {
            return false;
        }

        if (session.transition_cooldown_seconds > 0.0) {
            session.transition_cooldown_seconds
                = std::max(0.0, session.transition_cooldown_seconds - dt);
            return false;
        }

        const auto& player = session.engine->player();
        const auto player_column
            = static_cast<int>(player.getX() / session.world->getCellDx());
        const auto player_row
            = static_cast<int>(player.getY() / session.world->getCellDy());

        int matching_index = -1;
        for (int i = 0; i < static_cast<int>(session.transitions.size()); ++i) {
            const auto& transition = session.transitions[i];
            if (transition.from_layer == session.active_layer_id
                && transition.trigger_row == player_row
                && transition.trigger_column == player_column) {
                matching_index = i;
                break;
            }
        }

        if (matching_index < 0) {
            session.pending_transition_index = -1;
            session.pending_transition_seconds = 0.0;
            return false;
        }

        if (session.pending_transition_index != matching_index) {
            session.pending_transition_index = matching_index;
            session.pending_transition_seconds = 0.0;
            play_project_sound("effects/Elevator_Entrance.mp3");
        }

        auto& transition = session.transitions[matching_index];
        session.pending_transition_seconds += dt;
        if (session.pending_transition_seconds < transition.wait_seconds) {
            return false;
        }

        auto transition_copy = transition;
        play_project_sound("effects/Elevator_Opening_Sequence.mp3");
        if (!load_project_layer_into_session(session.project_path,
                transition_copy.to_layer, &transition_copy)) {
            return false;
        }

        session.transition_cooldown_seconds = 4.0;
        session.pending_transition_index = -1;
        session.pending_transition_seconds = 0.0;
        return true;
    }

#ifdef _WIN32
    bool present_framebuffer_to_gdi(
        const FrameBuffer& frame, int x, int y, int width, int height)
    {
        if (nu::_os_get_screen_mode() == 0 || frame.empty()) {
            return false;
        }

        HDC hdc = static_cast<HDC>(nu_winconsole_get_hdc());
        if (!hdc) {
            return false;
        }

        BITMAPINFO bmp_info;
        std::memset(&bmp_info, 0, sizeof(bmp_info));
        bmp_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmp_info.bmiHeader.biWidth = static_cast<LONG>(frame.width());
        bmp_info.bmiHeader.biHeight = -static_cast<LONG>(frame.height());
        bmp_info.bmiHeader.biPlanes = 1;
        bmp_info.bmiHeader.biBitCount = 32;
        bmp_info.bmiHeader.biCompression = BI_RGB;

        const auto ok = StretchDIBits(hdc, x, y, width, height, 0, 0,
                            static_cast<int>(frame.width()),
                            static_cast<int>(frame.height()), frame.data(),
                            &bmp_info, DIB_RGB_COLORS, SRCCOPY)
            != GDI_ERROR;

        nu_winconsole_release_hdc(hdc);
        return ok;
    }
#endif

    RaycastEngine* checked_engine(
        rt_prog_ctx_t& ctx, const std::string& function_name)
    {
        if (!ensure_session_initialized()) {
            ctx.set_errno(EINVAL);
            syntax_error_if(true,
                "'" + function_name + "': cannot initialize raycast session");
        }

        return raycast_session().engine.get();
    }

    WorldMap* checked_world(
        rt_prog_ctx_t& ctx, const std::string& function_name)
    {
        if (!ensure_session_initialized()) {
            ctx.set_errno(EINVAL);
            syntax_error_if(true,
                "'" + function_name + "': cannot initialize raycast session");
        }

        return raycast_session().world.get();
    }

    std::uint64_t fnv1a_frame_hash(const FrameBuffer& frame) noexcept
    {
        constexpr std::uint64_t kOffset = 14695981039346656037ull;
        constexpr std::uint64_t kPrime = 1099511628211ull;
        auto hash = kOffset;
        const auto* pixels = frame.pixels();
        const auto count = static_cast<size_t>(frame.width()) * frame.height();
        for (size_t i = 0; i < count; ++i) {
            auto value = pixels[i];
            for (int byte = 0; byte < 4; ++byte) {
                hash ^= static_cast<unsigned char>(value & 0xffu);
                hash *= kPrime;
                value >>= 8;
            }
        }
        return hash;
    }
#endif

    class raycast_builtin_module_t final : public builtin_module_t {
    public:
        const std::string& name() const noexcept override
        {
            static const std::string module_name = "raycast";
            return module_name;
        }

        const builtin_export_list_t& exports() const noexcept override
        {
            static const builtin_export_list_t module_exports = {
                "rayavailable", "rayinit", "rayloadworld", "rayrender",
                "rayframehash", "rayplayerx", "rayplayery", "rayplayerfacing",
                "raycurrentlayer$", "raycurrentlayer", "rayloadweapon",
                "rayhasweapon", "raysetplayer", "raymove", "raystrafe",
                "rayturn", "raymaprows", "raymapcols", "raycelldx", "raycelldy",
                "rayissolidcell", "raycellkind", "raymapunlockcount",
                "raykeyatcell", "rayloadproject", "raypresent", "raykeydown",
                "rayupdate", "raydamageenemy", "rayconsumeplayerdamage",
                "rayconsumeplayerhealing", "rayitemcount",
                "raycollecteditemcount", "raydestroyedobjectcount",
                "rayplaysound", "rayspritecount", "rayactorcount",
                "rayenemycount", "raykilledenemycount", "raysetbasedir",
                "raysetplayerslope", "rayplayerslope", "raysetplayerviewcenter",
                "rayplayerviewcenter", "rayplayerstandingon"
            };
            return module_exports;
        }

        void register_functions(global_function_tbl_t& fmap) const override
        {
            fmap["rayavailable"] = [](rt_prog_ctx_t&, const std::string& name,
                                       const func_args_t& args) {
                check_arg_num(args, 0, name);
#ifdef NUBASIC_HAS_RAYCAST
                return variant_t(integer_t(1));
#else
                return variant_t(integer_t(0));
#endif
            };

#ifdef NUBASIC_HAS_RAYCAST
            fmap["rayinit"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                  const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::INTEGER, variant_t::type_t::INTEGER },
                    vargs);

                auto& session = raycast_session();
                session.projection_width
                    = std::max(16, static_cast<int>(vargs[0].to_int()));
                session.projection_height
                    = std::max(16, static_cast<int>(vargs[1].to_int()));
                session.engine.reset();
                session.world.reset();
                ensure_session_initialized();
                return variant_t(integer_t(1));
            };

            fmap["rayloadworld"] = [](rt_prog_ctx_t& ctx,
                                       const std::string& name,
                                       const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::STRING }, vargs);

                const auto path = resolve_session_path(vargs[0].to_str());
                if (!load_world_into_session(path, std::string(), nullptr)) {
                    ctx.set_errno(EINVAL);
                    return variant_t(integer_t(0));
                }
                return variant_t(integer_t(1));
            };

            fmap["rayloadproject"] = [](rt_prog_ctx_t& ctx,
                                         const std::string& name,
                                         const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::STRING }, vargs);

                const auto project_path
                    = resolve_session_path(vargs[0].to_str());
                auto& session = raycast_session();
                session.project_path = project_path;
                session.transitions = load_layer_transitions(project_path);
                session.pending_transition_index = -1;
                session.pending_transition_seconds = 0.0;
                if (!load_project_layer_into_session(
                        project_path, std::string())) {
                    ctx.set_errno(EINVAL);
                    return variant_t(integer_t(0));
                }
                refresh_project_progress_totals(session, project_path);
                return variant_t(integer_t(1));
            };

            fmap["rayrender"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                    const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::INTEGER, variant_t::type_t::INTEGER },
                    vargs);

                auto* engine = checked_engine(ctx, name);
                auto* world = checked_world(ctx, name);
                const auto width
                    = std::max(16, static_cast<int>(vargs[0].to_int()));
                const auto height
                    = std::max(16, static_cast<int>(vargs[1].to_int()));
                engine->renderToFrameBuffer(*world,
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height));
                return variant_t(integer_t(1));
            };

            fmap["rayframehash"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      auto* engine = checked_engine(ctx, name);
                      const auto& frame = engine->frameBuffer();
                      if (frame.empty()) {
                          return variant_t(integer_t(0));
                      }

                      return variant_t(
                          integer_t(fnv1a_frame_hash(frame) & 0x7fffffffull));
                  };

            fmap["rayplayerx"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                     const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(
                    double_t(checked_engine(ctx, name)->player().getX()));
            };

            fmap["rayplayery"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                     const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(
                    double_t(checked_engine(ctx, name)->player().getY()));
            };

            fmap["rayplayerfacing"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      return variant_t(double_t(camera_facing_degrees(
                          checked_engine(ctx, name)->player())));
                  };

            fmap["rayplayerslope"] = [](rt_prog_ctx_t& ctx,
                                         const std::string& name,
                                         const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(
                    integer_t(checked_engine(ctx, name)->player().getSlope()));
            };

            fmap["raysetplayerslope"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      std::vector<variant_t> vargs;
                      get_functor_vargs(ctx, name, args,
                          { variant_t::type_t::INTEGER }, vargs);
                      checked_engine(ctx, name)->player().setSlope(
                          static_cast<int>(vargs[0].to_int()));
                      return variant_t(integer_t(1));
                  };

            fmap["rayplayerviewcenter"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      return variant_t(double_t(
                          checked_engine(ctx, name)->player().getCenterProj()));
                  };

            fmap["raysetplayerviewcenter"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      std::vector<variant_t> vargs;
                      get_functor_vargs(ctx, name, args,
                          { variant_t::type_t::DOUBLE }, vargs);
                      auto center = vargs[0].to_double();
                      center = std::clamp(center, 0.35, 0.65);
                      checked_engine(ctx, name)->player().setCenterProj(center);
                      return variant_t(integer_t(1));
                  };

            fmap["raycurrentlayer$"]
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      return variant_t(raycast_session().active_layer_id);
                  };
            fmap["raycurrentlayer"] = fmap["raycurrentlayer$"];

            fmap["rayloadweapon"] = [](rt_prog_ctx_t& ctx,
                                        const std::string& name,
                                        const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::STRING }, vargs);

                checked_engine(ctx, name);
                auto& session = raycast_session();
                const auto requested_weapon = vargs[0].to_str();
                return variant_t(integer_t(
                    load_session_weapon(session, requested_weapon, &ctx) ? 1
                                                                         : 0));
            };

            fmap["rayhasweapon"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      std::vector<variant_t> vargs;
                      get_functor_vargs(ctx, name, args,
                          { variant_t::type_t::STRING }, vargs);
                      checked_engine(ctx, name);
                      return variant_t(
                          integer_t(has_weapon_in_inventory(
                                        raycast_session(), vargs[0].to_str())
                                  ? 1
                                  : 0));
                  };

            fmap["raysetplayer"] = [](rt_prog_ctx_t& ctx,
                                       const std::string& name,
                                       const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::DOUBLE, variant_t::type_t::DOUBLE,
                        variant_t::type_t::DOUBLE },
                    vargs);

                auto* engine = checked_engine(ctx, name);
                auto& player = engine->player();
                player.setPos({ vargs[0].to_double(), vargs[1].to_double() });
                player.setAlpha(player_alpha_for_facing_degrees(
                    player, vargs[2].to_double()));
                return variant_t(integer_t(1));
            };

            fmap["raymove"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                  const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::DOUBLE }, vargs);
                auto* engine = checked_engine(ctx, name);
                auto* world = checked_world(ctx, name);
                return variant_t(integer_t(engine->player().moveTo(
                    static_cast<int>(std::round(vargs[0].to_double())),
                    *world)));
            };

            fmap["raystrafe"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                    const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::DOUBLE }, vargs);
                auto* engine = checked_engine(ctx, name);
                auto* world = checked_world(ctx, name);
                return variant_t(integer_t(engine->player().moveToH(
                    static_cast<int>(std::round(vargs[0].to_double())),
                    *world)));
            };

            fmap["rayturn"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                  const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::DOUBLE }, vargs);
                auto* engine = checked_engine(ctx, name);
                auto& player = engine->player();
                const auto units = vargs[0].to_double()
                    * static_cast<double>(player.deg360()) / 360.0;
                player.rotate(units);
                return variant_t(integer_t(1));
            };

            fmap["raymaprows"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                     const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(
                    integer_t(checked_world(ctx, name)->getRowCount()));
            };

            fmap["raymapcols"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                     const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(
                    integer_t(checked_world(ctx, name)->getColCount()));
            };

            fmap["raycelldx"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                    const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(
                    integer_t(checked_world(ctx, name)->getCellDx()));
            };

            fmap["raycelldy"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                    const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(
                    integer_t(checked_world(ctx, name)->getCellDy()));
            };

            fmap["rayissolidcell"] = [](rt_prog_ctx_t& ctx,
                                         const std::string& name,
                                         const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::INTEGER, variant_t::type_t::INTEGER },
                    vargs);
                auto* world = checked_world(ctx, name);
                const auto row = static_cast<int>(vargs[0].to_int());
                const auto col = static_cast<int>(vargs[1].to_int());
                if (row < 0 || col < 0 || row >= world->getRowCount()
                    || col >= world->getColCount()) {
                    return variant_t(integer_t(1));
                }

                const auto x = (static_cast<double>(col) + 0.5)
                    * static_cast<double>(world->getCellDx());
                const auto y = (static_cast<double>(row) + 0.5)
                    * static_cast<double>(world->getCellDy());
                return variant_t(
                    integer_t(world->isSolidAtWorld(x, y) ? 1 : 0));
            };

            fmap["raycellkind"] = [](rt_prog_ctx_t& ctx,
                                      const std::string& name,
                                      const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::INTEGER, variant_t::type_t::INTEGER },
                    vargs);
                auto* world = checked_world(ctx, name);
                const auto row = static_cast<int>(vargs[0].to_int());
                const auto col = static_cast<int>(vargs[1].to_int());
                if (row < 0 || col < 0 || row >= world->getRowCount()
                    || col >= world->getColCount()) {
                    return variant_t(integer_t(1));
                }

                const auto* block = world->blockAtCell(row, col);
                if (block != nullptr && block->door.enabled) {
                    return variant_t(integer_t(2));
                }

                const auto x = (static_cast<double>(col) + 0.5)
                    * static_cast<double>(world->getCellDx());
                const auto y = (static_cast<double>(row) + 0.5)
                    * static_cast<double>(world->getCellDy());
                return variant_t(
                    integer_t(world->isSolidAtWorld(x, y) ? 1 : 0));
            };

            fmap["raymapunlockcount"] = [](rt_prog_ctx_t&,
                                            const std::string& name,
                                            const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(integer_t(raycast_session().map_unlock_count));
            };

            fmap["raykeyatcell"] = [](rt_prog_ctx_t& ctx,
                                       const std::string& name,
                                       const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::INTEGER, variant_t::type_t::INTEGER },
                    vargs);
                auto* engine = checked_engine(ctx, name);
                auto* world = checked_world(ctx, name);
                const auto row = static_cast<int>(vargs[0].to_int());
                const auto col = static_cast<int>(vargs[1].to_int());

                for (const auto& info : raycast_session().runtime_sprites) {
                    if (info.consumed
                        || !contains_ignore_case(info.sprite_set, "key")) {
                        continue;
                    }

                    const auto* sprite = engine->sprite(info.sprite_index);
                    if (sprite == nullptr || !sprite->visible) {
                        continue;
                    }

                    const auto sprite_col
                        = static_cast<int>(sprite->x / world->getCellDx());
                    const auto sprite_row
                        = static_cast<int>(sprite->y / world->getCellDy());
                    if (sprite_row != row || sprite_col != col) {
                        continue;
                    }

                    if (contains_ignore_case(info.sprite_set, "green")) {
                        return variant_t(integer_t(1));
                    }
                    if (contains_ignore_case(info.sprite_set, "red")) {
                        return variant_t(integer_t(2));
                    }
                    if (contains_ignore_case(info.sprite_set, "blue")) {
                        return variant_t(integer_t(3));
                    }
                }

                return variant_t(integer_t(0));
            };

            fmap["rayplayerstandingon"] = [](rt_prog_ctx_t& ctx,
                                              const std::string& name,
                                              const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::STRING, variant_t::type_t::DOUBLE },
                    vargs);
                auto* engine = checked_engine(ctx, name);
                auto* world = checked_world(ctx, name);
                const auto fragment = vargs[0].to_str();
                const auto radius_cells = std::max(0.0, vargs[1].to_double());
                const auto radius = radius_cells
                    * (static_cast<double>(world->getCellDx())
                        + static_cast<double>(world->getCellDy()))
                    * 0.5;
                const auto player_x
                    = static_cast<double>(engine->player().getX());
                const auto player_y
                    = static_cast<double>(engine->player().getY());

                for (const auto& info : raycast_session().runtime_sprites) {
                    if (info.consumed || info.destroyed
                        || !contains_ignore_case(info.sprite_set, fragment)) {
                        continue;
                    }

                    const auto* sprite = engine->sprite(info.sprite_index);
                    if (sprite == nullptr || !sprite->visible) {
                        continue;
                    }

                    const auto dx = sprite->x - player_x;
                    const auto dy = sprite->y - player_y;
                    if (std::sqrt(dx * dx + dy * dy) <= radius) {
                        return variant_t(integer_t(1));
                    }
                }

                return variant_t(integer_t(0));
            };

            fmap["rayupdate"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                    const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::DOUBLE }, vargs);
                auto& session = raycast_session();
                auto* engine = checked_engine(ctx, name);
                auto* world = checked_world(ctx, name);
                const auto dt = std::clamp(vargs[0].to_double(), 0.0, 0.1);
                world->advanceDynamicTextures(dt);
                std::vector<Point2d> actor_positions;
                actor_positions.reserve(session.actors.size());
                for (const auto& actor : session.actors) {
                    if (actor.dead) {
                        continue;
                    }

                    const auto* sprite = engine->sprite(actor.spriteIndex);
                    if (sprite != nullptr && sprite->visible) {
                        actor_positions.push_back({ sprite->x, sprite->y });
                    }
                }
                std::vector<WorldMap::DoorEvent> door_events;
                world->updateDoors(engine->player().getX(),
                    engine->player().getY(), actor_positions, dt, &door_events);
                for (const auto& event : door_events) {
                    if (event.type
                        != WorldMap::DoorEvent::Type::OpeningStarted) {
                        continue;
                    }

                    const auto* block
                        = world->blockAtCell(event.row, event.column);
                    if (block != nullptr && !block->door.openSound.empty()) {
                        play_project_sound(block->door.openSound);
                    }
                }
                if (update_layer_transition(session, dt)) {
                    return variant_t(integer_t(1));
                }
                update_player_pickups(session);
                session.actor_system.update(
                    *engine, *world, session.actors, dt);
                update_enemy_attacks(session);
                update_runtime_effects(session, dt);
                engine->advanceSpriteAnimations(dt);
                engine->advanceViewWeapon(dt, false);
                return variant_t(integer_t(1));
            };

            fmap["raypresent"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                     const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::INTEGER, variant_t::type_t::INTEGER,
                        variant_t::type_t::INTEGER,
                        variant_t::type_t::INTEGER },
                    vargs);
#ifdef _WIN32
                auto* engine = checked_engine(ctx, name);
                const auto ok = present_framebuffer_to_gdi(
                    engine->frameBuffer(), static_cast<int>(vargs[0].to_int()),
                    static_cast<int>(vargs[1].to_int()),
                    static_cast<int>(vargs[2].to_int()),
                    static_cast<int>(vargs[3].to_int()));
                return variant_t(integer_t(ok ? 1 : 0));
#else
                (void)ctx;
                return variant_t(integer_t(0));
#endif
            };

            fmap["raykeydown"] = [](rt_prog_ctx_t& ctx, const std::string& name,
                                     const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::INTEGER }, vargs);
#ifdef _WIN32
                return variant_t(integer_t(
                    (GetAsyncKeyState(static_cast<int>(vargs[0].to_int()))
                        & 0x8000)
                        ? 1
                        : 0));
#else
                return variant_t(integer_t(0));
#endif
            };

            fmap["raydamageenemy"] = [](rt_prog_ctx_t& ctx,
                                         const std::string& name,
                                         const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::DOUBLE, variant_t::type_t::DOUBLE,
                        variant_t::type_t::DOUBLE },
                    vargs);
                auto& session = raycast_session();
                auto* engine = checked_engine(ctx, name);
                auto* world = checked_world(ctx, name);
                const auto damage = std::max(0.0, vargs[0].to_double());
                const auto range = std::max(0.0, vargs[1].to_double())
                    * (static_cast<double>(world->getCellDx())
                        + static_cast<double>(world->getCellDy()))
                    * 0.5;
                const auto half_fov = std::max(0.0, vargs[2].to_double()) * 0.5;
                const auto& player = engine->player();
                const auto player_x = static_cast<double>(player.getX());
                const auto player_y = static_cast<double>(player.getY());
                const auto facing = camera_facing_degrees(player);
                SpriteActor* best_actor = nullptr;
                raycast_session_t::RuntimeSpriteInfo* best_info = nullptr;
                Sprite* best_sprite = nullptr;
                double best_distance = range;

                for (auto& actor : session.actors) {
                    if (!is_completion_enemy(actor) || actor.dead
                        || actor.health <= 0.0) {
                        continue;
                    }

                    auto* sprite = engine->sprite(actor.spriteIndex);
                    if (sprite == nullptr || !sprite->visible) {
                        continue;
                    }

                    const auto dx = sprite->x - player_x;
                    const auto dy = sprite->y - player_y;
                    const auto distance = std::sqrt(dx * dx + dy * dy);
                    if (distance > range || distance <= 0.000001) {
                        continue;
                    }

                    auto angle = std::atan2(dy, dx) * 180.0 / kPi;
                    if (angle < 0.0) {
                        angle += 360.0;
                    }

                    if (absolute_angle_delta_degrees(angle, facing)
                        > half_fov) {
                        continue;
                    }

                    if (best_actor == nullptr || distance < best_distance) {
                        best_actor = &actor;
                        best_info = nullptr;
                        best_sprite = sprite;
                        best_distance = distance;
                    }
                }

                for (auto& info : session.runtime_sprites) {
                    if (info.destroyed || !is_damage_reactive(info)) {
                        continue;
                    }

                    auto* sprite = engine->sprite(info.sprite_index);
                    if (sprite == nullptr || !sprite->visible) {
                        continue;
                    }

                    const auto dx = sprite->x - player_x;
                    const auto dy = sprite->y - player_y;
                    const auto distance = std::sqrt(dx * dx + dy * dy);
                    if (distance > range || distance <= 0.000001) {
                        continue;
                    }

                    auto angle = std::atan2(dy, dx) * 180.0 / kPi;
                    if (angle < 0.0) {
                        angle += 360.0;
                    }

                    if (absolute_angle_delta_degrees(angle, facing)
                        > half_fov) {
                        continue;
                    }

                    if ((best_actor == nullptr && best_info == nullptr)
                        || distance < best_distance) {
                        best_actor = nullptr;
                        best_info = &info;
                        best_sprite = sprite;
                        best_distance = distance;
                    }
                }

                trigger_weapon_fire(*engine);

                if (best_actor == nullptr && best_info == nullptr) {
                    return variant_t(integer_t(0));
                }

                if (best_info != nullptr) {
                    best_info->explosive_health
                        = std::max(0.0, best_info->explosive_health - damage);
                    if (best_info->explosive_health <= 0.0) {
                        best_info->destroyed = true;
                        session.destroyed_object_keys.insert(
                            best_info->persistence_key);
                        if (best_sprite != nullptr) {
                            const auto effect_sprite_set
                                = !best_info->damage_response_effect_sprite_set
                                       .empty()
                                ? best_info->damage_response_effect_sprite_set
                                : best_info->explosion_sprite_set;
                            const auto effect_animation
                                = !best_info->damage_response_effect_animation
                                       .empty()
                                ? best_info->damage_response_effect_animation
                                : std::string("explode");
                            const auto effect_scale_cells
                                = best_info->damage_response_effect_scale_cells
                                    > 0.0
                                ? best_info->damage_response_effect_scale_cells
                                : best_info->explosion_scale_cells;
                            spawn_runtime_effect(session, best_sprite->x,
                                best_sprite->y,
                                kDefaultCellSize * effect_scale_cells,
                                effect_sprite_set, effect_animation);

                            if (!best_info->destroyed_sprite_set.empty()) {
                                const auto set_it
                                    = session.loaded_sprite_sets.find(
                                        best_info->destroyed_sprite_set);
                                if (set_it != session.loaded_sprite_sets.end()
                                    && apply_loaded_sprite_set(
                                        *best_sprite, set_it->second)) {
                                    best_sprite->scale = kDefaultCellSize
                                        * std::max(0.1,
                                            best_info->destroyed_scale_cells);
                                    best_sprite->collisionRadius = 0.0;
                                    best_sprite->setAnimationOrFallback(
                                        "idle", "");
                                } else {
                                    best_sprite->visible = false;
                                }
                            } else {
                                best_sprite->visible = false;
                            }
                        }
                        if (!best_info->damage_response_sound.empty()) {
                            play_project_sound(
                                best_info->damage_response_sound);
                        } else if (best_info->explosive) {
                            play_project_sound(
                                "effects/explosions/cannon1.mp3");
                        }
                        if (best_info->explosion_radius_cells > 0.0
                            && best_info->explosion_damage > 0.0
                            && best_sprite != nullptr) {
                            const auto dx = best_sprite->x - player_x;
                            const auto dy = best_sprite->y - player_y;
                            const auto radius
                                = best_info->explosion_radius_cells
                                * (static_cast<double>(world->getCellDx())
                                    + static_cast<double>(world->getCellDy()))
                                * 0.5;
                            const auto dist = std::sqrt(dx * dx + dy * dy);
                            if (dist <= radius) {
                                const auto falloff = radius <= 0.0
                                    ? 1.0
                                    : std::max(0.0, 1.0 - dist / radius);
                                session.pending_player_damage
                                    += best_info->explosion_damage * falloff;
                            }
                        }
                        return variant_t(integer_t(3));
                    }

                    return variant_t(integer_t(1));
                }

                best_actor->health = std::max(0.0, best_actor->health - damage);
                best_actor->state = ActorState::Chasing;
                if (best_actor->health <= 0.0) {
                    start_actor_death(*best_actor, best_sprite);
                    session.killed_enemy_keys.insert(
                        best_actor->persistenceKey);
                    return variant_t(integer_t(2));
                }

                return variant_t(integer_t(1));
            };

            fmap["rayconsumeplayerdamage"]
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      auto& session = raycast_session();
                      const auto damage = session.pending_player_damage;
                      session.pending_player_damage = 0.0;
                      return variant_t(double_t(damage));
                  };

            fmap["rayconsumeplayerhealing"]
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      auto& session = raycast_session();
                      const auto healing = session.pending_player_healing;
                      session.pending_player_healing = 0.0;
                      return variant_t(double_t(healing));
                  };

            fmap["rayitemcount"] = [](rt_prog_ctx_t&, const std::string& name,
                                       const func_args_t& args) {
                check_arg_num(args, 0, name);
                const auto& session = raycast_session();
                if (session.total_completion_items > 0) {
                    return variant_t(integer_t(session.total_completion_items));
                }

                integer_t count = 0;
                for (const auto& info : session.runtime_sprites) {
                    if (is_completion_item(info)) {
                        ++count;
                    }
                }
                return variant_t(count);
            };

            fmap["raycollecteditemcount"]
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      const auto& session = raycast_session();
                      if (session.total_completion_items > 0) {
                          return variant_t(
                              integer_t(session.collected_item_keys.size()));
                      }

                      integer_t count = 0;
                      for (const auto& info : session.runtime_sprites) {
                          if (is_completion_item(info) && info.consumed) {
                              ++count;
                          }
                      }
                      return variant_t(count);
                  };

            fmap["raydestroyedobjectcount"]
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      const auto& session = raycast_session();
                      if (session.total_damage_reactive_objects > 0) {
                          return variant_t(
                              integer_t(session.destroyed_object_keys.size()));
                      }

                      integer_t count = 0;
                      for (const auto& info : session.runtime_sprites) {
                          if (is_damage_reactive(info) && info.destroyed) {
                              ++count;
                          }
                      }
                      return variant_t(count);
                  };

            fmap["rayplaysound"] = [](rt_prog_ctx_t& ctx,
                                       const std::string& name,
                                       const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::STRING }, vargs);
                return variant_t(
                    integer_t(play_project_sound(vargs[0].to_str()) ? 1 : 0));
            };

            fmap["rayspritecount"] = [](rt_prog_ctx_t& ctx,
                                         const std::string& name,
                                         const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(
                    integer_t(checked_engine(ctx, name)->sprites().size()));
            };

            fmap["rayactorcount"] = [](rt_prog_ctx_t&, const std::string& name,
                                        const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(integer_t(raycast_session().actors.size()));
            };

            fmap["rayenemycount"] = [](rt_prog_ctx_t&, const std::string& name,
                                        const func_args_t& args) {
                check_arg_num(args, 0, name);
                const auto& session = raycast_session();
                if (session.total_completion_enemies > 0) {
                    return variant_t(
                        integer_t(session.total_completion_enemies));
                }

                integer_t count = 0;
                for (const auto& actor : session.actors) {
                    if (actor.maxHealth > 0.0 && actor.chasePlayer) {
                        ++count;
                    }
                }
                return variant_t(count);
            };

            fmap["raykilledenemycount"]
                = [](rt_prog_ctx_t&, const std::string& name,
                      const func_args_t& args) {
                      check_arg_num(args, 0, name);
                      const auto& session = raycast_session();
                      if (session.total_completion_enemies > 0) {
                          return variant_t(
                              integer_t(session.killed_enemy_keys.size()));
                      }

                      integer_t count = 0;
                      for (const auto& actor : session.actors) {
                          if (actor.maxHealth > 0.0 && actor.chasePlayer
                              && (actor.dead || actor.health <= 0.0)) {
                              ++count;
                          }
                      }
                      return variant_t(count);
                  };

            fmap["raysetbasedir"]
                = [](rt_prog_ctx_t& ctx, const std::string& name,
                      const func_args_t& args) {
                      std::vector<variant_t> vargs;
                      get_functor_vargs(ctx, name, args,
                          { variant_t::type_t::STRING }, vargs);
                      raycast_session().base_dir
                          = normalize_base_dir_path(vargs[0].to_str());
                      return variant_t(integer_t(1));
                  };
#else
            for (const auto& function_name : exports()) {
                if (function_name == "rayavailable") {
                    continue;
                }

                fmap[function_name] = [](rt_prog_ctx_t&,
                                          const std::string& name,
                                          const func_args_t&) {
                    syntax_error_if(true,
                        "'" + name + "': raycast support is not built in.");
                    return variant_t(integer_t(0));
                };
            }
#endif
        }
    };

} // namespace

const builtin_module_t& get_raycast_builtin_module()
{
    static raycast_builtin_module_t module;
    return module;
}

} // namespace nu
