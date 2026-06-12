// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include <memory>
#include <string>

class TextToSpeechPlayer {
public:
    TextToSpeechPlayer();
    ~TextToSpeechPlayer();

    TextToSpeechPlayer(const TextToSpeechPlayer&) = delete;
    TextToSpeechPlayer& operator=(const TextToSpeechPlayer&) = delete;

    bool speak(const std::string& text, std::string* error = nullptr);
    void stop() noexcept;

private:
    struct Impl;

    bool ensureReady(std::string* error);

    std::unique_ptr<Impl> m_impl;
};
