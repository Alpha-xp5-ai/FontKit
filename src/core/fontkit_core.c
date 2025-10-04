/**
 * fontkit_core.c - Core implementation
 * PRIVATE - Implementation hidden from users
 */

#include "FontKit.h"
#include "fontkit_types.h"
#include "fontkit_core.h"
#include "loaders/ttf_loader.h"
#include "bitmap_loader.h"
#include "rasterizer/rasterizer.h"
#include "utils/memory.h"
#include "utils/utf8.h"
#include <stdio.h>
#include <string.h>

/* Global context */
FK_Context g_fk_context = {0};

/* ============================================================================
 * Initialization
 * ========================================================================== */

FK_Error fk_init(void) {
    if (g_fk_context.initialized) {
        return FK_OK;
    }
    
    g_fk_context.initialized = 1;
    g_fk_context.last_error = FK_OK;
    
    /* Initialize memory subsystem */
    fk_memory_init();
    
    return FK_OK;
}

void fk_shutdown(void) {
    if (!g_fk_context.initialized) {
        return;
    }
    
    fk_memory_shutdown();
    g_fk_context.initialized = 0;
}

const char* fk_version(void) {
    static char version[32];
    snprintf(version, sizeof(version), "FontKit %d.%d.%d",
             FONTKIT_VERSION_MAJOR, FONTKIT_VERSION_MINOR, FONTKIT_VERSION_PATCH);
    return version;
}

/* ============================================================================
 * Error Handling
 * ========================================================================== */

static void fk_set_error(FK_Error error) {
    g_fk_context.last_error = error;
}

FK_Error fk_get_last_error(void) {
    return g_fk_context.last_error;
}

const char* fk_error_string(FK_Error error) {
    switch (error) {
        case FK_OK: return "Success";
        case FK_ERROR_FILE_NOT_FOUND: return "File not found";
        case FK_ERROR_INVALID_FORMAT: return "Invalid font format";
        case FK_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case FK_ERROR_INVALID_GLYPH: return "Invalid glyph";
        case FK_ERROR_UNSUPPORTED: return "Unsupported feature";
        case FK_ERROR_INVALID_PARAMETER: return "Invalid parameter";
        default: return "Unknown error";
    }
}

/* ============================================================================
 * Font Detection
 * ========================================================================== */

static FK_FontFormat detect_format(const uint8_t *data, size_t size) {
    if (size < 4) return FK_FORMAT_UNKNOWN;
    
    /* TrueType signature */
    if ((data[0] == 0x00 && data[1] == 0x01 && data[2] == 0x00 && data[3] == 0x00) ||
        (data[0] == 't' && data[1] == 'r' && data[2] == 'u' && data[3] == 'e') ||
        (data[0] == 't' && data[1] == 'y' && data[2] == 'p' && data[3] == '1')) {
        return FK_FORMAT_TRUETYPE;
    }
    
    /* Bitmap formats (BDF, PCF) */
    if (data[0] == 'S' && data[1] == 'T' && data[2] == 'A' && data[3] == 'R') {
        return FK_FORMAT_BITMAP;
    }
    
    return FK_FORMAT_UNKNOWN;
}

/* ============================================================================
 * Font Loading
 * ========================================================================== */

static uint8_t* load_file(const char *path, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fk_set_error(FK_ERROR_FILE_NOT_FOUND);
        return NULL;
    }
    
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    uint8_t *data = fk_malloc(*size);
    if (!data) {
        fclose(f);
        fk_set_error(FK_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    fread(data, 1, *size, f);
    fclose(f);
    
    return data;
}

FK_Font* fk_load_font(const char *path, int font_size) {
    if (!g_fk_context.initialized) {
        fk_set_error(FK_ERROR_INVALID_PARAMETER);
        return NULL;
    }
    
    size_t file_size;
    uint8_t *data = load_file(path, &file_size);
    if (!data) return NULL;
    
    FK_Font *font = fk_load_font_memory(data, file_size, font_size);
    fk_free(data);
    
    return font;
}

FK_Font* fk_load_font_memory(const void *data, size_t size, int font_size) {
    if (!g_fk_context.initialized || !data || size == 0 || font_size <= 0) {
        fk_set_error(FK_ERROR_INVALID_PARAMETER);
        return NULL;
    }
    
    /* Detect format */
    FK_FontFormat format = detect_format(data, size);
    if (format == FK_FORMAT_UNKNOWN) {
        fk_set_error(FK_ERROR_INVALID_FORMAT);
        return NULL;
    }
    
    /* Allocate font structure */
    FK_Font *font = fk_calloc(1, sizeof(FK_Font));
    if (!font) {
        fk_set_error(FK_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    font->font_size = font_size;
    font->dpi = 96;  /* Default DPI */
    font->format = format;
    font->style = FK_STYLE_NORMAL;  /* Default style */
    
    /* Create appropriate loader */
    FK_Loader *loader = NULL;
    
    switch (format) {
        case FK_FORMAT_TRUETYPE:
            loader = fk_ttf_loader_create();
            break;
        case FK_FORMAT_BITMAP:
            loader = fk_bitmap_loader_create();
            break;
        default:
            fk_free(font);
            fk_set_error(FK_ERROR_UNSUPPORTED);
            return NULL;
    }
    
    if (!loader) {
        fk_free(font);
        return NULL;
    }
    
    /* Load font data */
    FK_Error err = loader->load(loader, data, size);
    if (err != FK_OK) {
        loader->destroy(loader);
        fk_free(font);
        fk_set_error(err);
        return NULL;
    }
    
    font->loader = loader;
    
    /* Create rasterizer */
    font->rasterizer = fk_rasterizer_create(font_size, font->dpi);
    if (!font->rasterizer) {
        loader->destroy(loader);
        fk_free(font);
        fk_set_error(FK_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    fk_set_error(FK_OK);
    return font;
}

void fk_free_font(FK_Font *font) {
    if (!font) return;
    
    if (font->loader) {
        font->loader->destroy(font->loader);
    }
    
    if (font->rasterizer) {
        fk_rasterizer_destroy(font->rasterizer);
    }
    
    fk_free(font);
}

FK_FontFormat fk_get_format(const FK_Font *font) {
    return font ? font->format : FK_FORMAT_UNKNOWN;
}

void fk_set_dpi(FK_Font *font, int dpi) {
    if (font && dpi > 0) {
        font->dpi = dpi;
        if (font->rasterizer) {
            fk_rasterizer_set_dpi(font->rasterizer, dpi);
        }
    }
}

void fk_set_font_style(FK_Font *font, FK_FontStyle style) {
    if (font) {
        font->style = style;
    }
}

FK_FontStyle fk_get_font_style(const FK_Font *font) {
    return font ? font->style : FK_STYLE_NORMAL;
}

/* ============================================================================
 * Glyph Rendering
 * ========================================================================== */

FK_Glyph* fk_render_glyph(FK_Font *font, uint32_t codepoint, 
                          const FK_RenderOptions *options) {
    if (!font || !font->loader) {
        fk_set_error(FK_ERROR_INVALID_PARAMETER);
        return NULL;
    }
    
    /* Use default options if not provided */
    FK_RenderOptions default_opts = {
        .quality = FK_QUALITY_NORMAL,
        .hinting = FK_HINT_NORMAL,
        .gamma = 1.0f,
        .subpixel = 0,
        .style = font->style,
        .color = FK_COLOR_BLACK
    };
    
    const FK_RenderOptions *opts = options ? options : &default_opts;
    
    /* Override style with font's current style if not specified */
    FK_RenderOptions actual_opts = *opts;
    if (options && options->style == FK_STYLE_NORMAL) {
        actual_opts.style = font->style;
    }
    
    /* Allocate glyph */
    FK_Glyph *glyph = fk_calloc(1, sizeof(FK_Glyph));
    if (!glyph) {
        fk_set_error(FK_ERROR_OUT_OF_MEMORY);
        return NULL;
    }
    
    /* Get outline from loader */
    FK_Error err = font->loader->get_glyph_outline(font->loader, codepoint, &glyph->outline);
    if (err != FK_OK) {
        fk_free(glyph);
        fk_set_error(err);
        return NULL;
    }
    
    /* Get metrics */
    err = font->loader->get_metrics(font->loader, codepoint, &glyph->metrics);
    if (err != FK_OK) {
        fk_free(glyph);
        fk_set_error(err);
        return NULL;
    }
    
    /* Rasterize outline to bitmap */
    err = fk_rasterizer_render(font->rasterizer, &glyph->outline, &glyph->bitmap, &actual_opts);
    if (err != FK_OK) {
        fk_free(glyph);
        fk_set_error(err);
        return NULL;
    }
    
    fk_set_error(FK_OK);
    return glyph;
}

FK_Error fk_get_glyph_metrics(const FK_Glyph *glyph, FK_GlyphMetrics *metrics) {
    if (!glyph || !metrics) {
        return FK_ERROR_INVALID_PARAMETER;
    }
    
    *metrics = glyph->metrics;
    return FK_OK;
}

FK_Error fk_get_glyph_bitmap(const FK_Glyph *glyph, FK_Bitmap *bitmap) {
    if (!glyph || !bitmap) {
        return FK_ERROR_INVALID_PARAMETER;
    }
    
    *bitmap = glyph->bitmap;
    return FK_OK;
}

void fk_free_glyph(FK_Glyph *glyph) {
    if (!glyph) return;
    
    /* Free bitmap */
    if (glyph->bitmap.pixels) {
        fk_free(glyph->bitmap.pixels);
    }
    
    /* Free outline contours */
    for (int i = 0; i < glyph->outline.contour_count; i++) {
        if (glyph->outline.contours[i].points) {
            fk_free(glyph->outline.contours[i].points);
        }
    }
    if (glyph->outline.contours) {
        fk_free(glyph->outline.contours);
    }
    
    fk_free(glyph);
}

/* ============================================================================
 * Text Measurement
 * ========================================================================== */

FK_Error fk_measure_text(FK_Font *font, const char *text, int *width, int *height) {
    if (!font || !text || !width || !height) {
        return FK_ERROR_INVALID_PARAMETER;
    }
    
    *width = 0;
    *height = 0;
    
    uint32_t prev_codepoint = 0;
    const char *p = text;
    
    while (*p) {
        uint32_t codepoint = fk_utf8_decode(&p);
        if (codepoint == 0) break;
        
        FK_GlyphMetrics metrics;
        FK_Error err = font->loader->get_metrics(font->loader, codepoint, &metrics);
        if (err != FK_OK) continue;
        
        *width += metrics.advance;
        
        if (metrics.height > *height) {
            *height = metrics.height;
        }
        
        /* Add kerning */
        if (prev_codepoint) {
            *width += font->loader->get_kerning(font->loader, prev_codepoint, codepoint);
        }
        
        prev_codepoint = codepoint;
    }
    
    return FK_OK;
}

int fk_get_kerning(FK_Font *font, uint32_t left, uint32_t right) {
    if (!font || !font->loader || !font->loader->get_kerning) {
        return 0;
    }
    
    return font->loader->get_kerning(font->loader, left, right);
}

/* ============================================================================
 * Color & Style Utilities
 * ========================================================================== */

FK_Color fk_color_rgb(uint8_t r, uint8_t g, uint8_t b) {
    FK_Color color = {r, g, b, 255};
    return color;
}

FK_Color fk_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    FK_Color color = {r, g, b, a};
    return color;
}

FK_Color fk_color_from_hex(const char *hex) {
    FK_Color color = FK_COLOR_BLACK;
    
    if (!hex) return color;
    
    /* Skip '#' if present */
    if (hex[0] == '#') hex++;
    
    /* Parse hex string */
    unsigned int value = 0;
    sscanf(hex, "%x", &value);
    
    if (strlen(hex) == 6) {
        /* RGB format */
        color.r = (value >> 16) & 0xFF;
        color.g = (value >> 8) & 0xFF;
        color.b = value & 0xFF;
        color.a = 255;
    } else if (strlen(hex) == 8) {
        /* RGBA format */
        color.r = (value >> 24) & 0xFF;
        color.g = (value >> 16) & 0xFF;
        color.b = (value >> 8) & 0xFF;
        color.a = value & 0xFF;
    }
    
    return color;
}

uint32_t fk_color_to_u32(FK_Color color) {
    return ((uint32_t)color.a << 24) | ((uint32_t)color.r << 16) | 
           ((uint32_t)color.g << 8) | color.b;
}

/* ============================================================================
 * Utilities
 * ========================================================================== */

FK_Error fk_export_glyph_ppm(const FK_Glyph *glyph, const char *filename) {
    if (!glyph || !filename) {
        return FK_ERROR_INVALID_PARAMETER;
    }
    
    FILE *f = fopen(filename, "wb");
    if (!f) {
        return FK_ERROR_FILE_NOT_FOUND;
    }
    
    /* Write PPM header */
    fprintf(f, "P5\n%d %d\n255\n", glyph->bitmap.width, glyph->bitmap.height);
    
    /* Write pixel data */
    fwrite(glyph->bitmap.pixels, 1, 
           glyph->bitmap.width * glyph->bitmap.height, f);
    
    fclose(f);
    return FK_OK;
}