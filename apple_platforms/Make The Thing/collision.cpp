//
//  collision.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/5/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "collision_defaults.hpp"
#include "drawtalk_shared_types.hpp"

#include "word_info.hpp"

namespace mtt {

void Collision_System_init(Collision_System* sys, usize layer_count, mem::Allocator allocator, mtt::World* world)
{
    sys->max_layer_count = (layer_count == 0 || COLLISION_MAX_LAYERS < layer_count) ? COLLISION_MAX_LAYERS : layer_count;
    sys->layer_count = 0;
    
    sys->next_available_ID = 1;
    //sys->colliders = std::vector<Collider>();
    
    
    mem::Pool_Allocation_init(&sys->collider_pool, allocator, 128, sizeof(mtt::Collider), 16);
    sys->user_data = nullptr;
    
    sys->collision_handler_AABB_default = mtt::collision_handler_AABB_default;
    
    sys->world = world;
    
    qt_create(&sys->spatial_index, INT_MAX / 2, INT_MAX / 2, 16, 8);
}
usize Collision_System_layer_make(Collision_System* sys, usize dimensions)
{
    dimensions = (dimensions < 16) ? 16 : dimensions;
    
    if (sys->layer_count > 0) {
        ASSERT_MSG(dimensions >= sys->layers[sys->layer_count - 1].grid_cell_dimensions, "Collision layers must be in non-decreasing order");
    }
    
    if (sys->layer_count == sys->max_layer_count) {
        MTT_error("%s", "ERROR: maximum number of collision layers reached\n");
        return -1;
    }
    
    sys->layers[sys->layer_count] = Spatial_Grid();
    sys->layers[sys->layer_count].grid_cell_dimensions = dimensions;
    sys->layers[sys->layer_count].ID = sys->layer_count;
    
    sys->layer_count += 1;
    
    return sys->layer_count - 1;
}

void Collision_System_set_layer_dimensions(Collision_System* sys, usize layer, usize dimensions)
{
    sys->layers[layer].grid_cell_dimensions = dimensions;
}


void Box_print(Box* box)
{
    MTT_print(
              "tl=[%f,%f] br=[%f,%f]\n",
              box->tl.x, box->tl.y,
              box->br.x, box->br.y
              );
}
void Box_print(const Box* box)
{
    MTT_print(
              "tl=[%f,%f] br=[%f,%f]\n",
              box->tl.x, box->tl.y,
              box->br.x, box->br.y
              );
}

void Box_print_dimensions(Box* box)
{
    MTT_print(
              "tl=[%f,%f] br=[%f,%f]\n",
              box->tl.x, box->tl.y,
              box->tl.x + box->dimensions.x, box->br.y + box->dimensions.y
              );
}
void Box_print_dimensions(const Box* box)
{
    MTT_print(
              "tl=[%f,%f] br=[%f,%f]\n",
              box->tl.x, box->tl.y,
              box->tl.x + box->dimensions.x, box->br.y + box->dimensions.y
              );
}

void Collider_print(const Collider* collider)
{
    MTT_print(
              "Collider {\n\t"
              "ID=[%d],\n\t"
              "type=[%s]\n\t"
              "priority=[%llu]\n"
              "}\n",
              collider->ID,
              collider_type_to_string[collider->type],
              collider->priority
              );
    switch (collider->type) {
    case COLLIDER_TYPE_AABB: {
        MTT_print(
                  "tl=[%f,%f] br=[%f,%f]\n",
                  collider->aabb.tl.x, collider->aabb.tl.y,
                  collider->aabb.br.x, collider->aabb.br.y
                  );
        break;
    }
    case COLLIDER_TYPE_CIRCLE: {
        MTT_print("center=[%f,%f], radius=[%f]\n", collider->circle.center.x, collider->circle.center.y,
                  collider->circle.radius);
        break;
    }
    case COLLIDER_TYPE_POINT: {
        m::Vec2_print(collider->point.coord);
        break;
    }
    case COLLIDER_TYPE_LINE_SEGMENT: {
        MTT_print("%s", "TODO DETAILS\n");
        break;
    }
    case COLLIDER_TYPE_CONCAVE_HULL: {
        MTT_print("%s", "TODO DETAILS\n");
        break;
    }
    default: { break; }
    }
}


inline static void collision_begin(mtt::World* world, Collision_Record* record)
{
    // MTT_print("%s\n", "COLLISION BEGIN");
    
    auto& collide = world->collide_tag;
    auto& collide_begin = world->collide_begin_tag;
    
    record->a.add(collide_begin, record->b);
    record->b.add(collide_begin, record->a);
    
    record->a.add(collide, record->b);
    record->b.add(collide, record->a);
    
    record->event_stamp_begin = world->eval_count;
    
    record->a.add(world->overlap_begin_tag, record->b);
    record->b.add(world->overlap_begin_tag, record->a);
}
inline static void collision_continue(mtt::World* world, Collision_Record* record)
{
 //   MTT_print("%s\n", "COLLISION CONTINUE");
    
    auto& collide = world->collide_tag;
    auto& collide_begin = world->collide_begin_tag;
    
    
    
    if (record->event_stamp_begin != world->eval_count) {
        record->a.remove(collide_begin, record->b);
        record->b.remove(collide_begin, record->a);
    }
}
inline static void collision_end(mtt::World* world, Collision_Record* record)
{
 //   MTT_print("%s\n", "COLLISION END");
    
    auto& collide = world->collide_tag;
    auto& collide_end = world->collide_end_tag;
    
    record->a.remove(collide, record->b);
    record->b.remove(collide, record->a);
    
    record->a.add(collide_end, record->b);
    record->b.add(collide_end, record->a);
    
    record->a.remove(world->overlap_begin_tag, record->b);
    record->b.remove(world->overlap_begin_tag, record->a);
}

bool Things_are_overlapping(mtt::World* world, mtt::Thing* thing_a, mtt::Thing* thing_b)
{
    return thing_a->ecs_entity.has(world->overlap_begin_tag, thing_b->ecs_entity);
}


void collision_contact_begin(World* world, Thing* a, Thing* b, Collider* collider_a, Collider* collider_b, Collision_Record* record)
{
    //auto* collision_system = &world->collision_system;
    
    collision_begin(world, record);
    
    //assert(record->a.has(world->overlap->typename_desc, record->b));
    //assert(record->b.has(world->overlap->typename_desc, record->a));
    
    //MTT_print("BEGIN COLLISION!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
}


void collision_contact_continue(World* world, Thing* a, Thing* b, Collider* collider_a, Collider* collider_b, Collision_Record* record)
{
    //auto* collision_system = &world->collision_system;
    
    
    collision_continue(world, record);
    //assert(!record->a.has(collision_between, record->b) && ! record->b.has(collision_between, record->a));
}

void collision_contact_end(World* world, Collider* collider_a, Collider* collider_b, Collision_Record* record)
{
    //assert(collider_a != NULL && collider_b != NULL && record->a.is_valid() && record->b.is_valid());
    
    collision_end(world, record);
    
    Collision_Record saved_record = {};
    saved_record.a = record->a;
    saved_record.b = record->b;
    saved_record.ca = record->ca;
    saved_record.cb = record->cb;
    saved_record.key = record->key;
    
//    mtt::send_system_message(&world->message_passer, mtt::MESSAGE_TYPE_FROM_TAG, mtt::Thing_ID_INVALID, (void*)saved_record, mtt::Procedure_make([](void* data, mtt::Procedure_Input_Output* args) -> mtt::Procedure_Return_Type {
//
//        mtt::Message* msg = static_cast<mtt::Message*>(args->input);
//        Collision_Record* saved_record = static_cast<Collision_Record*>(msg->contents);
//        mtt::World* world = static_cast<mtt::World*>(data);
//
//        saved_record->a.remove(world->exit->typename_desc, saved_record->b);
//        saved_record->b.remove(world->exit->typename_desc, saved_record->a);
//
//        return true;
//    }, (void*)world));
    
    auto it = world->collision_system.curr_collisions.find(record->key);
    //ASSERT_MSG(it != world->collision_system.curr_collisions.end(), "Should exist!");
    world->collision_system.curr_collisions.erase(it);
    
    world->collision_system.exit_collisions.push_back(saved_record);
    
    //MTT_print("END COLLISION!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    
    
    
}

void collisions_pre(Collision_System* c)
{
    auto& exit_tag = c->world->collide_end_tag;
    for (auto it = c->exit_collisions.begin(); it != c->exit_collisions.end(); ++it) {
        auto* saved_record = &(*it);
        saved_record->a.remove(exit_tag, saved_record->b);
        saved_record->b.remove(exit_tag, saved_record->a);
    }
    c->exit_collisions.clear();
}


void broad_phase(Collision_System* sys, usize id)
{
    Spatial_Grid* const layer = &sys->layers[id];
    
    const auto* const active = &layer->active_cells;
    
    ///
    // for tracking changes in collisions
    
    sys->not_anymore_collisions = sys->curr_collisions;
    auto& not_anymore_collisions = sys->not_anymore_collisions;
    ///

    //std::cout << "active cell count " << active->size() << std::endl;
    for (auto j = active->cbegin(); j != active->cend(); ++j) {
        const uint64 idx = j->first;
        const Spatial_Grid_Cell* const block = &layer->collision_map[idx];
        
        const usize collider_count = block->colliders.size();
        //std::cout << "COLLIDER COUNT " << collider_count << std::endl;
        for (usize t0 = 0; t0 < collider_count; t0 += 1) {
            Collider_ID a_ID = block->colliders[t0]->ID;
            
            for (usize t1 = t0 + 1; t1 < collider_count; t1 += 1) {
                Collider_ID b_ID = block->colliders[t1]->ID;
                
                Collider* c_lower;
                Collider* c_upper;
                isize priority_comp = block->colliders[t0]->priority - block->colliders[t1]->priority;
                if (priority_comp == 0) {
                    if (a_ID < b_ID) {
                        c_lower = block->colliders[t0];
                        c_upper = block->colliders[t1];
                    } else {
                        c_lower = block->colliders[t1];
                        c_upper = block->colliders[t0];
                    }
                } else if (priority_comp < 0) {
                    c_lower = block->colliders[t1];
                    c_upper = block->colliders[t0];
                } else {
                    c_lower = block->colliders[t0];
                    c_upper = block->colliders[t1];
                }

                narrow_phase_pair(sys, c_lower, c_upper);
            }
        }
        
        
    }
    
#undef BROAD_PHASE_OUTER
#undef BROAD_PHASE_INNER
    
//    {
//        auto& collisions_to_remove = sys->world->collisions_to_remove;
//        const auto collisions_to_remove_count = sys->world->collisions_to_remove_count;
//        auto collision_between = sys->world->collision_between->typename_desc;
//        for (usize c = 0; c < collisions_to_remove_count; c += 1) {
//            auto& col_record = collisions_to_remove[c];
//            col_record.a.add(collision_between, col_record.b);
//            col_record.b.add(collision_between, col_record.a);
//        }
//    }
    {
        for (auto it = not_anymore_collisions.begin();
             it != not_anymore_collisions.end();) {
            
            collision_contact_end(sys->world, it->second.ca, it->second.cb, &it->second);
            
            it = not_anymore_collisions.erase(it);
        }
        
        
    }
}




void register_collision(World* world, Thing* a, Thing* b, Collider* collider_a, Collider* collider_b)
{
    
    
    //    {
    //        auto* thing_to_word = &ct::DrawTalk::ctx()->lang_ctx.dictionary.thing_to_word;
    //        auto a_it = thing_to_word->find(a->id);
    //        auto b_it = thing_to_word->find(b->id);
    //        MTT_print("{\n");
    //        if (a_it != thing_to_word->end()) {
    //            MTT_print("\t%s\n", a_it->second->name.c_str());
    //        }
    //        if (b_it != thing_to_word->end()) {
    //            MTT_print("\t%s\n", b_it->second->name.c_str());
    //        }
    //        MTT_print("}\n");
    //    }
    //    ASSERT_MSG(b->ecs_entity.is_alive() && a->ecs_entity.is_alive(), "INACTIVE ENTITIES\n");
    //    std::cout << world->collisions_between.id() << ", " << a->ecs_entity.id() << ", " << b->ecs_entity.id() << std::endl;
    
    
    //    auto A = a->ecs_entity.add(world->collision_between->typename_desc, b->ecs_entity);
    //    auto B = b->ecs_entity.add(world->collision_between->typename_desc, a->ecs_entity);
    //    printf("[%llu -> %llu][%llu], [%llu -> %llu][%llu] cb %llu\n", a->ecs_entity.id(), b->ecs_entity.id(), A.id(), b->ecs_entity.id(), a->ecs_entity.id(), B.id(), world->collision_between->typename_desc.id());
    uint64 key = (uint64)((((uint64)collider_a->ID) << 32) + collider_b->ID);
    
    mtt::Map<uint64, Collision_Record>& curr_collisions = world->collision_system.curr_collisions;
    mtt::Map<uint64, Collision_Record>& not_anymore_collisions = world->collision_system.not_anymore_collisions;
    
    {
//        MTT_print("{\n");
//        MTT_print("[\n");
//        mtt::Thing_print(a);
//        Collider_print(collider_a);
//        MTT_print("]\n");
//        MTT_print("[\n");
//        mtt::Thing_print(b);
//        Collider_print(collider_b);
//        MTT_print("]\n");
//        MTT_print("}\n");
        
        auto it = curr_collisions.find(key);
        if (it == curr_collisions.end()) {
            auto& record = curr_collisions.insert({key, (Collision_Record) {
                .a = a->ecs_entity,
                .b = b->ecs_entity,
                .ca = collider_a,
                .cb = collider_b,
                .key = key,
            }}).first->second;
            
            collision_contact_begin(world, a, b, collider_a, collider_b, &record);
        } else {
            Collision_Record* record_ref;
            if (mtt::map_try_get(&curr_collisions, key, &record_ref)) {
                
                collision_contact_continue(world, a, b, collider_a, collider_b, record_ref);
                
            } else {
                ASSERT_MSG(false, "collision record should exist!\n");
            }
        }
        {
            auto it = not_anymore_collisions.find(key);
            if (it != not_anymore_collisions.end()) {
                not_anymore_collisions.erase(it);
            }
        }
        
    }
    
//    const usize count = world->collisions_to_remove_count;
//    if (world->collisions_to_remove.size() == count) {
//        world->collisions_to_remove.emplace_back((Collision_Record) {
//            .a = a->ecs_entity,
//            .b = b->ecs_entity,
//            .ca = collider_a,
//            .cb = collider_b,
//        });
//    } else {
//        auto& record = world->collisions_to_remove[count];
//        record.a = a->ecs_entity;
//        record.b = b->ecs_entity;
//        record.ca = collider_a;
//        record.cb = collider_b;
//    }
//    world->collisions_to_remove_count += 1;
}

void narrow_phase(Collision_System* sys, usize id)
{
    
    Spatial_Grid* const layer = &sys->layers[id];
    
    auto* found_collisions = &sys->found_collisions;
    
    for (auto it = found_collisions->begin(); it != found_collisions->end(); ++it) {
        Broad_Phase_Collision_Record* collision = &it->second;
        
        //        MTT_print("%llu : {", collision->collider_primary->ID);
        
        //        MTT_print("}\n");
        
        
        collision->collider_primary->handler(
                                             collision, sys
                                             );
    }
    
}

inline static bool should_allow_self_intersection(Collider* a, Collider* b)
{
    return (a->allow_collision_in_hierarchy && b->allow_collision_in_hierarchy);
}
inline static bool is_self_intersection(mtt::Thing* a, mtt::Thing* b)
{
    return mtt::exists_in_other_hierarchy(a, b);
}

void narrow_phase_pair(Collision_System* sys, Collider* primary, Collider* against)
{
    mtt::World* world = sys->world;
    
    Collider* col_primary = primary;
    mtt::Thing_ID primary_thing_id = (Thing_ID)(col_primary->user_data);

    
    AABB* aabb_primary = &col_primary->aabb;
    {
        Collider* col_against = against;
        
        mtt::Thing_ID against_thing_id = (Thing_ID)(col_against->user_data);
        // disallow self-intersection
        if (primary_thing_id == against_thing_id ||
            (!mtt::Colliders_are_compatible(col_primary, col_against))) {
            
            return;
        }
            
        switch (col_against->type) {
        default: {
            break;
        }
        case COLLIDER_TYPE_AABB: {
            AABB* aabb_against = &col_against->aabb;
            
//            MTT_print("%llu\n",
//                      world->step_count);
            
            if ((!mtt::OBB_OBB_intersection(aabb_primary, aabb_against)) ^ (primary->negated ^ against->negated)) {
            //if (!AABB_AABB_intersection(aabb_primary, aabb_against)) {
//                MTT_print("[\n");
//                Collider_print(primary);
//                Collider_print(against);
//                MTT_print("]\n");
                break;
            }
            
            mtt::Thing* primary_thing = nullptr;
            world->Thing_get(primary_thing_id, &primary_thing);
            mtt::Thing* against_thing = nullptr;
            world->Thing_get(against_thing_id, &against_thing);
            
            if (
                (primary_thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH || primary_thing->archetype_id == mtt::ARCHETYPE_NUMBER)
                
                &&
                
                ((against_thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH) || against_thing->archetype_id == mtt::ARCHETYPE_NUMBER))
            {
                if (!should_allow_self_intersection(col_primary, col_against)
                    && is_self_intersection(primary_thing, against_thing)) {
                    break;
                }
                
                mtt::register_collision(world, primary_thing, against_thing, col_primary, col_against);
            }
            break;
        }
        }

    }
    
}

bool aabb_query(Collision_System* sys, uint32 id, vec2 tl, vec2 br, robin_hood::unordered_flat_set<Collider*>* out)
{
    Spatial_Grid* grid = &sys->layers[id];
    usize grid_size = grid->grid_cell_dimensions;
    
    int32 tlx = (int32)(tl.x / (float32)grid_size);
    int32 tly = (int32)(tl.y / (float32)grid_size);
    
    int32 brx = (int32)(br.x / (float32)grid_size);
    int32 bry = (int32)(br.y / (float32)grid_size);
    
    for (int32 y = tly; y <= bry; y += 1) {
        for (int32 x = tlx; x <= brx; x += 1) {
            uint64 key = (uint64)((((uint64)y) << 32) + x);
            
            auto result = grid->collision_map.find(key);
            if (result == grid->collision_map.cend()) {
                return false;
            }
            
            if (result->second.colliders.size() == 0) {
                return false;
            }
            auto begin = result->second.colliders.begin();
            auto end = result->second.colliders.end();
            out->insert(begin, end);
        }
    }
    
    return true;
}

bool point_query(Collision_System* sys, uint32 id, vec2 point, std::vector<Collider*>* out)
{
    float64 x = point.x;
    float64 y = point.y;
    
    usize idx = id;
    const usize layer_count = sys->layer_count;
    do {
        Spatial_Grid* grid = &sys->layers[idx];
        
        int32 grid_x = (int32)(x / (float64)grid->grid_cell_dimensions);
        int32 grid_y = (int32)(y / (float64)grid->grid_cell_dimensions);
        
        uint64 key = (uint64)((((uint64)grid_y) << 32) + grid_x);
        
        auto result = grid->collision_map.find(key);
        if (result == grid->collision_map.cend() ||
            result->second.colliders.empty()) {
            idx += 1;
            continue;
        }
        
        auto begin = result->second.colliders.begin();
        auto end = result->second.colliders.end();
        
        out->insert(out->end(), begin, end);
        
        idx += 1;
        
    } while (idx < layer_count);
    
    return (!out->empty());
}

bool point_query_narrow(mtt::Collision_System* sys, vec2 pos, mtt::Collider** out_collider, Hit& hit, float32* min_area_ref, mtt::Set_Stable<mtt::Thing_ID>* selected_things)
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
    
    
    
    for (auto it = out.cbegin(); it != out.end(); ++it) {
        mtt::Collider* c = *it;
        Thing_ID thing_id  = (mtt::Thing_ID)c->user_data;
        if (selected_things->find(thing_id) != selected_things->end()) {
            continue;
        }
        mtt::Thing* thing = sys->world->Thing_try_get(thing_id);
        if (thing == nullptr || !mtt::is_active(thing)) {
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

bool point_query_narrow_including_selections(mtt::Collision_System* sys, vec2 pos, mtt::Collider** out_collider, Hit& hit, float32* min_area_ref)
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
    
    
    
    for (auto it = out.cbegin(); it != out.end(); ++it) {
        mtt::Collider* c = *it;
        Thing_ID thing_id  = (mtt::Thing_ID)c->user_data;

        mtt::Thing* thing = sys->world->Thing_try_get(thing_id);
        if (thing == nullptr || !mtt::is_active(thing)) {
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

bool point_query_narrow_with_exclusion(mtt::Collision_System* sys, vec2 pos, mtt::Collider** out_collider, Hit& hit, float32* min_area_ref, mtt::Thing_ID excluded)
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
    
    
    
    for (auto it = out.cbegin(); it != out.end(); ++it) {
        mtt::Collider* c = *it;
        Thing_ID thing_id  = (mtt::Thing_ID)c->user_data;

        if (thing_id == excluded) {
            continue;
        }
        mtt::Thing* thing = sys->world->Thing_try_get(thing_id);
        if (thing == nullptr || !mtt::is_active(thing)) {
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

bool point_query_on_colliders(mtt::Collider** colliders, usize to_check_count, vec2 pos, mtt::Collider** out_collider, Hit& hit, float32* min_area_ref)
{
    Collider* c_hit = nullptr;
    
    float32 min_area = *min_area_ref;
    
    Point point;
    point.coord = pos;
    
    bool narrow_hit = false;
    bool was_hit = false;
    

    for (usize i = 0; i < to_check_count; i += 1) {
        mtt::Collider* c = colliders[i];
        Thing_ID thing_id  = (mtt::Thing_ID)c->user_data;
        
        mtt::Thing* thing = mtt::Thing_try_get(c->system->world, thing_id);
        if (thing == nullptr || !mtt::is_active(thing)) {
            continue;
        }
        
        switch (c->type) {
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

bool circle_query(Collision_System* sys, uint32 id, vec2 center, float32 radius, robin_hood::unordered_flat_set<Collider*>* out)
{
    Spatial_Grid* grid = &sys->layers[id];
    usize grid_size = grid->grid_cell_dimensions;
    
    
    vec2 tl = center - radius;
    vec2 br = center + radius;
    
    int32 tlx = (int32)(tl.x / (float32)grid_size);
    int32 tly = (int32)(tl.y / (float32)grid_size);
    
    int32 brx = (int32)(br.x / (float32)grid_size);
    int32 bry = (int32)(br.y / (float32)grid_size);
    
    for (int32 y = tly; y <= bry; y += 1) {
        for (int32 x = tlx; x <= brx; x += 1) {
            uint64 key = (uint64)((((uint64)y) << 32) + x);
            
            auto result = grid->collision_map.find(key);
            if (result == grid->collision_map.cend()) {
                return false;
            }
            
            if (result->second.colliders.size() == 0) {
                return false;
            }
            auto begin = result->second.colliders.begin();
            auto end = result->second.colliders.end();
            out->insert(begin, end);
        }
    }
    
    return true;
}

bool line_segment_query(Collision_System* sys, usize id, vec4 segment, robin_hood::unordered_flat_set<Collider*>* out)
{
    Spatial_Grid* grid = &sys->layers[id];
    
#define PUSH(line_segments, y, x) do { \
uint64 key = (uint64)((((uint64)y) << 32) + x); \
auto result = grid->collision_map.find(key); \
\
if (result == grid->collision_map.end()) { \
\
} else { \
out->insert(result->second.colliders.cbegin(), result->second.colliders.cend()); \
} \
} while (0) \

    
    char line_segments;
    (void)line_segments;
    
    usize grid_size = grid->grid_cell_dimensions;
    
    float64 x0 = (((float64)segment[0] / grid_size));
    float64 y0 = (((float64)segment[1] / grid_size));
    float64 x1 = (((float64)segment[2] / grid_size));
    float64 y1 = (((float64)segment[3] / grid_size));
    
    double dx = fabs(x1 - x0);
    double dy = fabs(y1 - y0);
    
    int x = int(floor(x0));
    int y = int(floor(y0));
    
    int n = 1;
    int x_inc, y_inc;
    double error;
    
    if (dx == 0)
    {
        x_inc = 0;
        error = std::numeric_limits<double>::infinity();
    }
    else if (x1 > x0)
    {
        x_inc = 1;
        n += int(floor(x1)) - x;
        error = (floor(x0) + 1 - x0) * dy;
    }
    else
    {
        x_inc = -1;
        n += x - int(floor(x1));
        error = (x0 - floor(x0)) * dy;
    }
    
    if (dy == 0)
    {
        y_inc = 0;
        error -= std::numeric_limits<double>::infinity();
    }
    else if (y1 > y0)
    {
        y_inc = 1;
        n += int(floor(y1)) - y;
        error -= (floor(y0) + 1 - y0) * dx;
    }
    else
    {
        y_inc = -1;
        n += y - int(floor(y1));
        error -= (y0 - floor(y0)) * dx;
    }
    
    for (; n > 0; --n)
    {
        PUSH(line_segments,y, x);
        
        if (error > 0)
        {
            y += y_inc;
            error -= dx;
        }
        else
        {
            x += x_inc;
            error += dy;
        }
    }
    
    return out->size() > 0;
    
#undef PUSH
}

void resolve(Collision_System* sys, usize id)
{
    
}

void clear(Collision_System* sys, usize layer)
{
    Spatial_Grid* grid = &sys->layers[layer];
    for (usize i = 0; i < grid->active_cells.size(); i += 1) {
        grid->collision_map.erase(grid->active_cells[i]);
    }
    grid->active_cells.clear();
    sys->found_collisions.clear();
}

//usize get_collider(Collision_System* sys)
//{
//    sys->colliders.push_back(Collider());
//    return sys->colliders.size() - 1;
//}
//Collider* collider_by_id(Collision_System* sys, usize id)
//{
//    return &sys->colliders[id];
//}

Quad calc_transformed_quad_from_aabb(Collider* collider)
{
    AABB* col = &collider->aabb;
    
    vec2 tl = col->tl;
    vec2 br = col->br;
    
    const vec2 center = collider->center_anchor;
    // translated
    const vec2 t_tl = tl - center;
    const vec2 t_br = br - center;
    
    const mat4& xform = collider->transform;
    
    tl = center + vec2(xform * vec4(t_tl, 0.0f, 1.0f));
    br = center + vec2(xform * vec4(t_br, 0.0f, 1.0f));
    vec2 tr = center + vec2(xform * vec4(t_br.x, t_tl.y, 0.0f, 1.0f));
    vec2 bl = center + vec2(xform * vec4(t_tl.x, t_br.y, 0.0f, 1.0f));
    
//    const vec2 corners[4] = {
//        tl, br, tr, bl
//    };
    col->saved_quad = (Quad){
        .tl = tl,
        .bl = bl,
        .br = br,
        .tr = tr,
    };
    
    OBB_compute_axes(col);
    
    return col->saved_quad;
}

Box calc_transformed_aabb(Collider* collider)
{
    AABB* col = &collider->aabb;
    
    vec2 tl = col->tl;
    vec2 br = col->br;
    
    const vec2 center = collider->center_anchor;
    // translated
    const vec2 t_tl = tl - center;
    const vec2 t_br = br - center;
    
    const mat4& xform = collider->transform;
    
    tl = center + vec2(xform * vec4(t_tl, 0.0f, 1.0f));
    br = center + vec2(xform * vec4(t_br, 0.0f, 1.0f));
    vec2 tr = center + vec2(xform * vec4(t_br.x, t_tl.y, 0.0f, 1.0f));
    vec2 bl = center + vec2(xform * vec4(t_tl.x, t_br.y, 0.0f, 1.0f));
    
    const vec2 corners[4] = {
        tl, br, tr, bl
    };
    
    // calculate a bounding box
    for (usize i = 0; i < 4; i += 1) {
        tl.x = m::min(tl.x, corners[i].x);
        tl.y = m::min(tl.y, corners[i].y);
        br.x = m::max(br.x, corners[i].x);
        br.y = m::max(br.y, corners[i].y);
    }
    
    col->saved_box.tl = tl;
    col->saved_box.br = br;
    return col->saved_box;
}

Box calc_transformed_aabb_no_save(Collider* collider)
{
    AABB* col = &collider->aabb;
    
    vec2 tl = col->tl;
    vec2 br = col->br;
    
    const vec2 center = collider->center_anchor;
    // translated
    const vec2 t_tl = tl - center;
    const vec2 t_br = br - center;
    
    const mat4& xform = collider->transform;
    
    tl = center + vec2(xform * vec4(t_tl, 0.0f, 1.0f));
    br = center + vec2(xform * vec4(t_br, 0.0f, 1.0f));
    vec2 tr = center + vec2(xform * vec4(t_br.x, t_tl.y, 0.0f, 1.0f));
    vec2 bl = center + vec2(xform * vec4(t_tl.x, t_br.y, 0.0f, 1.0f));
    
    const vec2 corners[4] = {
        tl, br, tr, bl
    };
    
    // calculate a bounding box
    for (usize i = 0; i < 4; i += 1) {
        tl.x = m::min(tl.x, corners[i].x);
        tl.y = m::min(tl.y, corners[i].y);
        br.x = m::max(br.x, corners[i].x);
        br.y = m::max(br.y, corners[i].y);
    }
    
    
    Box box;
    box.tl = tl;
    box.br = br;
    return box;
}

void push_AABB(Collision_System* sys, Collider* box)
{
    push_AABBs(sys, box, 1, 0);
}

void push_AABBs(Collision_System* sys, Collider* boxes, usize count, usize layer)
{
    Spatial_Grid* grid = &sys->layers[layer];
    usize grid_size = grid->grid_cell_dimensions;
    for (usize i = 0; i < count; i += 1) {
        if (Collider_is_pushed(&boxes[i])) {
            continue;
        }
        
        const Box box = calc_transformed_aabb(&boxes[i]);
        const Quad quad = calc_transformed_quad_from_aabb(&boxes[i]);
        vec2 tl = box.tl;
        vec2 br = box.br;
        
        int64 tlx = (int64)((float64)tl.x / (float64)grid_size);
        int64 tly = (int64)((float64)tl.y / (float64)grid_size);
        
        int64 brx = (int64)((float64)br.x / (float64)grid_size);
        int64 bry = (int64)((float64)br.y / (float64)grid_size);
        
        for (int64 y = tly; y <= bry; y += 1) {
            for (int64 x = tlx; x <= brx; x += 1) {
                uint64 key = (uint64)((((uint64)y) << 32) + x);
                boxes[i].bin_keys.emplace_back((uintptr)key);
                //ASSERT_MSG(boxes[i].bin_keys.size() == 1, "WHAT");
                // FIXME: assume exactly 1 cell, only for an N^2 result

                
                auto result = grid->collision_map.find(key);
                // does not exist
                if (result == grid->collision_map.end()) {
                    
                    Spatial_Grid_Cell cell;
                    cell.key = (uintptr)key;
                    //cell.colliders_AABB.push_back(&boxes[i]);
                    cell.colliders.push_back(&boxes[i]);
                    grid->collision_map[key] = cell;
                    
                    cell.collider_to_idx[boxes[i].ID] = cell.colliders.size() - 1;
                    
                    // exists
                } else {
                    
                    result->second.colliders_AABB.push_back(&boxes[i]);
                    result->second.colliders.push_back(&boxes[i]);
                    
                    result->second.collider_to_idx[boxes[i].ID] = result->second.colliders.size() - 1;
                }
                
                auto it = grid->active_cells.find(key);
                if (it == grid->active_cells.end()) {
                    grid->active_cells.emplace(key, 1);
                } else {
                    it->second += 1;
                }
                
                
                //grid->active_cells.push_back(key);
                
                //MTT_print("added collider at y=[%u],x=[%u]\n", y, x);
            }
        }
    }
}
#include <limits> // for infinity
// http://playtechs.blogspot.com/2007/03/raytracing-on-grid.html?m=1
void line_trace(float64 x0, float64 y0, float64 x1, float64 y1, Spatial_Grid* grid, Collider* line_segments, usize idx)
{
    
    
#define PUSH(line_segments, y, x) do { \
uint64 key = (uint64)((((uint64)y) << 32) + x); \
line_segments[idx].bin_keys.emplace_back((uintptr)key); \
\
auto result = grid->collision_map.find(key); \
\
if (result == grid->collision_map.end()) { \
Spatial_Grid_Cell cell; \
cell.key = (uintptr)key; \
cell.colliders_Line_Segment.push_back(&line_segments[idx]); \
cell.colliders.push_back(&line_segments[idx]); \
grid->collision_map[key] = cell; \
\
cell.collider_to_idx[line_segments[idx].ID] = cell.colliders.size() - 1;\
\
} else { \
result->second.colliders_Line_Segment.push_back(&line_segments[idx]); \
result->second.colliders.push_back(&line_segments[idx]); \
\
result->second.collider_to_idx[line_segments[idx].ID] = result->second.colliders.size() - 1;\
\
} \
\
auto it = grid->active_cells.find(key); \
if (it == grid->active_cells.end()) { \
grid->active_cells.emplace(key, 1); \
} else { \
it->second += 1; \
} \
\
} while (0) \

    
    double dx = fabs(x1 - x0);
    double dy = fabs(y1 - y0);
    
    int x = int(floor(x0));
    int y = int(floor(y0));
    
    int n = 1;
    int x_inc, y_inc;
    double error;
    
    if (dx == 0)
    {
        x_inc = 0;
        error = std::numeric_limits<double>::infinity();
    }
    else if (x1 > x0)
    {
        x_inc = 1;
        n += int(floor(x1)) - x;
        error = (floor(x0) + 1 - x0) * dy;
    }
    else
    {
        x_inc = -1;
        n += x - int(floor(x1));
        error = (x0 - floor(x0)) * dy;
    }
    
    if (dy == 0)
    {
        y_inc = 0;
        error -= std::numeric_limits<double>::infinity();
    }
    else if (y1 > y0)
    {
        y_inc = 1;
        n += int(floor(y1)) - y;
        error -= (floor(y0) + 1 - y0) * dx;
    }
    else
    {
        y_inc = -1;
        n += y - int(floor(y1));
        error -= (y0 - floor(y0)) * dx;
    }
    
    for (; n > 0; --n)
    {
        PUSH(line_segments,y, x);
        
        if (error > 0)
        {
            y += y_inc;
            error -= dx;
        }
        else
        {
            x += x_inc;
            error += dy;
        }
    }
    
    
    
    //     int64 i;               // loop counter
    //     int64 ystep, xstep;    // the step on y and x axis
    //     int64 error;           // the error accumulated during the increment
    //     int64 errorprev;       // *vision the previous value of the error variable
    //     int64 y = y1, x = x1;  // the line points
    //     int64 ddy, ddx;        // compulsory variables: the double values of dy and dx
    //     int64 dx = x2 - x1;
    //     int64 dy = y2 - y1;
    //
    //
    //     PUSH(line_segments, y1, x1);  // first point
    //     // NB the last point can't be here, because of its previous point (which has to be verified)
    //     if (dy < 0){
    //       ystep = -1;
    //       dy = -dy;
    //     }else
    //       ystep = 1;
    //     if (dx < 0){
    //       xstep = -1;
    //       dx = -dx;
    //     }else
    //       xstep = 1;
    //     ddy = 2 * dy;  // work with double values for full precision
    //     ddx = 2 * dx;
    //     if (ddx >= ddy){  // first octant (0 <= slope <= 1)
    //       // compulsory initialization (even for errorprev, needed when dx==dy)
    //       errorprev = error = dx;  // start in the middle of the square
    //       for (i=0 ; i < dx ; i++){  // do not use the first point (already done)
    //         x += xstep;
    //         error += ddy;
    //         if (error > ddx){  // increment y if AFTER the middle ( > )
    //           y += ystep;
    //           error -= ddx;
    //           // three cases (octant == right->right-top for directions below):
    //           if (error + errorprev < ddx)  // bottom square also
    //             PUSH(line_segments,y-ystep, x);
    //           else if (error + errorprev > ddx)  // left square also
    //            PUSH(line_segments,y, x-xstep);
    //           else{  // corner: bottom and left squares also
    //            PUSH(line_segments,y-ystep, x);
    //            PUSH(line_segments,y, x-xstep);
    //           }
    //         }
    //        PUSH(line_segments,y, x);
    //         errorprev = error;
    //       }
    //     }else{  // the same as above
    //       errorprev = error = dy;
    //       for (i=0 ; i < dy ; i++){
    //         y += ystep;
    //         error += ddx;
    //         if (error > ddy){
    //           x += xstep;
    //           error -= ddy;
    //           if (error + errorprev < ddy)
    //            PUSH(line_segments,y, x-xstep);
    //           else if (error + errorprev > ddy)
    //             PUSH(line_segments,y-ystep, x);
    //           else{
    //             PUSH(line_segments,y, x-xstep);
    //             PUSH(line_segments,y-ystep, x);
    //           }
    //         }
    //         PUSH(line_segments,y, x);
    //         errorprev = error;
    //       }
    //     }
    //    x0 = 0;
    //    x1 = 15;
    //    y0 = 3;
    //    y1 = 10;
    //    int32 dx = std::abs(x1 - x0);
    //    int32 dy = std::abs(y1 - x0);
    //    int32 x = x0;
    //    int32 y = y0;
    //    int32 n = 1 + dx + dy;
    //    int32 x_inc = (x1 > x0) ? 1 : -1;
    //    int32 y_inc = (y1 > y0) ? 1 : -1;
    //    int32 error = dx - dy;
    //    dx *= 2;
    //    dy *= 2;
    //
    //    MTT_print("\n inside: \n f start y,x=(%f,%f), end y,x=(%f,%f) grid_size=%llu\n"
    //              "i start y,x-(%lld,%lld), end y,x=(%lld,%lld)",
    //              line_segments[i].line_segment.a.y,
    //              line_segments[i].line_segment.a.x,
    //              line_segments[i].line_segment.b.y,
    //              line_segments[i].line_segment.b.x,
    //              64,
    //              (int64)(line_segments[i].line_segment.a.y / 64),
    //              (int64)(line_segments[i].line_segment.a.x / 64),
    //              (int64)(line_segments[i].line_segment.b.y / 64),
    //              (int64)(line_segments[i].line_segment.b.x / 64)
    //
    //              );
    //
    //
    //    for (; n > 0; --n) {
    //        MTT_print("y,x=(%lld,%lld)\n", y, x);
    //        PUSH(line_segments, y, x);
    //
    //        if (error > 0) {
    //            x += x_inc;
    //            error -= dy;
    //        } else if (error < 0) {
    //            y += y_inc;
    //            error += dx;
    //        } else {
    //            x += x_inc;
    //            error -= dy;
    //            y += y_inc;
    //            error += dx;
    //            n--;
    //        }
    //    }
    //
    //    return;
    //    int dx, dy;
    //    int stepx, stepy;
    //    step = 1;
    //    dx = x1 - x0; dy = y1 - y0;
    //    if(dy<0){
    //        dy=-dy;
    //        stepy=-1;
    //    } else {
    //        stepy=1;
    //    }
    //    if (dx<0) {
    //        dx=-dx; stepx=-1;
    //    } else {
    //        stepx=1;
    //    }
    //    dy <<= 1; /* dy is now 2*dy */
    //    dx <<= 1; /* dx is now 2*dx */
    //    if ((0 <= x0) && (x0 < RDim) && (0 <= y0) && (y0 < RDim)) {
    //        PUSH(line_segments, y0, x0);
    //    }
    //
    //    if (dx > dy) {
    //        int fraction = dy - (dx >> 1);
    //        while (x0 != x1) {
    //            x0 += stepx;
    //            if (fraction >= 0) {
    //                y0 += stepy;
    //                fraction -= dx;
    //            }
    //            fraction += dy;
    //            if ((0 <= x0) && (x0 < RDim) && (0 <= y0) && (y0 < RDim)) {
    //                PUSH(line_segments, y0, x0);
    //            }
    //        }
    //    } else {
    //        int fraction = dx - (dy >> 1);
    //        while (y0 != y1) {
    //            if (fraction >= 0) {
    //                x0 += stepx;
    //                fraction -= dy;
    //            }
    //            y0 += stepy;
    //            fraction += dx;
    //            if ((0 <= x0) && (x0 < RDim) && (0 <= y0) && (y0 < RDim)) {
    //                PUSH(line_segments, y0, x0);
    //            }
    //        }
    //    }
    
#undef PUSH
}
void push_line_segments(Collision_System* sys,  Collider* line_segments, usize count, usize layer)
{
    Spatial_Grid* grid = &sys->layers[layer];
    usize grid_size = grid->grid_cell_dimensions;
    
    for (usize i = 0; i < count; i += 1) {
        if (Collider_is_pushed(&line_segments[i])) {
            continue;
        }
        
        line_trace(
                   (((float64)line_segments[i].line_segment.a.x / grid_size)),
                   (((float64)line_segments[i].line_segment.a.y / grid_size)),
                   (((float64)line_segments[i].line_segment.b.x / grid_size)),
                   (((float64)line_segments[i].line_segment.b.y / grid_size)),
                   grid,
                   line_segments,
                   i
                   );
    }
}

void push_points(Collision_System* sys,  Collider* points, usize count, usize layer)
{
    Spatial_Grid* grid = &sys->layers[layer];
    usize grid_size = grid->grid_cell_dimensions;
    
    for (usize i = 0; i < count; i += 1) {
        if (Collider_is_pushed(&points[i])) {
            continue;
        }
        
        Point* col = &points[i].point;
        
        int32 x = (int32)(col->coord.x / (float32)grid_size);
        int32 y = (int32)(col->coord.y / (float32)grid_size);
        
        uint64 key = (uint64)((((uint64)y) << 32) + x);
        points[i].bin_keys.emplace_back((uintptr)key);
        
        
        auto result = grid->collision_map.find(key);
        // does not exist
        if (result == grid->collision_map.end()) {
            Spatial_Grid_Cell cell;
            cell.key = (uintptr)key;
            cell.colliders_Point.push_back(&points[i]);
            cell.colliders.push_back(&points[i]);
            grid->collision_map[key] = cell;
            
            cell.collider_to_idx[points[i].ID] = cell.colliders.size() - 1;
            
            // exists
        } else {
            result->second.colliders_Concave_Hull.push_back(&points[i]);
            result->second.colliders.push_back(&points[i]);
            
            result->second.collider_to_idx[points[i].ID] = result->second.colliders.size() - 1;
            
        }
        
        auto it = grid->active_cells.find(key);
        if (it == grid->active_cells.end()) {
            grid->active_cells.emplace(key, 1);
        } else {
            it->second += 1;
        }
    }
}

void push_circles(Collision_System* sys, Collider* circles, usize count, usize layer)
{
    Spatial_Grid* grid = &sys->layers[layer];
    usize grid_size = grid->grid_cell_dimensions;
    for (usize i = 0; i < count; i += 1) {
        if (Collider_is_pushed(&circles[i])) {
            continue;
        }
        
        Circle* col = &circles[i].circle;
        
        vec2 tl = col->center - col->radius;
        vec2 br = col->center + col->radius;
        
        int32 tlx = (int32)(tl.x / (float32)grid_size);
        int32 tly = (int32)(tl.y / (float32)grid_size);
        
        int32 brx = (int32)(br.x / (float32)grid_size);
        int32 bry = (int32)(br.y / (float32)grid_size);
        
        for (int32 y = tly; y <= bry; y += 1) {
            for (int32 x = tlx; x <= brx; x += 1) {
                uint64 key = (uint64)((((uint64)y) << 32) + x);
                circles[i].bin_keys.emplace_back((uintptr)key);
                
                
                auto result = grid->collision_map.find(key);
                // does not exist
                if (result == grid->collision_map.end()) {
                    Spatial_Grid_Cell cell;
                    cell.key = (uintptr)key;
                    cell.colliders_Circle.push_back(&circles[i]);
                    cell.colliders.push_back(&circles[i]);
                    grid->collision_map[key] = cell;
                    
                    cell.collider_to_idx[circles[i].ID] = cell.colliders.size() - 1;
                    
                    // exists
                } else {
                    result->second.colliders_Circle.push_back(&circles[i]);
                    result->second.colliders.push_back(&circles[i]);
                    
                    result->second.collider_to_idx[circles[i].ID] = result->second.colliders.size() - 1;
                }
                
                auto it = grid->active_cells.find(key);
                if (it == grid->active_cells.end()) {
                    grid->active_cells.emplace(key, 1);
                } else {
                    it->second += 1;
                }
            }
        }
        
    }
}

void push_polygons(Collision_System* sys, Collider* polygons, usize count, usize layer)
{
    Spatial_Grid* grid = &sys->layers[layer];
    usize grid_size = grid->grid_cell_dimensions;
    for (usize i = 0; i < count; i += 1) {
        if (Collider_is_pushed(&polygons[i])) {
            continue;
        }
        
        Concave_Hull* col = &polygons[i].concave_hull;
        
        vec2* points = col->data();
        usize count = col->size();
        
        vec2 tl = {POSITIVE_INFINITY, POSITIVE_INFINITY};
        vec2 br = {NEGATIVE_INFINITY, NEGATIVE_INFINITY};
        
        // calculate a bounding box
        for (usize i = 0; i < count; i += 1) {
            tl.x = m::min(tl.x, points[i].x);
            tl.y = m::min(tl.y, points[i].y);
            br.x = m::max(br.x, points[i].x);
            br.y = m::max(br.y, points[i].y);
        }
        
        int32 tlx = (int32)(tl.x / (float32)grid_size);
        int32 tly = (int32)(tl.y / (float32)grid_size);
        
        int32 brx = (int32)(br.x / (float32)grid_size);
        int32 bry = (int32)(br.y / (float32)grid_size);
        
        // TODO: Do I need an AABB to go along with the polygon?
        // it will probably be useful to some extent
        col->aabb.tl = tl;
        col->aabb.br = br;
        
        
        for (int32 y = tly; y <= bry; y += 1) {
            for (int32 x = tlx; x <= brx; x += 1) {
                uint64 key = (uint64)((((uint64)y) << 32) + x);
                polygons[i].bin_keys.emplace_back((uintptr)key);
                
                
                auto result = grid->collision_map.find(key);
                // does not exist
                if (result == grid->collision_map.end()) {
                    Spatial_Grid_Cell cell;
                    cell.key = (uintptr)key;
                    cell.colliders_Concave_Hull.push_back(&polygons[i]);
                    cell.colliders.push_back(&polygons[i]);
                    grid->collision_map[key] = cell;
                    
                    cell.collider_to_idx[polygons[i].ID] = cell.colliders.size() - 1;
                    
                    // exists
                } else {
                    result->second.colliders_Concave_Hull.push_back(&polygons[i]);
                    result->second.colliders.push_back(&polygons[i]);
                    
                    result->second.collider_to_idx[polygons[i].ID] = result->second.colliders.size() - 1;
                    
                }
                
                auto it = grid->active_cells.find(key);
                if (it == grid->active_cells.end()) {
                    grid->active_cells.emplace(key, 1);
                } else {
                    it->second += 1;
                }
                
                
                //MTT_print("added collider at y=[%u],x=[%u]\n", y, x);
            }
        }
        
    }
}

static Collision_Handler_Result collision_handler_noop(Broad_Phase_Collision_Record* record, Collision_System* system)
{
    return true;
}

Collider* Collider_make(Collision_System* sys, usize layer)
{
    Collider_ID ID = Collider_ID_INVALID;
    
    if (sys->free_ids.empty()) {
        ID = sys->next_available_ID;
        sys->next_available_ID += 1;
    } else {
        ID = sys->free_ids.back();
        sys->free_ids.pop_back();
    }
    
    Collider* c = mem::alloc_init<mtt::Collider>(&sys->collider_pool.allocator);
    c->ID = ID;
    c->priority = 0;
    c->user_data = nullptr;
    c->layer     = layer;
    c->priority_layer = 0;
    c->system    = sys;
    c->mask_bits     = MASK_BITS_EVERYTHING;
    c->category_bits = CATEGORY_BITS_DEFAULT;
    c->negated = false;
    
    c->label = COLLIDER_LABEL_NEAR;
    c->center_anchor = vec3(0.0f);
    c->pivot_anchor_offset = vec3(0.0f);
    c->transform = mat4(1.0f);
    c->handler   = collision_handler_noop;
    c->allow_collision_in_hierarchy = true;
    
    return c;
}

void Collider_copy_into(Collider* c_cpy, Collider* c_src, void* user_data)
{
    c_cpy->type = c_src->type;
    c_cpy->label = c_src->label;
    c_cpy->priority = c_src->priority;
    c_cpy->priority_layer = c_src->priority_layer;
    
    c_cpy->mask_bits = c_src->mask_bits;
    c_cpy->category_bits = c_src->category_bits;
    c_cpy->negated = c_src->negated;
    //c_cpy->bin_keys = c_src->bin_keys;
    
    c_cpy->handler = c_src->handler;
    c_cpy->center_anchor = c_src->center_anchor;
    c_cpy->pivot_anchor_offset = c_src->pivot_anchor_offset;
    c_cpy->transform = c_src->transform;
    c_cpy->allow_collision_in_hierarchy = c_src->allow_collision_in_hierarchy;
    
    
    c_cpy->user_data = user_data;
    
    switch (c_src->type) {
    case COLLIDER_TYPE_AABB:
        c_cpy->aabb = c_src->aabb;
        break;
    case COLLIDER_TYPE_LINE_SEGMENT:
        c_cpy->line_segment = c_src->line_segment;
        break;
    case COLLIDER_TYPE_CIRCLE:
        c_cpy->circle = c_src->circle;
        break;
    case COLLIDER_TYPE_POINT:
        c_cpy->point = c_src->point;
        break;
    case COLLIDER_TYPE_CONCAVE_HULL:
        c_cpy->concave_hull = c_src->concave_hull;
        break;
    case COLLIDER_TYPE_CONVEX_HULL:
        c_cpy->convex_hull = c_src->convex_hull;
        break;
    default: {
        break;
    }
    }
    
    c_cpy->is_inactive = c_src->is_inactive;
}

Collider* Collider_copy(Collider* c_src, void* user_data)
{
    Collider* c_cpy = Collider_make(c_src->system, 0);
    
    Collider_copy_into(c_cpy, c_src, user_data);
    
    c_cpy->user_data = user_data;
    
    return c_cpy;
}

Collider* Collider_copy(Collider* c_src, void* user_data, Collision_System* dst_collision_system)
{
    Collider* c_cpy = Collider_make(dst_collision_system, 0);
    
    Collider_copy_into(c_cpy, c_src, user_data);
    
    c_cpy->user_data = user_data;
    
    return c_cpy;
}

void Collider_push(Collider* c)
{
//    if constexpr (std::is_same<C_TYPE, mtt::AABB>::value) {
//        push_AABBs(c->system, c, 1, 0);
//    } else if constexpr (std::is_same<C_TYPE, mtt::Circle>::value) {
//        push_circles(c->system, c, 1, 0);
//    } else if constexpr (std::is_same<C_TYPE, mtt::Point>::value) {
//        push_points(c->system, c, 1, 0);
//    } else if constexpr (std::is_same<C_TYPE, mtt::Line_Segment>::value) {
//        push_line_segments(c->system, c, 1, 0);
//    } else  {
//        ASSERT_MSG(false, "Collider type not supported here.");
//    }
    
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
        break;
    }
    }
}




Collider* Collider_make_aabb(Collision_System* sys)
{
    Collider* c = Collider_make(sys, 0);
    
    c->type = mtt::COLLIDER_TYPE_AABB;
    //c->aabb = AABB();
    c->aabb.tl = vec2(-0.5f);
    c->aabb.br = vec2(0.5f);
    c->aabb.half_extent = vec2(0.5f);
    c->center_anchor = vec3(0.0f);
    c->pivot_anchor_offset = vec3(0.0f);
    c->handler = sys->collision_handler_AABB_default;
    
    return c;
}

Collider* Collider_make_circle(Collision_System* sys)
{
    Collider* c = Collider_make(sys, 0);
    
    c->type = mtt::COLLIDER_TYPE_CIRCLE;
    c->center_anchor = vec3(0.0f);
    c->pivot_anchor_offset = vec3(0.0f);
    c->handler = sys->collision_handler_Circle_default;
    c->circle.center = c->center_anchor;
    c->circle.radius = 0.5f;
    
    return c;
}

void Collider_destroy(Collision_System* sys, Collider* c)
{
    const uint64 ID = c->ID;
    
    sys->to_be_freed_ids.push_back((To_Be_Freed_ID_Record){.id = ID, .step_count = 0 });
    
    mem::deallocate<mtt::Collider>(&sys->collider_pool.allocator, c);
}

void Collision_System_end_step(Collision_System* sys)
{
    auto& to_be_freed_ids = sys->to_be_freed_ids;
    for (auto it = to_be_freed_ids.begin(); it != to_be_freed_ids.end();) {
        if (it->step_count == 4) {
            sys->free_ids.emplace_back(it->id);
            it = to_be_freed_ids.erase(it);
        } else {
            it->step_count += 1;
            ++it;
        }
    }
}

void calc_aabb_from_points(Collider* collider, Array_Slice<vec2> points)
{
    vec2 tl = {POSITIVE_INFINITY, POSITIVE_INFINITY};
    vec2 br = {NEGATIVE_INFINITY, NEGATIVE_INFINITY};
    for (usize i = 0; i < points.count; i += 1) {
        vec2 point = points[i];
        
        tl[0] = m::min(tl[0], point[0]);
        tl[1] = m::min(tl[1], point[1]);
        br[0] = m::max(br[0], point[0]);
        br[1] = m::max(br[1], point[1]);
    }
    
    auto* aabb = &collider->aabb;
    aabb->tl = tl;
    aabb->br = br;
    aabb->half_extent = (br - tl) / 2.0f;
    collider->center_anchor = vec3((tl + br) / 2.0f, collider->center_anchor.z);
}

void calc_aabb_from_points(Collider* collider, Array_Slice<vec3> points)
{
    vec2 tl = {POSITIVE_INFINITY, POSITIVE_INFINITY};
    vec2 br = {NEGATIVE_INFINITY, NEGATIVE_INFINITY};
    for (usize i = 0; i < points.count; i += 1) {
        vec3 point = points[i];
        
        tl[0] = m::min(tl[0], point[0]);
        tl[1] = m::min(tl[1], point[1]);
        br[0] = m::max(br[0], point[0]);
        br[1] = m::max(br[1], point[1]);
    }
    
    auto* aabb = &collider->aabb;
    aabb->tl = tl;
    aabb->br = br;
    aabb->half_extent = (br - tl) / 2.0f;
    aabb->saved_box.tl = tl;
    aabb->saved_box.br = br;
    collider->center_anchor = vec3((tl + br) / 2.0f, collider->center_anchor.z);
}

void Collider_remove(Collision_System* sys, usize layer, Collider* c)
{
    sys = c->system;
    
    Spatial_Grid* grid = &sys->layers[layer];
    ASSERT_MSG(c->bin_keys.size() == 1 || c->bin_keys.size() == 0, "For now, there should only be one grid space...! size=[%llu]\n", c->bin_keys.size());
    for (auto key = c->bin_keys.begin(); key != c->bin_keys.end(); ++key) {
        Spatial_Grid_Cell* block = &grid->collision_map[(uint64)*key];
        std::vector<Collider*>& colliders = block->colliders;
        if (colliders.empty()) {
            continue;
        }
        
        uint64 collider_idx = block->collider_to_idx[c->ID];
        Collider* swap_in = colliders.back();
        block->collider_to_idx[swap_in->ID] = collider_idx;
        colliders[collider_idx] = swap_in;
        colliders.pop_back();
  
        // clearing grid cells should be deferred to the end of the frame
        auto it = grid->active_cells.find((uintptr)*key);
        if (it != grid->active_cells.end()) {
            it->second -= 1;
            if (it->second == 0) {
                //grid->active_cells.erase(it);
                //grid->cells_to_clear.push_back({key, layer});
            }
        }
    }
    
    c->bin_keys.clear();
//    auto result = sys->found_collisions.find(c->ID);
//    if (result != sys->found_collisions.end()) {
//        sys->found_collisions.erase(result);
//    }
}


const Cell_List active_cells(Collision_System* sys, usize layer)
{
    Spatial_Grid* grid = &sys->layers[layer];
    
    return {
        .set = grid->active_cells
        //.cell_ID_list = grid->active_cells.data(),
        //.count        = grid->active_cells.size()
    };
}

usize collision_layer_active_count(Collision_System* system, usize layer)
{
    return system->layers[layer].active_cells.size();
}

Spatial_Grid_Cell* get_cell(Collision_System* sys, usize layer, uint64 key)
{
    Spatial_Grid* grid = &sys->layers[layer];
    
    auto result = grid->collision_map.find(key);
    if (result == grid->collision_map.end()) {
        return nullptr;
    } else {
        return &result->second;
    }
}


bool AABB_AABB_intersection(AABB* a, AABB* b)
{
    if ((a->saved_box.br[0] < b->saved_box.tl[0] || a->saved_box.tl[0] > b->saved_box.br[0]) ||
        (a->saved_box.br[1] < b->saved_box.tl[1] || a->saved_box.tl[1] > b->saved_box.br[1])) {
        return false;
    }
    
    return true;
}
bool Box_Box_intersection(Box* a, Box* b)
{
    if ((a->br[0] < b->tl[0] || a->tl[0] > b->br[0]) ||
        (a->br[1] < b->tl[1] || a->tl[1] > b->br[1])) {
        return false;
    }
    
    return true;
}

bool Box_Box_intersection(float32* a_tl, float32* a_br, float32* b_tl, float32* b_br)
{
    if ((a_br[0] < b_tl[0] || a_tl[0] > b_br[0]) ||
        (a_br[1] < b_tl[1] || a_tl[1] > b_br[1])) {
        return false;
    }
    
    return true;
}

void enclosing_AABB(AABB* a, AABB* b, AABB* out)
{
    out->tl[0] = m::min(a->tl[0], b->tl[0]);
    out->br[0] = m::max(a->br[0], b->br[0]);
    out->tl[1] = m::min(a->tl[1], b->tl[1]);
    out->br[1] = m::max(a->br[1], b->br[1]);
}
void enclosing_Box(Box* a, Box* b, Box* out)
{
    out->tl[0] = m::min(a->tl[0], b->tl[0]);
    out->br[0] = m::max(a->br[0], b->br[0]);
    out->tl[1] = m::min(a->tl[1], b->tl[1]);
    out->br[1] = m::max(a->br[1], b->br[1]);
}





bool Box_Line_Segment_intersection(float32* tl, float32* br, float32* l_a, float32* l_b)
{
    vec2 l_av = vec2(l_a[0], l_a[1]);
    vec2 l_delta = vec2(l_b[0], l_b[1]) - l_av;
    
    auto scale = vec2(1.0) / l_delta;
    
    auto sign = m::sign(scale);
    auto half_x = (br[0] - tl[0]) * 0.5;
    auto half_y = (br[1] - tl[1]) * 0.5;
    auto posx = tl[0] + half_x;
    auto posy = tl[1] + half_y;
    auto near_time_x = (posx - sign.x * (half_x) - l_av[0]) * scale.x;
    auto near_time_y = (posy - sign.y * (half_y) - l_av[1]) * scale.y;
    auto far_time_x = (posx + sign.x * (half_x) - l_av[0]) * scale.x;
    auto far_time_y = (posy + sign.y * (half_y) - l_av[1]) * scale.y;
    
    if (isnan(near_time_y)) {
        near_time_y = POSITIVE_INFINITY;
    }
    if (isnan(far_time_y)) {
        far_time_y = POSITIVE_INFINITY;
    }
    
    if (near_time_x > far_time_y || near_time_y > far_time_x) {
        return false;
    }
    
    const auto near_time = near_time_x > near_time_y ? near_time_x : near_time_y;
    const auto far_time = far_time_x < far_time_y ? far_time_x : far_time_y;
    
    if (near_time >= 1 || far_time <= 0) {
        return false;
    }
    
    return true;
    
    vec2 dir = m::normalize(vec2(l_b[0], l_b[1]) - vec2(l_a[0], l_a[1]));
    vec2 dir_frac = 1.0f / dir;
    
    float32 t1 = (tl[0] - l_a[0]) * dir_frac.x;
    float32 t2 = (br[0] - l_b[0]) * dir_frac.x;
    float32 t3 = (tl[1] - l_a[1]) * dir_frac.y;
    float32 t4 = (br[1] - l_b[1]) * dir_frac.y;
    
    float32 tmin = m::max(m::min(t1, t2), m::min(t3, t4));
    float32 tmax = m::min(m::max(t1, t2), m::max(t3, t4));
    
    if (tmax < 0.0f || tmin > tmax) {
        return false;
    }

    return true;
}


bool AABB_Line_Segment_intersection(AABB* box, Line_Segment* s, Hit* hit)
{
    //    vec2 box_half_pos = (box->tl + box->br) / 2.0;
    //    vec2 width = ((box->br - box->tl) / 2.0);
    ASSERT_MSG(false, "broken");
    vec2 dir = m::normalize(s->b - s->a);
    vec2 dir_frac = 1.0f / dir;
    
    float32 t1 = (box->tl.x - s->a.x) * dir_frac.x;
    float32 t2 = (box->br.x - s->a.x) * dir_frac.x;
    float32 t3 = (box->tl.y - s->a.y) * dir_frac.y;
    float32 t4 = (box->br.y - s->a.y) * dir_frac.y;
    
    float32 tmin = m::max(m::min(t1, t2), m::min(t3, t4));
    float32 tmax = m::min(m::max(t1, t2), m::max(t3, t4));
    
    float32 t = 0.0f;
    if (tmax < 0.0f) {
        t = tmax;
        hit->t = t;
        return false;
    }
    
    if (tmin > tmax) {
        t = tmax;
        hit->t = t;
        return false;
    }
    
    t = tmin;
    hit->t = t;
    return true;
}

bool AABB_Circle_intersection(AABB* box, Circle* circle)
{
    float32 x = m::max(box->tl.x, m::min(circle->center.x, box->br.x));
    float32 y = m::max(box->tl.y, m::min(circle->center.y, box->br.y));
    
    float32 square_distance = (x - circle->center.x) * (x - circle->center.x) +
    (y - circle->center.y) * (y - circle->center.y);
    
    return square_distance < circle->radius * circle->radius;;
}
bool AABB_Point_intersection(AABB* box, Point* point, Hit* hit)
{
    vec2 box_half_pos = (box->tl + box->br) / 2.0;
    vec2 box_half_dims = (box->br - box->tl) / 2.0;
    
    float32 dx = point->coord.x - box_half_pos.x;
    float32 px = box_half_dims.x - m::abs(dx);
    if (px <= 0) {
        return false;
    }
    
    float32 dy = point->coord.y - box_half_pos.y;
    float32 py = box_half_dims.y - m::abs(dy);
    if (py <= 0) {
        return false;
    }
    
    if (px < py) {
        float32 sx = m::sign(dx);
        hit->delta.x = px * sx;
        hit->normal.x = sx;
        hit->pos.x = box_half_pos.x + (((box->br.x - box->tl.x) / 2.0) * sx);
        hit->pos.y = point->coord.y;
    } else {
        float32 sy = m::sign(dy);
        hit->delta.x = px * sy;
        hit->normal.y = sy;
        hit->pos.x = point->coord.x;
        hit->pos.y = box_half_pos.y + (((box->br.y - box->tl.y) / 2.0) * sy);
    }
    
    return true;
    
    //    return (point->coord.x >= box->tl.x && point->coord.x <= box->br.x) &&
    //            (point->coord.y >= box->tl.y && point->coord.y <= box->br.y);
}

bool Box_point_check(float box[4], vec2 point)
{
    vec2 tl = vec2(box[0], box[1]);
    vec2 br = vec2(box[2], box[3]);
    vec2 box_half_pos = (tl + br) / 2.0;
    vec2 box_half_dims = (br - tl) / 2.0;
    
    float32 dx = point.x - box_half_pos.x;
    float32 px = box_half_dims.x - m::abs(dx);
    if (px <= 0) {
        return false;
    }
    
    float32 dy = point.y - box_half_pos.y;
    float32 py = box_half_dims.y - m::abs(dy);
    if (py <= 0) {
        return false;
    }
    
    return true;
}

bool Box_Circle_intersection(float box[4], Circle* circle)
{
    float32 x = m::max(box[0], m::min(circle->center.x, box[2]));
    float32 y = m::max(box[1], m::min(circle->center.y, box[3]));
    
    float32 square_distance = (x - circle->center.x) * (x - circle->center.x) +
    (y - circle->center.y) * (y - circle->center.y);
    
    return square_distance < circle->radius * circle->radius;;
}


bool Line_Segment_intersection_query(Line_Segment* s0, Line_Segment* s1, Hit* out)
{
    f64 Ax = s0->a.x;
    f64 Ay = s0->a.y;
    f64 Bx = s0->b.x;
    f64 By = s0->b.y;
    
    f64 Cx = s1->a.x;
    f64 Cy = s1->a.y;
    f64 Dx = s1->b.x;
    f64 Dy = s1->b.y;
    
    f64 distAB, theCos, theSin, newX, ABpos;
    
    // if ((Ax == Bx && Ay == By) || (Cx == Dx && Cy == Dy)) {
    //     return false;
    // }
    
    // //  Fail if the segments share an end-point.
    // if ((Ax==Cx && Ay==Cy) || (Bx==Cx && By==Cy) ||
    //     (Ax==Dx && Ay==Dy) || (Bx==Dx && By==Dy)) {
    //     return false;
    // }
    
    //  (1) Translate the system so that point A is on the origin.
    Bx -= Ax;
    By -= Ay;
    
    Cx -= Ax;
    Cy -= Ay;
    
    Dx -= Ax;
    Dy -= Ay;
    
    //  Discover the length of segment A-B.
    distAB = m::sqrt(Bx*Bx + By*By);
    
    //  (2) Rotate the system so that point B is on the positive X axis.
    theCos = Bx / distAB;
    theSin = By / distAB;
    newX   = Cx*theCos + Cy*theSin;
    Cy     = Cy*theCos - Cx*theSin;
    Cx = newX;
    newX = Dx*theCos + Dy*theSin;
    Dy   = Dy*theCos - Dx*theSin;
    Dx = newX;
    
    //  Fail if segment C-D doesn't cross line A-B.
    if ((Cy < 0.0 && Dy < 0.0) || (Cy >= 0.0 && Dy >= 0.0)) {
        return false;
    }
    
    //  (3) Discover the position of the intersection point along line A-B.
    ABpos = Dx + (Cx-Dx)*Dy / (Dy-Cy);
    
    //  Fail if segment C-D crosses line A-B outside of segment A-B.
    if (ABpos < 0.0 || ABpos > distAB) {
        return false;
    }
    
    //  (4) Apply the discovered position to line A-B in the original coordinate system.
    out->pos.x =Ax+ABpos * theCos;
    out->pos.y =Ay+ABpos * theSin;
    
    //  Success.
    return true;
}

bool Line_Segment_intersection(Line_Segment* s0, Line_Segment* s1, Hit* out)
{
    vec2 b = s0->b - s0->a;
    vec2 d = s1->b - s1->a;
    
    float32 cross = b.x * d.y - b.y * d.x;
    
    if (cross == 0) {
//        Point point = {};
//        point.coord = s1->a;
//        Point point2 = {};
//        point2.coord = s1->b;
//        if (Line_Segment_Point_intersection(s0, &point)) {
//            out->pos = s1->a;
//            out->t = INFINITY;
//            return true;
//        } else if (Line_Segment_Point_intersection(s0, &point2)) {
//            out->pos = s1->b;
//            out->t = INFINITY;
//            return true;
//        } else {
//            return false;
//        }
        
        if (s0->a.x == s1->b.x) {
            if (s0->a.y >= s1->a.y && s0->a.y <= s1->b.y) {
                out->pos = vec2(s0->a.x, s0->a.y);
                out->t = INFINITY;
                return true;
            } else {
                out->pos = vec2(s0->b.x, s0->b.y);
                out->t = INFINITY;
                return true;
            }
        } else {
            if (s0->a.x >= s1->a.x && s0->a.x <= s1->b.x) {
                out->pos = vec2(s0->a.x, s0->a.y);
                out->t = INFINITY;
                return true;
            } else {
                out->pos = vec2(s0->b.x, s0->b.y);
                out->t = INFINITY;
                return true;
            }
        }
        
//        if (line1.x0 == line1.x1) {
//            if (line1.y0 >= line2.y0 && line1.y0 <= line2.y1)
//            {
//                out->pos= (line1.x0,line1.y0), out->t=0;
//            }
//            else
//            {
//                out->pos= (line1.x1,line1.y1), out->t=0;
//            }
//        } else {
//            if (line1.x0 >= line2.x0 && line1.x0 <= line2.x1)
//            {
//                out->pos= (line1.x0,line1.y0), out->t=0;
//            }
//            else
//            {
//                out->pos= (line1.x1,line1.y1), out->t=0;
//            }
//        }
        return false;
    }
    
    vec2 c = s1->a - s0->a;
    float32 t = (c.x * d.y - c.y * d.x) / cross;
    if (t < 0 || t > 1) {
        return false;
    }
    
    float u = (c.x * b.y - c.y * b.x) / cross;
    if (u < 0 || u > 1) {
        return false;
    }
    
    out->pos = s0->a + t * b;
    out->t = t;
    
    return true;
}


bool Line_Segment_Circle_intersection(Line_Segment* s, Circle* c)
{
    float32 distance_squared = m::dist_to_segment_squared(s->a, s->b, c->center);
    
    return distance_squared < c->radius * c->radius;
}

bool Line_Segment_Point_intersection(Line_Segment* s, Point* p)
{
    float32 cross = (p->coord.y - s->a.y) * (s->b.x - s->a.x) - (p->coord.x - s->a.x) * (s->b.y - s->a.y);
    if (m::abs(cross) > 0.001) {
        return false;
    }
    
    float32 dot = (p->coord.x - s->a.x) * (s->b.x - s->a.x) + (p->coord.y - s->a.y) * (s->b.y - s->a.y);
    if (dot < 0) {
        return false;
    }
    
    float32 sqr_len = (s->b.x - s->a.x) * (s->b.x - s->a.x) + (s->b.y - s->a.y) * (s->b.y - s->a.y);
    if (dot > sqr_len) {
        return false;
    }
    
    return true;
}

bool Circle_Circle_intersection(Circle* a, Circle* b)
{
    float32 square_distance = m::dist_squared(a->center, b->center);
    
    return square_distance < (a->radius + b->radius) * (a->radius + b->radius);
}
bool Circle_Circle_intersection(Circle* a, Circle* b, float32* out_square_distance)
{
    float32 square_distance = m::dist_squared(a->center, b->center);
    
    *out_square_distance = square_distance;
    
    return square_distance < (a->radius + b->radius) * (a->radius + b->radius);
}

bool Circle_Point_intersection(Circle* circle, Point* point)
{
    float32 square_distance = m::dist_squared(circle->center, point->coord);
    
    return square_distance < circle->radius * circle->radius;
}

bool Circle_Concave_Poly_intersection(Circle* circle, Concave_Hull* poly)
{
    // assume 3 points at least
    auto& points = (poly->points);
    
    // check if circle intersects any of the polygon's line segments
    {
        Line_Segment seg;
        
        for (usize i = 0; i < poly->size() - 1; i += 1) {
            seg.a = points[i];
            seg.b = points[i + 1];
            
            // do center + radius^2 against segment check
            if (Line_Segment_Circle_intersection(&seg, circle)) {
                return true;
            }
        }
        
        seg.a = points[poly->size() - 1];
        seg.b = points[0];
        if (Line_Segment_Circle_intersection(&seg, circle)) {
            return true;
        }
    }
    
    Point point;
    point.coord = circle->center;
    return Point_Concave_Poly_intersection(&point, poly);
}


bool Point_Quad_intersection(vec2 point, vec2 quad_tl, vec2 quad_bl, vec2 quad_br, vec2 quad_tr)
{
    isize i, j, c = 0;
    float32 x = point.x;
    float32 y = point.y;
    
    vec2 points[4] = {quad_tl, quad_bl, quad_br, quad_tr};
    constexpr const isize npol = 4;
    
    for (i = 0, j = npol-1; i < npol; j = i++) {
        vec2* pi = &points[i];
        vec2* pj = &points[j];
        if ((((pi->y <= y) && (y < pj->y)) ||
             ((pj->y <= y) && (y < pi->y))) &&
            (x < (pj->x - pi->x) * (y - pi->y) / (pj->y - pi->y) + pi->x))
            c = !c;
    }
    return c;
}

bool Point_Quad_intersection(Point* point, Quad* quad)
{
    return Point_Quad_intersection(point->coord, quad->tl, quad->bl, quad->br, quad->tr);
}


bool Point_Concave_Poly_intersection(Point* point, Concave_Hull* poly/*int npol, float *xp, float *yp, float x, float y*/)
{
    isize i, j, c = 0;
    float32 x = point->coord.x;
    float32 y = point->coord.y;
    
    auto& points = poly->points;
    isize npol = poly->size();
    
    for (i = 0, j = npol-1; i < npol; j = i++) {
        vec2* pi = &points[i];
        vec2* pj = &points[j];
        if ((((pi->y <= y) && (y < pj->y)) ||
             ((pj->y <= y) && (y < pi->y))) &&
            (x < (pj->x - pi->x) * (y - pi->y) / (pj->y - pi->y) + pi->x))
            c = !c;
    }
    return c;
}

Collision_Handler_Result collision_handler_no_op(Broad_Phase_Collision_Record* collisions, Collision_System* system)
{
    return true;
}


void OBB_compute_axes(AABB* box)
{
    auto* axis = box->axis;
    axis[0] = vec2(box->saved_quad.tr) - vec2(box->saved_quad.tl);
    axis[1] = vec2(box->saved_quad.bl) - vec2(box->saved_quad.tl);
    
    auto* origin = box->origin;
    
    for (int a = 0; a < 2; a += 1) {
        axis[a] /= m::length2(axis[a]);
        origin[a] = m::dot(box->saved_quad.tl, axis[a]);
    }
    
}

bool OBB_overlap_1way(AABB* boxa, AABB* boxb)
{
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
    
    const vec2* axis = boxa->axis;
    const vec2 corner[4] = {boxb->saved_quad.tl, boxb->saved_quad.bl, boxb->saved_quad.br, boxb->saved_quad.tr};
    for (int a = 0; a < 2; a += 1) {
        float64 t = m::dot(corner[0], axis[a]);
        
        float64 tMin = t;
        float64 tMax = t;
        
        for (int c = 1; c < 4; c += 1) {
            t = m::dot(corner[c], axis[a]);
            
            if (t < tMin) {
                tMin = t;
            } else if (t > tMax) {
                tMax = t;
            }
        }
        
        if ((tMin > 1 + boxa->origin[a]) || (tMax < boxa->origin[a])) {
            return false;
        }
    }
    
    return true;
}
bool OBB_OBB_intersection(AABB* a, AABB* b)
{
    return AABB_AABB_intersection(a, b) && OBB_overlap_1way(a, b) && OBB_overlap_1way(b, a);
}

bool Collider_is_pushed(Collider* c)
{
    return !c->bin_keys.empty();
}




bool rectangle_ray_intersection(vec2 rayOrigin, vec2 rayDir, vec2 boxMin, vec2 boxMax, vec2* out)
{
    vec2 invDir = vec2(1.0) / rayDir;
    vec2 tMin = (boxMin - rayOrigin) * invDir;
    vec2 tMax = (boxMax - rayOrigin) * invDir;
    vec2 t1 = min(tMin, tMax);
    vec2 t2 = max(tMin, tMax);
    float32 tNear = m::max(t1.x, t1.y);
    float32 tFar = m::min(t2.x, t2.y);
    //return vec2(tNear, tFar);
    
    bool hit = (tFar >= 0) && (tFar >= tNear);
    if (hit) {
        *out = vec2(tNear, tFar);
    }
    return hit;
};


vec2 rectangle_ray_intersection_interior_guaranteed(vec2 rayOrigin, vec2 rayDir, vec2 boxMin, vec2 boxMax)
{
    vec2 invDir = vec2(1.0) / rayDir;
    vec2 tMin = (boxMin - rayOrigin) * invDir;
    vec2 tMax = (boxMax - rayOrigin) * invDir;
    vec2 t1 = min(tMin, tMax);
    vec2 t2 = max(tMin, tMax);
    float32 tNear = m::max(t1.x, t1.y);
    float32 tFar = m::min(t2.x, t2.y);
    
    return vec2(tNear, tFar);
}

}
