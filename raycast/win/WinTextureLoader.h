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
#include <windows.h>

std::shared_ptr<Texture> loadTextureFromBitmap(
    HDC hdc, HBITMAP hBitmap, int width, int height);

std::shared_ptr<Texture> loadTextureFromFile(
    const std::string& path, int width, int height);
