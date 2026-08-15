#include "Color.h"
#include "ViewWeapon.h"

#include <gtest/gtest.h>

#include <memory>

namespace {
std::shared_ptr<Texture> makeTexture(Color color)
{
    auto texture = std::make_shared<Texture>(2, 2);
    for (uint32_t y = 0; y < 2; ++y) {
        for (uint32_t x = 0; x < 2; ++x) {
            texture->setPixel(x, y, color);
        }
    }

    return texture;
}
}

TEST(ViewWeaponTests, NonLoopingAnimationReturnsToIdle)
{
    ViewWeapon weapon;

    ViewWeapon::Animation idle;
    idle.name = "idle";
    idle.frames = { makeTexture(makeColor(8, 16, 24)) };
    idle.frameDurationMs = 100.0;
    idle.loop = true;
    weapon.addAnimation(idle);

    ViewWeapon::Animation fire;
    fire.name = "fire";
    fire.frames = {
        makeTexture(makeColor(255, 64, 0)),
        makeTexture(makeColor(255, 180, 0)),
    };
    fire.frameDurationMs = 50.0;
    fire.loop = false;
    weapon.addAnimation(fire);

    ASSERT_TRUE(weapon.setAnimation("fire"));
    EXPECT_EQ("fire", weapon.activeAnimationName());

    weapon.advance(0.12, false);

    EXPECT_EQ("idle", weapon.activeAnimationName());
}

TEST(ViewWeaponTests, BobFollowsMovementState)
{
    ViewWeapon weapon;

    ViewWeapon::Animation idle;
    idle.name = "idle";
    idle.frames = { makeTexture(makeColor(1, 2, 3)) };
    idle.frameDurationMs = 100.0;
    weapon.addAnimation(idle);

    weapon.setBob(16.0, 10.0, 2.0);
    weapon.advance(0.1, true);

    EXPECT_NE(0.0, weapon.bobOffsetX());
    EXPECT_NE(0.0, weapon.bobOffsetY());

    weapon.advance(1.0, false);

    EXPECT_DOUBLE_EQ(0.0, weapon.bobOffsetX());
    EXPECT_DOUBLE_EQ(0.0, weapon.bobOffsetY());
}

TEST(ViewWeaponTests, BobCanBeDisabledAndScaled)
{
    ViewWeapon weapon;

    ViewWeapon::Animation idle;
    idle.name = "idle";
    idle.frames = { makeTexture(makeColor(1, 2, 3)) };
    idle.frameDurationMs = 100.0;
    weapon.addAnimation(idle);

    weapon.setBob(false, 1.0, 16.0, 10.0, 2.0);
    weapon.advance(0.1, true);

    EXPECT_DOUBLE_EQ(0.0, weapon.bobOffsetX());
    EXPECT_DOUBLE_EQ(0.0, weapon.bobOffsetY());

    weapon.setBob(true, 0.25, 16.0, 10.0, 2.0);
    weapon.advance(0.1, true);

    EXPECT_NE(0.0, weapon.bobOffsetX());
    EXPECT_NE(0.0, weapon.bobOffsetY());
    EXPECT_DOUBLE_EQ(0.25, weapon.bobAmountScale());
}

TEST(ViewWeaponTests, CombatValuesAreClamped)
{
    ViewWeapon weapon;

    weapon.setDamage(42.0);
    weapon.setRangeCells(12.5);

    EXPECT_DOUBLE_EQ(42.0, weapon.damage());
    EXPECT_DOUBLE_EQ(12.5, weapon.rangeCells());

    weapon.setDamage(-1.0);
    weapon.setRangeCells(-1.0);

    EXPECT_DOUBLE_EQ(0.0, weapon.damage());
    EXPECT_DOUBLE_EQ(0.0, weapon.rangeCells());
}

TEST(ViewWeaponTests, AmmoMagazineReloadsFromReserve)
{
    ViewWeapon weapon;

    weapon.setAmmo(2, 14, 14);

    EXPECT_TRUE(weapon.usesAmmo());
    EXPECT_EQ(2, weapon.magazineSize());
    EXPECT_EQ(14, weapon.maxAmmo());
    EXPECT_EQ(2, weapon.ammoInMagazine());
    EXPECT_EQ(12, weapon.reserveAmmo());
    EXPECT_EQ(14, weapon.totalAmmo());

    EXPECT_TRUE(weapon.consumeRound());
    EXPECT_TRUE(weapon.consumeRound());
    EXPECT_FALSE(weapon.canFire());
    EXPECT_TRUE(weapon.needsReload());

    EXPECT_TRUE(weapon.reload());
    EXPECT_EQ(2, weapon.ammoInMagazine());
    EXPECT_EQ(10, weapon.reserveAmmo());
    EXPECT_EQ(12, weapon.totalAmmo());
}

TEST(ViewWeaponTests, AmmoPickupRefillsMagazineAndReserveToMax)
{
    ViewWeapon weapon;

    weapon.setAmmo(2, 14, 14);
    EXPECT_TRUE(weapon.consumeRound());
    EXPECT_TRUE(weapon.consumeRound());
    EXPECT_TRUE(weapon.reload());
    EXPECT_TRUE(weapon.consumeRound());

    EXPECT_EQ(1, weapon.ammoInMagazine());
    EXPECT_EQ(10, weapon.reserveAmmo());
    EXPECT_EQ(11, weapon.totalAmmo());

    EXPECT_TRUE(weapon.refillAmmoToMax());
    EXPECT_EQ(2, weapon.ammoInMagazine());
    EXPECT_EQ(12, weapon.reserveAmmo());
    EXPECT_EQ(14, weapon.totalAmmo());

    EXPECT_FALSE(weapon.refillAmmoToMax());
}

TEST(ViewWeaponTests, AmmoCountsCanBeRestoredWithinWeaponLimits)
{
    ViewWeapon weapon;

    weapon.setAmmo(2, 14, 14);
    weapon.setAmmoCounts(1, 4);

    EXPECT_EQ(1, weapon.ammoInMagazine());
    EXPECT_EQ(4, weapon.reserveAmmo());
    EXPECT_EQ(5, weapon.totalAmmo());

    weapon.setAmmoCounts(9, 99);

    EXPECT_EQ(2, weapon.ammoInMagazine());
    EXPECT_EQ(12, weapon.reserveAmmo());
    EXPECT_EQ(14, weapon.totalAmmo());
}

TEST(ViewWeaponTests, AutomaticFireCooldownAndAnimationRestart)
{
    ViewWeapon weapon;

    ViewWeapon::Animation idle;
    idle.name = "idle";
    idle.frames = { makeTexture(makeColor(1, 2, 3)) };
    idle.frameDurationMs = 100.0;
    idle.loop = true;
    weapon.addAnimation(idle);

    ViewWeapon::Animation fire;
    fire.name = "fire";
    fire.frames = {
        makeTexture(makeColor(255, 64, 0)),
        makeTexture(makeColor(255, 180, 0)),
    };
    fire.frameDurationMs = 50.0;
    fire.loop = false;
    weapon.addAnimation(fire);

    weapon.setFireBehavior(true, 120.0, 900.0);
    EXPECT_TRUE(weapon.automaticFire());
    EXPECT_TRUE(weapon.fireEventReady());
    EXPECT_TRUE(weapon.fireSoundReady());

    weapon.markFireEventStarted();
    weapon.markFireSoundStarted();
    EXPECT_FALSE(weapon.fireEventReady());
    EXPECT_FALSE(weapon.fireSoundReady());
    weapon.advance(0.05, false);
    EXPECT_FALSE(weapon.fireEventReady());
    EXPECT_FALSE(weapon.fireSoundReady());
    weapon.advance(0.07, false);
    EXPECT_TRUE(weapon.fireEventReady());
    EXPECT_FALSE(weapon.fireSoundReady());
    weapon.advance(0.78, false);
    EXPECT_TRUE(weapon.fireSoundReady());

    ASSERT_TRUE(weapon.restartAnimation("fire"));
    weapon.advance(0.06, false);
    ASSERT_TRUE(weapon.restartAnimation("fire"));
    EXPECT_EQ("fire", weapon.activeAnimationName());
    weapon.advance(0.04, false);
    EXPECT_EQ("fire", weapon.activeAnimationName());
}
