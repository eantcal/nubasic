// This file is part of the WinRayCast Application (a 3D Engine Demo).
// Copyright (C) 2005 - 2018
// Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.
// Licensed under the MIT License.
// See COPYING file in the project root for full license information.

#include "WinTextureLoader.h"

#include <algorithm>
#include <cctype>
#include <comdef.h>
#include <cstring>
#include <wincodec.h>

namespace {
struct ComInitializer {
    HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    ~ComInitializer()
    {
        if (SUCCEEDED(result)) {
            CoUninitialize();
        }
    }
};

template <typename T> struct ComPtr {
    T* ptr = nullptr;

    ~ComPtr()
    {
        if (ptr != nullptr) {
            ptr->Release();
        }
    }

    T** operator&() noexcept { return &ptr; }

    T* operator->() const noexcept { return ptr; }

    explicit operator bool() const noexcept { return ptr != nullptr; }
};

std::wstring widen(const std::string& value)
{
    if (value.empty()) {
        return {};
    }

    const int required
        = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, nullptr, 0);

    if (required <= 0) {
        return std::wstring(value.begin(), value.end());
    }

    std::wstring result(static_cast<size_t>(required - 1), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), -1, &result[0], required);
    return result;
}

std::string lowerExtension(const std::string& path)
{
    const auto dot = path.find_last_of('.');
    if (dot == std::string::npos) {
        return {};
    }

    auto extension = path.substr(dot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return extension;
}
} // namespace

std::shared_ptr<Texture> loadTextureFromBitmap(
    HDC hdc, HBITMAP hBitmap, int width, int height)
{
    if (!hdc || !hBitmap || width <= 0 || height <= 0) {
        return {};
    }

    HDC textureHdc = CreateCompatibleDC(hdc);
    if (!textureHdc) {
        return {};
    }

    SelectObject(textureHdc, hBitmap);

    BITMAPINFO bmpInfo;
    std::memset(&bmpInfo, 0, sizeof(BITMAPINFOHEADER));
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = width;
    bmpInfo.bmiHeader.biHeight = -height;
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 32;
    bmpInfo.bmiHeader.biCompression = BI_RGB;
    bmpInfo.bmiHeader.biClrUsed = 0;
    bmpInfo.bmiHeader.biClrImportant = 0;

    auto texture = std::make_shared<Texture>(width, height);

    const auto copiedLines = GetDIBits(textureHdc, hBitmap, 0, height,
        texture->pixels(), &bmpInfo, DIB_RGB_COLORS);

    DeleteDC(textureHdc);

    if (copiedLines == 0) {
        return {};
    }

    return texture;
}

std::shared_ptr<Texture> loadTextureFromFile(
    const std::string& path, int width, int height)
{
    if (path.empty()) {
        return {};
    }

    ComInitializer com;
    if (FAILED(com.result) && com.result != RPC_E_CHANGED_MODE) {
        return {};
    }

    ComPtr<IWICImagingFactory> factory;
    auto hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory.ptr));
    if (FAILED(hr) || !factory) {
        return {};
    }

    ComPtr<IWICBitmapDecoder> decoder;
    const auto widePath = widen(path);
    hr = factory->CreateDecoderFromFilename(widePath.c_str(), nullptr,
        GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder.ptr);
    if (FAILED(hr) || !decoder) {
        return {};
    }

    ComPtr<IWICBitmapFrameDecode> frame;
    hr = decoder->GetFrame(0, &frame.ptr);
    if (FAILED(hr) || !frame) {
        return {};
    }

    UINT sourceWidth = 0;
    UINT sourceHeight = 0;
    hr = frame->GetSize(&sourceWidth, &sourceHeight);
    if (FAILED(hr) || sourceWidth == 0 || sourceHeight == 0) {
        return {};
    }

    const auto targetWidth = width > 0 ? static_cast<UINT>(width) : sourceWidth;
    const auto targetHeight
        = height > 0 ? static_cast<UINT>(height) : sourceHeight;
    if (targetWidth == 0 || targetHeight == 0) {
        return {};
    }

    ComPtr<IWICFormatConverter> converter;
    hr = factory->CreateFormatConverter(&converter.ptr);
    if (FAILED(hr) || !converter) {
        return {};
    }

    IWICBitmapSource* bitmapSource = frame.ptr;
    ComPtr<IWICBitmapScaler> scaler;
    if (targetWidth != sourceWidth || targetHeight != sourceHeight) {
        hr = factory->CreateBitmapScaler(&scaler.ptr);
        if (FAILED(hr) || !scaler) {
            return {};
        }

        hr = scaler->Initialize(frame.ptr, targetWidth, targetHeight,
            WICBitmapInterpolationModeFant);
        if (FAILED(hr)) {
            return {};
        }

        bitmapSource = scaler.ptr;
    }

    hr = converter->Initialize(bitmapSource, GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr)) {
        return {};
    }

    auto texture = std::make_shared<Texture>(static_cast<uint32_t>(targetWidth),
        static_cast<uint32_t>(targetHeight));
    texture->setHasAlpha(lowerExtension(path) == ".png");

    const auto stride = static_cast<UINT>(targetWidth * sizeof(Texture::Pixel));
    const auto bufferSize = stride * targetHeight;
    hr = converter->CopyPixels(nullptr, stride, bufferSize,
        reinterpret_cast<BYTE*>(texture->pixels()));
    if (FAILED(hr)) {
        return {};
    }

    return texture;
}
