/* ============================================================================
 * src/rasterizer/rasterizer.h - Glyph Rasterizer
 * ========================================================================== */
#ifndef FONTKIT_RASTERIZER_H
#define FONTKIT_RASTERIZER_H

#include "fontkit_types.h"
#include "FontKit.h"

typedef struct FK_Rasterizer FK_Rasterizer;

FK_Rasterizer* fk_rasterizer_create(int font_size, int dpi);
void fk_rasterizer_destroy(FK_Rasterizer *rast);
void fk_rasterizer_set_dpi(FK_Rasterizer *rast, int dpi);

FK_Error fk_rasterizer_render(FK_Rasterizer *rast, const FK_Outline *outline,
                               FK_Bitmap *bitmap, const FK_RenderOptions *opts);

#endif /* FONTKIT_RASTERIZER_H */