    /* Demo 4: Render bold+italic *//* ============================================================================
 * examples/demo.c - Basic FontKit Demo
 * ========================================================================== */
#include <FontKit.h>
#include <stdio.h>
#include <stdlib.h>

#define FONT_PATH "assets/fonts/truetype/Roboto-Regular.ttf"

int main(int argc, char **argv) {
    const char *font_path = FONT_PATH;
    const char *output_file = "output.ppm";
    int font_size = 48;
    
    /* Parse arguments */
    if (argc > 1) font_path = argv[1];
    if (argc > 2) font_size = atoi(argv[2]);
    if (argc > 3) output_file = argv[3];
    
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

}