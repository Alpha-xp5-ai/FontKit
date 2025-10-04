/**
 * fontkit_types.h - Internal type definitions
 * PRIVATE - Not exposed to library users
 */

#ifndef FONTKIT_TYPES_H
#define FONTKIT_TYPES_H

#include "FontKit.h"

/* Maximum values */
#define FK_MAX_CONTOURS 256
#define FK_MAX_POINTS 4096
#define FK_MAX_GLYPHS 65536

/* Forward declarations */
typedef struct FK_Loader FK_Loader;
typedef struct FK_Rasterizer FK_Rasterizer;

/* Point representation */
typedef struct {
    float x, y;
    uint8_t on_curve;
} FK_Point;

/* Contour (closed path) */
typedef struct {
    FK_Point *points;
    int point_count;
    int capacity;
} FK_Contour;

/* Vector glyph outline */
typedef struct {
    FK_Contour *contours;
    int contour_count;
    int capacity;
    
    /* Bounding box */
    int xmin, ymin;
    int xmax, ymax;
} FK_Outline;

/* Font loader interface */
struct FK_Loader {
    FK_FontFormat format;
    
    /* Function pointers */
    FK_Error (*load)(FK_Loader *self, const void *data, size_t size);
    FK_Error (*get_glyph_outline)(FK_Loader *self, uint32_t codepoint, FK_Outline *outline);
    FK_Error (*get_metrics)(FK_Loader *self, uint32_t codepoint, FK_GlyphMetrics *metrics);
    int (*get_kerning)(FK_Loader *self, uint32_t left, uint32_t right);
    void (*destroy)(FK_Loader *self);
    
    void *loader_data;  /* Format-specific data */
};

/* Font structure (internal) */
struct FK_Font {
    FK_Loader *loader;
    int font_size;
    int dpi;
    FK_Rasterizer *rasterizer;
    FK_FontStyle style;  /* Current style flags */
    
    /* Font metadata */
    char family_name[256];
    FK_FontFormat format;
};

/* Glyph structure (internal) */
struct FK_Glyph {
    FK_GlyphMetrics metrics;
    FK_Bitmap bitmap;
    FK_Outline outline;  /* Keep outline for potential reuse */
};

/* Global library state */
typedef struct {
    int initialized;
    FK_Error last_error;
} FK_Context;

/* Global context */
extern FK_Context g_fk_context;

#endif /* FONTKIT_TYPES_H */