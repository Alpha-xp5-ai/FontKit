/**
 * ttf_loader.c - TrueType font loader
 * Implements TrueType font parsing without external dependencies
 */

#include "ttf_loader.h"
#include "fontkit_types.h"
#include "memory.h"
#include <string.h>
#include <math.h>

/* TrueType table tags */
#define TAG(a,b,c,d) ((uint32_t)(((a)<<24)|((b)<<16)|((c)<<8)|(d)))
#define TAG_HEAD TAG('h','e','a','d')
#define TAG_HHEA TAG('h','h','e','a')
#define TAG_HMTX TAG('h','m','t','x')
#define TAG_MAXP TAG('m','a','x','p')
#define TAG_LOCA TAG('l','o','c','a')
#define TAG_GLYF TAG('g','l','y','f')
#define TAG_CMAP TAG('c','m','a','p')
#define TAG_KERN TAG('k','e','r','n')
#define TAG_NAME TAG('n','a','m','e')

/* Flags for glyph outlines */
#define FLAG_ON_CURVE      0x01
#define FLAG_X_SHORT       0x02
#define FLAG_Y_SHORT       0x04
#define FLAG_REPEAT        0x08
#define FLAG_X_SAME        0x10
#define FLAG_Y_SAME        0x20

/* Simple glyph flags */
#define SIMPLE_GLYPH_FLAG  0x01

/* TrueType specific data */
typedef struct {
    const uint8_t *font_data;
    size_t font_size;
    
    /* Table offsets */
    uint32_t head_offset;
    uint32_t hhea_offset;
    uint32_t hmtx_offset;
    uint32_t maxp_offset;
    uint32_t loca_offset;
    uint32_t glyf_offset;
    uint32_t cmap_offset;
    uint32_t kern_offset;
    
    /* Font metrics */
    int units_per_em;
    int ascent;
    int descent;
    int line_gap;
    int num_glyphs;
    int index_to_loc_format;  /* 0=short, 1=long */
    int num_h_metrics;
    
    /* Cmap data */
    uint32_t cmap_offset_table;
    int cmap_format;
} TTF_Data;

/* ============================================================================
 * Binary Reading Utilities
 * ========================================================================== */

static uint16_t read_u16(const uint8_t *data) {
    return (data[0] << 8) | data[1];
}

static int16_t read_i16(const uint8_t *data) {
    return (int16_t)read_u16(data);
}

static uint32_t read_u32(const uint8_t *data) {
    return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8) | data[3];
}

/* ============================================================================
 * Table Directory
 * ========================================================================== */

static uint32_t find_table(const uint8_t *font_data, size_t font_size, uint32_t tag) {
    if (font_size < 12) return 0;
    
    uint16_t num_tables = read_u16(font_data + 4);
    const uint8_t *table_dir = font_data + 12;
    
    for (int i = 0; i < num_tables; i++) {
        const uint8_t *entry = table_dir + i * 16;
        if (font_data + (entry - font_data) + 16 > font_data + font_size) {
            return 0;
        }
        
        uint32_t entry_tag = read_u32(entry);
        if (entry_tag == tag) {
            return read_u32(entry + 8);  /* offset */
        }
    }
    
    return 0;
}

/* ============================================================================
 * Character to Glyph Index Mapping
 * ========================================================================== */

static int get_glyph_index(TTF_Data *ttf, uint32_t codepoint) {
    if (!ttf->cmap_offset) return 0;
    
    const uint8_t *cmap = ttf->font_data + ttf->cmap_offset_table;
    
    if (ttf->cmap_format == 4) {
        /* Format 4: Segment mapping to delta values */
        uint16_t seg_count = read_u16(cmap + 6) / 2;
        const uint8_t *end_code = cmap + 14;
        const uint8_t *start_code = end_code + seg_count * 2 + 2;
        const uint8_t *id_delta = start_code + seg_count * 2;
        const uint8_t *id_range_offset = id_delta + seg_count * 2;
        
        for (int i = 0; i < seg_count; i++) {
            uint16_t end = read_u16(end_code + i * 2);
            if (codepoint <= end) {
                uint16_t start = read_u16(start_code + i * 2);
                if (codepoint >= start) {
                    uint16_t range_offset = read_u16(id_range_offset + i * 2);
                    if (range_offset == 0) {
                        int16_t delta = read_i16(id_delta + i * 2);
                        return (codepoint + delta) & 0xFFFF;
                    } else {
                        const uint8_t *glyph_idx_ptr = id_range_offset + i * 2 + range_offset +
                                                       (codepoint - start) * 2;
                        return read_u16(glyph_idx_ptr);
                    }
                }
                break;
            }
        }
    }
    
    return 0;  /* Glyph not found */
}

/* ============================================================================
 * Glyph Loading
 * ========================================================================== */

static uint32_t get_glyph_offset(TTF_Data *ttf, int glyph_index) {
    if (glyph_index < 0 || glyph_index >= ttf->num_glyphs) {
        return 0;
    }
    
    const uint8_t *loca = ttf->font_data + ttf->loca_offset;
    
    if (ttf->index_to_loc_format == 0) {
        /* Short format */
        return read_u16(loca + glyph_index * 2) * 2;
    } else {
        /* Long format */
        return read_u32(loca + glyph_index * 4);
    }
}

static FK_Error load_simple_glyph(TTF_Data *ttf, const uint8_t *glyph_data,
                                  int16_t num_contours, FK_Outline *outline) {
    const uint8_t *p = glyph_data + 10;  /* Skip header */
    
    /* Read end points of contours */
    uint16_t *end_pts = fk_malloc(num_contours * sizeof(uint16_t));
    for (int i = 0; i < num_contours; i++) {
        end_pts[i] = read_u16(p);
        p += 2;
    }
    
    int num_points = end_pts[num_contours - 1] + 1;
    
    /* Skip instructions */
    uint16_t instruction_length = read_u16(p);
    p += 2 + instruction_length;
    
    /* Read flags */
    uint8_t *flags = fk_malloc(num_points);
    for (int i = 0; i < num_points; i++) {
        flags[i] = *p++;
        if (flags[i] & FLAG_REPEAT) {
            uint8_t repeat_count = *p++;
            for (int j = 0; j < repeat_count; j++) {
                flags[++i] = flags[i - 1];
            }
        }
    }
    
    /* Read X coordinates */
    int16_t *x_coords = fk_malloc(num_points * sizeof(int16_t));
    int x = 0;
    for (int i = 0; i < num_points; i++) {
        if (flags[i] & FLAG_X_SHORT) {
            int dx = *p++;
            x += (flags[i] & FLAG_X_SAME) ? dx : -dx;
        } else {
            if (!(flags[i] & FLAG_X_SAME)) {
                x += read_i16(p);
                p += 2;
            }
        }
        x_coords[i] = x;
    }
    
    /* Read Y coordinates */
    int16_t *y_coords = fk_malloc(num_points * sizeof(int16_t));
    int y = 0;
    for (int i = 0; i < num_points; i++) {
        if (flags[i] & FLAG_Y_SHORT) {
            int dy = *p++;
            y += (flags[i] & FLAG_Y_SAME) ? dy : -dy;
        } else {
            if (!(flags[i] & FLAG_Y_SAME)) {
                y += read_i16(p);
                p += 2;
            }
        }
        y_coords[i] = y;
    }
    
    /* Build contours */
    outline->contour_count = num_contours;
    outline->contours = fk_calloc(num_contours, sizeof(FK_Contour));
    
    int start_idx = 0;
    for (int c = 0; c < num_contours; c++) {
        int end_idx = end_pts[c];
        int point_count = end_idx - start_idx + 1;
        
        FK_Contour *contour = &outline->contours[c];
        contour->point_count = point_count;
        contour->points = fk_malloc(point_count * sizeof(FK_Point));
        
        for (int i = 0; i < point_count; i++) {
            int idx = start_idx + i;
            contour->points[i].x = x_coords[idx];
            contour->points[i].y = y_coords[idx];
            contour->points[i].on_curve = (flags[idx] & FLAG_ON_CURVE) ? 1 : 0;
        }
        
        start_idx = end_idx + 1;
    }
    
    fk_free(end_pts);
    fk_free(flags);
    fk_free(x_coords);
    fk_free(y_coords);
    
    return FK_OK;
}

/* ============================================================================
 * Loader Interface Implementation
 * ========================================================================== */

static FK_Error ttf_load(FK_Loader *loader, const void *data, size_t size) {
    TTF_Data *ttf = fk_calloc(1, sizeof(TTF_Data));
    if (!ttf) return FK_ERROR_OUT_OF_MEMORY;
    
    ttf->font_data = data;
    ttf->font_size = size;
    
    /* Find required tables */
    ttf->head_offset = find_table(data, size, TAG_HEAD);
    ttf->hhea_offset = find_table(data, size, TAG_HHEA);
    ttf->hmtx_offset = find_table(data, size, TAG_HMTX);
    ttf->maxp_offset = find_table(data, size, TAG_MAXP);
    ttf->loca_offset = find_table(data, size, TAG_LOCA);
    ttf->glyf_offset = find_table(data, size, TAG_GLYF);
    ttf->cmap_offset = find_table(data, size, TAG_CMAP);
    ttf->kern_offset = find_table(data, size, TAG_KERN);
    
    if (!ttf->head_offset || !ttf->hhea_offset || !ttf->maxp_offset ||
        !ttf->loca_offset || !ttf->glyf_offset || !ttf->cmap_offset) {
        fk_free(ttf);
        return FK_ERROR_INVALID_FORMAT;
    }
    
    /* Parse head table */
    const uint8_t *head = (const uint8_t *)data + ttf->head_offset;
    ttf->units_per_em = read_u16(head + 18);
    ttf->index_to_loc_format = read_i16(head + 50);
    
    /* Parse hhea table */
    const uint8_t *hhea = (const uint8_t *)data + ttf->hhea_offset;
    ttf->ascent = read_i16(hhea + 4);
    ttf->descent = read_i16(hhea + 6);
    ttf->line_gap = read_i16(hhea + 8);
    ttf->num_h_metrics = read_u16(hhea + 34);
    
    /* Parse maxp table */
    const uint8_t *maxp = (const uint8_t *)data + ttf->maxp_offset;
    ttf->num_glyphs = read_u16(maxp + 4);
    
    /* Parse cmap table - find format 4 subtable */
    const uint8_t *cmap = (const uint8_t *)data + ttf->cmap_offset;
    uint16_t num_subtables = read_u16(cmap + 2);
    
    for (int i = 0; i < num_subtables; i++) {
        const uint8_t *subtable_entry = cmap + 4 + i * 8;
        uint16_t platform_id = read_u16(subtable_entry);
        uint16_t encoding_id = read_u16(subtable_entry + 2);
        uint32_t offset = read_u32(subtable_entry + 4);
        
        if ((platform_id == 3 && encoding_id == 1) ||  /* Windows Unicode BMP */
            (platform_id == 0 && encoding_id == 3)) {   /* Unicode 2.0 BMP */
            const uint8_t *subtable = cmap + offset;
            uint16_t format = read_u16(subtable);
            
            if (format == 4) {
                ttf->cmap_format = 4;
                ttf->cmap_offset_table = ttf->cmap_offset + offset;
                break;
            }
        }
    }
    
    if (ttf->cmap_format == 0) {
        fk_free(ttf);
        return FK_ERROR_INVALID_FORMAT;
    }
    
    loader->loader_data = ttf;
    return FK_OK;
}

static FK_Error ttf_get_glyph_outline(FK_Loader *loader, uint32_t codepoint,
                                      FK_Outline *outline) {
    TTF_Data *ttf = loader->loader_data;
    
    /* Get glyph index */
    int glyph_index = get_glyph_index(ttf, codepoint);
    if (glyph_index == 0 && codepoint != 0) {
        return FK_ERROR_INVALID_GLYPH;
    }
    
    /* Get glyph data offset */
    uint32_t glyph_offset = get_glyph_offset(ttf, glyph_index);
    uint32_t next_glyph_offset = get_glyph_offset(ttf, glyph_index + 1);
    
    if (glyph_offset == next_glyph_offset) {
        /* Empty glyph */
        outline->contour_count = 0;
        outline->contours = NULL;
        return FK_OK;
    }
    
    const uint8_t *glyph_data = ttf->font_data + ttf->glyf_offset + glyph_offset;
    
    /* Read glyph header */
    int16_t num_contours = read_i16(glyph_data);
    outline->xmin = read_i16(glyph_data + 2);
    outline->ymin = read_i16(glyph_data + 4);
    outline->xmax = read_i16(glyph_data + 6);
    outline->ymax = read_i16(glyph_data + 8);
    
    if (num_contours > 0) {
        /* Simple glyph */
        return load_simple_glyph(ttf, glyph_data, num_contours, outline);
    } else if (num_contours < 0) {
        /* Composite glyph - not implemented in this basic version */
        return FK_ERROR_UNSUPPORTED;
    }
    
    return FK_OK;
}

static FK_Error ttf_get_metrics(FK_Loader *loader, uint32_t codepoint,
                                FK_GlyphMetrics *metrics) {
    TTF_Data *ttf = loader->loader_data;
    
    int glyph_index = get_glyph_index(ttf, codepoint);
    if (glyph_index < 0 || glyph_index >= ttf->num_glyphs) {
        return FK_ERROR_INVALID_GLYPH;
    }
    
    /* Get horizontal metrics */
    const uint8_t *hmtx = ttf->font_data + ttf->hmtx_offset;
    
    int advance_width, left_side_bearing;
    
    if (glyph_index < ttf->num_h_metrics) {
        advance_width = read_u16(hmtx + glyph_index * 4);
        left_side_bearing = read_i16(hmtx + glyph_index * 4 + 2);
    } else {
        advance_width = read_u16(hmtx + (ttf->num_h_metrics - 1) * 4);
        left_side_bearing = read_i16(hmtx + ttf->num_h_metrics * 4 +
                                     (glyph_index - ttf->num_h_metrics) * 2);
    }
    
    /* Scale to pixel units (simplified - would need proper scaling) */
    metrics->advance = advance_width;
    metrics->bearing_x = left_side_bearing;
    metrics->bearing_y = ttf->ascent;
    metrics->width = advance_width;
    metrics->height = ttf->ascent - ttf->descent;
    
    return FK_OK;
}

static int ttf_get_kerning(FK_Loader *loader, uint32_t left, uint32_t right) {
    /* Kerning not implemented in this basic version */
    return 0;
}

static void ttf_destroy(FK_Loader *loader) {
    if (loader->loader_data) {
        fk_free(loader->loader_data);
    }
    fk_free(loader);
}

/* ============================================================================
 * Public Constructor
 * ========================================================================== */

FK_Loader* fk_ttf_loader_create(void) {
    FK_Loader *loader = fk_calloc(1, sizeof(FK_Loader));
    if (!loader) return NULL;
    
    loader->format = FK_FORMAT_TRUETYPE;
    loader->load = ttf_load;
    loader->get_glyph_outline = ttf_get_glyph_outline;
    loader->get_metrics = ttf_get_metrics;
    loader->get_kerning = ttf_get_kerning;
    loader->destroy = ttf_destroy;
    
    return loader;
}