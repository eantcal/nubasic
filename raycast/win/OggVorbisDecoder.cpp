// This file is part of nuRCADE (New (nu) Raycasting Classic Arcade Development Engine).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "OggVorbisDecoder.h"

#include <cstdlib>

#if __has_include("../../third_party/stb/stb_vorbis.c")
#define NURCADE_HAS_STB_VORBIS 1
#pragma warning(push)
#pragma warning(disable : 4244 4267 4996)
#include "../../third_party/stb/stb_vorbis.c"
#pragma warning(pop)
#else
#define NURCADE_HAS_STB_VORBIS 0
#endif

bool decodeVorbisFile(const std::string& path, std::vector<short>& samples,
    int& channels, int& sampleRate, std::string* error)
{
#if NURCADE_HAS_STB_VORBIS
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
#else
    (void)path;
    samples.clear();
    channels = 0;
    sampleRate = 0;

    if (error != nullptr) {
        *error = "Ogg Vorbis support is not available in this build.";
    }

    return false;
#endif
}
