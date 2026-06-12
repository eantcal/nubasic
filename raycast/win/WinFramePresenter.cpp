// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "WinFramePresenter.h"

#include "DdxDevice.h"

#include <cstring>
#include <windows.h>

void presentFrameBuffer(int videoPosX, int videoPosY,
    const FrameBuffer& frameBuffer, int sourceWidth, int sourceHeight)
{
    if (!DdxDevice::getInstance().ready() || frameBuffer.empty()) {
        return;
    }

    DdxDevice::Ctx dctx(DdxDevice::getInstance());
    HDC dxHdc = dctx.getDc();

    if (!dxHdc) {
        return;
    }

    BITMAPINFO bmpInfo;
    std::memset(&bmpInfo, 0, sizeof(BITMAPINFOHEADER));

    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = frameBuffer.width();
    bmpInfo.bmiHeader.biHeight = frameBuffer.height();
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 32;
    bmpInfo.bmiHeader.biCompression = BI_RGB;
    bmpInfo.bmiHeader.biClrUsed = 0;
    bmpInfo.bmiHeader.biClrImportant = 0;

    StretchDIBits(dxHdc, videoPosX,
        static_cast<int>(frameBuffer.height()) + videoPosY, frameBuffer.width(),
        -static_cast<int>(frameBuffer.height()), 0, 0, sourceWidth,
        sourceHeight, frameBuffer.data(), &bmpInfo, DIB_RGB_COLORS, SRCCOPY);
}
