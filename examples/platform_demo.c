/* ============================================================================
 * examples/platform_demo.c - Cross-platform GUI Demo
 * Compile on Windows: gcc platform_demo.c -o platform_demo -L../dist/lib -lFontKit -lgdi32 -lm
 * Compile on Linux:   gcc platform_demo.c -o platform_demo -L../dist/lib -lFontKit -lX11 -lm
 * ========================================================================== */

#include <FontKit.h>
#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>

int main(int argc, char **argv) {
    const char *font_path = FK_FONT_FAMILY_PRIMARY;
    
    printf("╔════════════════════════════════════════╗\n");
    printf("║   FontKit X11 Window Demo             ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    
    // Initialize FontKit
    printf("Init FontKit... ");
    fflush(stdout);
    if (fk_init() != FK_OK) {
        printf("FAILED\n");
        return 1;
    }
    printf("OK\n");
    
    // Load font
    printf("Load font... ");
    fflush(stdout);
    FK_Font *font = fk_load_font(font_path, 48);
    if (!font) {
        printf("FAILED: %s\n", fk_error_string(fk_get_last_error()));
        fk_shutdown();
        return 1;
    }
    printf("OK\n");
    
    // Open X11 display
    printf("Open X11... ");
    fflush(stdout);
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        printf("FAILED - Check DISPLAY variable\n");
        fk_free_font(font);
        fk_shutdown();
        return 1;
    }
    printf("OK\n");
    
    int screen = DefaultScreen(display);
    
    // Create window
    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        100, 100, 800, 600, 2,
        BlackPixel(display, screen),
        WhitePixel(display, screen)
    );
    
    XStoreName(display, window, "FontKit Demo");
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    
    GC gc = XCreateGC(display, window, 0, NULL);
    XSetForeground(display, gc, BlackPixel(display, screen));
    
    // Load X11 font for debug text
    XFontStruct *xfont = XLoadQueryFont(display, "fixed");
    if (xfont) {
        XSetFont(display, gc, xfont->fid);
    }
    
    XMapWindow(display, window);
    XFlush(display);
    
    printf("\n✓ Window opened!\n");
    printf("✓ Rendering glyphs...\n\n");
    
    // Prepare to render FontKit glyphs
    FK_RenderOptions opts = {
        .quality = FK_QUALITY_HIGH,
        .hinting = FK_HINT_NORMAL,
        .gamma = 1.8f,
        .style = FK_STYLE_NORMAL,
        .color = FK_COLOR_BLACK
    };
    
    XEvent event;
    int running = 1;
    int rendered = 0;
    
    while (running) {
        XNextEvent(display, &event);
        
        if (event.type == Expose && !rendered) {
            // Clear window
            XClearWindow(display, window);
            
            // Draw debug text with X11 font
            XDrawString(display, window, gc, 20, 30, 
                "FontKit Demo - Ubuntu Window", 28);
            XDrawString(display, window, gc, 20, 50,
                "=================================", 33);
            
            // Render FontKit glyph 'A'
            printf("Rendering 'A'...\n");
            FK_Glyph *glyph = fk_render_glyph(font, 'A', &opts);
            
            if (glyph) {
                FK_Bitmap bmp;
                if (fk_get_glyph_bitmap(glyph, &bmp) == FK_OK) {
                    printf("  Bitmap: %dx%d pixels\n", bmp.width, bmp.height);
                    
                    // Draw glyph pixel by pixel
                    int start_x = 100;
                    int start_y = 100;
                    
                    for (int y = 0; y < bmp.height; y++) {
                        for (int x = 0; x < bmp.width; x++) {
                            uint8_t alpha = bmp.pixels[y * bmp.pitch + x];
                            
                            // Draw if pixel is visible
                            if (alpha > 128) {
                                XDrawPoint(display, window, gc, 
                                          start_x + x, start_y + y);
                            }
                        }
                    }
                    
                    // Draw label
                    XDrawString(display, window, gc, start_x, start_y - 10,
                        "Letter 'A' from FontKit:", 24);
                    
                    printf("  ✓ Rendered at (%d, %d)\n", start_x, start_y);
                } else {
                    printf("  Failed to get bitmap\n");
                }
                
                fk_free_glyph(glyph);
            } else {
                printf("  Failed to render glyph\n");
            }
            
            // Render 'B' in bold
            printf("Rendering 'B' (bold)...\n");
            opts.style = FK_STYLE_BOLD;
            glyph = fk_render_glyph(font, 'B', &opts);
            
            if (glyph) {
                FK_Bitmap bmp;
                if (fk_get_glyph_bitmap(glyph, &bmp) == FK_OK) {
                    int start_x = 250;
                    int start_y = 100;
                    
                    for (int y = 0; y < bmp.height; y++) {
                        for (int x = 0; x < bmp.width; x++) {
                            uint8_t alpha = bmp.pixels[y * bmp.pitch + x];
                            if (alpha > 128) {
                                XDrawPoint(display, window, gc, 
                                          start_x + x, start_y + y);
                            }
                        }
                    }
                    
                    XDrawString(display, window, gc, start_x, start_y - 10,
                        "Letter 'B' (Bold):", 18);
                    
                    printf("  ✓ Rendered at (%d, %d)\n", start_x, start_y);
                }
                fk_free_glyph(glyph);
            }
            
            // Render 'C' in italic
            printf("Rendering 'C' (italic)...\n");
            opts.style = FK_STYLE_ITALIC;
            glyph = fk_render_glyph(font, 'C', &opts);
            
            if (glyph) {
                FK_Bitmap bmp;
                if (fk_get_glyph_bitmap(glyph, &bmp) == FK_OK) {
                    int start_x = 400;
                    int start_y = 100;
                    
                    // Use red color
                    XSetForeground(display, gc, 0xFF0000);
                    
                    for (int y = 0; y < bmp.height; y++) {
                        for (int x = 0; x < bmp.width; x++) {
                            uint8_t alpha = bmp.pixels[y * bmp.pitch + x];
                            if (alpha > 128) {
                                XDrawPoint(display, window, gc, 
                                          start_x + x, start_y + y);
                            }
                        }
                    }
                    
                    XSetForeground(display, gc, BlackPixel(display, screen));
                    XDrawString(display, window, gc, start_x, start_y - 10,
                        "Letter 'C' (Italic):", 20);
                    
                    printf("  ✓ Rendered at (%d, %d)\n", start_x, start_y);
                }
                fk_free_glyph(glyph);
            }
            
            // Instructions
            XDrawString(display, window, gc, 20, 550,
                "Press any key to exit", 21);
            
            XFlush(display);
            rendered = 1;
            
            printf("\n✓ All glyphs rendered!\n");
            printf("✓ Press any key in window to exit\n");
        }
        
        if (event.type == KeyPress) {
            printf("Exiting...\n");
            running = 0;
        }
    }
    
    // Cleanup
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    
    fk_free_font(font);
    fk_shutdown();
    
    printf("✓ Done!\n");
    
    return 0;
}

#else
int main() {
    printf("This requires Linux/X11\n");
    return 1;
}
#endif