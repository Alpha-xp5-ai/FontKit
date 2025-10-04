/**
 * rasterizer.c - Vector to bitmap rasterizer
 * Implements scanline rasterization with antialiasing
 */

#include "rasterizer.h"
#include "bezier.h"
#include "fontkit_types.h"
#include "memory.h"
#include <math.h>
#include <string.h>

#define MAX_SCANLINES 2048
#define FIXED_SHIFT 16
#define FIXED_ONE (1 << FIXED_SHIFT)

/* Fixed-point edge for scanline algorithm */
typedef struct {
    int x;           /* Current X position (fixed-point) */
    int dx;          /* X step per scanline (fixed-point) */
    int y_start;     /* Start Y */
    int y_end;       /* End Y */
    int winding;     /* Winding direction */
} Edge;

/* Rasterizer state */
struct FK_Rasterizer {
    int font_size;
    int dpi;
    float scale;
    
    /* Edge buffer */
    Edge *edges;
    int edge_count;
    int edge_capacity;
    
    /* Scanline buffer */
    float *scanline;
    int scanline_width;
};

/* ============================================================================
 * Edge Management
 * ========================================================================== */

static void add_edge(FK_Rasterizer *rast, float x0, float y0, float x1, float y1) {
    if (y0 == y1) return;  /* Horizontal edge */
    
    /* Ensure y0 < y1 */
    if (y0 > y1) {
        float tx = x0, ty = y0;
        x0 = x1; y0 = y1;
        x1 = tx; y1 = ty;
    }
    
    /* Expand edge buffer if needed */
    if (rast->edge_count >= rast->edge_capacity) {
        rast->edge_capacity *= 2;
        rast->edges = fk_realloc(rast->edges, 
                                 rast->edge_capacity * sizeof(Edge));
    }
    
    Edge *e = &rast->edges[rast->edge_count++];
    e->y_start = (int)floorf(y0);
    e->y_end = (int)ceilf(y1);
    e->x = (int)(x0 * FIXED_ONE);
    e->dx = (int)(((x1 - x0) / (y1 - y0)) * FIXED_ONE);
    e->winding = (y1 > y0) ? 1 : -1;
}

static void add_line(FK_Rasterizer *rast, FK_Point p0, FK_Point p1) {
    add_edge(rast, p0.x, p0.y, p1.x, p1.y);
}

static void add_quadratic_bezier(FK_Rasterizer *rast, FK_Point p0, 
                                 FK_Point p1, FK_Point p2) {
    /* Flatten bezier curve into line segments */
    const int steps = 16;
    FK_Point prev = p0;
    
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        FK_Point curr = fk_bezier_quadratic_eval(p0, p1, p2, t);
        add_line(rast, prev, curr);
        prev = curr;
    }
}

/* ============================================================================
 * Scanline Rasterization
 * ========================================================================== */

static int edge_compare(const void *a, const void *b) {
    const Edge *ea = a;
    const Edge *eb = b;
    return ea->x - eb->x;
}

static void rasterize_scanline(FK_Rasterizer *rast, int y, 
                               Edge *active_edges, int num_active) {
    if (num_active == 0) return;
    
    /* Sort edges by X */
    qsort(active_edges, num_active, sizeof(Edge), edge_compare);
    
    /* Clear scanline */
    memset(rast->scanline, 0, rast->scanline_width * sizeof(float));
    
    /* Fill using non-zero winding rule */
    int winding = 0;
    int prev_x = 0;
    
    for (int i = 0; i < num_active; i++) {
        int x = active_edges[i].x >> FIXED_SHIFT;
        
        /* Fill from prev_x to x if inside */
        if (winding != 0 && x >= 0 && prev_x < rast->scanline_width) {
            int start = (prev_x < 0) ? 0 : prev_x;
            int end = (x >= rast->scanline_width) ? rast->scanline_width - 1 : x;
            
            for (int px = start; px <= end && px < rast->scanline_width; px++) {
                rast->scanline[px] += 1.0f;
            }
        }
        
        winding += active_edges[i].winding;
        prev_x = x;
    }
    
    /* Clamp coverage values */
    for (int x = 0; x < rast->scanline_width; x++) {
        if (rast->scanline[x] > 1.0f) rast->scanline[x] = 1.0f;
        if (rast->scanline[x] < 0.0f) rast->scanline[x] = 0.0f;
    }
}

/* ============================================================================
 * Outline Processing
 * ========================================================================== */

static void apply_bold_effect(FK_Outline *outline, float strength) {
    /* Expand outline by offsetting contours */
    for (int c = 0; c < outline->contour_count; c++) {
        FK_Contour *contour = &outline->contours[c];
        
        for (int i = 0; i < contour->point_count; i++) {
            /* Simple expansion - move points outward */
            contour->points[i].x += strength;
            contour->points[i].y += strength;
        }
    }
    
    outline->xmax += (int)strength;
    outline->ymax += (int)strength;
}

static void apply_italic_effect(FK_Outline *outline, float slant) {
    /* Apply shear transformation for italic effect */
    for (int c = 0; c < outline->contour_count; c++) {
        FK_Contour *contour = &outline->contours[c];
        
        for (int i = 0; i < contour->point_count; i++) {
            float y_offset = contour->points[i].y - outline->ymin;
            contour->points[i].x += y_offset * slant;
        }
    }
    
    outline->xmax += (int)(outline->ymax * slant);
}

static FK_Error process_outline(FK_Rasterizer *rast, FK_Outline *outline,
                                const FK_RenderOptions *opts) {
    /* Apply style effects */
    if (opts && opts->style) {
        if (opts->style & FK_STYLE_BOLD) {
            apply_bold_effect(outline, 1.5f);
        }
        if (opts->style & FK_STYLE_ITALIC) {
            apply_italic_effect(outline, 0.3f);  /* 0.3 = 16.7 degree slant */
        }
    }
    
    /* Clear edges */
    rast->edge_count = 0;
    
    /* Process each contour */
    for (int c = 0; c < outline->contour_count; c++) {
        const FK_Contour *contour = &outline->contours[c];
        if (contour->point_count < 2) continue;
        
        /* Process points in contour */
        for (int i = 0; i < contour->point_count; i++) {
            int next = (i + 1) % contour->point_count;
            
            FK_Point p0 = contour->points[i];
            FK_Point p1 = contour->points[next];
            
            if (p0.on_curve && p1.on_curve) {
                /* Simple line segment */
                add_line(rast, p0, p1);
            } else if (p0.on_curve && !p1.on_curve) {
                /* Quadratic bezier curve */
                int next2 = (i + 2) % contour->point_count;
                FK_Point p2 = contour->points[next2];
                
                if (!p2.on_curve) {
                    /* Implied on-curve point between two off-curve points */
                    FK_Point implied = {
                        (p1.x + p2.x) / 2.0f,
                        (p1.y + p2.y) / 2.0f,
                        1
                    };
                    add_quadratic_bezier(rast, p0, p1, implied);
                    i++;  /* Skip next point */
                } else {
                    add_quadratic_bezier(rast, p0, p1, p2);
                    i++;  /* Skip next point */
                }
            }
        }
    }
    
    return FK_OK;
}

/* ============================================================================
 * Main Rendering
 * ========================================================================== */

FK_Error fk_rasterizer_render(FK_Rasterizer *rast, const FK_Outline *outline,
                               FK_Bitmap *bitmap, const FK_RenderOptions *opts) {
    if (!rast || !outline || !bitmap) {
        return FK_ERROR_INVALID_PARAMETER;
    }
    
    /* Make a copy of outline for style modifications */
    FK_Outline glyph_outline = *outline;
    glyph_outline.contours = NULL;
    
    /* Deep copy contours */
    if (outline->contour_count > 0) {
        glyph_outline.contours = fk_malloc(outline->contour_count * sizeof(FK_Contour));
        if (!glyph_outline.contours) {
            return FK_ERROR_OUT_OF_MEMORY;
        }
        
        for (int c = 0; c < outline->contour_count; c++) {
            glyph_outline.contours[c] = outline->contours[c];
            glyph_outline.contours[c].points = fk_malloc(
                outline->contours[c].point_count * sizeof(FK_Point));
            
            if (!glyph_outline.contours[c].points) {
                /* Cleanup on error */
                for (int i = 0; i < c; i++) {
                    fk_free(glyph_outline.contours[i].points);
                }
                fk_free(glyph_outline.contours);
                return FK_ERROR_OUT_OF_MEMORY;
            }
            
            memcpy(glyph_outline.contours[c].points, outline->contours[c].points,
                   outline->contours[c].point_count * sizeof(FK_Point));
        }
    }
    
    /* Calculate dimensions */
    int width = (glyph_outline.xmax - glyph_outline.xmin + 63) / 64;
    int height = (glyph_outline.ymax - glyph_outline.ymin + 63) / 64;
    
    if (width <= 0 || height <= 0) {
        /* Cleanup copied outline */
        if (glyph_outline.contours) {
            for (int c = 0; c < glyph_outline.contour_count; c++) {
                fk_free(glyph_outline.contours[c].points);
            }
            fk_free(glyph_outline.contours);
        }
        
        bitmap->pixels = NULL;
        bitmap->width = 0;
        bitmap->height = 0;
        bitmap->pitch = 0;
        return FK_OK;
    }
    
    /* Apply quality scaling */
    int scale = (opts && opts->quality > 0) ? opts->quality : FK_QUALITY_NORMAL;
    int render_width = width * scale;
    int render_height = height * scale;
    
    /* Allocate scanline buffer */
    if (rast->scanline_width < render_width) {
        rast->scanline = fk_realloc(rast->scanline, render_width * sizeof(float));
        rast->scanline_width = render_width;
    }
    
    /* Allocate bitmap */
    bitmap->width = width;
    bitmap->height = height;
    bitmap->pitch = width;
    bitmap->pixels = fk_calloc(width * height, 1);
    
    if (!bitmap->pixels) {
        return FK_ERROR_OUT_OF_MEMORY;
    }
    
    /* Process outline to edges */
    FK_Error err = process_outline(rast, &glyph->outline, opts);
    if (err != FK_OK) {
        fk_free(bitmap->pixels);
        return err;
    }
    
    /* Allocate temp buffer for supersampled rendering */
    uint8_t *temp_buffer = fk_calloc(render_width * render_height, 1);
    if (!temp_buffer) {
        fk_free(bitmap->pixels);
        return FK_ERROR_OUT_OF_MEMORY;
    }
    
    /* Rasterize each scanline */
    Edge *active_edges = fk_malloc(rast->edge_count * sizeof(Edge));
    
    for (int y = 0; y < render_height; y++) {
        int num_active = 0;
        
        /* Collect active edges for this scanline */
        for (int i = 0; i < rast->edge_count; i++) {
            Edge *e = &rast->edges[i];
            if (y >= e->y_start && y < e->y_end) {
                active_edges[num_active] = *e;
                active_edges[num_active].x += (y - e->y_start) * e->dx;
                num_active++;
            }
        }
        
        /* Rasterize this scanline */
        rasterize_scanline(rast, y, active_edges, num_active);
        
        /* Copy scanline to temp buffer */
        for (int x = 0; x < render_width; x++) {
            float coverage = rast->scanline[x];
            temp_buffer[y * render_width + x] = (uint8_t)(coverage * 255.0f);
        }
    }
    
    /* Downsample to final resolution */
    int samples = scale * scale;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int sum = 0;
            for (int sy = 0; sy < scale; sy++) {
                for (int sx = 0; sx < scale; sx++) {
                    int sample_x = x * scale + sx;
                    int sample_y = y * scale + sy;
                    sum += temp_buffer[sample_y * render_width + sample_x];
                }
            }
            bitmap->pixels[y * width + x] = sum / samples;
        }
    }
    
    /* Apply gamma correction if requested */
    if (opts && opts->gamma != 1.0f) {
        float inv_gamma = 1.0f / opts->gamma;
        for (int i = 0; i < width * height; i++) {
            float normalized = bitmap->pixels[i] / 255.0f;
            normalized = powf(normalized, inv_gamma);
            bitmap->pixels[i] = (uint8_t)(normalized * 255.0f);
        }
    }
    
    fk_free(active_edges);
    fk_free(temp_buffer);
    
    /* Cleanup copied outline */
    if (glyph_outline.contours) {
        for (int c = 0; c < glyph_outline.contour_count; c++) {
            fk_free(glyph_outline.contours[c].points);
        }
        fk_free(glyph_outline.contours);
    }
    
    return FK_OK;
}

/* ============================================================================
 * Public API
 * ========================================================================== */

FK_Rasterizer* fk_rasterizer_create(int font_size, int dpi) {
    FK_Rasterizer *rast = fk_calloc(1, sizeof(FK_Rasterizer));
    if (!rast) return NULL;
    
    rast->font_size = font_size;
    rast->dpi = dpi;
    rast->scale = (font_size * dpi) / (72.0f * 64.0f);
    
    rast->edge_capacity = 1024;
    rast->edges = fk_malloc(rast->edge_capacity * sizeof(Edge));
    
    rast->scanline_width = 256;
    rast->scanline = fk_malloc(rast->scanline_width * sizeof(float));
    
    return rast;
}

void fk_rasterizer_destroy(FK_Rasterizer *rast) {
    if (!rast) return;
    
    if (rast->edges) fk_free(rast->edges);
    if (rast->scanline) fk_free(rast->scanline);
    fk_free(rast);
}

void fk_rasterizer_set_dpi(FK_Rasterizer *rast, int dpi) {
    if (rast) {
        rast->dpi = dpi;
        rast->scale = (rast->font_size * dpi) / (72.0f * 64.0f);
    }
}