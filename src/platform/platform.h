/* ============================================================================
 * src/platform/platform.h - Cross-platform abstraction
 * ========================================================================== */
#ifndef FONTKIT_PLATFORM_H
#define FONTKIT_PLATFORM_H

#include "../../include/FontKit.h"

/* Platform-specific surface for rendering */
typedef struct FK_Surface FK_Surface;

/* Platform initialization */
FK_Error fk_platform_init(void);
void fk_platform_shutdown(void);

/* Surface management */
FK_Surface* fk_surface_create(int width, int height, const char *title);
void fk_surface_destroy(FK_Surface *surface);
void fk_surface_present(FK_Surface *surface);

/* Drawing operations */
void fk_surface_clear(FK_Surface *surface, uint32_t color);
void fk_surface_draw_glyph(FK_Surface *surface, const FK_Glyph *glyph, 
                           int x, int y, uint32_t color);
void fk_surface_draw_text(FK_Surface *surface, FK_Font *font, 
                         const char *text, int x, int y, uint32_t color);

/* Event handling */
int fk_surface_should_close(FK_Surface *surface);
void fk_surface_poll_events(void);

#endif /* FONTKIT_PLATFORM_H */

