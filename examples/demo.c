#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FontKit.h"

#define FONT_PATH "assets/fonts/truetype/Roboto-Regular.ttf"

// ANSI colors for WSL/Linux
#define GREEN  "\033[32m"
#define RED    "\033[31m"
#define CYAN   "\033[36m"
#define YELLOW "\033[33m"
#define RESET  "\033[0m"
#define BOLD   "\033[1m"

static void print_header() {
    printf(BOLD CYAN "FontKit Demo v%s\n" RESET, fk_version());
    printf("====================\n\n");
}

int main(int argc, char **argv) {
    const char *font_path = FONT_PATH;
    const char *text_to_render = "FontKit";
    int font_size = 48;

    if (argc > 1) font_path = argv[1];
    if (argc > 2) font_size = atoi(argv[2]);
    if (argc > 3) text_to_render = argv[3];

    print_header();

    printf("Initializing FontKit...\n");
    if (fk_init() != FK_OK) {
        printf(RED "Error: Failed to initialize FontKit.\n" RESET);
        return 1;
    }

    printf("Loading font: %s (size: %d)\n", font_path, font_size);
    FK_Font *font = fk_load_font(font_path, font_size);
    if (!font) {
        printf(RED "Error: %s\n" RESET, fk_error_string(fk_get_last_error()));
        fk_shutdown();
        return 1;
    }

    printf("Font format: ");
    switch (fk_get_format(font)) {
        case FK_FORMAT_TRUETYPE: printf("TrueType\n\n"); break;
        case FK_FORMAT_BITMAP:   printf("Bitmap\n\n"); break;
        case FK_FORMAT_VECTOR:   printf("Vector\n\n"); break;
        default:                 printf("Unknown\n\n"); break;
    }

    // --- DEMO 1: Normal ---
    printf(BOLD "Demo 1:" RESET " Rendering character 'A' (normal)\n");
    FK_RenderOptions opts_normal = {
        .quality = FK_QUALITY_HIGH,
        .hinting = FK_HINT_NORMAL,
        .gamma = 1.8f,
        .subpixel = 0,
        .style = FK_STYLE_NORMAL,
        .color = FK_COLOR_BLACK
    };
    FK_Glyph *glyph_a = fk_render_glyph(font, 'A', &opts_normal);
    if (glyph_a) {
        fk_export_glyph_ppm(glyph_a, "output_normal.ppm");
        printf("  Saved: output_normal.ppm\n");
        fk_free_glyph(glyph_a);
    } else {
        printf(RED "  Error: glyph render failed.\n" RESET);
    }

    // --- DEMO 2: Bold ---
    printf(BOLD "Demo 2:" RESET " Rendering character 'A' (bold)\n");
    FK_RenderOptions opts_bold = opts_normal;
    opts_bold.style = FK_STYLE_BOLD;
    FK_Glyph *glyph_bold = fk_render_glyph(font, 'A', &opts_bold);
    if (glyph_bold) {
        fk_export_glyph_ppm(glyph_bold, "output_bold.ppm");
        printf("  Saved: output_bold.ppm\n");
        fk_free_glyph(glyph_bold);
    }

    // --- DEMO 3: Italic ---
    printf(BOLD "Demo 3:" RESET " Rendering character 'A' (italic)\n");
    FK_RenderOptions opts_italic = opts_normal;
    opts_italic.style = FK_STYLE_ITALIC;
    FK_Glyph *glyph_italic = fk_render_glyph(font, 'A', &opts_italic);
    if (glyph_italic) {
        fk_export_glyph_ppm(glyph_italic, "output_italic.ppm");
        printf("  Saved: output_italic.ppm\n");
        fk_free_glyph(glyph_italic);
    }

    // --- DEMO 4: Bold + Italic ---
    printf(BOLD "Demo 4:" RESET " Rendering character 'A' (bold+italic)\n");
    FK_RenderOptions opts_both = opts_normal;
    opts_both.style = FK_STYLE_BOLD | FK_STYLE_ITALIC;
    FK_Glyph *glyph_both = fk_render_glyph(font, 'A', &opts_both);
    if (glyph_both) {
        fk_export_glyph_ppm(glyph_both, "output_bold_italic.ppm");
        printf("  Saved: output_bold_italic.ppm\n");
        fk_free_glyph(glyph_both);
    }

    // --- DEMO 5: Color utilities ---
    printf("\n" BOLD "Demo 5:" RESET " Color utilities\n");
    FK_Color red = fk_color_rgb(255, 0, 0);
    FK_Color blue = fk_color_from_hex("#0000FF");
    FK_Color green = fk_color_from_hex("00FF00");
    printf("  Red:   0x%08X\n", fk_color_to_u32(red));
    printf("  Blue:  0x%08X\n", fk_color_to_u32(blue));
    printf("  Green: 0x%08X\n\n", fk_color_to_u32(green));

    // --- DEMO 6: Text measurement ---
    printf(BOLD "Demo 6:" RESET " Text measurement\n");
    int text_width, text_height;
    if (fk_measure_text(font, text_to_render, &text_width, &text_height) == FK_OK)
        printf("  Text \"%s\" dimensions: %dx%d\n\n", text_to_render, text_width, text_height);

    // --- DEMO 7: Font style control ---
    printf(BOLD "Demo 7:" RESET " Font style control\n");
    fk_set_font_style(font, FK_STYLE_BOLD);
    printf("  Current style: %s\n\n",
           (fk_get_font_style(font) & FK_STYLE_BOLD) ? "BOLD" : "NORMAL");

    // --- CLEANUP ---
    printf("Cleaning up...\n");
    fk_free_font(font);
    fk_shutdown();
    printf(GREEN "✓ Demo complete!\n" RESET);

    printf("\nGenerated files:\n");
    printf("  - output_normal.ppm\n");
    printf("  - output_bold.ppm\n");
    printf("  - output_italic.ppm\n");
    printf("  - output_bold_italic.ppm\n\n");
    printf("Open these files with an image viewer that supports PPM format.\n");

    return 0;
}
