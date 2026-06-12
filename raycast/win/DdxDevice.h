// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.


/* -------------------------------------------------------------------------- */

#pragma once

#include <ddraw.h>
#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

#include <memory>


/* -------------------------------------------------------------------------- */

class DdxDevice {
public:
    friend class Ctx;

    class Ctx {
    public:
        Ctx(DdxDevice& renderer)
            : m_renderer(renderer)
        {
            if ((renderer.m_pDDSPrimary)->GetDC(&m_hdc) != DD_OK) {
                m_hdc = nullptr;
            }
        }

        HDC getDc() const noexcept { return m_hdc; }

        ~Ctx()
        {
            if (m_hdc) {
                m_renderer.m_pDDSPrimary->ReleaseDC(m_hdc);
            }
        }

    private:
        DdxDevice& m_renderer;
        HDC m_hdc = nullptr;
    };

    enum class error_t {
        Success,
        AlreadyInitialized,
        DirectDrawCreateExFailed,
        SetCooperativeLevelFailed,
        SetDisplayModeFailed,
        CreateSurfaceFailed,
    };

    static DdxDevice& getInstance() noexcept;

    error_t init(HWND hWnd, bool fullScreen, int xres, int yres);

    void releaseObjects() noexcept;

    bool ready() const noexcept { return m_pDD && m_pDDSPrimary; }

private:
    template <typename T> struct ComReleaser {
        void operator()(T* value) const noexcept
        {
            if (value) {
                value->Release();
            }
        }
    };

    using DirectDrawPtr
        = std::unique_ptr<IDirectDraw7, ComReleaser<IDirectDraw7>>;
    using DirectDrawSurfacePtr = std::unique_ptr<IDirectDrawSurface7,
        ComReleaser<IDirectDrawSurface7>>;

    DdxDevice() {}
    ~DdxDevice() = default;

    // DirectDraw object
    DirectDrawPtr m_pDD;

    // DirectDraw primary surface
    DirectDrawSurfacePtr m_pDDSPrimary;
};


/* -------------------------------------------------------------------------- */
