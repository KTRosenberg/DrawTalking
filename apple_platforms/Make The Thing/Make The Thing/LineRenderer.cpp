/*
IMPORTANT INFO:
This code is made to be renderer-agnostic.
You would complete the implementation by implementing the triangle() function for your particular renderer.
The code is not written to make use of an index buffer, so it is not perfectly efficient in terms of memory usage.
You can replace the vector / matrix math with types from your own math library if you want, or leave it as-is.
This implementation assumes coordinates are in screen pixel space (circleDetail() relies on that fact).
If you want to draw partially transparent lines, you will need render all triangles of a line at the same depth
    and use the less-than or greater-than or not-equal depth test (e.g. glDepthFunc(GL_LESS) or GL_GREATER or GL_NOTEQUAL)
    because some triangles overlap.

EXAMPLE USAGE:
    LineRenderer line;
    line.cap = ...;
    line.join = ...;
    line.r = ...; //in pixels
    line.strokeColor = ...;

    line.beginLine();
    line.lineVertex(...);
    line.lineVertex(...);
    line.lineVertex(...);
    //etc.
    line.endLine(...); //boolean argument = whether to connect the first and last point of the line together
*/

#include "LineRenderer.h"



namespace nnnv { // notnullnotvoid

static const FLOAT nnnv_PI = M_PI;
static const FLOAT HALF_PI = M_PI_2;
static const FLOAT QUARTER_PI = M_PI_4;

struct nnnv_Vec2 { FLOAT x, y; };
struct nnnv_Mat2 { FLOAT m00, m01, m10, m11; };

static inline nnnv_Vec2 nnnv_vec2(FLOAT f) { return { f, f }; }
static inline nnnv_Vec2 nnnv_vec2(FLOAT x, FLOAT y) { return { x, y }; }

static inline nnnv_Vec2 operator-(nnnv_Vec2 v) { return { -v.x, -v.y }; }
static inline nnnv_Vec2 operator-(nnnv_Vec2 l, nnnv_Vec2 r) { return { l.x - r.x, l.y - r.y }; }
static inline nnnv_Vec2 operator+(nnnv_Vec2 l, nnnv_Vec2 r) { return { l.x + r.x, l.y + r.y }; }
static inline nnnv_Vec2 operator*(FLOAT f, nnnv_Vec2  v) { return { f * v.x, f * v.y }; }
static inline nnnv_Vec2 operator*(nnnv_Vec2  v, FLOAT f) { return { f * v.x, f * v.y }; }
static inline nnnv_Vec2 operator*(nnnv_Vec2  l, nnnv_Vec2  r) { return { l.x * r.x, l.y * r.y }; }
static inline nnnv_Vec2 operator/(nnnv_Vec2  v, FLOAT f) { return { v.x / f, v.y / f }; }
static inline nnnv_Vec2 operator/(nnnv_Vec2  l, nnnv_Vec2  r) { return { l.x / r.x, l.y / r.y }; }

static inline nnnv_Vec2 nor(nnnv_Vec2 v) {
    FLOAT f = 1 / sqrt(v.x * v.x + v.y * v.y);
    return { v.x * f, v.y * f };
}

static inline FLOAT dot(nnnv_Vec2 l, nnnv_Vec2 r) {
    return l.x * r.x + l.y * r.y;
}

static inline FLOAT cross(nnnv_Vec2 l, nnnv_Vec2 r) {
    return l.x * r.y - l.y * r.x;
}

static inline nnnv_Vec2 operator*(nnnv_Mat2 m, nnnv_Vec2 v) {
    return { m.m00 * v.x + m.m01 * v.y,
             m.m10 * v.x + m.m11 * v.y, };
}

//returns roughly the number of triangles needed for a triangle fan of a given arc length
//        to look perfectly circular at a given size on screen
int circleDetail(FLOAT radius, FLOAT delta) {
    return fmin(11, sqrt(radius / 4)) / QUARTER_PI * abs(delta) + 1;
}

void LineRenderer::arcJoin(FLOAT x, FLOAT y, FLOAT dx1, FLOAT dy1, FLOAT dx2, FLOAT dy2) {
    nnnv_Vec2 a = nnnv_vec2(dx1, dy1), b = nnnv_vec2(dx2, dy2);
    FLOAT dot_product = dot(a, b);
    FLOAT cross_product = cross(a, b);
    FLOAT theta = atan2(cross_product, dot_product);
    int segments = circleDetail(r, theta);
    FLOAT px = x + dx1, py = y + dy1;
    if (segments > 1) {
        FLOAT c = cos(theta / segments);
        FLOAT s = sin(theta / segments);
        nnnv_Mat2 rot = { c, -s, s, c };
        for (int i = 1; i < segments; ++i) {
            a = rot * a;
            FLOAT nx = x + a.x;
            FLOAT ny = y + a.y;
            triangle(user_data, x, y, px, py, nx, ny, strokeColor, strokeColor, strokeColor);
            px = nx;
            py = ny;
        }
    }
    triangle(user_data, x, y, px, py, x + dx2, y + dy2, strokeColor, strokeColor, strokeColor);
}

void LineRenderer::lineCap(FLOAT x, FLOAT y, FLOAT dx, FLOAT dy) {
    int segments = circleDetail(r, HALF_PI);
    nnnv_Vec2 p = nnnv_vec2(dy, -dx);
    if (segments > 1) {
        FLOAT c = cos(HALF_PI / segments);
        FLOAT s = sin(HALF_PI / segments);
        nnnv_Mat2 rot = { c, -s, s, c };
        for (int i = 1; i < segments; ++i) {
            nnnv_Vec2 n = rot * p;
            triangle(user_data, x, y, x + p.x, y + p.y, x + n.x, y + n.y, strokeColor, strokeColor, strokeColor);
            triangle(user_data, x, y, x - p.y, y + p.x, x - n.y, y + n.x, strokeColor, strokeColor, strokeColor);
            p = n;
        }
    }
    triangle(user_data, x, y, x + p.x, y + p.y, x + dx, y + dy, strokeColor, strokeColor, strokeColor);
    triangle(user_data, x, y, x - p.y, y + p.x, x - dy, y + dx, strokeColor, strokeColor, strokeColor);
}

void LineRenderer::beginLine() {
    lineVertexCount = 0;
    r_prev = 0;
    prevColor = strokeColor;
    secondColor = strokeColor;
}

void LineRenderer::lineVertex(FLOAT x, FLOAT y) {
    //disallow adding consecutive duplicate totalVerts,
    //as it is pointless and just creates an extra edge case
    if (lineVertexCount > 0 && x == lx && y == ly) {
        prevColor = strokeColor;
        return;
    }

    if (lineVertexCount == 0) {
        fx = x;
        fy = y;
        firstColor = strokeColor;
        first_r = r;
//#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
        circle(user_data, fx, fy, r, 14, strokeColor);
//#endif
    } else if (lineVertexCount == 1) {
        sx = x;
        sy = y;
        secondColor = strokeColor;
        //first_r = r;
    } else {
        nnnv_Vec2 p = nnnv_vec2(px, py);
        nnnv_Vec2 l = nnnv_vec2(lx, ly);
        nnnv_Vec2 v = nnnv_vec2(x, y);

        FLOAT cosPiOver15 = 0.97815f;
        nnnv_Vec2 leg1 = nor(l - p);
        nnnv_Vec2 leg2 = nor(v - l);
        if (join == JOIN_BEVEL || join == JOIN_ROUND || dot(leg1, -leg2) > cosPiOver15 || dot(leg1, -leg2) < -0.999) {
            FLOAT tx =  leg1.y * r;
            FLOAT ty = -leg1.x * r;

            if (lineVertexCount == 2) {
                sdx = tx;
                sdy = ty;
            } else {
//                    MTT_print("[\n%f,%f, %f,%f, %f,%f : \n%f,%f, %f,%f, %f,%f\n]\n",
//                              px - pdx, py - pdy, px + pdx, py + pdy, lx - tx, ly - ty,
//                              px + pdx, py + pdy, lx - tx, ly - ty, lx + tx, ly + ty);
                triangle(user_data, px - pdx, py - pdy, px + pdx, py + pdy, lx - tx, ly - ty, prevColor, prevColor, strokeColor);
                triangle(user_data, px + pdx, py + pdy, lx - tx, ly - ty, lx + tx, ly + ty, prevColor, strokeColor, strokeColor);
                
                
//                    rectangle(user_data,
//                              px - pdx, py - pdy, px + pdx, py + pdy, lx - tx, ly - ty,
//                              px + pdx, py + pdy, lx - tx, ly - ty, lx + tx, ly + ty,
//                              strokeColor);
            }

            FLOAT nx =  leg2.y * r;
            FLOAT ny = -leg2.x * r;

            if (join == JOIN_ROUND) {
                if (cross(leg1, leg2) > 0) {
                    arcJoin(lx, ly, tx, ty, nx, ny);
                } else {
                    arcJoin(lx, ly, -tx, -ty, -nx, -ny);
                }
            } else if (cross(leg1, leg2) > 0) {
                triangle(user_data, lx, ly, lx + tx, ly + ty, lx + nx, ly + ny, strokeColor, strokeColor, strokeColor);
            } else {
                triangle(user_data, lx, ly, lx - tx, ly - ty, lx - nx, ly - ny, strokeColor, strokeColor, strokeColor);
            }

            pdx = nx;
            pdy = ny;
        } else {
            nnnv_Vec2 a = leg2 - leg1;
            nnnv_Vec2 b = nnnv_vec2(leg1.y, -leg1.x);
            nnnv_Vec2 c = a * (r / dot(a, b));

            FLOAT bx = c.x, by = c.y;

            if (lineVertexCount == 2) {
                sdx = bx;
                sdy = by;
            } else {
//                    triangle(user_data, px - pdx, py - pdy, px + pdx, py + pdy, lx - bx, ly - by, strokeColor, strokeColor, strokeColor);
//                    triangle(user_data, px + pdx, py + pdy, lx - bx, ly - by, lx + bx, ly + by, strokeColor, strokeColor, strokeColor);
                
                triangle(user_data, px - pdx, py - pdy, px + pdx, py + pdy, lx - bx, ly - by, prevColor, prevColor, strokeColor);
                triangle(user_data, px + pdx, py + pdy, lx - bx, ly - by, lx + bx, ly + by, prevColor, strokeColor, strokeColor);
                
//                    rectangle(user_data,
//                              px - pdx, py - pdy, px + pdx, py + pdy, lx - bx, ly - by,
//                              px + pdx, py + pdy, lx - bx, ly - by, lx + bx, ly + by,
//                              strokeColor);
            }

            pdx = bx;
            pdy = by;
        }
    }

    px = lx;
    py = ly;
    lx = x;
    ly = y;

    lineVertexCount += 1;
    
    r_prev = r;
    
    prevColor = strokeColor;
}

void LineRenderer::endLine(bool close) {
    if (lineVertexCount == 0) {
        return;
    } else if (lineVertexCount == 1) {
        //circle(user_data, fx, fy, r, 14, strokeColor);
        return;
    } else if (lineVertexCount == 2) {
        
        const FLOAT last_vertex_x = lx;
        const FLOAT last_vertex_y = ly;
        
        // handle "too few vertices" edge case by self-overlapping
        lineVertex(fx, fy);
        lineVertex(last_vertex_x, last_vertex_y);
    }

    if (close) {
        //draw the last two legs
        lineVertex(fx, fy);
        lineVertex(sx, sy);

        //connect first and second vertices
        triangle(user_data, px - pdx, py - pdy, px + pdx, py + pdy, sx - sdx, sy - sdy, strokeColor, strokeColor, firstColor);
        triangle(user_data, px + pdx, py + pdy, sx - sdx, sy - sdy, sx + sdx, sy + sdy, strokeColor, firstColor, firstColor);
        
    } else {
        //draw last line (with cap)
        FLOAT dx = lx - px;
        FLOAT dy = ly - py;
        FLOAT d = sqrt(dx*dx + dy*dy);
        FLOAT tx =  dy / d * r;
        FLOAT ty = -dx / d * r;

        if (cap == CAP_PROJECT) {
            lx -= ty;
            ly += tx;
        }

//            triangle(user_data, px - pdx, py - pdy, px + pdx, py + pdy, lx - tx, ly - ty, strokeColor, strokeColor, strokeColor);
//            triangle(user_data, px + pdx, py + pdy, lx - tx, ly - ty, lx + tx, ly + ty, strokeColor, strokeColor, strokeColor);
        
        triangle(user_data, px - pdx, py - pdy, px + pdx, py + pdy, lx - tx, ly - ty, prevColor, prevColor, strokeColor);
        triangle(user_data, px + pdx, py + pdy, lx - tx, ly - ty, lx + tx, ly + ty, prevColor, strokeColor, strokeColor);
        
//            rectangle(user_data,
//                      px - pdx, py - pdy, px + pdx, py + pdy, lx - tx, ly - ty,
//                      px + pdx, py + pdy, lx - tx, ly - ty, lx + tx, ly + ty,
//                      strokeColor);
        {
            if (cap == CAP_ROUND) {
                lineCap(lx, ly, -ty, tx);
            }
            
            //draw first line (with cap)
            Color tmp = strokeColor;
            strokeColor = firstColor;
            dx = fx - sx;
            dy = fy - sy;
            d = sqrt(dx*dx + dy*dy);
            tx =  dy / d * first_r;
            ty = -dx / d * first_r;
            
            if (cap == CAP_PROJECT) {
                fx -= ty;
                fy += tx;
            }
            
            triangle(user_data, sx - sdx, sy - sdy, sx + sdx, sy + sdy, fx + tx, fy + ty, secondColor, secondColor, strokeColor);
            triangle(user_data, sx + sdx, sy + sdy, fx + tx, fy + ty, fx - tx, fy - ty, secondColor, strokeColor, strokeColor);
            //r = first_r;
            if (cap == CAP_ROUND) {
                lineCap(fx, fy, -ty, tx);
            }
            
            strokeColor = tmp;
        }
    }
}



//draws a triangle, or adds one to whatever mesh we are building




}
