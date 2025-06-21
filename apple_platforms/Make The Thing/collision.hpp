//
//  collision.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/5/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef collision_hpp
#define collision_hpp

#include "sort.hpp"
#include "thing_shared_types.hpp"

#include "quadtree.hpp"


namespace mtt {

constexpr const usize MAX_PRIORITY = INT_MAX;

#define along_line(a, b, t) (a + (t * m::normalize(b - a)))

struct Box {
    vec2 tl;
    union {
        vec2 br;
        vec2 dimensions;
    };
};

struct Box_F64 {
    vec2_64 tl;
    union {
        vec2_64 br;
        vec2_64 dimensions;
    };
};



struct AABB {
    vec2 tl;
    vec2 br;
    vec2 half_extent;
    Box saved_box;
    Quad saved_quad;
    
    //
    //vec2         corner[4];
    
        /** Two edges of the box extended away from corner[0]. */
    vec2         axis[2];
    
        /** origin[a] = corner[0].dot(axis[a]); */
    float64          origin[2];
};

void Box_print(Box* box);
void Box_print(const Box* box);
void Box_print_dimensions(Box* box);
void Box_print_dimensions(const Box* box);


struct Line_Segment {
    vec2 a;
    vec2 b;
};

struct Line_Segment_F64 {
    vec2_64 a;
    vec2_64 b;
};

struct Circle {
    vec2    center;
    float32 radius;
    //float32 radians_angle;
    //float32 radians_missing;
};
struct Circle_F64 {
    vec2_64 center;
    float64 radius;
    //float32 radians_angle;
    //float32 radians_missing;
};

struct Point {
    vec2 coord;
};

struct Point_F64 {
    vec2_64 coord;
};

struct Concave_Hull {
    SmallVector<vec2, 8> points;
    AABB aabb;
    usize count;
    vec2* data(void) { return this->points.data(); }
    usize size(void) { return count; }
};

typedef enum COLLIDER_TYPE {
    COLLIDER_TYPE_AABB,
    COLLIDER_TYPE_LINE_SEGMENT,
    COLLIDER_TYPE_CIRCLE,
    COLLIDER_TYPE_POINT,
    COLLIDER_TYPE_CONVEX_HULL,
    COLLIDER_TYPE_CONCAVE_HULL,
    
    COLLIDER_TYPE_COUNT,
} COLLIDER_TYPE;

static const char* const collider_type_to_string[COLLIDER_TYPE_COUNT] = {
    "COLLIDER_TYPE_AABB",
    "COLLIDER_TYPE_LINE_SEGMENT",
    "COLLIDER_TYPE_CIRCLE",
    "COLLIDER_TYPE_POINT",
    "COLLIDER_TYPE_CONVEX_HULL",
    "COLLIDER_TYPE_CONCAVE_HULL",
};

struct Hit {
    vec2    pos;
    vec2    normal;
    vec2    delta;
    float32 time;
    float32 t;
};


struct Collision_System;
struct Collider;
struct Broad_Phase_Collision_Record;
typedef bool Collision_Handler_Result;

struct Collision_Record {
    flecs::entity a;
    flecs::entity b;
    Collider* ca = nullptr;
    Collider* cb  = nullptr;
    uint64 key = 0;
    usize event_stamp_begin = ULLONG_MAX;
};



typedef Collision_Handler_Result (*Collision_Handler)(Broad_Phase_Collision_Record* collision, Collision_System* system);


typedef enum COLLIDER_LABEL {
    COLLIDER_LABEL_NEAR,
    COLLIDER_LABEL_GLOBAL,
} COLLIDER_LABEL;

typedef uint64 Priority_Layer;

typedef int32 Collider_ID;

struct Convex_Hull {
    AABB aabb;
};

struct Collider {
    Collider_ID ID;
    COLLIDER_TYPE type;
    COLLIDER_LABEL label;
    uint64 priority;
    //uint64 mask;
    
    uint32 mask_bits;
    uint32 category_bits;
    bool negated;
    bool allow_collision_in_hierarchy;
    
    void* user_data;
    
    bool pushed = false;
    
    Collision_System* system;
    SmallVector<uintptr, 4> bin_keys;
    //uint64 collision_key;
    
    union {
        AABB aabb;
        Line_Segment line_segment;
        Circle circle;
        Point point;
    };
    Convex_Hull convex_hull;
    Concave_Hull concave_hull;
    
    
    Hit hit;
    //uint64 next_ID;
    Collision_Handler handler;
    
    vec3 center_anchor;
    vec3 pivot_anchor_offset;
    
    mat4 transform;
    
    //Collider* next;
    
    usize layer;
    Priority_Layer priority_layer;
    
    usize global_idx;
    
    bool is_inactive = false;
    

    
    //mtt::Map<mtt::String, mtt::Attribute> attributes;
};

using Collider_List = std::vector<mtt::Collider*>;

inline static bool priority_layer_is_equal(Priority_Layer a, Priority_Layer b)
{
    return (a == b);
}
inline static bool priority_layer_is_le(Priority_Layer a, Priority_Layer b)
{
    return (a <= b);
}
inline static isize priority_layer_compare(isize32 a, isize32 b)
{
    return ((isize)b) - ((isize)a);
}

void Collider_print(const Collider* collider);

struct Compare_Routine_compare_by_ID {
    bool operator()(const Collider* const a, const Collider* const b) {
        return (a->ID < b->ID);
    }
};

struct Compare_Routine_compare_by_priority {
    bool operator()(const Collider* const a, const Collider* const b) {
        return (a->priority < b->priority);
    }
};

struct Compare_Routine_compare_by_ID_and_priority {
    bool operator()(const Collider* const a, const Collider* const b) {
        return ((a->ID != b->ID) ? a->ID < b->ID : (a->priority < b->priority));
    }
};

struct Spatial_Grid_Cell {
    Collider_List colliders_AABB;
    Collider_List colliders_Line_Segment;
    Collider_List colliders_Point;
    Collider_List colliders_Circle;
    Collider_List colliders_Concave_Hull;
    Collider_List colliders;
    robin_hood::unordered_flat_map<Collider_ID, usize> collider_to_idx;
    uintptr key;
};



struct Broad_Phase_Collision_Record {
    Collider*              collider_primary;
    robin_hood::unordered_flat_set<Collider*> colliders_against;
};


struct Broad_Phase_Pair {
    Collider* a;
    Collider* b;
};
struct Broad_Phase {
    usize count;
    std::vector<Broad_Phase_Pair> list;
    
    Broad_Phase() : count(0) {}
    
    void clear()
    {
        this->count = 0;
    }
};


struct Spatial_Grid {
    robin_hood::unordered_flat_map<usize, Spatial_Grid_Cell> collision_map;
    
    usize grid_cell_dimensions;
    usize ID;
    robin_hood::unordered_flat_map<usize, usize> active_cells;
};


struct To_Be_Freed_ID_Record {
    usize id;
    usize step_count;
};

#define COLLISION_MAX_LAYERS (16)
struct Collision_System {
    Spatial_Grid layers[COLLISION_MAX_LAYERS];
    usize       max_layer_count;
    usize       layer_count = 1;
    //std::vector<Collider*> colliders;
    [[deprecated]]
    robin_hood::unordered_flat_map<usize, Broad_Phase_Collision_Record> found_collisions;
    
    
    mem::Pool_Allocation collider_pool;
    
    Collider_ID next_available_ID;
    std::vector<Collider_ID> free_ids;
    std::vector<Collider*> active_colliders;
    std::vector<To_Be_Freed_ID_Record> to_be_freed_ids;
    
    Quadtree spatial_index;
    
    mtt::World* world;
    void* user_data;
    
    Collision_Handler_Result (*collision_handler_AABB_default)(Broad_Phase_Collision_Record* collisions, Collision_System* system);
    Collision_Handler_Result (*collision_handler_Circle_default)(Broad_Phase_Collision_Record* collisions, Collision_System* system);
    
    mtt::Map<uint64, Collision_Record> not_anymore_collisions;
    mtt::Map<uint64, Collision_Record> curr_collisions;
    
    std::vector<Collision_Record> exit_collisions;
    
    mtt::String label;
};

struct Collision_System_Group_World_Canvas {
    Collision_System* world = nullptr;
    Collision_System* canvas = nullptr;
};

constexpr const usize Collider_ID_INVALID = 0;

usize collision_layer_active_count(Collision_System* system, usize layer);

typedef uint64 Collision_Layer_Mask;
typedef robin_hood::unordered_flat_set<Collider*> Collision_Set;

void Collider_remove(Collision_System* sys, usize layer, Collider* c);



void Collision_System_init(Collision_System*, usize layer_count, mem::Allocator allocator, mtt::World* world);
usize Collision_System_layer_make(Collision_System* sys, usize dimensions);
void Collision_System_set_layer_dimensions(Collision_System* sys, usize layer, usize dimensions);


Collider* Collider_make(Collision_System* sys, usize layer);
void Collider_copy_into(Collider* c_cpy, Collider* c_src, void* user_data);
Collider* Collider_copy(Collider* c_src, void* user_data);
Collider* Collider_copy(Collider* c_src, void* user_data, Collision_System* dst_collision_system);
void Collider_destroy(Collision_System* sys, Collider* c);
void Collision_System_end_step(Collision_System* sys);


Collider* Collider_make_aabb(Collision_System* sys);
Collider* Collider_make_circle(Collision_System* sys);

void calc_aaab_from_points(Collider* c, Array_Slice<vec3> points);
void calc_aabb_from_points(Collider* c, Array_Slice<vec2> points);

static inline float32 calc_aabb_area(AABB* aabb)
{
    vec2 dimensions = aabb->br - aabb->tl;
    return dimensions.x * dimensions.y;
}

bool aabb_query(Collision_System* sys, uint32 id, vec2 tl, vec2 br, robin_hood::unordered_flat_set<Collider*>* out);
bool point_query(Collision_System* sys, uint32 id, vec2 point, std::vector<Collider*>* out);
bool point_query_narrow(mtt::Collision_System* sys, vec2 pos, mtt::Collider** out_collider, Hit& out_hit, float32* min_area_ref, mtt::Set_Stable<mtt::Thing_ID>* selected_things);
bool point_query_narrow_including_selections(mtt::Collision_System* sys, vec2 pos, mtt::Collider** out_collider, Hit& hit, float32* min_area_ref);
bool point_query_narrow_with_exclusion(mtt::Collision_System* sys, vec2 pos, mtt::Collider** out_collider, Hit& hit, float32* min_area_ref, mtt::Thing_ID excluded);





bool line_segment_query(Collision_System* sys, uint32 id, vec4 segment, robin_hood::unordered_flat_set<Collider*>* out);
bool circle_query(Collision_System* sys, uint32 id, vec2 center, float32 radius, robin_hood::unordered_flat_set<Collider*>* out);
void broad_phase(Collision_System*, usize);
void register_collision(World* world, Thing* a, Thing* b, Collider* collider_a, Collider* collider_b);
void narrow_phase(Collision_System* sys, usize);
void narrow_phase_pair(Collision_System* sys, Collider* primary, Collider* against);
void resolve(Collision_System*);
void clear(Collision_System* sys, uint32 id);

//usize     get_collider(Collision_System* sys);
//Collider* collider_by_id(Collision_System* sys, usize id);


Quad calc_transformed_quad_from_aabb(Collider* collider);
Box calc_transformed_aabb(Collider* collider);
Box calc_transformed_aabb_no_save(Collider* collider);

void push_AABBs(Collision_System*, Collider*, usize, usize);
void push_AABB(Collision_System*, Collider*);
void push_line_segments(Collision_System* sys,  Collider* line_segments, usize count, usize layer);
void push_points(Collision_System* sys,  Collider* points, usize count, usize layer);
void push_circles(Collision_System* sys, Collider* circles, usize count, usize layer);
void push_polygons(Collision_System* sys, Collider* polygons, usize count, usize layer);


void Collider_push(Collider* c);

template <typename C_TYPE>
void Collider_push(Collider* c)
{
    if constexpr (std::is_same<C_TYPE, mtt::AABB>::value) {
        push_AABBs(c->system, c, 1, 0);
    } else if constexpr (std::is_same<C_TYPE, mtt::Circle>::value) {
        push_circles(c->system, c, 1, 0);
    } else if constexpr (std::is_same<C_TYPE, mtt::Point>::value) {
        push_points(c->system, c, 1, 0);
    } else if constexpr (std::is_same<C_TYPE, mtt::Line_Segment>::value) {
        push_line_segments(c->system, c, 1, 0);
    } else  {
        ASSERT_MSG(false, "Collider type not supported here.");
    }
}

template <typename C_TYPE, typename Proc>
void Collider_update(Collider* c, Proc&& proc)
{
    Collider_remove(c->system, 0, c);
    proc(c);
    
    if constexpr (std::is_same<C_TYPE, mtt::AABB>::value) {
        push_AABBs(c->system, c, 1, 0);
    } else if constexpr (std::is_same<C_TYPE, mtt::Circle>::value) {
        push_circles(c->system, c, 1, 0);
    } else if constexpr (std::is_same<C_TYPE, mtt::Point>::value) {
        push_points(c->system, c, 1, 0);
    } else if constexpr (std::is_same<C_TYPE, mtt::Line_Segment>::value) {
        push_line_segments(c->system, c, 1, 0);
    } else  {
        ASSERT_MSG(false, "Collider type not supported here.");
    }
}

template <typename Proc>
void Collider_update_AABB(Collider* c, Proc&& proc)
{
    Collider_remove(c->system, 0, c);
    proc();
    push_AABB(c->system, c);
}

template<typename Proc>
void Collider_update(Collider* c, Proc&& proc)
{
    Collider_remove(c->system, 0, c);
    proc(c);
    
    switch (c->type) {
    case COLLIDER_TYPE_AABB:
        push_AABB(c->system, c);
        break;
    case COLLIDER_TYPE_LINE_SEGMENT:
        push_line_segments(c->system, c, 1, 0);
        break;
    case COLLIDER_TYPE_CIRCLE:
        push_circles(c->system, c, 1, 0);
        break;
    case COLLIDER_TYPE_POINT:
        push_points(c->system, c, 1, 0);
        break;
    case COLLIDER_TYPE_CONCAVE_HULL:
        push_polygons(c->system, c, 1, 0);
        break;
    default: {
        ASSERT_MSG(false, "Collider type not supported here.");
        break;
    }
    }
}



struct Cell_List {
    const robin_hood::unordered_flat_map<uint64, uint64>& set;
    uint64* cell_ID_list;
    usize count;
};

const Cell_List active_cells(Collision_System* sys, usize layer);
Spatial_Grid_Cell* get_cell(Collision_System* sys, usize layer, uint64 key);

bool AABB_AABB_intersection(AABB* a, AABB* b);
bool Box_Box_intersection(Box* a, Box* b);
bool Box_Box_intersection(float32* a_tl, float32* a_br, float32* b_tl, float32* b_br);
void enclosing_AABB(AABB* a, AABB* b, AABB* out);
void enclosing_Box(Box* a, Box* b, Box* out);
bool Box_Line_Segment_intersection(float32* tl, float32* br, float32* l_a, float32* l_b);
bool AABB_Line_Segment_intersection(AABB* box, Line_Segment* s, Hit* hit);
bool AABB_Circle_intersection(AABB* aabb, Circle* circle);
bool AABB_Point_intersection(AABB* aabb, Point* point, Hit* hit);

bool OBB_OBB_intersection(AABB* a, AABB* b);

// https://gamedev.stackexchange.com/questions/29479/swept-aabb-vs-line-segment-2d
//bool AABB_Line_Segment_sweep_intersection(AABB* a, Line_Segment* l, vec2 v, vec2* outVel, vec2* hitNormal)
//{
////    Vector2 lineDir = l->b - l->a;
////    Vector2 lineMin = Vector2(0,0);
////    Vector2 lineMax = Vector2(0,0);
////
////    if(lineDir.x > 0.0f) //right
////    {
////        lineMin.x = l->a.x;
////        lineMax.x = l->b.x;
////    }
////    else //left
////    {
////        lineMin.x = l->b.x;
////        lineMax.x = l->a.x;
////    }
////
////    if(lineDir.y > 0.0f) //up
////    {
////        lineMin.y = l->a.y;
////        lineMax.y = l->b.y;
////    }
////    else //down
////    {
////        lineMin.y = l->b.y;
////        lineMax.y = l->a.y;
////    }
////
////    //Initialise out info
////    *outVel = v;
////    *hitNormal = Vector2(0,0);
////
////    float hitTime = 0.0f;
////    float outTime = 1.0f;
////    Vector2 overlapTime = Vector2(0,0);
////
////    vec2 extent = a->br - a->tl;
////    vec2 center = (a->br + a->tl) / 2.0;
////
////    vec2 lN = -(l->b.x - l->a.x) / ((l->b.y - l->a.y) == 0) ? 0.001 : (l->b.y - l->a.y);
////
////    float r = extent.x * m::abs(lN.x) + extent.y * m::abs(lN.y); //radius to Line
////    float boxProj = m::dot(l->a - center, lN); //projected Line distance to N
////    float velProj = m::dot(vel, lN); //projected Velocity to N
////
////    if(velProj < 0f) r *= -1f;
////
////    hitTime = m::max( (boxProj - r ) / velProj, hitTime);
////    outTime = m::min( (boxProj + r ) / velProj, outTime);
////
////    // X axis overlap
////    if( v.x < 0 ) //Sweeping left
////    {
////        if( a->br.x < lineMin.x ) return false;
////        hitTime = Mathf.Max( (lineMax.x - a.min.x) / v.x, hitTime);
////        outTime = Mathf.Min( (lineMin.x - a.max.x) / v.x, outTime);
////    }
////    else if( v.x > 0 ) //Sweeping right
////    {
////        if( a.min.x > lineMax.x ) return false;
////        hitTime = Mathf.Max( (lineMin.x - a.max.x) / v.x, hitTime);
////        outTime = Mathf.Min( (lineMax.x - a.min.x) / v.x, outTime);
////    }
////    else
////    {
////        if(lineMin.x > a.max.x || lineMax.x < a.min.x) return false;
////    }
////
////    if( hitTime > outTime ) return false;
////
////    // Y axis overlap
////    if( v.y < 0 ) //Sweeping down
////    {
////        if( a.max.y < lineMin.y ) return false;
////        hitTime = Mathf.Max( (lineMax.y - a.min.y) / v.y, hitTime);
////        outTime = Mathf.Min( (lineMin.y - a.max.y) / v.y, outTime);
////    }
////    else if( v.y > 0 ) //Sweeping up
////    {
////        if( a.min.y > lineMax.y ) return false;
////        hitTime = Mathf.Max( (lineMin.y - a.max.y) / v.y, hitTime);
////        outTime = Mathf.Min( (lineMax.y - a.min.y) / v.y, outTime);
////    }
////    else
////    {
////        if(lineMin.y > a.max.y || lineMax.y < a.min.y) return false;
////    }
////
////    if( hitTime > outTime ) return false;
////
////    outVel = v * hitTime;
//
//    return true;
//}
bool Line_Segment_intersection_query(Line_Segment* s0, Line_Segment* s1, Hit* out);
bool Line_Segment_intersection(Line_Segment* s0, Line_Segment* s1, Hit* out);
bool Line_Segment_Circle_intersection(Line_Segment* s, Circle* c);
bool Line_Segment_Point_intersection(Line_Segment* s, Point* p);
bool Circle_Circle_intersection(Circle* a, Circle* b);
bool Circle_Circle_intersection(Circle* a, Circle* b, float32* out_square_distance);
bool Circle_Point_intersection(Circle* circle, Point* point);
bool Circle_Concave_Poly_intersection(Circle* circle, Concave_Hull* poly);
bool Point_Concave_Poly_intersection(Point* point, Concave_Hull* poly);

bool Point_Quad_intersection(vec2 point, vec2 quad_tl, vec2 quad_bl, vec2 quad_br, vec2 quad_tr);
bool Point_Quad_intersection(Point* point, Quad* quad);



constexpr const uint32 MASK_BITS_EVERYTHING  = 0xFFFF; // everything
constexpr const uint32 CATEGORY_BITS_DEFAULT = 0x0001; // first category

typedef enum COLLISION_CATEGORY {
    COLLISION_CATEGORY_WORLD = CATEGORY_BITS_DEFAULT,
    COLLISION_CATEGORY_UI
} COLLISION_CATEGORY;

constexpr const uint32 MASK_BITS_DEFAULT_WORLD = MASK_BITS_EVERYTHING & (~COLLISION_CATEGORY_UI);    // not UI
constexpr const uint32 MASK_BITS_DEFAULT_UI    = MASK_BITS_EVERYTHING & (~COLLISION_CATEGORY_WORLD); // not world
constexpr const uint32 MASK_BITS_DEFAULT = MASK_BITS_DEFAULT_WORLD;

inline bool Colliders_are_compatible(Collider* a, Collider* b)
{
    return !(a->is_inactive || b->is_inactive) && ((a->mask_bits & b->category_bits) != 0) &&
           ((a->category_bits & b->mask_bits) != 0);
}



Collision_Handler_Result collision_handler_no_op(Broad_Phase_Collision_Record* collisions, Collision_System* system);



//--------------------------------------------------------------------
//2D OBB Intersection
//
//For 2D graphics or movement of 3D objects on a 2D ground plane it is often
//useful to discover if the 2D oriented bounding boxes of two objects overlap
//(have a non-empty intersection).  One motivating example is the placement of
//a new building in a Real-Time Strategy game.  The UI needs to continuously
//check whether the footprint of the new building overlaps the footprint of
//any existing building.  If there is any overlap, the UI should indicate that
//is an illegal placement.
//
//Stefan Gottschalk's thesis (Collision Queries using Oriented Bounding
//Boxes, Ph.D. Thesis, Department of Computer Science, University of North
//Carolina at Chapel Hill, 1999) introduces the separating-axis method
//for performing the equivalent test on 3D oriented bounding boxes.
//This method depends on the observation that
//for two boxes to be disjoint (i.e. *not* intersecting), there must be some axis
//along which their projections are disjoint.  The 3D case considers each of 15
// axes as a potential
//separating axis.  These axes are the three edge axes of box 1, the three edge
//axes of box 2, and the nine cross products formed by taking some edge of box 1
//and some edge of box 2.
//
//In 2D this simplifies dramatically and only four axes need be considered.
//These are
//the orthogonal edges of each bounding box.  If a few values are precomputed
//every time a box moves, we end up performing only 16 dot products and some
//comparisons in the worst case for each overlap test. One nice property of the
//separating-axis method is that it can be
//structured in an early out fashion, so that many fewer operations are needed in
//the case where the boxes do not intersect.  In general, the first test is
//extremely
//likely to fail (and return "no overlap") when there is no overlap.  If it
//passes,
//the second test is even more likely to fail if there is no overlap, and so on.
//Only when the boxes are in extremely close proximity is there even a 50% chance
//of
//executing more than 2 tests.
//
//The C++ code sample provided efficiently computes this fast 2D oriented
//bounding box
//overlap.  I augmented the OBB2D class with some methods for rendering and
//construction to help visualize the result.  OBB2D::overlaps1Way performs the
//real work.  It tests to see whether the box passed as an argument overlaps the
//current box along either of the current box's dimensions.  Note that this test
//must be performed for each box on the other to determine whether there is truly
//any overlap.  To make the tests extremely efficient, OBB2D::origin stores the
//projection of corner number zero onto a box's axes and the axes are stored
//explicitly in OBB2D::axis.  The magnitude of these stored axes is the inverse
//of the corresponding edge length so that all overlap tests can be performed on
//the interval [0, 1] without normalization, and square roots are avoided
//throughout the entire class.
//
//Morgan McGuire morgan@cs.brown.edu



//class OBB2D {
//private:
//    /** Corners of the box, where 0 is the lower left. */
//    Vector2         corner[4];
//
//    /** Two edges of the box extended away from corner[0]. */
//    Vector2         axis[2];
//
//    /** origin[a] = corner[0].dot(axis[a]); */
//    double          origin[2];
//
//    /** Returns true if other overlaps one dimension of this. */
//    bool overlaps1Way(const OBB2D& other) const {
//        for (int a = 0; a < 2; ++a) {
//
//            double t = other.corner[0].dot(axis[a]);
//
//            // Find the extent of box 2 on axis a
//            double tMin = t;
//            double tMax = t;
//
//            for (int c = 1; c < 4; ++c) {
//                t = other.corner[c].dot(axis[a]);
//
//                if (t < tMin) {
//                    tMin = t;
//                } else if (t > tMax) {
//                    tMax = t;
//                }
//            }
//
//            // We have to subtract off the origin
//
//            // See if [tMin, tMax] intersects [0, 1]
//            if ((tMin > 1 + origin[a]) || (tMax < origin[a])) {
//                // There was no intersection along this dimension;
//                // the boxes cannot possibly overlap.
//                return false;
//            }
//        }
//
//        // There was no dimension along which there is no intersection.
//        // Therefore the boxes overlap.
//        return true;
//    }
//
//
//    /** Updates the axes after the corners move.  Assumes the
//        corners actually form a rectangle. */
//    void computeAxes() {
//        axis[0] = corner[1] - corner[0];
//        axis[1] = corner[3] - corner[0];
//
//        // Make the length of each axis 1/edge length so we know any
//        // dot product must be less than 1 to fall within the edge.
//
//        for (int a = 0; a < 2; ++a) {
//            axis[a] /= axis[a].squaredLength();
//            origin[a] = corner[0].dot(axis[a]);
//        }
//    }
//
//public:
//

void OBB_compute_axes(AABB* box);
bool OBB_overlap_1way(AABB* a, AABB* b);
bool OBB_OBB_intersection(AABB* a, AABB* b);


//    OBB2D(const Vector2& center, const double w, const double h, double angle)
//{
//        Vector2 X( cos(angle), sin(angle));
//        Vector2 Y(-sin(angle), cos(angle));
//
//        X *= w / 2;
//        Y *= h / 2;
//
//        corner[0] = center - X - Y;
//        corner[1] = center + X - Y;
//        corner[2] = center + X + Y;
//        corner[3] = center - X + Y;
//
//        computeAxes();
//    }
//
//
//    /** For testing purposes. */
//    void moveTo(const Vector2& center) {
//        Vector2 centroid = (corner[0] + corner[1] + corner[2] + corner[3]) / 4;
//
//        Vector2 translation = center - centroid;
//
//        for (int c = 0; c < 4; ++c) {
//            corner[c] += translation;
//        }
//
//        computeAxes();
//    }
//
//    /** Returns true if the intersection of the boxes is non-empty. */
//    bool overlaps(const OBB2D& other) const {
//        return overlaps1Way(other) && other.overlaps1Way(*this);
//    }
//
//    void render() const {
//        glBegin(GL_LINES);
//            for (int c = 0; c < 5; ++c) {
//              glVertex2fv(corner[c & 3]);
//            }
//        glEnd();
//    }
//};

bool Box_point_check(float box[4], vec2 point);

bool Box_Circle_intersection(float box[4], Circle* circle);

bool Collider_is_pushed(Collider* c);


void collisions_pre(Collision_System* world);


static bool is_active(mtt::Thing* thing);


template <typename FILTER_PROC>
bool point_query_narrow(mtt::Collision_System* sys, vec2 pos, mtt::Collider** out_collider, Hit& hit, float32* min_area_ref, mtt::Set_Stable<mtt::Thing_ID>* selected_things, FILTER_PROC passes_filter)
{
    Collider* c_hit = nullptr;
    
    float32 min_area = *min_area_ref;
    
    Point point;
    point.coord = pos;
    
    bool narrow_hit = false;
    bool was_hit = false;
    std::vector<mtt::Collider*> out;
    
    bool broad_hit = point_query(sys, 0, pos, &out);
    if (!broad_hit) {
        return false;
    }
    
    
    
    for (auto it = out.cbegin(); it != out.cend(); ++it) {
        mtt::Collider* c = *it;
        Thing_ID thing_id  = (mtt::Thing_ID)c->user_data;
        if (selected_things->find(thing_id) != selected_things->end()) {
            continue;
        }
        mtt::Thing* thing = mtt::Thing_try_get(sys->world, thing_id);
        if (thing == nullptr || !mtt::is_active(thing)) {
            continue;
        }
        
        if (!passes_filter(c, thing)) {
            continue;
        }
        
        
        
        switch ((*it)->type) {
        case mtt::COLLIDER_TYPE_AABB: {
            AABB mod = c->aabb;
            mod.tl = mod.saved_box.tl;
            mod.br = mod.saved_box.br;
            
            //if (mtt::AABB_Point_intersection(&mod, &point, &hit)) {
            if (mtt::Point_Quad_intersection(&point, &c->aabb.saved_quad)) {
                float32 area = calc_aabb_area(&mod);
                if (area >= min_area) {
                    break;
                }
                
                min_area = area;
                
                narrow_hit = true;
                c_hit = c;
                
#ifndef NDEBUG
                MTT_print("ACTUAL HIT! [%f,%f] box tl=[%f,%f]br=[%f,%f]\n\n", point.coord[0], point.coord[1], mod.tl[0],mod.tl[1], mod.br[0],mod.br[1]);
#endif
                
                break;
            }
            break;
        }
        default: { break; }
        }
    }
    was_hit = was_hit || narrow_hit;
    
    *min_area_ref = min_area;
    *out_collider = c_hit;
    
    return was_hit;
}

template <typename FILTER_PROC>
bool point_query_narrow_including_selections(mtt::Collision_System* sys, vec2 pos, mtt::Collider** out_collider, Hit& hit, float32* min_area_ref, FILTER_PROC passes_filter)
{
    Collider* c_hit = nullptr;
    
    float32 min_area = *min_area_ref;
    
    Point point;
    point.coord = pos;
    
    bool narrow_hit = false;
    bool was_hit = false;
    Collider_List out;
    
    bool broad_hit = point_query(sys, 0, pos, &out);
    if (!broad_hit) {
        return false;
    }
    
    
    
    for (auto it = out.cbegin(); it != out.cend(); ++it) {
        mtt::Collider* c = *it;
        Thing_ID thing_id  = (mtt::Thing_ID)c->user_data;
        
        mtt::Thing* thing = mtt::Thing_try_get(sys->world, thing_id);
        if (thing == nullptr || !mtt::is_active(thing)) {
            continue;
        }
        
        if (!passes_filter(c, thing)) {
            continue;
        }
        
        switch ((*it)->type) {
        case mtt::COLLIDER_TYPE_AABB: {
            AABB mod = c->aabb;
            mod.tl = mod.saved_box.tl;
            mod.br = mod.saved_box.br;
            
            //if (mtt::AABB_Point_intersection(&mod, &point, &hit)) {
            if (mtt::Point_Quad_intersection(&point, &c->aabb.saved_quad)) {
                float32 area = calc_aabb_area(&mod);
                if (area >= min_area) {
                    break;
                }
                
                min_area = area;
                
                narrow_hit = true;
                c_hit = c;
                
#ifndef NDEBUG
                MTT_print("ACTUAL HIT! [%f,%f] box tl=[%f,%f]br=[%f,%f]\n\n", point.coord[0], point.coord[1], mod.tl[0],mod.tl[1], mod.br[0],mod.br[1]);
#endif
                
                break;
            }
            break;
        }
        default: { break; }
        }
    }
    was_hit = was_hit || narrow_hit;
    
    *min_area_ref = min_area;
    *out_collider = c_hit;
    
    return was_hit;
}

bool point_query_on_colliders(mtt::Collider** colliders, usize to_check_count, vec2 pos, mtt::Collider** out_collider, Hit& hit, float32* min_area_ref);

struct Quad_As_Segments {
    Line_Segment segments[4];
};

static inline Quad_As_Segments Quad_segments(Quad* q)
{
    return {
        {
            {q->tl, q->bl},
            {q->bl, q->br},
            {q->br, q->tr},
            {q->tr, q->tl}
        }
    };
}

bool rectangle_ray_intersection(vec2 rayOrigin, vec2 rayDir, vec2 boxMin, vec2 boxMax, vec2* out);
vec2 rectangle_ray_intersection_interior_guaranteed(vec2 rayOrigin, vec2 rayDir, vec2 boxMin, vec2 boxMax);


bool Things_are_overlapping(mtt::World* world, mtt::Thing* thing_a, mtt::Thing* thing_b);

}

#endif /* collision_hpp */
