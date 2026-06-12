// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "ViewWeapon.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace {
constexpr double kPi = 3.14159265358979323846;

double clamp(double value, double minValue, double maxValue) noexcept
{
    return std::max(minValue, std::min(maxValue, value));
}
} // namespace

void ViewWeapon::setName(std::string name) { m_name = std::move(name); }

const std::string& ViewWeapon::name() const noexcept { return m_name; }

void ViewWeapon::setScreenHeightFraction(double value) noexcept
{
    m_screenHeightFraction = clamp(value, 0.05, 1.25);
}

double ViewWeapon::screenHeightFraction() const noexcept
{
    return m_screenHeightFraction;
}

void ViewWeapon::setDamage(double value) noexcept
{
    m_damage = std::max(0.0, value);
}

double ViewWeapon::damage() const noexcept { return m_damage; }

void ViewWeapon::setRangeCells(double value) noexcept
{
    m_rangeCells = clamp(value, 0.0, 64.0);
}

double ViewWeapon::rangeCells() const noexcept { return m_rangeCells; }

void ViewWeapon::setFireSoundPath(std::string path)
{
    m_fireSoundPath = std::move(path);
}

const std::string& ViewWeapon::fireSoundPath() const noexcept
{
    return m_fireSoundPath;
}

void ViewWeapon::setAmmo(
    int magazineSize, int maxAmmo, int initialAmmo) noexcept
{
    m_magazineSize = std::max(0, magazineSize);
    m_maxAmmo = std::max(0, maxAmmo);
    m_usesAmmo = m_magazineSize > 0 && m_maxAmmo > 0;

    if (!m_usesAmmo) {
        m_ammoInMagazine = 0;
        m_reserveAmmo = 0;
        return;
    }

    const auto totalAmmo = initialAmmo < 0
        ? m_maxAmmo
        : static_cast<int>(clamp(initialAmmo, 0, m_maxAmmo));
    m_ammoInMagazine = std::min(m_magazineSize, totalAmmo);
    m_reserveAmmo = totalAmmo - m_ammoInMagazine;
}

bool ViewWeapon::usesAmmo() const noexcept { return m_usesAmmo; }

int ViewWeapon::magazineSize() const noexcept { return m_magazineSize; }

int ViewWeapon::maxAmmo() const noexcept { return m_maxAmmo; }

int ViewWeapon::ammoInMagazine() const noexcept { return m_ammoInMagazine; }

int ViewWeapon::reserveAmmo() const noexcept { return m_reserveAmmo; }

int ViewWeapon::totalAmmo() const noexcept
{
    return m_ammoInMagazine + m_reserveAmmo;
}

void ViewWeapon::setAmmoCounts(int ammoInMagazine, int reserveAmmo) noexcept
{
    if (!m_usesAmmo) {
        return;
    }

    m_ammoInMagazine
        = static_cast<int>(clamp(ammoInMagazine, 0, m_magazineSize));
    m_reserveAmmo = static_cast<int>(
        clamp(reserveAmmo, 0, (std::max)(0, m_maxAmmo - m_ammoInMagazine)));
}

bool ViewWeapon::canFire() const noexcept
{
    return !m_usesAmmo || m_ammoInMagazine > 0;
}

bool ViewWeapon::consumeRound() noexcept
{
    if (!m_usesAmmo) {
        return true;
    }

    if (m_ammoInMagazine <= 0) {
        return false;
    }

    --m_ammoInMagazine;
    return true;
}

bool ViewWeapon::refillAmmoToMax() noexcept
{
    if (!m_usesAmmo) {
        return false;
    }

    const auto previousMagazine = m_ammoInMagazine;
    const auto previousReserve = m_reserveAmmo;
    m_ammoInMagazine = std::min(m_magazineSize, m_maxAmmo);
    m_reserveAmmo = std::max(0, m_maxAmmo - m_ammoInMagazine);
    return previousMagazine != m_ammoInMagazine
        || previousReserve != m_reserveAmmo;
}

bool ViewWeapon::canReload() const noexcept
{
    return m_usesAmmo && m_ammoInMagazine < m_magazineSize && m_reserveAmmo > 0;
}

bool ViewWeapon::reload() noexcept
{
    if (!canReload()) {
        return false;
    }

    const auto missingRounds = m_magazineSize - m_ammoInMagazine;
    const auto loadedRounds = std::min(missingRounds, m_reserveAmmo);
    m_ammoInMagazine += loadedRounds;
    m_reserveAmmo -= loadedRounds;
    return loadedRounds > 0;
}

bool ViewWeapon::needsReload() const noexcept
{
    return m_usesAmmo && m_ammoInMagazine <= 0 && m_reserveAmmo > 0;
}

void ViewWeapon::setAnchor(double x, double y) noexcept
{
    m_anchorX = clamp(x, 0.0, 1.0);
    m_anchorY = clamp(y, 0.0, 1.25);
}

double ViewWeapon::anchorX() const noexcept { return m_anchorX; }

double ViewWeapon::anchorY() const noexcept { return m_anchorY; }

void ViewWeapon::setBaseOffset(double x, double y) noexcept
{
    m_baseOffsetX = x;
    m_baseOffsetY = y;
}

double ViewWeapon::baseOffsetX() const noexcept { return m_baseOffsetX; }

double ViewWeapon::baseOffsetY() const noexcept { return m_baseOffsetY; }

void ViewWeapon::setBob(
    double amplitudeX, double amplitudeY, double frequencyHz) noexcept
{
    setBob(true, 1.0, amplitudeX, amplitudeY, frequencyHz);
}

void ViewWeapon::setBob(bool enabled, double amount, double amplitudeX,
    double amplitudeY, double frequencyHz) noexcept
{
    m_bobEnabled = enabled;
    m_bobAmountScale = clamp(amount, 0.0, 2.0);
    m_bobAmplitudeX = std::max(0.0, amplitudeX);
    m_bobAmplitudeY = std::max(0.0, amplitudeY);
    m_bobFrequencyHz = std::max(0.0, frequencyHz);
}

bool ViewWeapon::bobEnabled() const noexcept { return m_bobEnabled; }

double ViewWeapon::bobAmountScale() const noexcept { return m_bobAmountScale; }

double ViewWeapon::bobOffsetX() const noexcept { return m_bobOffsetX; }

double ViewWeapon::bobOffsetY() const noexcept { return m_bobOffsetY; }

void ViewWeapon::addAnimation(Animation animation)
{
    if (animation.name.empty() || animation.frames.empty()) {
        return;
    }

    if (animation.frameDurationMs <= 0.0) {
        animation.frameDurationMs = 100.0;
    }

    if (m_activeAnimation.empty()) {
        m_activeAnimation = animation.name;
    }

    m_animations.push_back(std::move(animation));
}

bool ViewWeapon::setAnimation(const std::string& name) noexcept
{
    if (name.empty()) {
        return false;
    }

    const auto item = std::find_if(m_animations.begin(), m_animations.end(),
        [&name](const Animation& animation) { return animation.name == name; });

    if (item == m_animations.end()) {
        return false;
    }

    if (m_activeAnimation != name) {
        m_activeAnimation = name;
        m_animationTimeSeconds = 0.0;
        m_frameIndex = 0;
    }

    return true;
}

bool ViewWeapon::setAnimationOrFallback(
    const std::string& name, const std::string& fallback) noexcept
{
    return setAnimation(name) || setAnimation(fallback);
}

bool ViewWeapon::restartAnimation(const std::string& name) noexcept
{
    if (name.empty()) {
        return false;
    }

    const auto item = std::find_if(m_animations.begin(), m_animations.end(),
        [&name](const Animation& animation) { return animation.name == name; });

    if (item == m_animations.end()) {
        return false;
    }

    m_activeAnimation = name;
    m_animationTimeSeconds = 0.0;
    m_frameIndex = 0;
    return true;
}

bool ViewWeapon::restartAnimationOrFallback(
    const std::string& name, const std::string& fallback) noexcept
{
    return restartAnimation(name) || restartAnimation(fallback);
}

const std::string& ViewWeapon::activeAnimationName() const noexcept
{
    return m_activeAnimation;
}

void ViewWeapon::setFireBehavior(
    bool automatic, double intervalMs, double soundIntervalMs) noexcept
{
    m_automaticFire = automatic;
    const auto defaultIntervalMs = automatic ? 120.0 : 0.0;
    const auto effectiveIntervalMs
        = intervalMs > 0.0 ? intervalMs : defaultIntervalMs;
    m_fireIntervalSeconds
        = clamp(effectiveIntervalMs / 1000.0, automatic ? 0.03 : 0.0, 5.0);
    m_fireCooldownSeconds = 0.0;

    const auto effectiveSoundIntervalMs
        = soundIntervalMs > 0.0 ? soundIntervalMs : effectiveIntervalMs;
    m_fireSoundIntervalSeconds
        = clamp(effectiveSoundIntervalMs / 1000.0, 0.0, 10.0);
    m_fireSoundCooldownSeconds = 0.0;
}

bool ViewWeapon::automaticFire() const noexcept { return m_automaticFire; }

bool ViewWeapon::fireEventReady() const noexcept
{
    return m_fireCooldownSeconds <= 0.0;
}

void ViewWeapon::markFireEventStarted() noexcept
{
    m_fireCooldownSeconds = m_fireIntervalSeconds;
}

bool ViewWeapon::fireSoundReady() const noexcept
{
    return m_fireSoundCooldownSeconds <= 0.0;
}

void ViewWeapon::markFireSoundStarted() noexcept
{
    m_fireSoundCooldownSeconds = m_fireSoundIntervalSeconds;
}

void ViewWeapon::advance(double deltaSeconds, bool playerIsMoving) noexcept
{
    if (deltaSeconds <= 0.0) {
        return;
    }

    m_fireCooldownSeconds = std::max(0.0, m_fireCooldownSeconds - deltaSeconds);
    m_fireSoundCooldownSeconds
        = std::max(0.0, m_fireSoundCooldownSeconds - deltaSeconds);

    const auto* animation = activeAnimation();
    if (animation != nullptr && !animation->frames.empty()) {
        m_animationTimeSeconds += deltaSeconds;
        const auto frameDurationSeconds = animation->frameDurationMs / 1000.0;
        if (frameDurationSeconds > 0.0) {
            const auto computedFrame = static_cast<size_t>(
                m_animationTimeSeconds / frameDurationSeconds);

            if (animation->loop) {
                m_frameIndex = computedFrame % animation->frames.size();
            } else if (computedFrame >= animation->frames.size()) {
                if (!setAnimation("idle")) {
                    m_frameIndex = animation->frames.size() - 1;
                }
            } else {
                m_frameIndex = computedFrame;
            }
        }
    }

    const auto targetBob
        = (m_bobEnabled && playerIsMoving) ? m_bobAmountScale : 0.0;
    const auto bobBlend = clamp(deltaSeconds * 7.0, 0.0, 1.0);
    m_bobAmount += (targetBob - m_bobAmount) * bobBlend;

    if (m_bobAmount > 0.001 && m_bobFrequencyHz > 0.0) {
        m_bobPhase += deltaSeconds * m_bobFrequencyHz * 2.0 * kPi;
        if (m_bobPhase > 2.0 * kPi) {
            m_bobPhase = std::fmod(m_bobPhase, 2.0 * kPi);
        }
    }

    m_bobOffsetX = std::sin(m_bobPhase) * m_bobAmplitudeX * m_bobAmount;
    m_bobOffsetY
        = (1.0 - std::cos(m_bobPhase)) * 0.5 * m_bobAmplitudeY * m_bobAmount;
}

const Texture* ViewWeapon::currentFrame() const noexcept
{
    const auto* animation = activeAnimation();
    if (animation == nullptr || animation->frames.empty()) {
        return nullptr;
    }

    const auto frame
        = animation
              ->frames[std::min(m_frameIndex, animation->frames.size() - 1)];
    return frame.get();
}

const ViewWeapon::Animation* ViewWeapon::activeAnimation() const noexcept
{
    const auto item = std::find_if(m_animations.begin(), m_animations.end(),
        [this](const Animation& animation) {
            return animation.name == m_activeAnimation;
        });

    return item == m_animations.end() ? nullptr : &*item;
}
