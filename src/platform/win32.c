/* ============================================================================
 * src/platform/win32.c - Windows Platform Implementation
 * ========================================================================== */
#ifdef _WIN32

#include "platform.h"
#include "utils/memory.h"
#include <windows.h>
#include <string.h>

struct FK_Surface {
    HWND hwnd;
    HDC hdc;
    HBITMAP bitmap;
    uint32_t *pixels;
    int width;
    int height;
    int should_close;
};

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    FK_Surface *surface = (FK_Surface*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg) {
        case WM_CLOSE:
            if (surface) surface->should_close = 1;
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (surface) {
                BitBlt(hdc, 0, 0, surface->width, surface->height,
                       surface->hdc, 0, 0, SRCCOPY);
            }
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

FK_Error fk_platform_init(void) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "FontKitWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClassEx(&wc)) {
        return FK_ERROR_UNSUPPORTED;
    }
    
    return FK_OK;
}

void fk_platform_shutdown(void) {
    UnregisterClass("FontKitWindow", GetModuleHandle(NULL));
}

FK_Surface* fk_surface_create(int width, int height, const char *title) {
    FK_Surface *surface = fk_calloc(1, sizeof(FK_Surface));
    if (!surface) return NULL;
    
    surface->width = width;
    surface->height = height;
    surface->should_close = 0;
    
    /* Create window */
    surface->hwnd = CreateWindowEx(
        0, "FontKitWindow", title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );
    
    if (!surface->hwnd) {
        fk_free(surface);
        return NULL;
    }
    
    SetWindowLongPtr(surface->hwnd, GWLP_USERDATA, (LONG_PTR)surface);
    
    /* Create DIB for drawing */
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;  /* Top-down */
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    surface->hdc = CreateCompatibleDC(NULL);
    surface->bitmap = CreateDIBSection(surface->hdc, &bmi, DIB_RGB_COLORS,
                                       (void**)&surface->pixels, NULL, 0);
    SelectObject(surface->hdc, surface->bitmap);
    
    ShowWindow(surface->hwnd, SW_SHOW);
    UpdateWindow(surface->hwnd);
    
    return surface;
}

void fk_surface_destroy(FK_Surface *surface) {
    if (!surface) return;
    
    if (surface->bitmap) DeleteObject(surface->bitmap);
    if (surface->hdc) DeleteDC(surface->hdc);
    if (surface->hwnd) DestroyWindow(surface->hwnd);
    
    fk_free(surface);
}

void fk_surface_present(FK_Surface *surface) {
    if (!surface) return;
    InvalidateRect(surface->hwnd, NULL, FALSE);
}

void fk_surface_clear(FK_Surface *surface, uint32_t color) {
    if (!surface || !surface->pixels) return;
    
    for (int i = 0; i < surface->width * surface->height; i++) {
        surface->pixels[i] = color;
    }
}

void fk_surface_draw_glyph(FK_Surface *surface, const FK_Glyph *glyph,
                           int x, int y, uint32_t color) {
    if (!surface || !glyph) return;
    
    FK_Bitmap bmp;
    if (fk_get_glyph_bitmap(glyph, &bmp) != FK_OK) return;
    
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    
    for (int gy = 0; gy < bmp.height; gy++) {
        for (int gx = 0; gx < bmp.width; gx++) {
            int px = x + gx;
            int py = y + gy;
            
            if (px >= 0 && px < surface->width && py >= 0 && py < surface->height) {
                uint8_t alpha = bmp.pixels[gy * bmp.pitch + gx];
                uint32_t *pixel = &surface->pixels[py * surface->width + px];
                
                /* Alpha blend */
                uint8_t bg_r = (*pixel >> 16) & 0xFF;
                uint8_t bg_g = (*pixel >> 8) & 0xFF;
                uint8_t bg_b = *pixel & 0xFF;
                
                uint8_t out_r = ((r * alpha) + (bg_r * (255 - alpha))) / 255;
                uint8_t out_g = ((g * alpha) + (bg_g * (255 - alpha))) / 255;
                uint8_t out_b = ((b * alpha) + (bg_b * (255 - alpha))) / 255;
                
                *pixel = (out_r << 16) | (out_g << 8) | out_b;
            }
        }
    }
}

void fk_surface_draw_text(FK_Surface *surface, FK_Font *font,
                         const char *text, int x, int y, uint32_t color) {
    if (!surface || !font || !text) return;
    
    int cursor_x = x;
    const char *p = text;
    
    while (*p) {
        uint32_t cp = fk_utf8_decode(&p);
        if (cp == 0) break;
        
        FK_Glyph *glyph = fk_render_glyph(font, cp, NULL);
        if (glyph) {
            fk_surface_draw_glyph(surface, glyph, cursor_x, y, color);
            
            FK_GlyphMetrics metrics;
            fk_get_glyph_metrics(glyph, &metrics);
            cursor_x += metrics.advance;
            
            fk_free_glyph(glyph);
        }
    }
}

int fk_surface_should_close(FK_Surface *surface) {
    return surface ? surface->should_close : 1;
}

void fk_surface_poll_events(void) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

#endif /* _WIN32 */