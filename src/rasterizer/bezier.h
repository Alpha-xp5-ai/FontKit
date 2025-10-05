/* ============================================================================
 * src/rasterizer/bezier.h - Bezier Curve Utilities
 * ========================================================================== */
#ifndef FONTKIT_BEZIER_H
#define FONTKIT_BEZIER_H

#include "fontkit_types.h"


FK_Point fk_bezier_quadratic_eval(FK_Point p0, FK_Point p1, FK_Point p2, float t);
FK_Point fk_bezier_cubic_eval(FK_Point p0, FK_Point p1, FK_Point p2, FK_Point p3, float t);

void fk_bezier_flatten_quadratic(FK_Point p0, FK_Point p1, FK_Point p2,
                                  FK_Point *points, int *count, int max_points,
                                  float tolerance);

#endif /* FONTKIT_BEZIER_H */