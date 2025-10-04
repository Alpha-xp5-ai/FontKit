/**
 * bitmap_loader.c - Bitmap font loader (stub implementation)
 * Can be extended to support BDF, PCF, etc.
 */

#include "bitmap_loader.h"
#include "utils/memory.h"

typedef struct {
    const uint8_t *font_data;
    size_t font_size;
} BitmapData;

static FK_Error bitmap_load(FK_Loader *loader, const void *data, size_t size) {
    BitmapData *bdata = fk_calloc(1, sizeof(BitmapData));
    if (!bdata) return FK_ERROR_OUT_OF_MEMORY;
    
    bdata->font_data = data;
    bdata->font_size = size;
    
    loader->loader_data = bdata;
    return FK_OK;
}

static FK_Error bitmap_get_glyph_outline(FK_Loader *loader, uint32_t codepoint,
                                         FK_Outline *outline) {
    (void)loader;
    (void)codepoint;
    
    /* Bitmap fonts don't have outlines */
    outline->contour_count = 0;
    outline->contours = NULL;
    return FK_ERROR_UNSUPPORTED;
}

static FK_Error bitmap_get_metrics(FK_Loader *loader, uint32_t codepoint,
                                   FK_GlyphMetrics *metrics) {
    (void)loader;
    (void)codepoint;
    (void)metrics;
    
    return FK_ERROR_UNSUPPORTED;
}

static int bitmap_get_kerning(FK_Loader *loader, uint32_t left, uint32_t right) {
    (void)loader;
    (void)left;
    (void)right;
    
    return 0;
}

static void bitmap_destroy(FK_Loader *loader) {
    if (loader->loader_data) {
        fk_free(loader->loader_data);
    }
    fk_free(loader);
}

FK_Loader* fk_bitmap_loader_create(void) {
    FK_Loader *loader = fk_calloc(1, sizeof(FK_Loader));
    if (!loader) return NULL;
    
    loader->format = FK_FORMAT_BITMAP;
    loader->load = bitmap_load;
    loader->get_glyph_outline = bitmap_get_glyph_outline;
    loader->get_metrics = bitmap_get_metrics;
    loader->get_kerning = bitmap_get_kerning;
    loader->destroy = bitmap_destroy;
    
    return loader;
}