//
//  curve.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/2/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef curve_hpp
#define curve_hpp

#include "stratadraw.h"

template <typename T>
void align_curve_bottom_left(std::vector<std::vector<T>>& curve, T& out_center, vec2& out_min, vec2& out_max)
{
    vec2 p_min = T(POSITIVE_INFINITY);
    vec2 p_max = T(NEGATIVE_INFINITY);
    for (usize i = 0; i < curve.size(); i += 1) {
        std::vector<T>& stroke = curve[i];
        
        for (usize j = 0; j < stroke.size(); j += 1) {
            p_min.x = m::min(p_min.x, stroke[j].x);
            p_min.y = m::min(p_min.y, stroke[j].y);
            p_max.x = m::max(p_max.x, stroke[j].x);
            p_max.y = m::max(p_max.y, stroke[j].y);
        }
    }
    
    vec3 p_center = vec3((p_min + p_max) / 2.0f, 0.0f);
    
    vec3 bottom_left = vec3(p_min.x, p_max.y, 0.0f);
    
    for (usize i = 0; i < curve.size(); i += 1) {
        std::vector<T>& stroke = curve[i];
        for (usize j = 0; j < stroke.size(); j += 1) {
            stroke[j] -= bottom_left;
        }
    }
    
    out_center = p_center;
    out_min    = p_min;
    out_max    = p_max;
}

template <typename T>
void align_curve_center(std::vector<std::vector<T>>& curve, T& out_center, vec2& out_min, vec2& out_max)
{
    vec2 p_min = T(POSITIVE_INFINITY);
    vec2 p_max = T(NEGATIVE_INFINITY);
    for (usize i = 0; i < curve.size(); i += 1) {
        std::vector<T>& stroke = curve[i];
        
        for (usize j = 0; j < stroke.size(); j += 1) {
            p_min.x = m::min(p_min.x, stroke[j].x);
            p_min.y = m::min(p_min.y, stroke[j].y);
            p_max.x = m::max(p_max.x, stroke[j].x);
            p_max.y = m::max(p_max.y, stroke[j].y);
        }
    }
    
    vec3 p_center = vec3((p_min + p_max) / 2.0f, 0.0f);
    
    for (usize i = 0; i < curve.size(); i += 1) {
        std::vector<T>& stroke = curve[i];
        for (usize j = 0; j < stroke.size(); j += 1) {
            stroke[j] -= p_center;
        }
    }
    
    out_center = p_center;
    out_min    = p_min;
    out_max    = p_max;
}

struct Lagrange_Polynomial_Term {
    float64 numerator;
    float64 denominator;
};
template <typename T>
struct Lagrange_Polynomial {
    std::vector<T> points;
    std::vector<Lagrange_Polynomial_Term> terms;
};

using Lagrange_Polynomial_2D = Lagrange_Polynomial<vec2>;
using Lagrange_Polynomial_3D = Lagrange_Polynomial<vec3>;

template <typename T>
Lagrange_Polynomial<T> Lagrange_Polynomial_make(const std::vector<T>* points)
{
    Lagrange_Polynomial<T> poly;
    poly.points = *points;
    poly.terms.resize(points->size());
    
    // for each term
    for (usize i = 0; i < points->size(); i += 1) {
        Lagrange_Polynomial_Term* term = &poly.terms[i];
        term->numerator = poly.points[i].y;
        term->denominator = 1.0;
        
        const float64 term_x = (*points)[i].x;
        for (usize j = 0; j < i; j += 1) {
            term->denominator *= (term_x) - poly.points[j].x;
        }
        for (usize j = i + 1; j < points->size(); j += 1) {
            term->denominator *= (term_x) - poly.points[j].x;
        }

    }
    
    return poly;
}



template <typename T>
float64 Lagrange_Polynomial_evaluate(Lagrange_Polynomial<T>* poly, float64 t)
{
    float64 eval = 0.0;
    
    for (usize i = 0; i < poly->points.size(); i += 1) {
        float64 term_eval = 1.0;
        
        for (usize j = 0; j < i; j += 1) {
            term_eval *= (t - poly->points[j].x);
        }
        for (usize j = i + 1; j < poly->points.size(); j += 1) {
            term_eval *= (t - poly->points[j].x);
        }
        
        eval += term_eval * ((poly->terms[i].numerator) / poly->terms[i].denominator);
    }
    
    
    return eval;
}

template <typename T>
struct Polynomial {
    std::vector<T> points;
    std::vector<float64> terms;
};

template <typename T>
void Lagrange_to_quadratic(Lagrange_Polynomial<T>* l_poly, Polynomial<T>* quadratic)
{
    quadratic->points = l_poly->points;
    quadratic->terms = {
        0.0,
        0.0,
        0.0
    };
    
    float32 x0 = quadratic->points[0].x;
    float32 x0_2 = x0*x0;
    float32 x1 = quadratic->points[1].x;
    float32 x1_2 = x1*x1;
    float32 x2 = quadratic->points[2].x;
    float32 x2_2 = x2*x2;
    
    mat3 mat_A = {
        x0_2, x0, 1,
        x1_2, x1, 1,
        x2_2, x2, 1
    };
    
    const mat3 Ainv = m::inverse_matrix3x3_last_column_ones(mat_A);
    
    // isolate A, B, C
    const vec3 yvals = {
        quadratic->points[0].y,
        quadratic->points[1].y,
        quadratic->points[2].y
    };
    

    float32 coeff_A = (Ainv[0][0] * yvals[0]) + (Ainv[0][1] * yvals[1]) + (Ainv[0][2] * yvals[2]);
    float32 coeff_B = (Ainv[1][0] * yvals[0]) + (Ainv[1][1] * yvals[1]) + (Ainv[1][2] * yvals[2]);
    float32 coeff_C = (Ainv[2][0] * yvals[0]) + (Ainv[2][1] * yvals[1]) + (Ainv[2][2] * yvals[2]);
    
    quadratic->terms[0] = coeff_A;
    quadratic->terms[1] = coeff_B;
    quadratic->terms[2] = coeff_C;
}

template <typename T>
float64 Polynomial_quadratic_evaluate(Polynomial<T>* poly, const float64 t)
{
    const float64 A = poly->terms[0];
    const float64 B = poly->terms[1];
    const float64 C = poly->terms[2];
    
    const float64 x  = t;
    
    const float64 x2 = x * x;
    
    return (A * x2) + (B * x) + C;
}

//template <typename T>
//struct CatmullRom_Segment {
//    T a;
//    T b;
//    T c;
//    T d;
//};
//
//template <typename T>
//struct CatmullRom_Curve {
//    std::vector<CatmullRom_Segment<T>> segments;
//};
//
//template <typename T>
//void CatmullRom_Curve_make(std::vector<T>* points, CatmullRom_Curve<T>* curve, float32 alpha, float32 tension)
//{
//
//    isize i = 0;
//    for (; i < points->size() - 3; i += 3) {
//        T p0 = (*points)[i];
//        T p1 = (*points)[i+1];
//        T p2 = (*points)[i+2];
//        T p3 = (*points)[i+3];
//
//        float t01 = m::pow(m::distance(p0, p1), alpha);
//        float t12 = m::pow(m::distance(p1, p2), alpha);
//        float t23 = m::pow(m::distance(p2, p3), alpha);
//
//        T m1 = (1.0f - tension) *
//            (p2 - p1 + t12 * ((p1 - p0) / t01 - (p2 - p0) / (t01 + t12)));
//        T m2 = (1.0f - tension) *
//            (p2 - p1 + t12 * ((p3 - p2) / t23 - (p3 - p1) / (t12 + t23)));
//
//        CatmullRom_Segment<T> segment;
//        segment.a = 2.0f * (p1 - p2) + m1 + m2;
//        segment.b = -3.0f * (p1 - p2) - m1 - m1 - m2;
//        segment.c = m1;
//        segment.d = p1;
//
//        curve->segments.push_back(segment);
//    }
//    if (i - points->size() == 1) {
//        T p0 = (*points)[i];
//        T p1 = (*points)[i] + 0.001f;
//        T p2 = (*points)[i] + 0.002f;
//        T p3 = (*points)[i] + 0.003f;
//
//        float t01 = m::pow(m::distance(p0, p1), alpha);
//        float t12 = m::pow(m::distance(p1, p2), alpha);
//        float t23 = m::pow(m::distance(p2, p3), alpha);
//
//        T m1 = (1.0f - tension) *
//            (p2 - p1 + t12 * ((p1 - p0) / t01 - (p2 - p0) / (t01 + t12)));
//        T m2 = (1.0f - tension) *
//            (p2 - p1 + t12 * ((p3 - p2) / t23 - (p3 - p1) / (t12 + t23)));
//
//        CatmullRom_Segment<T> segment;
//        segment.a = 2.0f * (p1 - p2) + m1 + m2;
//        segment.b = -3.0f * (p1 - p2) - m1 - m1 - m2;
//        segment.c = m1;
//        segment.d = p1;
//
//        curve->segments.push_back(segment);
//    } else if (i - points->size() == 2) {
//        T p0 = (*points)[i];
//        T p1 = ((*points)[i] + (*points)[i + 1]) / 3.0f;
//        T p2 = (2.0f * ((*points)[i] + (*points)[i + 1])) / 3.0f;
//        T p3 = (*points)[i+1];
//
//        float t01 = m::pow(m::distance(p0, p1), alpha);
//        float t12 = m::pow(m::distance(p1, p2), alpha);
//        float t23 = m::pow(m::distance(p2, p3), alpha);
//
//        T m1 = (1.0f - tension) *
//            (p2 - p1 + t12 * ((p1 - p0) / t01 - (p2 - p0) / (t01 + t12)));
//        T m2 = (1.0f - tension) *
//            (p2 - p1 + t12 * ((p3 - p2) / t23 - (p3 - p1) / (t12 + t23)));
//
//        CatmullRom_Segment<T> segment;
//        segment.a = 2.0f * (p1 - p2) + m1 + m2;
//        segment.b = -3.0f * (p1 - p2) - m1 - m1 - m2;
//        segment.c = m1;
//        segment.d = p1;
//
//        curve->segments.push_back(segment);
//    } else if (i - points->size() == 3) {
//        T p0 = (*points)[i];
//        T p1 = (*points)[i+1];
//        T p2 = ((*points)[i+1] + (*points)[i+2]) / 2.0f;
//        T p3 = (*points)[i+2];
//
//        float t01 = m::pow(m::distance(p0, p1), alpha);
//        float t12 = m::pow(m::distance(p1, p2), alpha);
//        float t23 = m::pow(m::distance(p2, p3), alpha);
//
//        T m1 = (1.0f - tension) *
//            (p2 - p1 + t12 * ((p1 - p0) / t01 - (p2 - p0) / (t01 + t12)));
//        T m2 = (1.0f - tension) *
//            (p2 - p1 + t12 * ((p3 - p2) / t23 - (p3 - p1) / (t12 + t23)));
//
//        CatmullRom_Segment<T> segment;
//        segment.a = 2.0f * (p1 - p2) + m1 + m2;
//        segment.b = -3.0f * (p1 - p2) - m1 - m1 - m2;
//        segment.c = m1;
//        segment.d = p1;
//
//        curve->segments.push_back(segment);
//    }
//}
//
//template <typename T>
//T CatmullRom_Curve_evaluate(CatmullRom_Curve<T>* curve, float32 t) {
//
//    uint64 which = (uint64)(t * (curve->segments.size() - 1));
//    if (which > curve->segments.size()) {
//        which = curve->segments.size() - 1;
//    }
//
//    float64 part = 1.0 / curve->segments.size();
//    float64 section = which * part;
//    float32 t_first = t;
//    t = m::min(1.0, (t - section)/part);
//
//    CatmullRom_Segment<T>& segment = curve->segments[which];
//
//    T point = segment.a * t * t * t +
//    segment.b * t * t +
//    segment.c * t +
//    segment.d;
//
//    MTT_print("t_global=[%f]t=[%f],which=[%llu][%f,%f]\n", t_first, t, which, point[0],point[1]);
//
//    return point;
//}
//


// https://github.com/JPBotelho/Catmull-Rom-Splines
struct CatmullRomPoint {
    Vector3 position;
    Vector3 tangent;
    Vector3 normal;

    CatmullRomPoint() {}
    
    CatmullRomPoint(Vector3 position, Vector3 tangent, Vector3 normal) :
    position(position), tangent(tangent), normal(normal)
    {
    }
};

struct CatmullRom {
     int resolution; //Amount of points between control points. [Tesselation factor]
     bool closedLoop;

     std::vector<CatmullRomPoint> splinePoints; //Generated spline points

     std::vector<Vector3> controlPoints;

     //Returns spline points. Count is contorolPoints * resolution + [resolution] points if closed loop.
     std::vector<CatmullRomPoint>& GetPoints()
     {
         return splinePoints;
     }
    
     
     void init(std::vector<Vector3>& controlPoints, int resolution, bool closedLoop)
     {
         if (controlPoints.size() < 2) {
             return;
         }


         if (controlPoints.size() == 2)
         {
             this->controlPoints.reserve(4);
             this->controlPoints.push_back(controlPoints[0]);
             this->controlPoints.push_back(lerp(controlPoints[0], controlPoints[1], 0.25f));
             this->controlPoints.push_back(lerp(controlPoints[0], controlPoints[1], 0.75f));
             this->controlPoints.push_back(controlPoints[1]);
         } else {
             this->controlPoints = controlPoints;
         }


         this->resolution = (resolution < 2) ? 2 : resolution;
         this->closedLoop = closedLoop;

         GenerateSplinePoints();
     }

     //Updates control points
     void Update(std::vector<Vector3>& controlPoints)
     {
         if(controlPoints.size() <= 0)
         {
             MTT_error("%s\n", "Invalid control points\n");
             return;
         }

         GenerateSplinePoints();
     }

     //Updates resolution and closed loop values
     void Update(int resolution, bool closedLoop)
     {
         if(resolution < 2)
         {
             MTT_error("%s", "Invalid Resolution. Make sure it's >= 2\n");
             return;
         }
         this->resolution = resolution;
         this->closedLoop = closedLoop;

         GenerateSplinePoints();
     }

     //Draws a line between every point and the next.
     void DrawSpline(sd::Renderer* renderer)
     {
         if (splinePoints.size() == 0) {
             return;
         }
         
         for(int i = 0; i < splinePoints.size(); i++)
         {
             sd::path_vertex_v3(renderer, splinePoints[i].position);
         }
         if (closedLoop) {
             sd::path_vertex_v3(renderer, splinePoints[0].position);
         }
     }
    
    void DrawSplineWithArrow(sd::Renderer* renderer, float32 size, float32 scale)
    {
        if (splinePoints.size() == 0) {
            return;
        }
        
        for(int i = 0; i < splinePoints.size(); i++)
        {
            sd::path_vertex_v3(renderer, splinePoints[i].position);
        }
        if (closedLoop) {
            sd::path_vertex_v3(renderer, splinePoints[0].position);
            
            if (splinePoints.size() >= 2) {
                sd::break_path(renderer);
                {
                    sd::path_arrow_head(renderer, splinePoints[splinePoints.size() - 2].position, splinePoints[splinePoints.size() - 1].position, m::max(12.0f, size * scale));
                }
            }
        } else if (splinePoints.size() >= 2) {
            sd::break_path(renderer);
            {
                vec3 last = splinePoints[splinePoints.size() - 1].position;
                vec3 prev = splinePoints[splinePoints.size() - 2].position;
                float32 dist = m::dist_squared(last, prev);
                float32 scale_multiplier = m::max(1.0f, scale);
                for (isize i = splinePoints.size() - 3; i >= 0 && (dist < (scale_multiplier * scale_multiplier * 16.0f * 16.0f)); i -= 1) {
                    prev = splinePoints[i].position;
                    dist = m::dist_squared(last, prev);
                }
                sd::path_arrow_head(renderer, prev, last, m::max(12.0f, size * scale));
            }
        }
    }

     //Sets the length of the point array based on resolution/closed loop.
     void InitializeProperties()
     {
         usize pointsToCreate;
         if (closedLoop)
         {
             pointsToCreate = resolution * controlPoints.size(); //Loops back to the beggining, so no need to adjust for arrays starting at 0
         }
         else
         {
             pointsToCreate = resolution * (controlPoints.size() - 1);
         }
         
         splinePoints = std::vector<CatmullRomPoint>(pointsToCreate);
     }

     //Math stuff to generate the spline points
     void GenerateSplinePoints()
     {
         InitializeProperties();

         Vector3 p0, p1; //Start point, end point
         Vector3 m0, m1; //Tangents

         // First for loop goes through each individual control point and connects it to the next, so 0-1, 1-2, 2-3 and so on
         int closedAdjustment = closedLoop ? 0 : 1;
         for (int currentPoint = 0; currentPoint < controlPoints.size() - closedAdjustment; currentPoint++)
         {
             bool closedLoopFinalPoint = (closedLoop && currentPoint == controlPoints.size() - 1);

             p0 = controlPoints[currentPoint];
             
             if(closedLoopFinalPoint)
             {
                 p1 = controlPoints[0];
             }
             else
             {
                 p1 = controlPoints[currentPoint + 1];
             }

             // m0
             if (currentPoint == 0) // Tangent M[k] = (P[k+1] - P[k-1]) / 2
             {
                 if(closedLoop)
                 {
                     m0 = p1 - controlPoints[controlPoints.size() - 1];
                 }
                 else
                 {
                     m0 = p1 - p0;
                 }
             }
             else
             {
                 m0 = p1 - controlPoints[currentPoint - 1];
             }

             // m1
             if (closedLoop)
             {
                 if (currentPoint == controlPoints.size() - 1) //Last point case
                 {
                     m1 = controlPoints[(currentPoint + 2) % controlPoints.size()] - p0;
                 }
                 else if (currentPoint == 0) //First point case
                 {
                     m1 = controlPoints[currentPoint + 2] - p0;
                 }
                 else
                 {
                     m1 = controlPoints[(currentPoint + 2) % controlPoints.size()] - p0;
                 }
             }
             else
             {
                 if (currentPoint < controlPoints.size() - 2)
                 {
                     m1 = controlPoints[(currentPoint + 2) % controlPoints.size()] - p0;
                 }
                 else
                 {
                     m1 = p1 - p0;
                 }
             }

             m0 *= 0.5f; //Doing this here instead of  in every single above statement
             m1 *= 0.5f;

             float pointStep = 1.0f / resolution;

             if ((currentPoint == controlPoints.size() - 2 && !closedLoop) || closedLoopFinalPoint) //Final point
             {
                 pointStep = 1.0f / (resolution - 1);  // last point of last segment should reach p1
             }

             // Creates [resolution] points between this control point and the next
             for (int tesselatedPoint = 0; tesselatedPoint < resolution; tesselatedPoint++)
             {
                 float t = tesselatedPoint * pointStep;

                 CatmullRomPoint point = Evaluate(p0, p1, m0, m1, t);

                 splinePoints[currentPoint * resolution + tesselatedPoint] = point;
             }
         }
     }

     //Evaluates curve at t[0, 1]. Returns point/normal/tan struct. [0, 1] means clamped between 0 and 1.
     static CatmullRomPoint Evaluate(Vector3 start, Vector3 end, Vector3 tanPoint1, Vector3 tanPoint2, float t)
     {
         Vector3 position = CalculatePosition(start, end, tanPoint1, tanPoint2, t);
         Vector3 tangent = CalculateTangent(start, end, tanPoint1, tanPoint2, t);
         Vector3 normal = NormalFromTangent(tangent);

         return CatmullRomPoint(position, tangent, normal);
     }

     //Calculates curve position at t[0, 1]
     static Vector3 CalculatePosition(Vector3 start, Vector3 end, Vector3 tanPoint1, Vector3 tanPoint2, float t)
     {
         // Hermite curve formula:
         // (2t^3 - 3t^2 + 1) * p0 + (t^3 - 2t^2 + t) * m0 + (-2t^3 + 3t^2) * p1 + (t^3 - t^2) * m1
         Vector3 position = (2.0f * t * t * t - 3.0f * t * t + 1.0f) * start
             + (t * t * t - 2.0f * t * t + t) * tanPoint1
             + (-2.0f * t * t * t + 3.0f * t * t) * end
             + (t * t * t - t * t) * tanPoint2;

         return position;
     }

     //Calculates tangent at t[0, 1]
     static Vector3 CalculateTangent(Vector3 start, Vector3 end, Vector3 tanPoint1, Vector3 tanPoint2, float t)
     {
         // Calculate tangents
         // p'(t) = (6t² - 6t)p0 + (3t² - 4t + 1)m0 + (-6t² + 6t)p1 + (3t² - 2t)m1
         Vector3 tangent = (6 * t * t - 6 * t) * start
             + (3 * t * t - 4 * t + 1) * tanPoint1
             + (-6 * t * t + 6 * t) * end
             + (3 * t * t - 2 * t) * tanPoint2;

         return m::normalize(tangent);
     }
     
     //Calculates normal vector from tangent
     static Vector3 NormalFromTangent(Vector3 tangent)
     {
         return m::normalize(m::cross(tangent, {0.0f, 1.0f, 0.0f})) / 2.0f;
     }
};

#endif /* curve_hpp */
