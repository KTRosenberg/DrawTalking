//
//  LineRenderer.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/23/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef LineRenderer_h
#define LineRenderer_h

#define USE_LINES_V1 (true)

namespace nnnv {

typedef double FLOAT;

//typedef struct Color { uint8_t r, g, b, a; } Color;
typedef struct Color { float r, g, b, a; } Color;

}

namespace sd {
struct Line_Renderer {
    enum Cap { CAP_SQUARE, CAP_PROJECT, CAP_ROUND };
    enum Join { JOIN_MITER, JOIN_BEVEL, JOIN_ROUND };
    Cap cap;
    Join join;
    
    NVGwinding winding = NVG_CCW;
    
    
    typedef float64 FLOAT;
    typedef NVGpoint_TEST Stroke_Point;
    
    std::vector<Stroke_Point> points = {};
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
    FLOAT r = 1;
    FLOAT depth;
    
    void (*triangle)(void* user_data, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT x3, FLOAT y3, ::nnnv::Color c1, ::nnnv::Color c2, ::nnnv::Color c3);
    void (*rectangle)(void* user_data, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT x3, FLOAT y3, FLOAT x4, FLOAT y4,  FLOAT x5, FLOAT y5, FLOAT x6, FLOAT y6, ::nnnv::Color fill);
    void (*circle)(void* user_data, FLOAT x1, FLOAT y1, FLOAT r, usize, ::nnnv::Color fill);
    
    void* user_data;
    
    void beginLine()
    {
        points.resize(0);
    }
    
    inline static bool comp_eq(FLOAT x0, FLOAT y0, FLOAT x1, FLOAT y1)
    {
        return ((x0 == x1) && (y0 == y1));
    }
    
    void lineVertex(FLOAT x, FLOAT y)
    {
        vec3 next_pt = vec3(x, y, this->depth);
        if (!points.empty()) {
            auto& el = points.back();
            if (comp_eq(el.x, el.y, next_pt.x, next_pt.y)) {
                return;
            }
        }
        
        Stroke_Point entry = {};
        entry.x = next_pt.x;
        entry.y = next_pt.y;
        
        entry.r = this->color.r;
        entry.g = this->color.g;
        entry.b = this->color.b;
        entry.a = this->color.a;
        entry.width = this->r;
        points.push_back(entry);
    }
    
    void endLine(bool close)
    {
        // TODO:
    }
};

}

namespace nnnv {

struct LineRenderer {
    enum Cap { CAP_SQUARE, CAP_PROJECT, CAP_ROUND };
    enum Join { JOIN_MITER, JOIN_BEVEL, JOIN_ROUND };
    Cap cap;
    Join join;
    double r; //line width (radius)
    double first_r;
    double r_prev;
    Color strokeColor;
    Color firstColor;
    Color secondColor;
    Color prevColor;
    
    FLOAT depth;
    
    //line drawing state
    int lineVertexCount;
    FLOAT fx, fy; //first vertex
    FLOAT sx, sy, sdx, sdy; //second vertex
    FLOAT px, py, pdx, pdy; //previous vertex
    FLOAT lx, ly; //last vertex
    
    FLOAT first_point_x, first_point_y;
    FLOAT last_point_x,  last_point_y;
    
    void* user_data;
    
    FLOAT z;
    
    void (*triangle)(void* user_data, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT x3, FLOAT y3, Color c1, Color c2, Color c3);
    void (*rectangle)(void* user_data, FLOAT x1, FLOAT y1, FLOAT x2, FLOAT y2, FLOAT x3, FLOAT y3, FLOAT x4, FLOAT y4,  FLOAT x5, FLOAT y5, FLOAT x6, FLOAT y6, Color fill);
    void (*circle)(void* user_data, FLOAT x1, FLOAT y1, FLOAT r, usize, Color fill);
    
    void arcJoin(FLOAT x, FLOAT y, FLOAT dx1, FLOAT dy1, FLOAT dx2, FLOAT dy2);
    
    void lineCap(FLOAT x, FLOAT y, FLOAT dx, FLOAT dy);
    
    void beginLine();
    
    void lineVertex(FLOAT x, FLOAT y);
    
    void endLine(bool close);
};

}

namespace sd {
using FLOAT = nnnv::FLOAT;
using Color = nnnv::Color;
}

#if USE_LINES_V1

namespace sd {
using LineRenderer = nnnv::LineRenderer;
}

#else

namespace sd {

using LineRenderer = Line_Renderer;

}

#endif

#endif /* LineRenderer_h */
