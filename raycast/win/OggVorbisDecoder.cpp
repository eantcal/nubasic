// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "OggVorbisDecoder.h"

#include <cstdlib>

#pragma warning(push)
#pragma warning(disable : 4244 4267 4996)
#include "../../third_party/stb/stb_vorbis.c"
#pragma warning(pop)

bool decodeVorbisFile(const std::string& path, std::vector<short>& samples,
    int& channels, int& sampleRate, std::string* error)
{
    short* output = nullptr;
    channels = 0;
    sampleRate = 0;

    const auto sampleCount = stb_vorbis_decode_filename(
        path.c_str(), &channels, &sampleRate, &output);
    if (sampleCount <= 0 || output == nullptr || channels <= 0
        || sampleRate <= 0) {
        if (error != nullptr) {
            *error = "Could not decode Ogg Vorbis file: " + path;
        }
        std::free(output);
        return false;
    }

    samples.assign(output, output + (sampleCount * channels));
    std::free(output);
    return !samples.empty();
}
