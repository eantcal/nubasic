// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "DdxDevice.h"


/* ------------------------------------------------------------------------- */

DdxDevice& DdxDevice::getInstance() noexcept
{
    static DdxDevice _instance;
    return _instance;
}


/* ------------------------------------------------------------------------- */

void DdxDevice::releaseObjects() noexcept
{
    m_pDDSPrimary.reset();
    m_pDD.reset();
}


/* ------------------------------------------------------------------------- */

DdxDevice::error_t DdxDevice::init(
    HWND hWnd, bool fullScreen, int xres, int yres)
{
    if (m_pDD) {
        return error_t::AlreadyInitialized;
    }

    // Create the main DirectDraw object
    LPDIRECTDRAW7 directDraw = nullptr;
    auto hRet = DirectDrawCreateEx(
        nullptr, (VOID**)&directDraw, IID_IDirectDraw7, nullptr);

    if (hRet != DD_OK || directDraw == nullptr) {
        return error_t::DirectDrawCreateExFailed;
    }

    m_pDD.reset(directDraw);

    // Get normal mode
    hRet = m_pDD->SetCooperativeLevel(
        hWnd, fullScreen ? DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN : DDSCL_NORMAL);

    if (hRet != DD_OK) {
        releaseObjects();
        return error_t::SetCooperativeLevelFailed;
    }

    DDSURFACEDESC2 ddsd = { 0 };
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;


    if (fullScreen) {
        hRet = m_pDD->SetDisplayMode(xres, yres, 32 /* bits per color */, 0, 0);
        if (hRet != DD_OK) {
            releaseObjects();
            return error_t::SetDisplayModeFailed;
        }

        // Create the primary surface with 1 back buffer
        ddsd.dwFlags |= DDSD_BACKBUFFERCOUNT;
        ddsd.ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
        ddsd.dwBackBufferCount = 1;
    }

    LPDIRECTDRAWSURFACE7 primarySurface = nullptr;
    hRet = m_pDD->CreateSurface(&ddsd, &primarySurface, nullptr);

    if (hRet != DD_OK) {
        releaseObjects();
        return error_t::CreateSurfaceFailed;
    }

    m_pDDSPrimary.reset(primarySurface);

    return error_t::Success;
}
