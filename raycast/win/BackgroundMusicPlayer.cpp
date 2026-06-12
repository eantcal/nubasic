// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "BackgroundMusicPlayer.h"
#include "OggVorbisDecoder.h"

#include <mmsystem.h>
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <mutex>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>

namespace {
std::atomic<unsigned> g_nextAlias{ 1 };

std::string lowerExtension(const std::string& path)
{
    const auto dot = path.find_last_of('.');
    if (dot == std::string::npos) {
        return {};
    }

    auto extension = path.substr(dot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return extension;
}

bool isOggExtension(const std::string& extension) noexcept
{
    return extension == ".ogg" || extension == ".oga";
}

std::string mciDeviceType(const std::string& path)
{
    const auto extension = lowerExtension(path);
    if (extension == ".wav") {
        return " type waveaudio";
    }

    if (extension == ".mp3" || extension == ".wma") {
        return " type mpegvideo";
    }

    if (extension == ".mid" || extension == ".midi") {
        return " type sequencer";
    }

    return {};
}

std::string mciErrorText(MCIERROR error)
{
    char text[256] = { 0 };
    if (mciGetErrorStringA(error, text, sizeof(text))) {
        return text;
    }

    std::ostringstream stream;
    stream << "MCI error " << error;
    return stream.str();
}

std::string waveErrorText(MMRESULT error)
{
    char text[256] = { 0 };
    if (waveOutGetErrorTextA(error, text, sizeof(text)) == MMSYSERR_NOERROR) {
        return text;
    }

    std::ostringstream stream;
    stream << "waveOut error " << error;
    return stream.str();
}

bool sendMci(const std::string& command, std::string* error)
{
    const auto result = mciSendStringA(command.c_str(), nullptr, 0, nullptr);
    if (result == 0) {
        return true;
    }

    if (error != nullptr) {
        *error = mciErrorText(result) + " [" + command + "]";
    }

    return false;
}

int clampVolumePercent(int volumePercent) noexcept
{
    if (volumePercent < 0) {
        return 0;
    }

    if (volumePercent > 100) {
        return 100;
    }

    return volumePercent;
}

int volumePercentToMci(int volumePercent) noexcept
{
    return clampVolumePercent(volumePercent) * 10;
}

DWORD volumePercentToWaveOut(int volumePercent) noexcept
{
    const auto value = static_cast<DWORD>(
        (clampVolumePercent(volumePercent) * 0xFFFF) / 100);
    return value | (value << 16);
}

bool setWaveVolume(HWAVEOUT device, int volumePercent, std::string* error)
{
    const auto result
        = waveOutSetVolume(device, volumePercentToWaveOut(volumePercent));
    if (result == MMSYSERR_NOERROR) {
        return true;
    }

    if (error != nullptr) {
        *error = waveErrorText(result);
    }
    return false;
}
} // namespace

struct BackgroundMusicPlayer::WaveBackend {
    std::vector<short> samples;
    int channels = 0;
    int sampleRate = 0;
    bool loop = true;
    std::atomic_bool stopRequested{ false };
    std::thread thread;
    std::mutex mutex;
    HWAVEOUT device = nullptr;
};

BackgroundMusicPlayer::BackgroundMusicPlayer()
{
    std::ostringstream alias;
    alias << "winraycast_music_" << g_nextAlias++;
    m_alias = alias.str();
}

BackgroundMusicPlayer::~BackgroundMusicPlayer() { stop(); }

bool BackgroundMusicPlayer::play(
    const std::string& path, bool loop, int volumePercent, std::string* error)
{
    stop();
    m_volumePercent = clampVolumePercent(volumePercent);

    if (path.empty()) {
        return true;
    }

    if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        if (error != nullptr) {
            *error = "Background music file not found: " + path;
        }
        return false;
    }

    const auto extension = lowerExtension(path);
    if (isOggExtension(extension)) {
        auto wave = std::make_unique<WaveBackend>();
        wave->loop = loop;

        if (!decodeVorbisFile(
                path, wave->samples, wave->channels, wave->sampleRate, error)) {
            return false;
        }

        WAVEFORMATEX format{};
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels = static_cast<WORD>(wave->channels);
        format.nSamplesPerSec = static_cast<DWORD>(wave->sampleRate);
        format.wBitsPerSample = 16;
        format.nBlockAlign
            = static_cast<WORD>((format.nChannels * format.wBitsPerSample) / 8);
        format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

        const auto openResult = waveOutOpen(
            &wave->device, WAVE_MAPPER, &format, 0, 0, CALLBACK_NULL);
        if (openResult != MMSYSERR_NOERROR) {
            if (error != nullptr) {
                *error = waveErrorText(openResult);
            }
            return false;
        }

        if (!setWaveVolume(wave->device, m_volumePercent, error)) {
            waveOutClose(wave->device);
            return false;
        }

        m_wave = std::move(wave);
        m_backend = Backend::WaveOut;
        m_open = true;
        m_path = path;
        m_wave->thread
            = std::thread([backend = m_wave.get()] { runWaveLoop(*backend); });
        return true;
    }

    const auto openCommand
        = "open \"" + path + "\"" + mciDeviceType(path) + " alias " + m_alias;
    if (!sendMci(openCommand, error)) {
        return false;
    }

    m_open = true;
    m_backend = Backend::Mci;
    m_path = path;

    if (!setVolumePercent(m_volumePercent, error)) {
        stop();
        return false;
    }

    const auto playCommand
        = "play " + m_alias + (loop ? " repeat" : std::string());
    if (!sendMci(playCommand, error)) {
        stop();
        return false;
    }

    return true;
}

bool BackgroundMusicPlayer::setVolumePercent(
    int volumePercent, std::string* error)
{
    m_volumePercent = clampVolumePercent(volumePercent);
    if (!m_open) {
        return true;
    }

    if (m_backend == Backend::WaveOut && m_wave != nullptr) {
        std::lock_guard<std::mutex> lock(m_wave->mutex);
        if (m_wave->device == nullptr) {
            return true;
        }
        return setWaveVolume(m_wave->device, m_volumePercent, error);
    }

    const auto command = "setaudio " + m_alias + " volume to "
        + std::to_string(volumePercentToMci(m_volumePercent));
    return sendMci(command, error);
}

void BackgroundMusicPlayer::stop() noexcept
{
    if (!m_open) {
        return;
    }

    if (m_backend == Backend::WaveOut && m_wave != nullptr) {
        m_wave->stopRequested.store(true);
        {
            std::lock_guard<std::mutex> lock(m_wave->mutex);
            if (m_wave->device != nullptr) {
                waveOutReset(m_wave->device);
            }
        }

        if (m_wave->thread.joinable()) {
            m_wave->thread.join();
        }

        {
            std::lock_guard<std::mutex> lock(m_wave->mutex);
            if (m_wave->device != nullptr) {
                waveOutClose(m_wave->device);
                m_wave->device = nullptr;
            }
        }

        m_wave.reset();
        m_open = false;
        m_backend = Backend::None;
        m_path.clear();
        return;
    }

    const auto stopCommand = "stop " + m_alias;
    mciSendStringA(stopCommand.c_str(), nullptr, 0, nullptr);

    const auto closeCommand = "close " + m_alias;
    mciSendStringA(closeCommand.c_str(), nullptr, 0, nullptr);

    m_open = false;
    m_backend = Backend::None;
    m_path.clear();
}

void BackgroundMusicPlayer::runWaveLoop(WaveBackend& backend) noexcept
{
    while (!backend.stopRequested.load()) {
        WAVEHDR header{};
        header.lpData = reinterpret_cast<LPSTR>(backend.samples.data());
        header.dwBufferLength
            = static_cast<DWORD>(backend.samples.size() * sizeof(short));

        {
            std::lock_guard<std::mutex> lock(backend.mutex);
            if (backend.device == nullptr) {
                return;
            }

            if (waveOutPrepareHeader(backend.device, &header, sizeof(header))
                != MMSYSERR_NOERROR) {
                return;
            }

            if (waveOutWrite(backend.device, &header, sizeof(header))
                != MMSYSERR_NOERROR) {
                waveOutUnprepareHeader(backend.device, &header, sizeof(header));
                return;
            }
        }

        while (!backend.stopRequested.load()
            && (header.dwFlags & WHDR_DONE) == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        {
            std::lock_guard<std::mutex> lock(backend.mutex);
            if (backend.device != nullptr) {
                waveOutUnprepareHeader(backend.device, &header, sizeof(header));
            }
        }

        if (!backend.loop) {
            return;
        }
    }
}
