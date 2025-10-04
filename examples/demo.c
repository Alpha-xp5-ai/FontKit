/* ============================================================================
 * examples/demo.c - Basic FontKit Demo
 * ========================================================================== */
#include <FontKit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *font_path = FK_FONT_FAMILY_PRIMARY;
    const char *output_file = "output.ppm";
    const char *text_to_render = "FontKit";
    int font_size = 48;
    
    /* Parse arguments */
    if (argc > 1) font_path = argv[1];
    if (argc > 2) font_size = atoi(argv[2]);
    if (argc > 3) output_file = argv[3];
    if (argc > 4) text_to_render = argv[4];
    
    printf("FontKit Demo v%s\n", fk_version());
    printf("====================\n\n");
    
    /* Initialize library */
    printf("Initializing FontKit...\n");
    if (fk_init() != FK_OK) {
        fprintf(stderr, "Failed to initialize FontKit\n");
        return 1;
    }
    
    /* Load font */
    printf("Loading font: %s (size: %d)\n", font_path, font_size);
    FK_Font *font = fk_load_font(font_path, font_size);
    
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", fk_error_string(fk_get_last_error()));
        fk_shutdown();
        return 1;
    }
    
    printf("Font format: ");
    switch (fk_get_format(font)) {
        case FK_FORMAT_TRUETYPE: printf("TrueType\n"); break;
        case FK_FORMAT_BITMAP: printf("Bitmap\n"); break;
        case FK_FORMAT_VECTOR: printf("Vector\n"); break;
        default: printf("Unknown\n"); break;
    }
    printf("\n");
    
    /* Demo 1: Render single character */
    printf("Demo 1: Rendering character 'A' (normal)\n");
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
    }
    
    /* Demo 2: Render bold */
    printf("Demo 2: Rendering character 'A' (bold)\n");
    FK_RenderOptions opts_bold = opts_normal;
    opts_bold.style = FK_STYLE_BOLD;
    
    FK_Glyph *glyph_bold = fk_render_glyph(font, 'A', &opts_bold);
    if (glyph_bold) {
        fk_export_glyph_ppm(glyph_bold, "output_bold.ppm");
        printf("  Saved: output_bold.ppm\n");
        fk_free_glyph(glyph_bold);
    }
    
    /* Demo 3: Render italic */
    printf("Demo 3: Rendering character 'A' (italic)\n");
    FK_RenderOptions opts_italic = opts_normal;
    opts_italic.style = FK_STYLE_ITALIC;
    
    FK_Glyph *glyph_italic = fk_render_glyph(font, 'A', &opts_italic);
    if (glyph_italic) {
        fk_export_glyph_ppm(glyph_italic, "output_italic.ppm");
        printf("  Saved: output_italic.ppm\n");
        fk_free_glyph(glyph_italic);
    }
    
}