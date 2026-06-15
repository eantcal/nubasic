// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "SoundEffectPlayer.h"
#include "OggVorbisDecoder.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mmsystem.h>
#include <windows.h>
#include <wrl/client.h>

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstring>
#include <functional>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>

namespace {
using Microsoft::WRL::ComPtr;

std::atomic<unsigned> g_nextEffectAlias{ 1 };

struct PcmPlayback {
    std::vector<BYTE> bytes;
    WAVEFORMATEX format{};
    WAVEHDR header{};
    HWAVEOUT device = nullptr;
};

struct DecodedPcmSound {
    std::vector<BYTE> bytes;
    WAVEFORMATEX format{};
};

std::mutex g_pcmSoundCacheMutex;
std::unordered_map<std::string, std::shared_ptr<const DecodedPcmSound>>
    g_pcmSoundCache;

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

std::string hresultText(HRESULT result)
{
    char text[256] = { 0 };
    const auto length = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
        static_cast<DWORD>(result), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        text, sizeof(text), nullptr);

    if (length > 0) {
        return text;
    }

    std::ostringstream stream;
    stream << "HRESULT 0x" << std::hex << static_cast<unsigned long>(result);
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

DWORD CALLBACK runBackgroundTask(LPVOID parameter)
{
    std::unique_ptr<std::function<void()>> task(
        static_cast<std::function<void()>*>(parameter));
    try {
        (*task)();
    } catch (...) {
    }

    return 0;
}

bool queueBackgroundTask(std::function<void()> task, std::string* error)
{
    auto context = std::make_unique<std::function<void()>>(std::move(task));
    if (!QueueUserWorkItem(
            runBackgroundTask, context.get(), WT_EXECUTEDEFAULT)) {
        if (error != nullptr) {
            *error = "Could not queue sound effect on the Windows thread pool.";
        }
        return false;
    }

    context.release();
    return true;
}

std::wstring widenPath(const std::string& path)
{
    auto codePage = CP_UTF8;
    auto length = MultiByteToWideChar(
        codePage, MB_ERR_INVALID_CHARS, path.c_str(), -1, nullptr, 0);
    if (length == 0) {
        codePage = CP_ACP;
        length = MultiByteToWideChar(codePage, 0, path.c_str(), -1, nullptr, 0);
    }

    if (length <= 0) {
        return {};
    }

    std::wstring wide(static_cast<size_t>(length), L'\0');
    MultiByteToWideChar(codePage,
        codePage == CP_UTF8 ? MB_ERR_INVALID_CHARS : 0, path.c_str(), -1,
        &wide[0], length);
    if (!wide.empty() && wide.back() == L'\0') {
        wide.pop_back();
    }
    return wide;
}

bool ensureMediaFoundationStarted(std::string* error)
{
    static const HRESULT startupResult = MFStartup(MF_VERSION);
    if (SUCCEEDED(startupResult)) {
        return true;
    }

    if (error != nullptr) {
        *error
            = "Could not start Media Foundation: " + hresultText(startupResult);
    }
    return false;
}

bool decodePcmWithMediaFoundation(const std::string& path,
    std::vector<BYTE>& bytes, WAVEFORMATEX& format, std::string* error)
{
    if (!ensureMediaFoundationStarted(error)) {
        return false;
    }

    const auto coinitResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    const auto shouldUninitialize
        = SUCCEEDED(coinitResult) && coinitResult != RPC_E_CHANGED_MODE;
    if (FAILED(coinitResult) && coinitResult != RPC_E_CHANGED_MODE) {
        if (error != nullptr) {
            *error = "Could not initialize COM for audio decoding: "
                + hresultText(coinitResult);
        }
        return false;
    }

    const auto cleanupCom = [shouldUninitialize]() {
        if (shouldUninitialize) {
            CoUninitialize();
        }
    };

    const auto widePath = widenPath(path);
    if (widePath.empty()) {
        if (error != nullptr) {
            *error = "Could not convert sound effect path to UTF-16: " + path;
        }
        cleanupCom();
        return false;
    }

    ComPtr<IMFSourceReader> reader;
    auto result
        = MFCreateSourceReaderFromURL(widePath.c_str(), nullptr, &reader);
    if (FAILED(result)) {
        if (error != nullptr) {
            *error = "Could not open sound effect with Media Foundation: "
                + hresultText(result);
        }
        cleanupCom();
        return false;
    }

    reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
    result
        = reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
    if (FAILED(result)) {
        if (error != nullptr) {
            *error = "Could not select audio stream: " + hresultText(result);
        }
        cleanupCom();
        return false;
    }

    ComPtr<IMFMediaType> outputType;
    result = MFCreateMediaType(&outputType);
    if (FAILED(result)) {
        if (error != nullptr) {
            *error
                = "Could not create audio output type: " + hresultText(result);
        }
        cleanupCom();
        return false;
    }

    outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    outputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    outputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    result = reader->SetCurrentMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, outputType.Get());
    if (FAILED(result)) {
        if (error != nullptr) {
            *error = "Could not request PCM sound effect output: "
                + hresultText(result);
        }
        cleanupCom();
        return false;
    }

    ComPtr<IMFMediaType> currentType;
    result = reader->GetCurrentMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM, &currentType);
    if (FAILED(result)) {
        if (error != nullptr) {
            *error = "Could not inspect decoded audio format: "
                + hresultText(result);
        }
        cleanupCom();
        return false;
    }

    UINT32 channels = 0;
    UINT32 sampleRate = 0;
    UINT32 bitsPerSample = 16;
    currentType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
    currentType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
    currentType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);

    if (channels == 0 || sampleRate == 0 || bitsPerSample == 0) {
        if (error != nullptr) {
            *error = "Decoded audio format is invalid.";
        }
        cleanupCom();
        return false;
    }

    format = {};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = static_cast<WORD>(channels);
    format.nSamplesPerSec = sampleRate;
    format.wBitsPerSample = static_cast<WORD>(bitsPerSample);
    format.nBlockAlign
        = static_cast<WORD>((format.nChannels * format.wBitsPerSample) / 8);
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

    bytes.clear();
    for (;;) {
        DWORD flags = 0;
        ComPtr<IMFSample> sample;
        result = reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0,
            nullptr, &flags, nullptr, &sample);

        if (FAILED(result)) {
            if (error != nullptr) {
                *error = "Could not read decoded audio sample: "
                    + hresultText(result);
            }
            cleanupCom();
            return false;
        }

        if ((flags & MF_SOURCE_READERF_ENDOFSTREAM) != 0) {
            break;
        }

        if (sample == nullptr) {
            continue;
        }

        ComPtr<IMFMediaBuffer> buffer;
        result = sample->ConvertToContiguousBuffer(&buffer);
        if (FAILED(result)) {
            if (error != nullptr) {
                *error = "Could not flatten decoded audio sample: "
                    + hresultText(result);
            }
            cleanupCom();
            return false;
        }

        BYTE* data = nullptr;
        DWORD maxLength = 0;
        DWORD currentLength = 0;
        result = buffer->Lock(&data, &maxLength, &currentLength);
        if (FAILED(result)) {
            if (error != nullptr) {
                *error = "Could not lock decoded audio sample: "
                    + hresultText(result);
            }
            cleanupCom();
            return false;
        }

        bytes.insert(bytes.end(), data, data + currentLength);
        buffer->Unlock();
    }

    cleanupCom();

    if (bytes.empty()) {
        if (error != nullptr) {
            *error = "Decoded sound effect is empty: " + path;
        }
        return false;
    }

    return true;
}

bool decodeVorbisPcm(
    const std::string& path, DecodedPcmSound& sound, std::string* error)
{
    std::vector<short> samples;
    int channels = 0;
    int sampleRate = 0;
    if (!decodeVorbisFile(path, samples, channels, sampleRate, error)) {
        return false;
    }

    sound.format = {};
    sound.format.wFormatTag = WAVE_FORMAT_PCM;
    sound.format.nChannels = static_cast<WORD>(channels);
    sound.format.nSamplesPerSec = static_cast<DWORD>(sampleRate);
    sound.format.wBitsPerSample = 16;
    sound.format.nBlockAlign = static_cast<WORD>(
        (sound.format.nChannels * sound.format.wBitsPerSample) / 8);
    sound.format.nAvgBytesPerSec
        = sound.format.nSamplesPerSec * sound.format.nBlockAlign;

    sound.bytes.resize(samples.size() * sizeof(short));
    std::memcpy(sound.bytes.data(), samples.data(), sound.bytes.size());
    return !sound.bytes.empty();
}

bool decodeSoundToPcm(const std::string& path, const std::string& extension,
    DecodedPcmSound& sound, std::string* error)
{
    if (isOggExtension(extension)) {
        return decodeVorbisPcm(path, sound, error);
    }

    return decodePcmWithMediaFoundation(path, sound.bytes, sound.format, error);
}

std::shared_ptr<const DecodedPcmSound> cachedPcmSound(const std::string& path)
{
    std::lock_guard<std::mutex> lock(g_pcmSoundCacheMutex);
    const auto item = g_pcmSoundCache.find(path);
    return item == g_pcmSoundCache.end() ? nullptr : item->second;
}

std::shared_ptr<const DecodedPcmSound> cachePcmSound(
    const std::string& path, std::shared_ptr<const DecodedPcmSound> sound)
{
    std::lock_guard<std::mutex> lock(g_pcmSoundCacheMutex);
    const auto item = g_pcmSoundCache.find(path);
    if (item != g_pcmSoundCache.end()) {
        return item->second;
    }

    g_pcmSoundCache.emplace(path, sound);
    return sound;
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

bool playPcmOnce(std::vector<BYTE> bytes, const WAVEFORMATEX& format,
    int volumePercent, std::string* error)
{
    auto playback = std::make_shared<PcmPlayback>();
    playback->bytes = std::move(bytes);
    playback->format = format;

    const auto openResult = waveOutOpen(
        &playback->device, WAVE_MAPPER, &playback->format, 0, 0, CALLBACK_NULL);
    if (openResult != MMSYSERR_NOERROR) {
        if (error != nullptr) {
            *error = waveErrorText(openResult);
        }
        return false;
    }

    if (!setWaveVolume(playback->device, volumePercent, error)) {
        waveOutClose(playback->device);
        playback->device = nullptr;
        return false;
    }

    playback->header.lpData = reinterpret_cast<LPSTR>(playback->bytes.data());
    playback->header.dwBufferLength
        = static_cast<DWORD>(playback->bytes.size());

    const auto prepareResult = waveOutPrepareHeader(
        playback->device, &playback->header, sizeof(playback->header));
    if (prepareResult != MMSYSERR_NOERROR) {
        if (error != nullptr) {
            *error = waveErrorText(prepareResult);
        }
        waveOutClose(playback->device);
        playback->device = nullptr;
        return false;
    }

    const auto writeResult = waveOutWrite(
        playback->device, &playback->header, sizeof(playback->header));
    if (writeResult != MMSYSERR_NOERROR) {
        if (error != nullptr) {
            *error = waveErrorText(writeResult);
        }
        waveOutUnprepareHeader(
            playback->device, &playback->header, sizeof(playback->header));
        waveOutClose(playback->device);
        playback->device = nullptr;
        return false;
    }

    std::thread([playback]() {
        while ((playback->header.dwFlags & WHDR_DONE) == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        waveOutUnprepareHeader(
            playback->device, &playback->header, sizeof(playback->header));
        waveOutClose(playback->device);
        playback->device = nullptr;
    }).detach();

    return true;
}

bool queuePcmSoundPlayback(const std::string& path,
    const std::string& extension, int volumePercent, std::string* error)
{
    return queueBackgroundTask(
        [path, extension, volumePercent]() {
            auto sound = cachedPcmSound(path);
            if (sound == nullptr) {
                auto decoded = std::make_shared<DecodedPcmSound>();
                std::string ignoredError;
                if (!decodeSoundToPcm(
                        path, extension, *decoded, &ignoredError)) {
                    return;
                }

                sound = cachePcmSound(path, decoded);
            }

            auto bytes = sound->bytes;
            std::string ignoredError;
            playPcmOnce(
                std::move(bytes), sound->format, volumePercent, &ignoredError);
        },
        error);
}

bool playVorbisOnce(
    const std::string& path, int volumePercent, std::string* error)
{
    DecodedPcmSound sound;
    if (!decodeVorbisPcm(path, sound, error)) {
        return false;
    }

    return playPcmOnce(
        std::move(sound.bytes), sound.format, volumePercent, error);
}
} // namespace

bool SoundEffectPlayer::playOnce(
    const std::string& path, int volumePercent, std::string* error)
{
    if (path.empty()) {
        if (error != nullptr) {
            *error = "Sound effect path is empty.";
        }
        return false;
    }

    if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        if (error != nullptr) {
            *error = "Sound effect file not found: " + path;
        }
        return false;
    }

    const auto extension = lowerExtension(path);
    if (isOggExtension(extension)) {
        return queuePcmSoundPlayback(path, extension, volumePercent, error);
    }

    if (extension == ".mp3" || extension == ".wav" || extension == ".wma"
        || extension == ".m4a" || extension == ".aac") {
        return queuePcmSoundPlayback(path, extension, volumePercent, error);
    }

    return queueBackgroundTask(
        [path, volumePercent]() {
            std::ostringstream aliasStream;
            aliasStream << "winraycast_effect_" << g_nextEffectAlias++;
            const auto alias = aliasStream.str();

            std::string ignoredError;
            if (!sendMci("open \"" + path + "\"" + mciDeviceType(path)
                        + " alias " + alias,
                    &ignoredError)) {
                return;
            }

            const auto closeAlias = [alias]() noexcept {
                mciSendStringA(("close " + alias).c_str(), nullptr, 0, nullptr);
            };

            if (!sendMci("setaudio " + alias + " volume to "
                        + std::to_string(volumePercentToMci(volumePercent)),
                    &ignoredError)) {
                closeAlias();
                return;
            }

            mciSendStringA(
                ("play " + alias + " wait").c_str(), nullptr, 0, nullptr);
            closeAlias();
        },
        error);
}
