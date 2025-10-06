/* ============================================================================
 * src/platform/x11.c - X11/Linux Platform Implementation (Color Channel Fix)
 * ========================================================================== */
#if defined(__linux__) || defined(__linux) || defined(linux)

#include "platform.h"
#include "utils/memory.h"
#include "utils/utf8.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct FK_Surface {
    Display *display;
    Window window;
    GC gc;
    XImage *image;
    uint32_t *pixels;
    int width;
    int height;
    int should_close;
    Atom wm_delete_window;
};

/* ==========================================================================
 * Init / Shutdown
 * ========================================================================== */
FK_Error fk_platform_init(void) {
    printf("[X11] fk_platform_init()\n");
    return FK_OK;
}

void fk_platform_shutdown(void) {
    printf("[X11] fk_platform_shutdown()\n");
}

/* ==========================================================================
 * Surface Creation
 * ========================================================================== */
FK_Surface* fk_surface_create(int width, int height, const char *title) {
    printf("[X11] Creating surface %dx%d...\n", width, height);

    FK_Surface *surface = fk_calloc(1, sizeof(FK_Surface));
    if (!surface) return NULL;

    surface->width = width;
    surface->height = height;

    surface->display = XOpenDisplay(NULL);
    if (!surface->display) {
        fprintf(stderr, "[X11] Failed to open display!\n");
        fk_free(surface);
        return NULL;
    }

    int screen = DefaultScreen(surface->display);
    surface->window = XCreateSimpleWindow(
        surface->display, RootWindow(surface->display, screen),
        0, 0, width, height, 1,
        BlackPixel(surface->display, screen),
        WhitePixel(surface->display, screen)
    );

    XStoreName(surface->display, surface->window, title);
    surface->wm_delete_window = XInternAtom(surface->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(surface->display, surface->window, &surface->wm_delete_window, 1);
    XSelectInput(surface->display, surface->window, ExposureMask | KeyPressMask | StructureNotifyMask);
    surface->gc = XCreateGC(surface->display, surface->window, 0, NULL);

    surface->pixels = fk_calloc(width * height, sizeof(uint32_t));
    if (!surface->pixels) {
        fprintf(stderr, "[X11] Failed to allocate pixel buffer\n");
        fk_surface_destroy(surface);
        return NULL;
    }

    Visual *visual = DefaultVisual(surface->display, screen);
    int depth = DefaultDepth(surface->display, screen);

    surface->image = XCreateImage(surface->display, visual, depth,
                                  ZPixmap, 0, (char*)surface->pixels,
                                  width, height, 32, width * 4);

    if (!surface->image) {
        fprintf(stderr, "[X11] Failed to create XImage\n");
        fk_surface_destroy(surface);
        return NULL;
    }

    XMapWindow(surface->display, surface->window);
    XFlush(surface->display);
    printf("[X11] Surface ready\n");
    return surface;
}

/* ==========================================================================
 * Surface Destruction
 * ========================================================================== */
void fk_surface_destroy(FK_Surface *surface) {
    if (!surface) return;
    printf("[X11] Destroying surface...\n");

    if (surface->image) {
        surface->image->data = NULL;
        XDestroyImage(surface->image);
    }
    if (surface->pixels) fk_free(surface->pixels);
    if (surface->gc) XFreeGC(surface->display, surface->gc);
    if (surface->window) XDestroyWindow(surface->display, surface->window);
    if (surface->display) XCloseDisplay(surface->display);
    fk_free(surface);
    printf("[X11] Surface destroyed.\n");
}

/* ==========================================================================
 * Clear / Draw
 * ========================================================================== */
void fk_surface_clear(FK_Surface *surface, uint32_t color) {
    if (!surface || !surface->pixels) return;

    /* Correct channel order: X11 expects 0xRRGGBB */
    for (int y = 0; y < surface->height; y++) {
        float t = (float)y / surface->height;
        uint8_t r = (uint8_t)(255 * t);
        uint8_t g = (uint8_t)(255 * (1.0 - t));
        uint8_t b = (uint8_t)(255 * (0.5 + 0.5 * (1.0 - t)));
        uint32_t grad = (r << 16) | (g << 8) | b;
        for (int x = 0; x < surface->width; x++)
            surface->pixels[y * surface->width + x] = grad;
    }
}

/* ---- Glyph Rendering ---- */
void fk_surface_draw_glyph(FK_Surface *surface, const FK_Glyph *glyph,
                           int x, int y, uint32_t color)
{
    if (!surface || !glyph) return;

    FK_Bitmap bmp;
    if (fk_get_glyph_bitmap(glyph, &bmp) != FK_OK || !bmp.pixels) return;

    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    for (int gy = 0; gy < bmp.height; gy++) {
        for (int gx = 0; gx < bmp.width; gx++) {
            int px = x + gx;
            int py = y + gy;
            if (px < 0 || py < 0 || px >= surface->width || py >= surface->height)
                continue;

            uint8_t alpha = bmp.pixels[gy * bmp.pitch + gx];
            if (alpha == 0) continue;

            uint32_t *dst = &surface->pixels[py * surface->width + px];
            uint8_t bg_r = (*dst >> 16) & 0xFF;
            uint8_t bg_g = (*dst >> 8) & 0xFF;
            uint8_t bg_b = *dst & 0xFF;

            uint8_t out_r = (r * alpha + bg_r * (255 - alpha)) / 255;
            uint8_t out_g = (g * alpha + bg_g * (255 - alpha)) / 255;
            uint8_t out_b = (b * alpha + bg_b * (255 - alpha)) / 255;
            *dst = (out_r << 16) | (out_g << 8) | out_b;
        }
    }
}

/* ---- Text Rendering ---- */
void fk_surface_draw_text(FK_Surface *surface, FK_Font *font,
                          const char *text, int x, int y, uint32_t color)
{
    if (!surface || !font || !text) return;

    int cursor_x = x;
    const char *p = text;

    while (*p) {
        uint32_t cp = fk_utf8_decode(&p);
        if (cp == 0) break;

        FK_Glyph *glyph = fk_render_glyph(font, cp, NULL);
        if (!glyph) continue;

        FK_GlyphMetrics metrics;
        fk_get_glyph_metrics(glyph, &metrics);

        int draw_y = y + surface->height / 2 - metrics.height / 2;
        fk_surface_draw_glyph(surface, glyph, cursor_x, draw_y, color);
        cursor_x += metrics.advance ? metrics.advance : (metrics.width + 2);

        fk_free_glyph(glyph);
    }
}

/* ==========================================================================
 * Present + Events
 * ========================================================================== */
void fk_surface_present(FK_Surface *surface) {
    if (!surface || !surface->image) return;
    XPutImage(surface->display, surface->window, surface->gc, surface->image,
              0, 0, 0, 0, surface->width, surface->height);
    XFlush(surface->display);
}

int fk_surface_should_close(FK_Surface *surface) {
    return surface ? surface->should_close : 1;
}

void fk_surface_poll_events_x11(FK_Surface *surface) {
    if (!surface) return;
    XEvent event;
    while (XPending(surface->display)) {
        XNextEvent(surface->display, &event);
        switch (event.type) {
            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == surface->wm_delete_window)
                    surface->should_close = 1;
                break;
            case Expose:
                fk_surface_present(surface);
                break;
        }
    }
}

void fk_surface_poll_events(void) { }

#endif /* linux */
