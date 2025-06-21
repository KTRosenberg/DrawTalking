//
//  collision_defaults.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/28/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#include "collision_defaults.hpp"

namespace mtt {

// MARK: default collision handlers


// MARK: - Collision Handlers (default)
Collision_Handler_Result collision_handler_AABB_default(Broad_Phase_Collision_Record* collisions, Collision_System* system)
{
    mtt::World* world = collisions->collider_primary->system->world;
    
    Collider* col_primary = collisions->collider_primary;
    mtt::Thing_ID primary_thing_id = (Thing_ID)(col_primary->user_data);
    mtt::Thing* primary_thing = nullptr;
    
    world->Thing_get(primary_thing_id, &primary_thing);
    
    AABB* aabb_primary = &col_primary->aabb;
    {
        Collider* col_against = nullptr;
        auto* against_set = &collisions->colliders_against;
        for (auto it = against_set->begin(); it != against_set->end(); ++it) {
            col_against = *it;
            
            mtt::Thing_ID against_thing_id = (Thing_ID)(col_against->user_data);
            // disallow self-intersection
            if (primary_thing_id == against_thing_id ||
                (!mtt::Colliders_are_compatible(col_primary, col_against))) {
                
                continue;
            }
            
            switch (col_against->type) {
            default: {
                break;
            }
            case COLLIDER_TYPE_AABB: {
                AABB* aabb_against = &col_against->aabb;
                
                if (!AABB_AABB_intersection(aabb_primary, aabb_against)) {
                    break;
                }
                
                
                
                mtt::Thing* against_thing = nullptr;
                world->Thing_get(against_thing_id, &against_thing);
                
                if (
                    (primary_thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH || primary_thing->archetype_id == mtt::ARCHETYPE_NUMBER)
                    
                    &&
                    
                    ((against_thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH) || against_thing->archetype_id == mtt::ARCHETYPE_NUMBER))
                {
                    mtt::register_collision(world, primary_thing, against_thing, col_primary, col_against);
                }
                break;
            }
            }
        }
    }
    //MTT_print("CALLING AABB handler\n");
    return true;
}

Collision_Handler_Result collision_handler_Line_Segment_default(Broad_Phase_Collision_Record* collisions, Collision_System* system)
{
    mtt::World* info = collisions->collider_primary->system->world;
    
    float32 t = POSITIVE_INFINITY;
    AABB* aabb = nullptr;
    
    {
        Line_Segment* c_self = &collisions->collider_primary->line_segment;
        Collider* against = nullptr;
        auto* against_set = &collisions->colliders_against;
        for (auto it = against_set->begin(); it != against_set->end(); ++it) {
            against = *it;
            
            switch (against->type) {
            case COLLIDER_TYPE_AABB: {
                //MTT_print("line vs aabb\n");
                vec4 saved_color = sd::get_color_rgba(info->renderer);
                
                Hit hit;
                if (AABB_Line_Segment_intersection(&against->aabb, c_self, &hit)) {
                    
                    if (t > hit.t) {
                        t = hit.t;
                        aabb = &against->aabb;
                    }
                    
                    
                } else {
                    
                    vec2 tl = against->aabb.tl;
                    vec2 br = against->aabb.br;
                    sd::rectangle(info->renderer, tl, br - tl, 201.0f);
                }
                
                sd::set_color_rgba_v4(info->renderer, saved_color);
                
                break;
            }
            case COLLIDER_TYPE_LINE_SEGMENT: {
                //MTT_print("line vs line\n");
                break;
            }
            case COLLIDER_TYPE_POINT: {
                
                if constexpr ((true)) {
                    Circle c;
                    c.center = against->point.coord;
                    c.radius = 10.0;
                    if (Line_Segment_Circle_intersection(c_self, &c)) {
                        sd::set_depth(info->renderer, 207);
                        sd::polygon_convex_regular_v2(info->renderer, 16.0f, against->point.coord, 32);
                    }
                } else {
                    vec4 saved_color = sd::get_color_rgba(info->renderer);
                    
                    if (Line_Segment_Point_intersection(c_self, &against->point)) {
                        sd::set_depth(info->renderer, 207);
                        sd::polygon_convex_regular_v2(info->renderer, 16.0f, against->point.coord, 32);
                    }
                }
                
                
                break;
            }
            case COLLIDER_TYPE_CIRCLE: {
                //MTT_print("line vs circle\n");
                break;
            }
            case COLLIDER_TYPE_CONCAVE_HULL: {
                //MTT_print("line vs polygon\n");
                break;
            }
            default: break;
            }
        }
    }
    
    if (t != POSITIVE_INFINITY) {
        sd::set_color_rgba_v4(info->renderer, {0.0, 0.0, 0.87, 1.0});
        
        vec2 tl = aabb->tl;
        vec2 br = aabb->br;
        sd::rectangle(info->renderer, tl, br - tl, 201.0f);
        
        sd::set_color_rgba_v4(info->renderer, {0.87, 0.0, 0.0, 1.0});
        
        vec2 along = collisions->collider_primary->line_segment.a + (t * normalize(collisions->collider_primary->line_segment.b - collisions->collider_primary->line_segment.a));
        sd::set_depth(info->renderer, 207);
        sd::polygon_convex_regular_v2(info->renderer, 16.0f, along, 32);
    }
    
    
    
    //AABB_Line_Segment_intersection(AABB* box, Line_Segment* s, Hit* hit);
    
    return true;
}

Collision_Handler_Result collision_handler_Point_default(Broad_Phase_Collision_Record* collisions, Collision_System* system)
{
    MTT_print("%s", "Handling point collision\n");
    
    return true;
}

Collision_Handler_Result collision_handler_Concave_Hull_default(Broad_Phase_Collision_Record* collisions, Collision_System* system)
{
    //MTT_print("CALLING Concave_Hull handler\n");
    return true;
}

Collision_Handler_Result collision_handler_Circle_default(Broad_Phase_Collision_Record* collisions, Collision_System* system)
{
    //MTT_print("CALLING Concave_Hull handler\n");
    return true;
}

}
