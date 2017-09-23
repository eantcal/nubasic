//  
// This file is part of nuBASIC
// Copyright (c) Antonino Calderone (antonino.calderone@gmail.com)
// All rights reserved.  
// Licensed under the MIT License. 
// See COPYING file in the project root for full license information.
//

/* -------------------------------------------------------------------------- */

#ifndef TINY_NUBASIC_VER


/* -------------------------------------------------------------------------- */

#ifdef _WIN32
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Gdiplus.lib")
#endif


/* -------------------------------------------------------------------------- */

#include "nu_os_gdi.h"
#include "nu_exception.h"
#include "nu_os_std.h"
#include "nu_os_std.h"
#include "nu_rt_prog_ctx.h"
#include "nu_stdtype.h"


/* -------------------------------------------------------------------------- */

#include <stdio.h>


/* -------------------------------------------------------------------------- */

#ifdef _WIN32
using namespace Gdiplus;

namespace nu {

static struct gdi_plus_t {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    gdi_plus_t()
    {
        // Initialize GDI+.
        GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    }

    ~gdi_plus_t() { GdiplusShutdown(gdiplusToken); }
} _gdi_plus_instance;


/* -------------------------------------------------------------------------- */

struct gdi_ctx_t {
    enum brush_t { NO_BRUSH, SOLID_BRUSH };

    gdi_ctx_t(const gdi_ctx_t&) = delete;


private:
    HWND _console_hwnd;
    HDC _hdc;
    HPEN _hpen;
    brush_t _brush = NO_BRUSH;
    HBRUSH _hbr;

public:
    gdi_ctx_t() noexcept : _console_hwnd(GetConsoleWindow()),
                           _hdc(GetDC(GetConsoleWindow())),
                           _hpen(nullptr),
                           _hbr(nullptr)
    {
    }

    gdi_ctx_t(gdi_iarg_t pen_color, brush_t brush_type, gdi_iarg_t brush_color,
        gdi_iarg_t pen_width) noexcept
        : _console_hwnd(GetConsoleWindow()),
          _hdc(GetDC(GetConsoleWindow())),
          _hpen(CreatePen(PS_SOLID, pen_width, pen_color)),
          _brush(brush_type),
          _hbr(HBRUSH(GetStockObject(HOLLOW_BRUSH)))
    {
        SelectObject(_hdc, _hpen);

        if (_brush == SOLID_BRUSH)
            _hbr = CreateSolidBrush(brush_color);

        SelectObject(_hdc, _hbr);
    }


    HDC get_hdc() const noexcept { return _hdc; }


    ~gdi_ctx_t() noexcept
    {
        if (_brush == SOLID_BRUSH && _hbr)
            DeleteObject(_hbr);

        if (_hpen)
            DeleteObject(_hpen);

        ReleaseDC(_console_hwnd, _hdc);
    }
};


/* -------------------------------------------------------------------------- */

int os_textout_t::operator()(rt_prog_ctx_t& ctx, gdi_vargs_t args)
{
    (void)ctx;

    enum { X, Y, TEXT, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    auto tx = args[X].get_type();
    auto ty = args[Y].get_type();
    auto tc = args[COL].get_type();
    auto tt = args[TEXT].get_type();

    auto is_not_a_number = [](
        nu::variable_t::type_t t) { return !nu::variable_t::is_number(t); };

    if (is_not_a_number(tx) || is_not_a_number(ty) || is_not_a_number(tc)
        || tt != variant_t::type_t::STRING) {
        return EINVAL;
    }

    int c = args[COL].to_int();

    gdi_ctx_t gdi_ctx(c, gdi_ctx_t::NO_BRUSH, 0 /*TODO*/, 1);

    int x = args[X].to_int();
    int y = args[Y].to_int();
    std::string text = args[TEXT].to_str();

    SetTextColor(gdi_ctx.get_hdc(), c);
    SetBkMode(gdi_ctx.get_hdc(), TRANSPARENT);
    BOOL ret = TextOut(gdi_ctx.get_hdc(), x, y, text.c_str(), int(text.size()));

    GdiFlush();

    return ret ? 0 : errno;
}


/* -------------------------------------------------------------------------- */

int os_line_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X1, Y1, X2, Y2, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx(args[COL], gdi_ctx_t::NO_BRUSH, 0, 1);

    POINT ppt = { 0 };
    BOOL ret = MoveToEx(gdi_ctx.get_hdc(), args[X1], args[Y1], &ppt);
    ret = ret && LineTo(gdi_ctx.get_hdc(), args[X2], args[Y2]);

    return ret ? 0 : errno;
}


/* -------------------------------------------------------------------------- */

int os_set_pixel_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X, Y, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx;

    POINT ppt = { 0 };
    BOOL ret = SetPixel(gdi_ctx.get_hdc(), args[X], args[Y], args[COL]);

    return ret ? 0 : errno;
}


/* -------------------------------------------------------------------------- */

int os_rect_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X1, Y1, X2, Y2, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx(args[COL], gdi_ctx_t::NO_BRUSH, 0, 1);

    return Rectangle(gdi_ctx.get_hdc(), args[X1], args[Y1], args[X2], args[Y2])
        ? 0
        : errno;
}


/* -------------------------------------------------------------------------- */

int os_ellipse_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X1, Y1, X2, Y2, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx(args[COL], gdi_ctx_t::NO_BRUSH, 0, 1);

    return Ellipse(gdi_ctx.get_hdc(), args[X1], args[Y1], args[X2], args[Y2])
        ? 0
        : errno;
}


/* -------------------------------------------------------------------------- */

int os_fillrect_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X1, Y1, X2, Y2, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx(args[COL], gdi_ctx_t::SOLID_BRUSH, args[COL], 1);

    return Rectangle(gdi_ctx.get_hdc(), args[X1], args[Y1], args[X2], args[Y2])
        ? 0
        : errno;
}


/* -------------------------------------------------------------------------- */

int os_fillellipse_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X1, Y1, X2, Y2, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx(args[COL], gdi_ctx_t::SOLID_BRUSH, args[COL], 1);

    return Ellipse(gdi_ctx.get_hdc(), args[X1], args[Y1], args[X2], args[Y2])
        ? 0
        : errno;
}


/* -------------------------------------------------------------------------- */

int os_plotimage_t::operator()(rt_prog_ctx_t& ctx, gdi_vargs_t args)
{
    (void)ctx;

    enum { FNAME, X, Y, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    auto tx = args[X].get_type();
    auto ty = args[Y].get_type();
    auto fname = args[FNAME].get_type();

    auto is_not_a_number = [](
        nu::variable_t::type_t t) { return !nu::variable_t::is_number(t); };

    if (is_not_a_number(tx) || is_not_a_number(ty)
        || (fname != variant_t::type_t::STRING)) {
        return EINVAL;
    }

    gdi_ctx_t gdi_ctx(0, gdi_ctx_t::NO_BRUSH, 0 /*TODO*/, 1);

    int x = args[X].to_int();
    int y = args[Y].to_int();
    std::string filename = args[FNAME].to_str();

    HDC hdc = gdi_ctx.get_hdc();

    Graphics graphics(hdc);
    WCHAR wsfname[2048] = { 0 };
    mbstowcs(wsfname, filename.c_str(), sizeof(wsfname) / sizeof(WCHAR));
    Image image(wsfname);
    const auto status = graphics.DrawImage(&image, x, y);
    return status != Status::Ok ? 0 : GetLastError();
}


/* -------------------------------------------------------------------------- */

int _os_get_screen_width() noexcept
{
    RECT r = { 0 };
    GetClientRect(GetConsoleWindow(), &r);
    return r.right - r.left;
}


/* -------------------------------------------------------------------------- */

int _os_get_screen_height() noexcept
{
    RECT r = { 0 };
    GetClientRect(GetConsoleWindow(), &r);
    return r.bottom - r.top;
}


/* -------------------------------------------------------------------------- */

int _os_get_mouse_x() noexcept
{
    POINT pt = { 0 };
    GetCursorPos(&pt);
    ScreenToClient(GetConsoleWindow(), &pt);

    return pt.x;
}


/* -------------------------------------------------------------------------- */

int _os_get_mouse_y() noexcept
{
    POINT pt = { 0 };
    GetCursorPos(&pt);
    ScreenToClient(GetConsoleWindow(), &pt);

    return pt.y;
}


/* -------------------------------------------------------------------------- */

int _os_get_mouse_btn() noexcept
{
    int ret = 0;

    if (GetAsyncKeyState(VK_LBUTTON))
        ret |= 1;

    if (GetAsyncKeyState(VK_RBUTTON))
        ret |= 4;

    if (GetAsyncKeyState(VK_MBUTTON))
        ret |= 2;

    return ret;
}


/* -------------------------------------------------------------------------- */

int _os_msg_box(const std::string& title, const std::string& message, int flg)
{
    (void)flg; // not yet used
    return ::MessageBox(
        0, message.c_str(), title.c_str(), MB_ICONINFORMATION | MB_OK);
}


/* -------------------------------------------------------------------------- */

int _os_play_sound(const std::string& filename, int flag)
{
    DWORD wflg = 0;

    switch (flag) {
    case 1:
        wflg = SND_ASYNC;
        break;

    case 0:
    default:
        wflg = SND_SYNC;
        break;
    }

    return PlaySound(filename.c_str(), NULL, wflg | SND_FILENAME);
}


/* -------------------------------------------------------------------------- */

int _os_move_window(int x, int y, int dx, int dy)
{
    return MoveWindow(GetConsoleWindow(), x, y, dx, dy, TRUE);
}


/* -------------------------------------------------------------------------- */

int _os_get_pixel(int x, int y)
{
    HDC hdc = GetDC(GetConsoleWindow());
    auto ret = GetPixel(hdc, x, y);
    ReleaseDC(GetConsoleWindow(), hdc);
    return ret;
}


/* -------------------------------------------------------------------------- */

int _os_get_window_x() noexcept
{
    RECT r = { 0 };
    GetWindowRect(GetConsoleWindow(), &r);
    return r.left;
}


/* -------------------------------------------------------------------------- */

int _os_get_window_y() noexcept
{
    RECT r = { 0 };
    GetWindowRect(GetConsoleWindow(), &r);
    return r.top;
}


/* -------------------------------------------------------------------------- */

int _os_get_window_dx() noexcept
{
    RECT r = { 0 };
    GetWindowRect(GetConsoleWindow(), &r);
    return std::abs(r.right - r.left);
}


/* -------------------------------------------------------------------------- */

int _os_get_window_dy() noexcept
{
    RECT r = { 0 };
    GetWindowRect(GetConsoleWindow(), &r);
    return std::abs(r.top - r.bottom);
}


/* -------------------------------------------------------------------------- */

int _os_set_topmost() noexcept
{
    return SetWindowPos(
        GetConsoleWindow(), 
        HWND_TOPMOST, 
        0, 0, 0, 0, 
        SWP_NOMOVE | SWP_NOSIZE
    ) ? 0 : -1;
}


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#else // LINUX


/* -------------------------------------------------------------------------- */

#include "nu_os_gdi.h"
#include "nu_playsnd.h"
#include "nu_stb_image.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


/* -------------------------------------------------------------------------- */

namespace nu {


struct gdi_ctx_t {
    enum brush_t { NO_BRUSH, SOLID_BRUSH };

    gdi_ctx_t() = delete;
    gdi_ctx_t(gdi_ctx_t&) = delete;

private:
    Window _xterm_win = 0;
    Display* _display = 0;
    int _screen_num = 0;
    Screen* _screen = 0;

    gdi_iarg_t _pen_color;
    brush_t _brush_type;
    gdi_iarg_t _brush_color;
    gdi_iarg_t _pen_width;
    XColor _xcolor;
    Colormap _cmap;
    GC _gc;
    XImage* _ximage = nullptr;

    int _win_width = 0;
    int _win_height = 0;
    mutable int _win_x = 0;
    mutable int _win_y = 0;

public:
    gdi_ctx_t(gdi_iarg_t pen_color, brush_t brush_type, gdi_iarg_t brush_color,
        gdi_iarg_t pen_width)
        : _pen_color(pen_color)
        , _brush_type(brush_type)
        , _brush_color(brush_color)
        , _pen_width(pen_width)
    {
        try {
            _xterm_win = nu::stoi(::getenv("WINDOWID"));
        } catch (...) {
            _xterm_win = 0;
        }

        if (_xterm_win) {
            _display = XOpenDisplay(0);

            if (_display) {
                _screen_num = DefaultScreen(_display);
                _screen = XScreenOfDisplay(_display, _screen_num);
                _gc = create_gc(_pen_width);
                create_rgb_color(_pen_color);
                XSetForeground(_display, _gc, _xcolor.pixel);
                XWindowAttributes attr = { 0 };
                XGetWindowAttributes(_display, _xterm_win, &attr);
                _win_width = attr.width;
                _win_height = attr.height;
                _win_x = attr.x;
                _win_y = attr.y;
            }
        }
    }


protected:
    GC create_gc(int line_width)
    {
        XGCValues gc_val = { 0 };

        gc_val.function = GXcopy;
        gc_val.plane_mask = AllPlanes;
        gc_val.foreground = WhitePixel(_display, _screen_num);
        gc_val.background = BlackPixel(_display, _screen_num);
        gc_val.line_width = line_width;
        gc_val.line_style = LineSolid;
        gc_val.cap_style = CapButt;
        gc_val.join_style = JoinMiter;
        gc_val.fill_style = FillOpaqueStippled;
        gc_val.fill_rule = WindingRule;
        gc_val.graphics_exposures = False;
        gc_val.clip_x_origin = 0;
        gc_val.clip_y_origin = 0;
        gc_val.clip_mask = None;
        gc_val.subwindow_mode = IncludeInferiors;

        return XCreateGC(_display, _xterm_win,
            GCFunction | GCPlaneMask | GCForeground | GCBackground | GCLineWidth
                | GCLineStyle | GCCapStyle | GCJoinStyle | GCFillStyle
                | GCFillRule | GCClipXOrigin | GCClipYOrigin | GCClipMask
                | GCSubwindowMode | GCGraphicsExposures,
            &gc_val);
    }


    void getXYcoords() const
    {
        if (!_display) {
            return;
        }

        int screen_x, screen_y;
        Window child_win;
        Window parent_win;
        Window root_win;
        Window* child_windows;
        unsigned int num_child_windows;

        // make the query for the above values
        XQueryTree(_display, _xterm_win, &root_win, &parent_win, &child_windows,
            &num_child_windows);

        // we need to free the list of child IDs, as it was dynamically
        // allocated
        // by the XQueryTree function.
        XFree(child_windows);

        XQueryTree(_display, parent_win, &root_win, &parent_win, &child_windows,
            &num_child_windows);

        XFree(child_windows);

        // make the coordinates translation, from the coordinates system
        // of the parent window, to the coordinates system of the root window,
        XTranslateCoordinates(_display, parent_win, root_win, _win_x, _win_y,
            &screen_x, &screen_y, &child_win);

        // screen_x and screen_y contain the location of our original
        // window, using screen coordinates
        _win_x = screen_x;
        _win_y = screen_y;
    }


    void create_rgb_color(unsigned int rgb)
    {
        _xcolor.red = ((rgb & 0xff) << 8) | (rgb & 0xff);
        _xcolor.green = (rgb & 0xff00) | ((rgb >> 8) & 0xff);
        _xcolor.blue = ((rgb >> 8) & 0xff00) | ((rgb >> 16) & 0xff);

        _xcolor.flags = DoRed | DoGreen | DoBlue;

        _cmap = DefaultColormap(_display, 0);

        XAllocColor(_display, _cmap, &_xcolor);
    }


public:
    int get_window_width() const noexcept { 
        return _win_width; 
    }

    int get_window_height() const noexcept { 
        return _win_height; 
    }


    int get_window_x() const noexcept {
        getXYcoords();
        return _win_x;
    }


    int get_window_y() const noexcept {
        getXYcoords();
        return _win_y;
    }


    int move_win(int x, int y) noexcept {
        return XMoveWindow(_display, _xterm_win, x, y);
    }


    int resize_win(int dx, int dy) noexcept {
        return XResizeWindow(_display, _xterm_win, dx, dy);
    }


    void textout(int x, int y, const std::string& text) {
        Font font = XLoadFont(_display, "fixed");
        XTextItem txt;
        txt.chars = const_cast<char*>(text.c_str());
        txt.nchars = text.size();
        txt.delta = 0;
        txt.font = font;
        XDrawText(_display, _xterm_win, _gc, x, y, &txt, 1);
        XUnloadFont(_display, font);
    }


    void rect(int x1, int y1, int x2, int y2) {
        int x = std::min(x1, x2);
        int y = std::min(y1, y2);
        XDrawRectangle(_display, _xterm_win, _gc, x, y, abs(abs(x2) - abs(x1)),
            abs(abs(y2) - abs(y1)));
    }


    void fillrect(int x1, int y1, int x2, int y2) {
        int x = std::min(x1, x2);
        int y = std::min(y1, y2);
        XFillRectangle(_display, _xterm_win, _gc, x, y,
            abs(abs(x2) - abs(x1)) + 1, abs(abs(y2) - abs(y1)) + 1);
    }


    void line(int x1, int y1, int x2, int y2) {
        XDrawLine(_display, _xterm_win, _gc, x1, y1, x2, y2);
    }


    void setpixel(int x, int y) { 
        XDrawPoint(_display, _xterm_win, _gc, x, y); 
    }


    void set_topmost() {
        XMapRaised( _display, _xterm_win );
    }


    int getpixel(int x, int y) {
        XColor color;

        XImage* image;
        image
            = XGetImage(_display, _xterm_win, x, y, 1, 1, AllPlanes, XYPixmap);

        color.pixel = XGetPixel(image, 0, 0);

        XFree(image);

        XQueryColor(_display,
            DefaultColormap(_display, DefaultScreen(_display)), &color);

        return (((color.red >> 8) & 0xff) | (color.green & 0xff00)
            | ((color.blue << 8) & 0xff0000));
    }


    void ellipse(int x1, int y1, int x2, int y2) {
        int x = std::min(x1, x2);
        int y = std::min(y1, y2);
        XDrawArc(_display, _xterm_win, _gc, x, y, abs(abs(x2) - abs(x1)),
            abs(abs(y2) - abs(y1)), 0, 360 * 64);
    }


    void fillellipse(int x1, int y1, int x2, int y2) {
        int x = std::min(x1, x2);
        int y = std::min(y1, y2);
        XFillArc(_display, _xterm_win, _gc, x, y, abs(abs(x2) - abs(x1)),
            abs(abs(y2) - abs(y1)), 0, 360 * 64);
    }


    bool plotimage(const std::string& filepath, int x, int y) {
        const char* filename = filepath.c_str();

        int w = 0;
        int h = 0;
        unsigned char* image = nu::image_load(filename, w, h);

        if (!image) {
            return false;
        }

        XImage* xi = XCreateImage(_display,
            DefaultVisual(_display, DefaultScreen(_display)),
            DefaultDepth(_display, DefaultScreen(_display)), ZPixmap, 0,
            (char*)image, w, h, 32, 0);

        if (xi) {
            // Swap R/B before plot the image
            const size_t size = w * h;

            for (std::size_t i = 0; i < size; ++i) {
                const auto p = image[(i << 2)];
                image[(i << 2)] = image[(i << 2) + 2];
                image[(i << 2) + 2] = p;
            }

            XPutImage(_display, _xterm_win, _gc, xi, 0, 0, x, y, w, h);
            _ximage = xi; // Memory of xi and image will be freed later
        } else {
            nu::image_free(image);
        }

        return true;
    }


    ~gdi_ctx_t() {
        if (_display) {
            XFlush(_display);
            XFreeColors(_display, _cmap, &_xcolor.pixel, 1, 0);
            XFreeGC(_display, _gc);
            XCloseDisplay(_display);

            if (_ximage) {
                auto data = _ximage->data;

                _ximage->data = nullptr;
                XDestroyImage(_ximage);

                nu::image_free((unsigned char*)data);
            }
        }
    }
};


/* -------------------------------------------------------------------------- */

struct mouse_ctx_t {
    mouse_ctx_t(mouse_ctx_t&) = delete;

private:
    Window _xterm_win = 0;
    Display* _display = 0;
    int _screen_num = 0;
    Screen* _screen = 0;

    int _win_x = 0;
    int _win_y = 0;

public:
    mouse_ctx_t()
    {
        try {
            _xterm_win = nu::stoi(::getenv("WINDOWID"));
        } catch (...) {
            _xterm_win = 0;
        }

        if (_xterm_win) {
            _display = XOpenDisplay(0);

            if (_display) {
                _screen_num = DefaultScreen(_display);
                _screen = XScreenOfDisplay(_display, _screen_num);
            }
        }
    }


    bool get_mouse_event(int& btn, std::pair<int, int>& mouse_coords)
    {

        int root_x = 0, root_y = 0;
        int win_x = 0, win_y = 0;
        unsigned int mask_return = 0;

        Window window_returned;

        unsigned int result = XQueryPointer(_display, _xterm_win,
            &window_returned, &window_returned, &root_x, &root_y, &win_x,
            &win_y, &mask_return);

        btn = (mask_return >> 8) & 7;
        mouse_coords.first = win_x;
        mouse_coords.second = win_y;

        return result != 0;
    }


public:
    ~mouse_ctx_t()
    {
        if (_display) {
            XCloseDisplay(_display);
        }
    }
};


/* -------------------------------------------------------------------------- */

int os_textout_t::operator()(rt_prog_ctx_t& ctx, gdi_vargs_t args)
{
    (void)ctx;

    enum { X, Y, TEXT, COL, NARGS };

    if (args.size() != NARGS) {
        return EINVAL;
    }

    auto tx = args[X].get_type();
    auto ty = args[Y].get_type();
    auto tc = args[COL].get_type();
    auto tt = args[TEXT].get_type();

    auto is_not_a_number = [](
        nu::variable_t::type_t t) { return !nu::variable_t::is_number(t); };

    if (is_not_a_number(tx) || is_not_a_number(ty) || is_not_a_number(tc)
        || tt != variant_t::type_t::STRING) {
        return EINVAL;
    }

    int c = args[COL].to_int();

    gdi_ctx_t gdi_ctx(c, gdi_ctx_t::NO_BRUSH, 0, 1);

    int x = args[X].to_int();
    int y = args[Y].to_int();
    std::string text = args[TEXT].to_str();

    gdi_ctx.textout(x, y, text);

    return 0;
}


/* -------------------------------------------------------------------------- */

int os_line_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X1, Y1, X2, Y2, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx(args[COL], gdi_ctx_t::NO_BRUSH, 0, 1);

    gdi_ctx.line(args[X1], args[Y1], args[X2], args[Y2]);

    return 0;
}


/* -------------------------------------------------------------------------- */

int os_set_pixel_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X, Y, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx(args[COL], gdi_ctx_t::NO_BRUSH, 0, 1);
    gdi_ctx.setpixel(args[X], args[Y]);

    return 0;
}


/* -------------------------------------------------------------------------- */

int os_rect_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X1, Y1, X2, Y2, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx(args[COL], gdi_ctx_t::NO_BRUSH, 0, 1);

    gdi_ctx.rect(args[X1], args[Y1], args[X2], args[Y2]);

    return 0;
}


/* -------------------------------------------------------------------------- */

int os_fillrect_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X1, Y1, X2, Y2, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx(args[COL], gdi_ctx_t::NO_BRUSH, 0, 1);

    gdi_ctx.fillrect(args[X1], args[Y1], args[X2], args[Y2]);

    return 0;
}


/* -------------------------------------------------------------------------- */

int os_ellipse_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X1, Y1, X2, Y2, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx(args[COL], gdi_ctx_t::NO_BRUSH, 0, 1);

    gdi_ctx.ellipse(args[X1], args[Y1], args[X2], args[Y2]);

    return 0;
}


/* -------------------------------------------------------------------------- */

int os_fillellipse_t::operator()(rt_prog_ctx_t& ctx, gdi_iargs_t args)
{
    (void)ctx;

    enum { X1, Y1, X2, Y2, COL, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    gdi_ctx_t gdi_ctx(args[COL], gdi_ctx_t::NO_BRUSH, 0, 1);

    gdi_ctx.fillellipse(args[X1], args[Y1], args[X2], args[Y2]);

    return 0;
}


/* -------------------------------------------------------------------------- */

int os_plotimage_t::operator()(rt_prog_ctx_t& ctx, gdi_vargs_t args)
{
    enum { FNAME, X, Y, NARGS };

    if (args.size() != NARGS)
        return EINVAL;

    auto tx = args[X].get_type();
    auto ty = args[Y].get_type();
    auto fname = args[FNAME].get_type();

    auto is_not_a_number = [](
        nu::variable_t::type_t t) { return !nu::variable_t::is_number(t); };

    if (is_not_a_number(tx) || is_not_a_number(ty)
        || (fname != variant_t::type_t::STRING)) {
        return EINVAL;
    }

    gdi_ctx_t gdi_ctx(0, gdi_ctx_t::NO_BRUSH, 0 /*TODO*/, 1);

    int x = args[X].to_int();
    int y = args[Y].to_int();
    std::string filename = args[FNAME].to_str();

    return gdi_ctx.plotimage(filename, x, y);
}


/* -------------------------------------------------------------------------- */

int _os_get_screen_height() noexcept
{
    gdi_ctx_t gdi_ctx(0, gdi_ctx_t::NO_BRUSH, 0, 1);
    return gdi_ctx.get_window_height();
}


/* -------------------------------------------------------------------------- */

int _os_get_screen_width() noexcept
{
    gdi_ctx_t gdi_ctx(0, gdi_ctx_t::NO_BRUSH, 0, 1);
    return gdi_ctx.get_window_width();
}


/* -------------------------------------------------------------------------- */

int _os_get_mouse_x() noexcept
{
    mouse_ctx_t ctx;
    int btn = 0;
    std::pair<int, int> pos = { 0, 0 };

    bool res = ctx.get_mouse_event(btn, pos);

    return res ? pos.first : 0;
}


/* -------------------------------------------------------------------------- */

int _os_get_mouse_y() noexcept
{
    mouse_ctx_t ctx;
    int btn = 0;
    std::pair<int, int> pos = { 0, 0 };

    bool res = ctx.get_mouse_event(btn, pos);

    return res ? pos.second : 0;
}


/* -------------------------------------------------------------------------- */

int _os_get_mouse_btn() noexcept
{
    mouse_ctx_t ctx;
    int btn = 0;
    std::pair<int, int> pos = { 0, 0 };

    bool res = ctx.get_mouse_event(btn, pos);

    return res ? btn : 0;
}


/* -------------------------------------------------------------------------- */

int _os_msg_box(
    const std::string& title, const std::string& message, int /*flg*/)
{
    // Display message using xmessage
    std::string msg = "xmessage -center \"" + title
        + (title.empty() ? "" : ": ") + message + "\"";

    return system(msg.c_str());
}


/* -------------------------------------------------------------------------- */

int _os_play_sound(const std::string& filename, int flag)
{
    bool sync = true;

    switch (flag) {
    case 1:
        sync = false;
        break;

    case 0:
    default:
        break;
    }

    return nu::play_sound(filename.c_str(), sync);
}


/* -------------------------------------------------------------------------- */

int _os_move_window(int x, int y, int dx, int dy)
{
    gdi_ctx_t gdi_ctx(0, gdi_ctx_t::NO_BRUSH, 0, 1);
    int ret = gdi_ctx.move_win(x, y);
    return ret && gdi_ctx.resize_win(dx, dy);
}


/* -------------------------------------------------------------------------- */

int _os_get_pixel(int x, int y)
{
    gdi_ctx_t gdi_ctx(0, gdi_ctx_t::NO_BRUSH, 0, 1);
    return gdi_ctx.getpixel(x, y);
}


/* -------------------------------------------------------------------------- */

int _os_get_window_x() noexcept
{
    gdi_ctx_t gdi_ctx(0, gdi_ctx_t::NO_BRUSH, 0, 1);
    return gdi_ctx.get_window_x();
}


/* -------------------------------------------------------------------------- */

int _os_get_window_y() noexcept
{
    gdi_ctx_t gdi_ctx(0, gdi_ctx_t::NO_BRUSH, 0, 1);
    return gdi_ctx.get_window_y();
}


/* -------------------------------------------------------------------------- */

int _os_get_window_dx() noexcept
{
    gdi_ctx_t gdi_ctx(0, gdi_ctx_t::NO_BRUSH, 0, 1);
    return gdi_ctx.get_window_width();
}


/* -------------------------------------------------------------------------- */

int _os_get_window_dy() noexcept
{
    gdi_ctx_t gdi_ctx(0, gdi_ctx_t::NO_BRUSH, 0, 1);
    return gdi_ctx.get_window_height();
}


/* -------------------------------------------------------------------------- */

int _os_set_topmost() noexcept
{
    gdi_ctx_t gdi_ctx(0, gdi_ctx_t::NO_BRUSH, 0, 1);
    gdi_ctx.set_topmost();

    return 0;
}


/* -------------------------------------------------------------------------- */

}


/* -------------------------------------------------------------------------- */

#endif // ... LINUX


/* -------------------------------------------------------------------------- */

#endif // TINY_NUBASIC_VER
