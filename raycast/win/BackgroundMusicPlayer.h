// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include <memory>
#include <string>

class BackgroundMusicPlayer {
public:
    BackgroundMusicPlayer();
    ~BackgroundMusicPlayer();

    BackgroundMusicPlayer(const BackgroundMusicPlayer&) = delete;
    BackgroundMusicPlayer& operator=(const BackgroundMusicPlayer&) = delete;

    bool play(const std::string& path, bool loop, int volumePercent,
        std::string* error = nullptr);
    void stop() noexcept;
    bool setVolumePercent(int volumePercent, std::string* error = nullptr);

    bool isOpen() const noexcept { return m_open; }
    const std::string& currentPath() const noexcept { return m_path; }
    int volumePercent() const noexcept { return m_volumePercent; }

private:
    enum class Backend { None, Mci, WaveOut };

    struct WaveBackend;

    static void runWaveLoop(WaveBackend& backend) noexcept;

    std::string m_alias;
    std::string m_path;
    std::unique_ptr<WaveBackend> m_wave;
    int m_volumePercent = 80;
    bool m_open = false;
    Backend m_backend = Backend::None;
};
