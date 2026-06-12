// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Licensed under the MIT License. See COPYING file in the project root.
//
// Hardware-accelerated presentation using Direct2D. The software-rendered 3D
// FrameBuffer is uploaded to a GPU bitmap and drawn (scaled) by the GPU,
// replacing the per-frame GDI StretchDIBits upscale. The HUD keeps its existing
// GDI drawing code, rendered through Direct2D's GDI-compatible interop DC, so
// the overlay pixels are unchanged but no longer go through the slow
// DirectDraw-primary GetDC path.
//
// All methods are no-ops / return false when the device is not ready, so the
// caller can fall back to the legacy DirectDraw present path.

#pragma once

#include "FrameBuffer.h"

#include <windows.h>

struct ID2D1Factory;
struct ID2D1HwndRenderTarget;
struct ID2D1Bitmap;
struct ID2D1GdiInteropRenderTarget;

class D2dPresenter {
public:
    D2dPresenter() = default;
    ~D2dPresenter();

    D2dPresenter(const D2dPresenter&) = delete;
    D2dPresenter& operator=(const D2dPresenter&) = delete;

    // Create the factory and an HWND render target sized to the client area.
    // Returns false if Direct2D is unavailable (caller should fall back).
    bool init(HWND hWnd) noexcept;

    bool ready() const noexcept { return m_renderTarget != nullptr; }

    // Match the render target to the current client size (call on WM_SIZE).
    void resize(UINT width, UINT height) noexcept;

    // Begin a frame; clears to black. Returns false if not ready.
    bool beginFrame() noexcept;

    // Draw the 3D frame buffer (its top-left srcWidth x srcHeight region)
    // scaled into destRect on screen, GPU-accelerated with linear filtering.
    void draw3D(const FrameBuffer& frame, const RECT& destRect, int srcWidth,
        int srcHeight) noexcept;

    // GDI interop: returns a DC carrying the current frame contents so the
    // existing GDI HUD code can draw over it. Must be paired with endGdi().
    HDC beginGdi() noexcept;
    void endGdi() noexcept;

    // Present the frame to the window. Returns false if the device was lost
    // (the caller should re-init); true on success.
    bool endFrame() noexcept;

private:
    void releaseTargetResources() noexcept;

    HWND m_hWnd = nullptr;
    ID2D1Factory* m_factory = nullptr;
    ID2D1HwndRenderTarget* m_renderTarget = nullptr;
    ID2D1Bitmap* m_frameBitmap = nullptr;
    ID2D1GdiInteropRenderTarget* m_gdiTarget = nullptr;
    HDC m_gdiDc = nullptr;
    UINT m_bitmapWidth = 0;
    UINT m_bitmapHeight = 0;
};
