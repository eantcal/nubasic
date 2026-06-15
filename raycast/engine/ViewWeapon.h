// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include "Texture.h"

#include <memory>
#include <string>
#include <vector>

class ViewWeapon {
public:
    struct Animation {
        std::string name;
        std::vector<std::shared_ptr<Texture>> frames;
        double frameDurationMs = 0.0;
        bool loop = true;
    };

    void setName(std::string name);
    const std::string& name() const noexcept;

    void setScreenHeightFraction(double value) noexcept;
    double screenHeightFraction() const noexcept;

    void setDamage(double value) noexcept;
    double damage() const noexcept;

    void setRangeCells(double value) noexcept;
    double rangeCells() const noexcept;

    void setFireSoundPath(std::string path);
    const std::string& fireSoundPath() const noexcept;

    void setAmmo(int magazineSize, int maxAmmo, int initialAmmo = -1) noexcept;
    bool usesAmmo() const noexcept;
    int magazineSize() const noexcept;
    int maxAmmo() const noexcept;
    int ammoInMagazine() const noexcept;
    int reserveAmmo() const noexcept;
    int totalAmmo() const noexcept;
    void setAmmoCounts(int ammoInMagazine, int reserveAmmo) noexcept;
    bool canFire() const noexcept;
    bool consumeRound() noexcept;
    bool refillAmmoToMax() noexcept;
    bool canReload() const noexcept;
    bool reload() noexcept;
    bool needsReload() const noexcept;

    void setAnchor(double x, double y) noexcept;
    double anchorX() const noexcept;
    double anchorY() const noexcept;

    void setBaseOffset(double x, double y) noexcept;
    double baseOffsetX() const noexcept;
    double baseOffsetY() const noexcept;

    void setBob(
        double amplitudeX, double amplitudeY, double frequencyHz) noexcept;
    void setBob(bool enabled, double amount, double amplitudeX,
        double amplitudeY, double frequencyHz) noexcept;
    bool bobEnabled() const noexcept;
    double bobAmountScale() const noexcept;
    double bobOffsetX() const noexcept;
    double bobOffsetY() const noexcept;

    void addAnimation(Animation animation);
    bool setAnimation(const std::string& name) noexcept;
    bool setAnimationOrFallback(
        const std::string& name, const std::string& fallback) noexcept;
    bool restartAnimation(const std::string& name) noexcept;
    bool restartAnimationOrFallback(
        const std::string& name, const std::string& fallback) noexcept;
    const std::string& activeAnimationName() const noexcept;

    void setFireBehavior(bool automatic, double intervalMs,
        double soundIntervalMs = 0.0) noexcept;
    bool automaticFire() const noexcept;
    bool fireEventReady() const noexcept;
    void markFireEventStarted() noexcept;
    bool fireSoundReady() const noexcept;
    void markFireSoundStarted() noexcept;

    void advance(double deltaSeconds, bool playerIsMoving) noexcept;
    const Texture* currentFrame() const noexcept;

private:
    const Animation* activeAnimation() const noexcept;

    std::string m_name;
    std::vector<Animation> m_animations;
    std::string m_activeAnimation;
    double m_animationTimeSeconds = 0.0;
    size_t m_frameIndex = 0;

    double m_screenHeightFraction = 0.45;
    double m_damage = 0.0;
    double m_rangeCells = 8.0;
    std::string m_fireSoundPath;
    bool m_usesAmmo = false;
    int m_magazineSize = 0;
    int m_maxAmmo = 0;
    int m_ammoInMagazine = 0;
    int m_reserveAmmo = 0;
    double m_anchorX = 0.5;
    double m_anchorY = 1.0;
    double m_baseOffsetX = 0.0;
    double m_baseOffsetY = 0.0;

    bool m_bobEnabled = true;
    double m_bobAmountScale = 1.0;
    double m_bobAmplitudeX = 6.0;
    double m_bobAmplitudeY = 4.0;
    double m_bobFrequencyHz = 3.0;
    double m_bobPhase = 0.0;
    double m_bobAmount = 0.0;
    double m_bobOffsetX = 0.0;
    double m_bobOffsetY = 0.0;

    bool m_automaticFire = false;
    double m_fireIntervalSeconds = 0.0;
    double m_fireCooldownSeconds = 0.0;
    double m_fireSoundIntervalSeconds = 0.0;
    double m_fireSoundCooldownSeconds = 0.0;
};
