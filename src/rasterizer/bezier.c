/**
 * bezier.c - Bezier curve utilities
 */

#include "bezier.h"
#include <math.h>

FK_Point fk_bezier_quadratic_eval(FK_Point p0, FK_Point p1, FK_Point p2, float t) {
    float t2 = t * t;
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    
    FK_Point result;
    result.x = mt2 * p0.x + 2.0f * mt * t * p1.x + t2 * p2.x;
    result.y = mt2 * p0.y + 2.0f * mt * t * p1.y + t2 * p2.y;
    result.on_curve = 1;
    
    return result;
}

FK_Point fk_bezier_cubic_eval(FK_Point p0, FK_Point p1, FK_Point p2, FK_Point p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;
    
    FK_Point result;
    result.x = mt3 * p0.x + 3.0f * mt2 * t * p1.x + 
               3.0f * mt * t2 * p2.x + t3 * p3.x;
    result.y = mt3 * p0.y + 3.0f * mt2 * t * p1.y + 
               3.0f * mt * t2 * p2.y + t3 * p3.y;
    result.on_curve = 1;
    
    return result;
}

void fk_bezier_flatten_quadratic(FK_Point p0, FK_Point p1, FK_Point p2,
                                  FK_Point *points, int *count, int max_points,
                                  float tolerance) {
    /* Simple recursive subdivision */
    float dx = p2.x - p0.x;
    float dy = p2.y - p0.y;
    float d = fabsf((p1.x - p2.x) * dy - (p1.y - p2.y) * dx);
    
    if (d * d < tolerance * (dx * dx + dy * dy) || *count >= max_points - 1) {
        /* Flat enough - add endpoint */
        if (*count < max_points) {
            points[(*count)++] = p2;
        }
        return;
    }
    
    /* Subdivide */
    FK_Point p01 = {(p0.x + p1.x) * 0.5f, (p0.y + p1.y) * 0.5f, 1};
    FK_Point p12 = {(p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f, 1};
    FK_Point p012 = {(p01.x + p12.x) * 0.5f, (p01.y + p12.y) * 0.5f, 1};
    
    fk_bezier_flatten_quadratic(p0, p01, p012, points, count, max_points, tolerance);
    fk_bezier_flatten_quadratic(p012, p12, p2, points, count, max_points, tolerance);
}