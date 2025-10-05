/**
 * FontKit v0.1 - Lightweight Font Rendering Library
 * Copyright (c) 2025
 * 
 * Public API - This is the only header users need to include
 */

#ifndef FONTKIT_H
#define FONTKIT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Version Information
 * ========================================================================== */
#define FONTKIT_VERSION_MAJOR 0
#define FONTKIT_VERSION_MINOR 1
#define FONTKIT_VERSION_PATCH 0

/* ============================================================================
 * Configuration
 * ========================================================================== */
#define FK_FONT_FAMILY_PRIMARY   "assets/fonts/truetype/Roboto-Regular.ttf"
#define FK_FONT_FAMILY_FALLBACK  "Segoe UI, Arial, sans-serif"

/* ============================================================================
 * Types & Structures
 * ========================================================================== */

typedef struct FK_Font FK_Font;
typedef struct FK_Glyph FK_Glyph;

/** Font format types */
typedef enum {
    FK_FORMAT_UNKNOWN = 0,
    FK_FORMAT_TRUETYPE,
    FK_FORMAT_BITMAP,
    FK_FORMAT_VECTOR
} FK_FontFormat;

/** Font style flags (can be combined with |) */
typedef enum {
    FK_STYLE_NORMAL = 0,
    FK_STYLE_BOLD = 1,
    FK_STYLE_ITALIC = 2,
    FK_STYLE_UNDERLINE = 4,
    FK_STYLE_STRIKETHROUGH = 8
} FK_FontStyle;

/** Rendering quality levels */
typedef enum {
    FK_QUALITY_DRAFT = 1,      /* 1x sampling */
    FK_QUALITY_NORMAL = 2,     /* 2x sampling */
    FK_QUALITY_HIGH = 4,       /* 4x sampling */
    FK_QUALITY_ULTRA = 8       /* 8x sampling */
} FK_Quality;

/** Hinting modes */
typedef enum {
    FK_HINT_NONE = 0,
    FK_HINT_LIGHT,
    FK_HINT_NORMAL,
    FK_HINT_FULL
} FK_HintMode;

/** Error codes */
typedef enum {
    FK_OK = 0,
    FK_ERROR_FILE_NOT_FOUND = -1,
    FK_ERROR_INVALID_FORMAT = -2,
    FK_ERROR_OUT_OF_MEMORY = -3,
    FK_ERROR_INVALID_GLYPH = -4,
    FK_ERROR_UNSUPPORTED = -5,
    FK_ERROR_INVALID_PARAMETER = -6
} FK_Error;

/** Color structure (RGBA) */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;  /* Alpha: 0=transparent, 255=opaque */
} FK_Color;

/** Predefined colors */
#define FK_COLOR_BLACK      ((FK_Color){0, 0, 0, 255})
#define FK_COLOR_WHITE      ((FK_Color){255, 255, 255, 255})
#define FK_COLOR_RED        ((FK_Color){255, 0, 0, 255})
#define FK_COLOR_GREEN      ((FK_Color){0, 255, 0, 255})
#define FK_COLOR_BLUE       ((FK_Color){0, 0, 255, 255})
#define FK_COLOR_YELLOW     ((FK_Color){255, 255, 0, 255})
#define FK_COLOR_CYAN       ((FK_Color){0, 255, 255, 255})
#define FK_COLOR_MAGENTA    ((FK_Color){255, 0, 255, 255})
#define FK_COLOR_GRAY       ((FK_Color){128, 128, 128, 255})
#define FK_COLOR_TRANSPARENT ((FK_Color){0, 0, 0, 0})

/** Glyph metrics */
typedef struct {
    int width;              /* Glyph width in pixels */
    int height;             /* Glyph height in pixels */
    int bearing_x;          /* Left side bearing */
    int bearing_y;          /* Top side bearing */
    int advance;            /* Horizontal advance */
} FK_GlyphMetrics;

/** Bitmap data */
typedef struct {
    uint8_t *pixels;        /* Grayscale bitmap (8bpp) */
    int width;
    int height;
    int pitch;              /* Bytes per row */
} FK_Bitmap;

/** Rendering options */
typedef struct {
    FK_Quality quality;     /* Antialiasing quality */
    FK_HintMode hinting;    /* Grid fitting mode */
    float gamma;            /* Gamma correction (1.0 = none) */
    int subpixel;           /* Subpixel rendering (0=off, 1=on) */
    FK_FontStyle style;     /* Font style flags */
    FK_Color color;         /* Text color (for colored rendering) */
} FK_RenderOptions;

/* ============================================================================
 * Core API
 * ========================================================================== */

/**
 * Initialize the FontKit library
 * Must be called before using any other functions
 */
FK_Error fk_init(void);

/**
 * Shutdown and cleanup the library
 */
void fk_shutdown(void);

/**
 * Get library version string
 */
const char* fk_version(void);

/* ============================================================================
 * Font Management
 * ========================================================================== */

/**
 * Load a font from file
 * @param path Path to font file
 * @param font_size Size in points (e.g., 12, 16, 24)
 * @return Font handle or NULL on error
 */
FK_Font* fk_load_font(const char *path, int font_size);

/**
 * Load font from memory buffer
 * @param data Font file data
 * @param size Size of data in bytes
 * @param font_size Size in points
 * @return Font handle or NULL on error
 */
FK_Font* fk_load_font_memory(const void *data, size_t size, int font_size);

/**
 * Free a font and all associated resources
 */
void fk_free_font(FK_Font *font);

/**
 * Get font format
 */
FK_FontFormat fk_get_format(const FK_Font *font);

/**
 * Set DPI for rendering (default: 96)
 */
void fk_set_dpi(FK_Font *font, int dpi);

/**
 * Set font style (bold, italic, etc.)
 * @param font Font handle
 * @param style Style flags (can be combined with |)
 */
void fk_set_font_style(FK_Font *font, FK_FontStyle style);

/**
 * Get current font style
 */
FK_FontStyle fk_get_font_style(const FK_Font *font);

/* ============================================================================
 * Glyph Operations
 * ========================================================================== */

/**
 * Load and rasterize a glyph for a character
 * @param font Font handle
 * @param codepoint Unicode codepoint
 * @param options Rendering options (NULL for defaults)
 * @return Glyph handle or NULL on error
 */
FK_Glyph* fk_render_glyph(FK_Font *font, uint32_t codepoint, 
                          const FK_RenderOptions *options);

/**
 * Get glyph metrics
 */
FK_Error fk_get_glyph_metrics(const FK_Glyph *glyph, FK_GlyphMetrics *metrics);

/**
 * Get glyph bitmap
 */
FK_Error fk_get_glyph_bitmap(const FK_Glyph *glyph, FK_Bitmap *bitmap);

/**
 * Free a glyph
 */
void fk_free_glyph(FK_Glyph *glyph);

/* ============================================================================
 * Text Measurement
 * ========================================================================== */

/**
 * Measure text dimensions
 * @param font Font handle
 * @param text UTF-8 encoded text
 * @param width Output: text width in pixels
 * @param height Output: text height in pixels
 */
FK_Error fk_measure_text(FK_Font *font, const char *text, 
                         int *width, int *height);

/**
 * Get kerning adjustment between two characters
 */
int fk_get_kerning(FK_Font *font, uint32_t left, uint32_t right);

/* ============================================================================
 * Color & Style Utilities
 * ========================================================================== */

/**
 * Create a color from RGB values
 */
FK_Color fk_color_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * Create a color from RGBA values
 */
FK_Color fk_color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

/**
 * Create a color from hex string (e.g., "#FF0000" or "FF0000")
 */
FK_Color fk_color_from_hex(const char *hex);

/**
 * Convert color to 32-bit integer (0xAARRGGBB)
 */
uint32_t fk_color_to_u32(FK_Color color);

/* ============================================================================
 * Utility Functions
 * ========================================================================== */

/**
 * Get the last error code
 */
FK_Error fk_get_last_error(void);

/**
 * Get human-readable error string
 */
const char* fk_error_string(FK_Error error);

/**
 * Export glyph to PPM image file (for debugging)
 */
FK_Error fk_export_glyph_ppm(const FK_Glyph *glyph, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* FONTKIT_H */