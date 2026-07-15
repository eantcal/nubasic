#include "ActorSystem.h"

#include <gtest/gtest.h>

namespace {
constexpr Cell kEmpty = 0x0000000000ULL;
constexpr Cell kWall = 0x0000000001ULL;
constexpr Cell kTransparentWall = 0x0000000001000000ULL;

WorldMap makeOpenMap()
{
    const Cell cells[] = {
        kEmpty, kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty, kEmpty,
        kEmpty, kEmpty, kEmpty, kEmpty,
    };

    WorldMap map;
    EXPECT_TRUE(map.setCells(cells, 3, 4));
    map.resizeCell(64, 64);
    return map;
}

WorldMap makeBlockedMap()
{
    const Cell cells[] = {
        kEmpty, kEmpty, kWall, kEmpty,
        kEmpty, kEmpty, kWall, kEmpty,
        kEmpty, kEmpty, kWall, kEmpty,
    };

    WorldMap map;
    EXPECT_TRUE(map.setCells(cells, 3, 4));
    map.resizeCell(64, 64);
    return map;
}

WorldMap makeTransparentBlockedMap()
{
    const Cell cells[] = {
        kEmpty, kEmpty, kTransparentWall, kEmpty,
        kEmpty, kEmpty, kTransparentWall, kEmpty,
        kEmpty, kEmpty, kTransparentWall, kEmpty,
    };

    WorldMap map;
    EXPECT_TRUE(map.setCells(cells, 3, 4));
    map.resizeCell(64, 64);
    return map;
}

Sprite makeActorSprite()
{
    Sprite sprite;
    sprite.x = 96.0;
    sprite.y = 96.0;
    sprite.collisionRadius = 8.0;
    sprite.frames = DirectionalSpriteFrames({ { 1 } });
    sprite.animations = {
        SpriteAnimationClip("idle", DirectionalSpriteFrames({ { 2 } }), {}, 160.0, true),
        SpriteAnimationClip("walk", DirectionalSpriteFrames({ { 3 } }), {}, 120.0, true),
        SpriteAnimationClip("death", DirectionalSpriteFrames({ { 4 } }), {}, 120.0, false),
    };
    sprite.setAnimation("idle");
    return sprite;
}

SpriteActor makeChasingActor()
{
    SpriteActor actor;
    actor.spriteIndex = 0;
    actor.speedCellsPerSecond = 1.0;
    actor.detectionRadiusCells = 4.0;
    actor.stoppingDistanceCells = 0.5;
    actor.collidesWithWorld = true;
    return actor;
}
}

TEST(ActorSystemTests, ChasingActorMovesTowardPlayer)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 224.0, 96.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    std::vector<SpriteActor> actors = { makeChasingActor() };
    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    ASSERT_EQ(1u, engine.sprites().size());
    EXPECT_DOUBLE_EQ(160.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
    EXPECT_EQ(ActorState::Chasing, actors[0].state);
    EXPECT_EQ("walk", engine.sprites()[0].activeAnimation);
}

TEST(ActorSystemTests, ChasingActorStopsAtStoppingDistance)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 120.0, 96.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    std::vector<SpriteActor> actors = { makeChasingActor() };
    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
    EXPECT_EQ(ActorState::Idle, actors[0].state);
    EXPECT_EQ("idle", engine.sprites()[0].activeAnimation);
}

TEST(ActorSystemTests, ChasingActorPlaysAttackAtStoppingDistance)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 120.0, 96.0 });

    auto sprite = makeActorSprite();
    sprite.animations.push_back(
        SpriteAnimationClip("attack", DirectionalSpriteFrames({ { 4 } }), {}, 110.0, true));
    RaycastEngine engine(player, 250000);
    engine.addSprite(sprite);

    std::vector<SpriteActor> actors = { makeChasingActor() };
    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_EQ(ActorState::Idle, actors[0].state);
    EXPECT_EQ("attack", engine.sprites()[0].activeAnimation);
}

TEST(ActorSystemTests, RangedActorStopsWhileBurstIsActive)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 224.0, 96.0 });

    auto sprite = makeActorSprite();
    sprite.animations.push_back(
        SpriteAnimationClip("attack", DirectionalSpriteFrames({ { 4 } }), {}, 110.0, true));

    RaycastEngine engine(player, 250000);
    engine.addSprite(sprite);

    auto actor = makeChasingActor();
    actor.rangedAttack = true;
    actor.attackBurstShotsRemaining = 2;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
    EXPECT_EQ(ActorState::Attacking, actors[0].state);
    EXPECT_EQ("attack", engine.sprites()[0].activeAnimation);
}

TEST(ActorSystemTests, ChasingActorRespectsSolidWallCollision)
{
    auto map = makeBlockedMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 224.0, 96.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    std::vector<SpriteActor> actors = { makeChasingActor() };
    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
    EXPECT_EQ(ActorState::Idle, actors[0].state);
}

TEST(ActorSystemTests, ChasingActorCanPassThroughWallsWhenConfigured)
{
    auto map = makeBlockedMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 224.0, 96.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    auto actor = makeChasingActor();
    actor.collidesWithWorld = false;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_DOUBLE_EQ(160.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
    EXPECT_EQ(ActorState::Chasing, actors[0].state);
}

TEST(ActorSystemTests, IdleActorDoesNotStartChaseInsideHysteresisBand)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 192.0, 96.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    auto actor = makeChasingActor();
    actor.detectionRadiusCells = 1.0;
    actor.engagementHysteresisCells = 1.0;
    actor.patrolCircuit = true;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_EQ(ActorState::Patrolling, actors[0].state);
    EXPECT_DOUBLE_EQ(160.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
}

TEST(ActorSystemTests, NoiseAlertStartsChaseBeyondDetectionRadius)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 224.0, 96.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    auto actor = makeChasingActor();
    actor.detectionRadiusCells = 1.0;
    actor.noiseAlertSecondsRemaining = 2.0;
    actor.noiseAlertRadiusCells = 3.0;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_EQ(ActorState::Chasing, actors[0].state);
    EXPECT_DOUBLE_EQ(160.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
    EXPECT_EQ("walk", engine.sprites()[0].activeAnimation);
}

TEST(ActorSystemTests, ExpiredNoiseAlertDoesNotStartChase)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 224.0, 96.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    auto actor = makeChasingActor();
    actor.detectionRadiusCells = 1.0;
    actor.noiseAlertSecondsRemaining = 0.05;
    actor.noiseAlertRadiusCells = 3.0;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 0.1);

    EXPECT_EQ(ActorState::Idle, actors[0].state);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
    EXPECT_EQ("idle", engine.sprites()[0].activeAnimation);
}

TEST(ActorSystemTests, ChasingActorKeepsChaseInsideHysteresisBand)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 192.0, 96.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    auto actor = makeChasingActor();
    actor.state = ActorState::Chasing;
    actor.detectionRadiusCells = 1.0;
    actor.engagementHysteresisCells = 1.0;
    actor.patrolCircuit = true;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_EQ(ActorState::Chasing, actors[0].state);
    EXPECT_DOUBLE_EQ(160.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
}

TEST(ActorSystemTests, ActorIgnoresPlayerOutsidePatrolRadius)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 256.0, 96.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    auto actor = makeChasingActor();
    actor.patrolRadiusCells = 1.0;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
    EXPECT_EQ(ActorState::Idle, actors[0].state);
}

TEST(ActorSystemTests, ActorReturnsHomeWhenOutsidePatrolRadius)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 32.0, 32.0 });

    auto sprite = makeActorSprite();
    sprite.x = 224.0;
    sprite.y = 96.0;

    RaycastEngine engine(player, 250000);
    engine.addSprite(sprite);

    auto actor = makeChasingActor();
    actor.homeX = 96.0;
    actor.homeY = 96.0;
    actor.hasHomePosition = true;
    actor.patrolRadiusCells = 1.0;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_DOUBLE_EQ(160.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
    EXPECT_EQ(ActorState::Returning, actors[0].state);
    EXPECT_EQ("walk", engine.sprites()[0].activeAnimation);
}

TEST(ActorSystemTests, PatrollingActorMovesWhenPlayerIsOutsideActionRadius)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 300.0, 220.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    auto actor = makeChasingActor();
    actor.patrolRadiusCells = 2.0;
    actor.patrolCircuit = true;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_DOUBLE_EQ(160.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
    EXPECT_EQ(ActorState::Patrolling, actors[0].state);
    EXPECT_EQ(0, actors[0].patrolDirection);
}

TEST(ActorSystemTests, PatrollingActorTurnsBeforeSolidWall)
{
    auto map = makeBlockedMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 300.0, 220.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    auto actor = makeChasingActor();
    actor.patrolRadiusCells = 2.0;
    actor.patrolCircuit = true;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(160.0, engine.sprites()[0].y);
    EXPECT_EQ(ActorState::Patrolling, actors[0].state);
    EXPECT_EQ(1, actors[0].patrolDirection);
}

TEST(ActorSystemTests, PatrollingActorTreatsTransparentWallAsBoundary)
{
    auto map = makeTransparentBlockedMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 300.0, 220.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    auto actor = makeChasingActor();
    actor.patrolRadiusCells = 2.0;
    actor.patrolCircuit = true;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 1.0);

    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(160.0, engine.sprites()[0].y);
    EXPECT_EQ(ActorState::Patrolling, actors[0].state);
}

TEST(ActorSystemTests, DeadActorPlaysDeathAnimationAndDoesNotChase)
{
    auto map = makeOpenMap();
    Player player(0, 0, 60, 60, 40);
    player.setPos({ 224.0, 96.0 });

    RaycastEngine engine(player, 250000);
    engine.addSprite(makeActorSprite());

    auto actor = makeChasingActor();
    actor.maxHealth = 50.0;
    actor.health = 0.0;
    std::vector<SpriteActor> actors = { actor };

    ActorSystem actorSystem;
    actorSystem.update(engine, map, actors, 0.1);

    EXPECT_TRUE(actors[0].dead);
    EXPECT_TRUE(actors[0].deathAnimationStarted);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].x);
    EXPECT_DOUBLE_EQ(96.0, engine.sprites()[0].y);
    EXPECT_EQ("death", engine.sprites()[0].activeAnimation);
}
