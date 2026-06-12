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
#include "Player.h"
#include "RaycastEngine.h"
#include "WorldJsonLoader.h"
#include "WorldMap.h"

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <memory>
#include <string>
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
        int projection_width = kDefaultProjectionWidth;
        int projection_height = kDefaultProjectionHeight;
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
        return true;
    }

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
                "rayturn"
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

                auto world = std::make_unique<WorldMap>();
                WorldJsonLoader loader;
                const auto path = vargs[0].to_str();
                const auto result = loader.loadFromFile(path, *world);
                if (!result.success) {
                    ctx.set_errno(EINVAL);
                    return variant_t(integer_t(0));
                }

                world->resizeCell(kDefaultCellSize, kDefaultCellSize);

                Player player(
                    0,
                    0,
                    kDefaultVisualDegrees,
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
                    player.setPos({
                        kDefaultCellSize * 1.5,
                        kDefaultCellSize * 1.5
                    });
                }

                auto& session = raycast_session();
                session.world = std::move(world);
                session.engine =
                    std::make_unique<RaycastEngine>(player, kDefaultScale);
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
