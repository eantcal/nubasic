// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Licensed under the MIT License. See COPYING file in the project root.

#include "D2dPresenter.h"

#include <d2d1.h>

#pragma comment(lib, "d2d1.lib")

namespace {

D2D1_SIZE_U clientSize(HWND hWnd) noexcept
{
    RECT rc{};
    GetClientRect(hWnd, &rc);
    const UINT w = static_cast<UINT>((std::max)(LONG(1), rc.right - rc.left));
    const UINT h = static_cast<UINT>((std::max)(LONG(1), rc.bottom - rc.top));
    return D2D1::SizeU(w, h);
}

} // namespace

D2dPresenter::~D2dPresenter()
{
    releaseTargetResources();
    if (m_renderTarget) {
        m_renderTarget->Release();
        m_renderTarget = nullptr;
    }
    if (m_factory) {
        m_factory->Release();
        m_factory = nullptr;
    }
}

void D2dPresenter::releaseTargetResources() noexcept
{
    if (m_gdiDc) {
        endGdi();
    }
    if (m_gdiTarget) {
        m_gdiTarget->Release();
        m_gdiTarget = nullptr;
    }
    if (m_frameBitmap) {
        m_frameBitmap->Release();
        m_frameBitmap = nullptr;
    }
    m_bitmapWidth = 0;
    m_bitmapHeight = 0;
}

bool D2dPresenter::init(HWND hWnd) noexcept
{
    m_hWnd = hWnd;

    if (!m_factory) {
        if (FAILED(D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_factory))
            || !m_factory) {
            return false;
        }
    }

    // GDI-compatible target so the existing GDI HUD code can draw through it.
    const auto rtProps = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
        0.0f, 0.0f, D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE);

    const auto hwndProps = D2D1::HwndRenderTargetProperties(
        hWnd, clientSize(hWnd), D2D1_PRESENT_OPTIONS_NONE);

    if (FAILED(m_factory->CreateHwndRenderTarget(
            rtProps, hwndProps, &m_renderTarget))
        || !m_renderTarget) {
        m_renderTarget = nullptr;
        return false;
    }

    if (FAILED(m_renderTarget->QueryInterface(&m_gdiTarget)) || !m_gdiTarget) {
        releaseTargetResources();
        m_renderTarget->Release();
        m_renderTarget = nullptr;
        return false;
    }

    return true;
}

void D2dPresenter::resize(UINT width, UINT height) noexcept
{
    if (!m_renderTarget || width == 0 || height == 0) {
        return;
    }
    m_renderTarget->Resize(D2D1::SizeU(width, height));
}

bool D2dPresenter::beginFrame() noexcept
{
    if (!m_renderTarget) {
        return false;
    }
    m_renderTarget->BeginDraw();
    m_renderTarget->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f));
    return true;
}

void D2dPresenter::draw3D(const FrameBuffer& frame, const RECT& destRect,
    int srcWidth, int srcHeight) noexcept
{
    if (!m_renderTarget || frame.empty() || srcWidth <= 0 || srcHeight <= 0) {
        return;
    }

    const UINT w = frame.width();
    const UINT h = frame.height();
    if (w == 0 || h == 0) {
        return;
    }

    // (Re)create the GPU bitmap when the frame buffer size changes.
    if (!m_frameBitmap || m_bitmapWidth != w || m_bitmapHeight != h) {
        if (m_frameBitmap) {
            m_frameBitmap->Release();
            m_frameBitmap = nullptr;
        }
        const auto bmpProps = D2D1::BitmapProperties(D2D1::PixelFormat(
            DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
        if (FAILED(m_renderTarget->CreateBitmap(
                D2D1::SizeU(w, h), bmpProps, &m_frameBitmap))
            || !m_frameBitmap) {
            m_frameBitmap = nullptr;
            return;
        }
        m_bitmapWidth = w;
        m_bitmapHeight = h;
    }

    // Upload the software frame (top-down, BGRA byte order — see Color.h).
    if (FAILED(m_frameBitmap->CopyFromMemory(
            nullptr, frame.data(), w * sizeof(FrameBuffer::Pixel)))) {
        return;
    }

    const auto dest = D2D1::RectF(static_cast<float>(destRect.left),
        static_cast<float>(destRect.top), static_cast<float>(destRect.right),
        static_cast<float>(destRect.bottom));
    const auto src = D2D1::RectF(0.0f, 0.0f, static_cast<float>(srcWidth),
        static_cast<float>(srcHeight));

    m_renderTarget->DrawBitmap(
        m_frameBitmap, dest, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src);
}

HDC D2dPresenter::beginGdi() noexcept
{
    if (!m_gdiTarget || m_gdiDc) {
        return nullptr;
    }
    // COPY: the interop DC starts with the current render-target contents, so
    // GDI draws compose over the already-drawn 3D frame.
    if (FAILED(m_gdiTarget->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &m_gdiDc))) {
        m_gdiDc = nullptr;
    }
    return m_gdiDc;
}

void D2dPresenter::endGdi() noexcept
{
    if (m_gdiTarget && m_gdiDc) {
        m_gdiTarget->ReleaseDC(nullptr); // whole target updated
    }
    m_gdiDc = nullptr;
}

bool D2dPresenter::endFrame() noexcept
{
    if (!m_renderTarget) {
        return false;
    }
    const HRESULT hr = m_renderTarget->EndDraw();
    if (hr == D2DERR_RECREATE_TARGET) {
        // Device lost: drop target-bound resources so the next init rebuilds.
        releaseTargetResources();
        m_renderTarget->Release();
        m_renderTarget = nullptr;
        return false;
    }
    return SUCCEEDED(hr);
}
