// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "TextToSpeechPlayer.h"

#include <sapi.h>
#include <windows.h>
#include <wrl/client.h>

#include <sstream>

namespace {
using Microsoft::WRL::ComPtr;

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

std::wstring utf8ToWide(const std::string& text)
{
    if (text.empty()) {
        return {};
    }

    auto needed = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, text.c_str(), -1, nullptr, 0);

    auto codePage = CP_UTF8;
    auto flags = MB_ERR_INVALID_CHARS;

    if (needed == 0) {
        needed = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, nullptr, 0);
        codePage = CP_ACP;
        flags = 0;
    }

    if (needed <= 0) {
        return {};
    }

    std::wstring result(static_cast<size_t>(needed), L'\0');
    const auto written = MultiByteToWideChar(
        codePage, flags, text.c_str(), -1, &result[0], needed);

    if (written == 0) {
        return {};
    }

    if (!result.empty() && result.back() == L'\0') {
        result.pop_back();
    }

    return result;
}
} // namespace

struct TextToSpeechPlayer::Impl {
    ComPtr<ISpVoice> voice;
    bool comInitialized = false;
};

TextToSpeechPlayer::TextToSpeechPlayer()
    : m_impl(std::make_unique<Impl>())
{
}

TextToSpeechPlayer::~TextToSpeechPlayer()
{
    stop();
    m_impl->voice.Reset();

    if (m_impl->comInitialized) {
        CoUninitialize();
    }
}

bool TextToSpeechPlayer::ensureReady(std::string* error)
{
    if (m_impl->voice) {
        return true;
    }

    const auto initResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(initResult)) {
        m_impl->comInitialized = true;
    } else if (initResult != RPC_E_CHANGED_MODE) {
        if (error != nullptr) {
            *error = "Could not initialize COM for text-to-speech: "
                + hresultText(initResult);
        }
        return false;
    }

    const auto createResult = CoCreateInstance(
        CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&m_impl->voice));

    if (FAILED(createResult) || !m_impl->voice) {
        if (error != nullptr) {
            *error = "Could not create the Windows text-to-speech voice: "
                + hresultText(createResult);
        }
        return false;
    }

    return true;
}

bool TextToSpeechPlayer::speak(const std::string& text, std::string* error)
{
    const auto wideText = utf8ToWide(text);
    if (wideText.empty()) {
        return true;
    }

    if (!ensureReady(error)) {
        return false;
    }

    const auto speakResult = m_impl->voice->Speak(wideText.c_str(),
        SPF_ASYNC | SPF_PURGEBEFORESPEAK | SPF_IS_NOT_XML, nullptr);

    if (FAILED(speakResult)) {
        if (error != nullptr) {
            *error
                = "Could not speak event message: " + hresultText(speakResult);
        }
        return false;
    }

    return true;
}

void TextToSpeechPlayer::stop() noexcept
{
    if (!m_impl || !m_impl->voice) {
        return;
    }

    m_impl->voice->Speak(L"", SPF_ASYNC | SPF_PURGEBEFORESPEAK, nullptr);
}
