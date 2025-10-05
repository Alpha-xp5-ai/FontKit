/* ============================================================================
 * examples/platform_demo.c - Cross-platform GUI Demo
 * Compile on Windows: gcc platform_demo.c -o platform_demo -L../dist/lib -lFontKit -lgdi32 -lm
 * Compile on Linux:   gcc platform_demo.c -o platform_demo -L../dist/lib -lFontKit -lX11 -lm
 * ========================================================================== */

#include <FontKit.h>
#include <stdio.h>
#include <stdlib.h>

/* Include platform header */
#ifdef _WIN32
    #include "../src/platform/platform.h"
#elif defined(__linux__)
    #include "../src/platform/platform.h"
    /* Forward declare X11-specific function */
    extern void fk_surface_poll_events_x11(FK_Surface *surface);
#endif

int main(int argc, char **argv) {
    const char *font_path = FK_FONT_FAMILY_PRIMARY;
    int font_size = 32;
    
    if (argc > 1) font_path = argv[1];
    if (argc > 2) font_size = atoi(argv[2]);
    
    printf("FontKit Platform Demo\n");
    printf("=====================\n\n");
    
    /* Initialize FontKit */
    if (fk_init() != FK_OK) {
        fprintf(stderr, "Failed to initialize FontKit\n");
        return 1;
    }
    
    /* Initialize platform */
    if (fk_platform_init() != FK_OK) {
        fprintf(stderr, "Failed to initialize platform\n");
        fk_shutdown();
        return 1;
    }
    
    /* Load font */
    printf("Loading font: %s\n", font_path);
    FK_Font *font = fk_load_font(font_path, font_size);
    
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", fk_error_string(fk_get_last_error()));
        fk_platform_shutdown();
        fk_shutdown();
        return 1;
    }
    
    /* Create window */
    FK_Surface *surface = fk_surface_create(800, 600, "FontKit Demo");
    if (!surface) {
        fprintf(stderr, "Failed to create surface\n");
        fk_free_font(font);
        fk_platform_shutdown();
        fk_shutdown();
        return 1;
    }
    
    printf("Window created. Close window to exit.\n\n");
    
    /* Main loop */
    while (!fk_surface_should_close(surface)) {
        /* Clear background */
        fk_surface_clear(surface, 0xFFFFFF);  /* White */
        
        /* Draw title */
        fk_surface_draw_text(surface, font, "FontKit Demo", 20, 20, 0x000000);
        
        /* Draw normal text */
        fk_set_font_style(font, FK_STYLE_NORMAL);
        fk_surface_draw_text(surface, font, "Normal Text", 20, 80, 0x000000);
        
        /* Draw bold text */
        fk_set_font_style(font, FK_STYLE_BOLD);
        fk_surface_draw_text(surface, font, "Bold Text", 20, 140, 0xFF0000);
        
        /* Draw italic text */
        fk_set_font_style(font, FK_STYLE_ITALIC);
        fk_surface_draw_text(surface, font, "Italic Text", 20, 200, 0x0000FF);
        
        /* Draw bold+italic */
        fk_set_font_style(font, FK_STYLE_BOLD | FK_STYLE_ITALIC);
        fk_surface_draw_text(surface, font, "Bold Italic", 20, 260, 0x00AA00);
        
        /* Colored text samples */
        fk_set_font_style(font, FK_STYLE_NORMAL);
        fk_surface_draw_text(surface, font, "Color: Red", 400, 80, 0xFF0000);
        fk_surface_draw_text(surface, font, "Color: Green", 400, 140, 0x00FF00);
        fk_surface_draw_text(surface, font, "Color: Blue", 400, 200, 0x0000FF);
        fk_surface_draw_text(surface, font, "Color: Cyan", 400, 260, 0x00FFFF);
        fk_surface_draw_text(surface, font, "Color: Magenta", 400, 320, 0xFF00FF);
        fk_surface_draw_text(surface, font, "Color: Yellow", 400, 380, 0xFFFF00);
        
        /* Display info */
        FK_Font *small_font = fk_load_font(font_path, 16);
        if (small_font) {
            char info[256];
            snprintf(info, sizeof(info), "FontKit v%s | Font: %s", 
                     fk_version(), font_path);
            fk_surface_draw_text(surface, small_font, info, 20, 550, 0x808080);
            fk_free_font(small_font);
        }
        
        /* Present */
        fk_surface_present(surface);
        
        /* Poll events */
#ifdef __linux__
        fk_surface_poll_events_x11(surface);
#else
        fk_surface_poll_events();
#endif
    }
    
    /* Cleanup */
    fk_surface_destroy(surface);
    fk_free_font(font);
    fk_platform_shutdown();
    fk_shutdown();
    
    printf("Demo complete!\n");
    
    return 0;
}