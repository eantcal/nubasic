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
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#include "WinTextureLoader.h"
#include "nu_os_console.h"
#include "nu_os_gdi.h"
#include "nu_winconsole_api.h"

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
    constexpr double kDefaultScale = 250000.0;

    struct raycast_session_t {
        std::unique_ptr<WorldMap> world;
        std::unique_ptr<RaycastEngine> engine;
        ActorSystem actor_system;
        std::vector<SpriteActor> actors;
        int projection_width = kDefaultProjectionWidth;
        int projection_height = kDefaultProjectionHeight;
        std::string base_dir;
        std::string project_dir;
        std::string world_path;
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
        const Player& player,
        double facing_degrees) noexcept
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
        const auto facing_ray =
            normalize_ray(player.getAlpha() + player.degHalfVisual(), player);
        return static_cast<double>(facing_ray) * 360.0
            / static_cast<double>(player.deg360());
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
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        return extension == ".bmp" || extension == ".png";
    }

    struct loaded_sprite_set_t {
        DirectionalSpriteFrames frames;
        std::vector<SpriteAnimationClip> animations;
        Color transparent_color = makeColor(0, 0, 0);
    };

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
        uint32_t resolution,
        const std::string& sprite_set_dir,
        WorldMap& world,
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
        const auto load_relative = [&](const std::string& image,
                                       int width = kDefaultCellSize,
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
                std::stoi(item.first, nullptr, 16),
                load_relative(item.second));
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
        const SceneLoader::Scene& scene,
        const std::string& project_dir)
    {
#ifdef _WIN32
        if (!session.engine || !session.world) {
            return false;
        }

        std::map<std::string, loaded_sprite_set_t> sprite_sets;
        auto next_texture_key =
            static_cast<MapCell::TextureResourceKey>(0x100);

        for (const auto& sprite_set_relative : scene.spriteSets) {
            const auto sprite_set_path =
                join_path(project_dir, sprite_set_relative);
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
                    resolution, sprite_set_dir, *session.world, next_texture_key,
                    loaded.frames)) {
                continue;
            }

            loaded.transparent_color = sprite_set.transparentColor();
            for (const auto& animation : sprite_set.animations()) {
                SpriteAnimationClip clip;
                clip.name = animation.name;
                clip.frameDurationMs = animation.frameDurationMs;
                clip.loop = animation.loop;
                build_directional_frames(sprite_set, animation.directions,
                    resolution, sprite_set_dir, *session.world, next_texture_key,
                    clip.frames);

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

        for (const auto& instance : scene.spriteInstances) {
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
            sprite.facingRadians = instance.facingDegrees * 3.14159265358979323846 / 180.0;
            sprite.collisionRadius =
                kDefaultCellSize * instance.collisionRadiusCells;
            sprite.visible = instance.visible;
            sprite.transparentColor = set_it->second.transparent_color;
            sprite.frames = set_it->second.frames;
            sprite.animations = set_it->second.animations;
            sprite.setAnimationOrFallback("idle", "");

            const auto sprite_index = session.engine->sprites().size();
            session.engine->addSprite(std::move(sprite));

            if (instance.chasePlayer || instance.patrolCircuit
                || instance.maxHealth > 0.0) {
                SpriteActor actor;
                actor.spriteIndex = sprite_index;
                actor.persistenceKey = instance.name;
                actor.homeX = kDefaultCellSize * instance.xCell;
                actor.homeY = kDefaultCellSize * instance.yCell;
                actor.hasHomePosition = true;
                actor.chasePlayer = instance.chasePlayer;
                actor.speedCellsPerSecond = instance.speedCellsPerSecond;
                actor.detectionRadiusCells = instance.detectionRadiusCells;
                actor.patrolRadiusCells = instance.patrolRadiusCells;
                actor.engagementHysteresisCells =
                    instance.engagementHysteresisCells;
                actor.patrolCircuit = instance.patrolCircuit;
                actor.stoppingDistanceCells = instance.stoppingDistanceCells;
                actor.collidesWithWorld = !instance.passThroughWalls;
                actor.maxHealth = std::max(0.0, instance.maxHealth);
                actor.health = actor.maxHealth > 0.0
                    ? std::clamp(instance.health, 0.0, actor.maxHealth)
                    : 0.0;
                actor.attackDamage = std::max(0.0, instance.attackDamage);
                actor.rangedAttack = instance.rangedAttack;
                actor.attackRangeCells = std::max(0.0, instance.attackRangeCells);
                actor.attackCooldownSeconds =
                    std::max(0.1, instance.attackCooldownSeconds);
                actor.attackFovDegrees =
                    std::max(1.0, instance.attackFovDegrees);
                actor.attackBurstShots = std::max(1, instance.attackBurstShots);
                actor.attackBurstPauseSeconds =
                    std::max(0.1, instance.attackBurstPauseSeconds);
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

    bool ensure_session_initialized()
    {
        auto& session = raycast_session();
        if (session.world && session.engine) {
            return true;
        }

        auto world = std::make_unique<WorldMap>();
        world->resizeCell(kDefaultCellSize, kDefaultCellSize);

        Player player(
            0,
            0,
            kDefaultVisualDegrees,
            session.projection_width,
            session.projection_height);
        player.setPos({ kDefaultCellSize * 1.5, kDefaultCellSize * 1.5 });

        session.world = std::move(world);
        session.engine = std::make_unique<RaycastEngine>(player, kDefaultScale);
        session.actors.clear();
        return true;
    }

    bool load_world_into_session(const std::string& world_path,
        const std::string& layer_id,
        const SceneLoader::Scene* scene)
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
                scene->playerStart.yCell,
                scene->playerStart.facingDegrees);
        }

        Player player(0, 0, kDefaultVisualDegrees,
            raycast_session().projection_width,
            raycast_session().projection_height);
        if (world->hasPlayerStart()) {
            const auto& start = world->getPlayerCellPos();
            player.setPos({
                start.first * static_cast<double>(world->getCellDx()),
                start.second * static_cast<double>(world->getCellDy())
            });
            player.setAlpha(player_alpha_for_facing_degrees(
                player,
                world->getPlayerFacingDegrees()));
        } else {
            player.setPos({ kDefaultCellSize * 1.5, kDefaultCellSize * 1.5 });
        }

        auto& session = raycast_session();
        const auto world_dir = parent_path_string(world_path);
        load_world_textures(*world, world_dir);
        session.world = std::move(world);
        session.engine = std::make_unique<RaycastEngine>(player, kDefaultScale);
        session.actors.clear();
        session.world_path = world_path;
        return true;
    }

#ifdef _WIN32
    bool present_framebuffer_to_gdi(
        const FrameBuffer& frame,
        int x,
        int y,
        int width,
        int height)
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
            static_cast<int>(frame.height()),
            frame.data(),
            &bmp_info,
            DIB_RGB_COLORS,
            SRCCOPY) != GDI_ERROR;

        nu_winconsole_release_hdc(hdc);
        return ok;
    }
#endif

    RaycastEngine* checked_engine(
        rt_prog_ctx_t& ctx,
        const std::string& function_name)
    {
        if (!ensure_session_initialized()) {
            ctx.set_errno(EINVAL);
            syntax_error_if(
                true,
                "'" + function_name + "': cannot initialize raycast session");
        }

        return raycast_session().engine.get();
    }

    WorldMap* checked_world(rt_prog_ctx_t& ctx, const std::string& function_name)
    {
        if (!ensure_session_initialized()) {
            ctx.set_errno(EINVAL);
            syntax_error_if(
                true,
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
                "rayavailable",
                "rayinit",
                "rayloadworld",
                "rayrender",
                "rayframehash",
                "rayplayerx",
                "rayplayery",
                "rayplayerfacing",
                "raysetplayer",
                "raymove",
                "raystrafe",
                "rayturn",
                "rayloadproject",
                "raypresent",
                "raykeydown",
                "rayupdate",
                "rayspritecount",
                "rayactorcount",
                "raysetbasedir"
            };
            return module_exports;
        }

        void register_functions(global_function_tbl_t& fmap) const override
        {
            fmap["rayavailable"] = [](rt_prog_ctx_t&,
                                       const std::string& name,
                                       const func_args_t& args) {
                check_arg_num(args, 0, name);
#ifdef NUBASIC_HAS_RAYCAST
                return variant_t(integer_t(1));
#else
                return variant_t(integer_t(0));
#endif
            };

#ifdef NUBASIC_HAS_RAYCAST
            fmap["rayinit"] = [](rt_prog_ctx_t& ctx,
                                  const std::string& name,
                                  const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::INTEGER,
                        variant_t::type_t::INTEGER },
                    vargs);

                auto& session = raycast_session();
                session.projection_width =
                    std::max(16, static_cast<int>(vargs[0].to_int()));
                session.projection_height =
                    std::max(16, static_cast<int>(vargs[1].to_int()));
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

                const auto project_path =
                    resolve_session_path(vargs[0].to_str());
                const auto project_dir = parent_path_string(project_path);
                SceneLoader loader;
                const auto result = loader.loadFromFile(project_path);
                if (!result.success) {
                    ctx.set_errno(EINVAL);
                    return variant_t(integer_t(0));
                }

                const auto world_path = result.scene.worldFile.empty()
                    ? project_path
                    : join_path(project_dir, result.scene.worldFile);
                auto& session = raycast_session();
                if (!load_world_into_session(
                        world_path,
                        result.scene.activeLayerId,
                        &result.scene)) {
                    ctx.set_errno(EINVAL);
                    return variant_t(integer_t(0));
                }

                session.project_dir = project_dir;
                load_scene_sprites(session, result.scene, project_dir);
                return variant_t(integer_t(1));
            };

            fmap["rayrender"] = [](rt_prog_ctx_t& ctx,
                                    const std::string& name,
                                    const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::INTEGER,
                        variant_t::type_t::INTEGER },
                    vargs);

                auto* engine = checked_engine(ctx, name);
                auto* world = checked_world(ctx, name);
                const auto width =
                    std::max(16, static_cast<int>(vargs[0].to_int()));
                const auto height =
                    std::max(16, static_cast<int>(vargs[1].to_int()));
                engine->renderToFrameBuffer(
                    *world,
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height));
                return variant_t(integer_t(1));
            };

            fmap["rayframehash"] = [](rt_prog_ctx_t& ctx,
                                       const std::string& name,
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

            fmap["rayplayerx"] = [](rt_prog_ctx_t& ctx,
                                     const std::string& name,
                                     const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(
                    double_t(checked_engine(ctx, name)->player().getX()));
            };

            fmap["rayplayery"] = [](rt_prog_ctx_t& ctx,
                                     const std::string& name,
                                     const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(
                    double_t(checked_engine(ctx, name)->player().getY()));
            };

            fmap["rayplayerfacing"] = [](rt_prog_ctx_t& ctx,
                                          const std::string& name,
                                          const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(double_t(camera_facing_degrees(
                    checked_engine(ctx, name)->player())));
            };

            fmap["raysetplayer"] = [](rt_prog_ctx_t& ctx,
                                       const std::string& name,
                                       const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::DOUBLE,
                        variant_t::type_t::DOUBLE,
                        variant_t::type_t::DOUBLE },
                    vargs);

                auto* engine = checked_engine(ctx, name);
                auto& player = engine->player();
                player.setPos({ vargs[0].to_double(), vargs[1].to_double() });
                player.setAlpha(player_alpha_for_facing_degrees(
                    player,
                    vargs[2].to_double()));
                return variant_t(integer_t(1));
            };

            fmap["raymove"] = [](rt_prog_ctx_t& ctx,
                                  const std::string& name,
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

            fmap["raystrafe"] = [](rt_prog_ctx_t& ctx,
                                    const std::string& name,
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

            fmap["rayturn"] = [](rt_prog_ctx_t& ctx,
                                  const std::string& name,
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

            fmap["rayupdate"] = [](rt_prog_ctx_t& ctx,
                                    const std::string& name,
                                    const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::DOUBLE }, vargs);
                auto& session = raycast_session();
                auto* engine = checked_engine(ctx, name);
                auto* world = checked_world(ctx, name);
                const auto dt = std::clamp(vargs[0].to_double(), 0.0, 0.1);
                world->advanceDynamicTextures(dt);
                world->updateDoors(engine->player().getX(),
                    engine->player().getY(), {}, dt, nullptr);
                session.actor_system.update(
                    *engine, *world, session.actors, dt);
                engine->advanceSpriteAnimations(dt);
                return variant_t(integer_t(1));
            };

            fmap["raypresent"] = [](rt_prog_ctx_t& ctx,
                                     const std::string& name,
                                     const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::INTEGER,
                        variant_t::type_t::INTEGER,
                        variant_t::type_t::INTEGER,
                        variant_t::type_t::INTEGER },
                    vargs);
#ifdef _WIN32
                auto* engine = checked_engine(ctx, name);
                const auto ok = present_framebuffer_to_gdi(
                    engine->frameBuffer(),
                    static_cast<int>(vargs[0].to_int()),
                    static_cast<int>(vargs[1].to_int()),
                    static_cast<int>(vargs[2].to_int()),
                    static_cast<int>(vargs[3].to_int()));
                return variant_t(integer_t(ok ? 1 : 0));
#else
                (void)ctx;
                return variant_t(integer_t(0));
#endif
            };

            fmap["raykeydown"] = [](rt_prog_ctx_t& ctx,
                                     const std::string& name,
                                     const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(ctx, name, args,
                    { variant_t::type_t::INTEGER }, vargs);
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

            fmap["rayspritecount"] = [](rt_prog_ctx_t& ctx,
                                         const std::string& name,
                                         const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(integer_t(
                    checked_engine(ctx, name)->sprites().size()));
            };

            fmap["rayactorcount"] = [](rt_prog_ctx_t&,
                                        const std::string& name,
                                        const func_args_t& args) {
                check_arg_num(args, 0, name);
                return variant_t(integer_t(raycast_session().actors.size()));
            };

            fmap["raysetbasedir"] = [](rt_prog_ctx_t& ctx,
                                        const std::string& name,
                                        const func_args_t& args) {
                std::vector<variant_t> vargs;
                get_functor_vargs(
                    ctx, name, args, { variant_t::type_t::STRING }, vargs);
                raycast_session().base_dir =
                    normalize_base_dir_path(vargs[0].to_str());
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
                    syntax_error_if(
                        true,
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
