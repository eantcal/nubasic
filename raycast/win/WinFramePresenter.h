// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#pragma once

#include "FrameBuffer.h"

void presentFrameBuffer(int videoPosX, int videoPosY,
    const FrameBuffer& frameBuffer, int sourceWidth, int sourceHeight);
