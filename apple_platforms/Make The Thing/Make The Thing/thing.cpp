//
//  thing.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/5/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "thing.hpp"
#include "thing_private.hpp"

#ifndef NDEBUG
namespace std {
template class std::vector<mtt::Connection>;
}

template class robin_hood::detail::Table<true, 80, unsigned long long, std::vector<mtt::Connection>, robin_hood::hash<unsigned long long>, std::equal_to<unsigned long long>>;

#endif

namespace mtt {



mtt::Thing_ID id(mtt::Thing* thing)
{
    return thing->id;
}

bool Thing_try_get(mtt::World* world, Thing_ID id, Thing** thing)
{
    return map_try_get(world->things.instances, id, thing);
}

Thing* Thing_try_get(mtt::World* world, Thing_ID id)
{
    mtt::Thing* thing;
    map_try_get(world->things.instances, id, &thing);
    return thing; 
}

MTT_NODISCARD Thing_Proxy_Storage* Thing_proxies_try_get(mtt::Thing* thing)
{
    auto* world = mtt::world(thing);
    Thing_Proxy_Storage* proxies = nullptr;
    map_try_get(&world->things.thing_to_proxy_map, thing->id, &proxies);
    return proxies;
}

MTT_NODISCARD Thing_Proxy_Storage* Thing_proxies_try_get(mtt::World* world, mtt::Thing_ID id)
{
    Thing_Proxy_Storage* proxies = nullptr;
    mtt::Thing* thing = nullptr;
    mtt::Thing_try_get(world, id, &thing);
    if (thing == nullptr) {
        return nullptr;
    }
    map_try_get(&world->things.thing_to_proxy_map, thing->id, &proxies);
    return proxies;
}

MTT_NODISCARD Thing_Proxy_Storage* Thing_proxies_try_get_for_scene(mtt::Thing* thing, usize scene_idx)
{
    auto* world = mtt::world(thing);
    Thing_Proxy_Storage* proxies = nullptr;
    map_try_get(&world->things.proxy_scenes[scene_idx].thing_to_proxy_map, thing->id, &proxies);
    return proxies;
}

MTT_NODISCARD Thing_Proxy_Storage* Thing_proxies_try_get_for_scene(mtt::World* world, mtt::Thing_ID id, usize scene_idx)
{
    Thing_Proxy_Storage* proxies = nullptr;
    mtt::Thing* thing = nullptr;
    mtt::Thing_try_get(world, id, &thing);
    if (thing == nullptr) {
        return nullptr;
    }
    map_try_get(&world->things.proxy_scenes[scene_idx].thing_to_proxy_map, thing->id, &proxies);
    return proxies;
}

Thing* Thing_get(mtt::World* world, Thing_ID id)
{
    mtt::Thing* thing;
    map_get(world->things.instances, id, &thing);
    return thing;
}

void Thing_get(mtt::World* world, Thing_ID id, Thing** thing)
{
    map_get(world->things.instances, id, thing);
}

bool Thing_Archetype_try_get(mtt::World* world, Thing_Archetype_ID id, Thing_Archetype** arch)
{
    return map_try_get(world->archetypes.instances, id, arch);
}

void Thing_Archetype_get(mtt::World* world, Thing_Archetype_ID id, Thing_Archetype** arch)
{
    map_get(world->archetypes.instances, id, arch);
}

mtt::World* world(mtt::Thing* thing)
{
    return thing->world();
}

Field_List& default_field_list(mtt::Thing* thing)
{
    return thing->field_descriptor.data;
}
void set_active_fields_to_default(mtt::Thing* thing)
{
    thing->active_fields = thing->field_descriptor.data;
}
void set_active_fields(mtt::Thing* thing, Field_List* field_list)
{
    thing->active_fields = *field_list;
}

Field_List* fields(mtt::Thing* thing)
{
    return &thing->active_fields;
}

void make_temp_array_for_ports(mtt::Any* out, usize count)
{
    mtt::World* world = mtt::ctx();
    out->type = MTT_LIST;
    out->contained_type = MTT_ANY;
    
    auto* arr = mem::alloc_init<mtt::Dynamic_Array<mtt::Any>>(&world->allocator_temporary());
    out->List = (uintptr)arr;
    mtt::init_from_ptr(arr, mem::alloc_array<mtt::Any>(&world->allocator_temporary(), count), count, count);
}
void make_temp_thing_list_for_ports(mtt::Any* out, usize count)
{
    mtt::World* world = mtt::ctx();
    out->type = MTT_LIST;
    out->contained_type = MTT_THING;
    
    auto* arr = mem::alloc_init<mtt::Dynamic_Array<mtt::Thing_Ref>>(&world->allocator_temporary());
    out->List = (uintptr)arr;
    mtt::init_from_ptr(arr, mem::alloc_array<mtt::Thing_Ref>(&world->allocator_temporary(), count), count, count);
}



bool remove_all_connections(World* world, Thing_ID thing_id);

bool Thing_get(usize i, Thing* container, Thing** dst)
{
    mtt::Thing_Ref* source = mtt::access<mtt::Thing_Ref>(container, "thing");
    
    return (source == nullptr) ? false : source->try_get(dst);
}

MTT_NODISCARD
mtt::Thing* Thing_make(World* world, Thing_Archetype_ID arch_id, const Thing_Make_Args& args)
{
    mtt::Thing* out = nullptr;
    Thing_make(world, arch_id, &out, args);
    return out;
}

void init_properties(mtt::Thing* thing)
{
    
    if (mtt::is_actor(thing)) {
        thing->world()->things.properties.insert({thing->id, Script_Lookup()});
    }
}
void deinit_properties(mtt::Thing* thing)
{
    if (mtt::is_actor(thing)) {
        auto* world = thing->world();
        auto* lookup = &world->things.properties;
        auto found = lookup->find(thing->id);
        if (found != lookup->end()) {
            lookup->erase(found);
        }
    }
}
void clear_properties(mtt::Thing* thing)
{
    if (mtt::is_actor(thing)) {
        auto* world = thing->world();
        auto* lookup = &world->things.properties;
        auto found = lookup->find(thing->id);
        if (found != lookup->end()) {
            found->second.clear();
        }
    }
}
void copy_properties(mtt::Thing* thing_dst, mtt::Thing* thing_src)
{
    if (mtt::is_actor(thing_dst) && thing_dst->archetype_id == thing_src->archetype_id) {
        auto& lookup = thing_dst->world()->things.properties;
        lookup[thing_dst->id] = lookup[thing_src->id];
    }
}

template <typename type>
void field_copy_array(mtt::Thing* to, mtt::Thing* from, const mtt::String& label)
{
    auto** ptr_from = mtt::access_pointer_to_pointer<mtt::Dynamic_Array<type>*>(from, label);
    
    auto** ptr_to = mtt::access_pointer_to_pointer<mtt::Dynamic_Array<type>*>(to, label);
    
    clone(*ptr_to, *ptr_from);
};

void field_copy_thing_list(mtt::Thing* to, mtt::Thing* from, const mtt::String& label)
{
    auto* ptr_from = mtt::access<mtt::Thing_List>(from, label);
    auto* ptr_to = mtt::access<mtt::Thing_List>(to, label);
    clone(ptr_to, ptr_from);
    
}


Thing_ID Thing_make(World* world, Thing_Archetype_ID arch_id, Thing** out, const Thing_Make_Args& args)
{
    Thing_ID next_id = world->things.next_thing_id;
    world->things.next_thing_id += 1;
    
    world->things.instances.insert({next_id, Thing()});
    Thing& thing_ref = world->things.instances.find(next_id)->second;
    thing_ref.archetype_id = arch_id;
    thing_ref.id = next_id;
    //thing_ref.group_id    = 1;
//    world->things.groups.find(thing_ref.group_id)->second.things.emplace(thing_ref.id);
    thing_ref.eval_index = 0;
    
    Thing_Archetype& arch = world->archetypes.instances[arch_id];
    copy_field_list(world, field_allocator(world), &thing_ref.field_descriptor, &arch.field_descriptor);
    mtt::set_active_fields_to_default(&thing_ref);
    
    thing_ref.is_resident = true;
    
    thing_ref.is_root    = true;
    thing_ref.is_visible = true;
    thing_ref.is_locked  = false;
    thing_ref.is_visited = false;
    thing_ref.is_user_drawable = true;
    thing_ref.is_user_destructible = true;
    thing_ref.parent_thing_id = mtt::Thing_ID_INVALID;
    thing_ref.first_child = mtt::Thing_ID_INVALID;
    thing_ref.do_evaluation = true;
    thing_ref.is_user_movable = true;
    thing_ref.graph = curr_graph(world);
    thing_ref.lock_user_movement_if_not_root = false;
    thing_ref.lock_to_canvas                 = false;
    thing_ref.forward_input_to_root          = false;
    
    
    mtt::init_flag(&thing_ref);
    mtt::set_flag(&thing_ref,
                  (THING_FLAG)(
                               THING_FLAG_is_active |
                               THING_FLAG_is_resident |
                               THING_FLAG_is_active_group |
                               THING_FLAG_is_root |
                               THING_FLAG_is_visible |
                               THING_FLAG_is_locked |
                               THING_FLAG_is_user_drawable |
                               THING_FLAG_is_user_destructible |
                               THING_FLAG_do_evaluation |
                               THING_FLAG_is_user_movable |
                               THING_FLAG_is_user_erasable)
                  );
    
    //mtt::set_is_active(&thing_ref);
    //mtt::set_is_active_group(&thing_ref);
    
    
    //world->things.thing_to_run_ctx_specific_flags[thing_ref.id][ctx] = thing_ref.flags;
    
    
    thing_ref.root_thing_id   = mtt::Thing_ID_INVALID;
    thing_ref.next_id   = mtt::Thing_ID_INVALID;
    thing_ref.prev_id   = mtt::Thing_ID_INVALID;
    thing_ref.mapped_thing_id = mtt::Thing_ID_INVALID;
    
    
    thing_ref.ports  = arch.port_descriptor;
    
    
    thing_ref.graph->incoming[thing_ref.id].resize(thing_ref.ports.in_ports.size());
    thing_ref.graph->outgoing[thing_ref.id].resize(thing_ref.ports.out_ports.size());
    
    thing_ref.eval_out = &thing_ref.graph->output;
    
    thing_ref.eval_priority = arch.eval_priority;
    
    thing_ref.logic  = arch.logic;
    thing_ref.message_handler = arch.message_handler;
    thing_ref.on_destroy = arch.on_thing_destroy;
    
    arch.instance_ids.insert(next_id);
    
    //    world->things.representation.emplace(thing_ref.id, mtt::Representation());
    
    thing_ref.input_handlers = arch.input_handlers;
    
    if (current_is_root_graph(world)) {
        thing_ref.ctx_id = Context_ID_ROOT;
    } else {
        thing_ref.ctx_id = Context_ID_SUB;
    }
    
    //MTT_print("You made a thing, yay!\n");
    
    // must be global context
    thing_ref.graph->is_modified = true;
    
    if (out != nullptr) {
        *out = &thing_ref;
    }
    
    
    
    
    thing_ref.ecs_entity = Entity(world->ecs_world, ("MTTID" + std::to_string(next_id)).c_str());
    thing_ref.ecs_entity.set<Thing_Info>({.thing_id = next_id, .world = world});
    
    
    auto* id_is = dt::attribute_add("id_is");
    thing_ref.ecs_entity.add(id_is->typename_desc, thing_ref.ecs_entity);
    
    init_properties(&thing_ref);
    
    
    if (world->on_thing_make != nullptr) {
        world->on_thing_make(world, &thing_ref);
    }
    
    arch.on_thing_make(&thing_ref);
    
    world->custom_on_thing_make(&thing_ref);
    
    
    world->things.count += 1;
    
    thing_ref.selection_count = 0;
    
    //MTT_print("Made thing with ID=[%llu], type=[%s]\n", next_id, arch_to_name(arch_id));
    return next_id;
}

Thing* Thing_make_with_collider(World* world, mtt::Rep** rep, mtt::ARCHETYPE arch, mtt::COLLIDER_TYPE collider_type, bool is_in_world, mtt::Collision_System* collision_system)
{
    
    mtt::Thing* out = Thing_make(world, arch);
    
    mtt::Collider* collider = nullptr;
    
    mtt::rep(out, rep);
    
    if (collision_system == nullptr) {
        collision_system = (is_in_world) ? &world->collision_system : & world->collision_system_canvas;
    }
    
    switch (collider_type) {
    case COLLIDER_TYPE_AABB:
        collider = Collider_make_aabb(collision_system);
        break;
    case COLLIDER_TYPE_CIRCLE: {
        collider = Collider_make_circle(collision_system);
        break;
    }
    default: {
        collider->handler = mtt::collision_handler_no_op;
        break;
    }
    }
    
    (*rep)->colliders.emplace_back(collider);
    collider->user_data = (void*)((out)->id);
    
    return out;
}


Thing* Thing_copy(Thing* thing)
{
    ASSERT_MSG(!Thing_is_proxy(thing), "should not copy a proxy!");

    mtt::World* world = mtt::world(thing);

    auto* p = get_parent(thing);
    if (p != nullptr) {
        mtt::disconnect_child_from_parent(world, thing);
    }
    
    Thing* thing_copy = mtt::Thing_make(world, thing->archetype_id);
    thing_copy->is_user_drawable = thing->is_user_drawable;
    thing_copy->logic = thing->logic;
    thing_copy->flags = thing->flags;
    
    {
        cstring cstr = MTT_string_ref_to_cstring(thing->label);
        if (thing->label.id == 0) {
            mtt::Thing_set_label(thing_copy, "");
        } else {
            mtt::Thing_set_label(thing_copy, cstr);
        }
        
    }
    
    auto& src_rep = *mtt::rep(thing);
    auto& copy_rep = *mtt::rep(thing_copy);
    
    copy_properties(thing_copy, thing);
    
    
    {
        copy_rep.model_transform         = src_rep.model_transform;
        copy_rep.model_transform_inverse = src_rep.model_transform_inverse;
        
        
        copy_rep.hierarchy_model_transform         = src_rep.hierarchy_model_transform;
        //copy_rep.world_transform_inverse = src_rep.world_transform_inverse;
        
        copy_rep.pose_transform          = src_rep.pose_transform;
        copy_rep.offset                  = src_rep.offset;
        
        
        
        copy_rep.points                  = src_rep.points;
        copy_rep.points_alt              = src_rep.points_alt;
        copy_rep.radii                   = src_rep.radii;
        copy_rep.colors                  = src_rep.colors;
        copy_rep.representation_types    = src_rep.representation_types;
        
        copy_rep.velocity            = src_rep.velocity;
        copy_rep.acceleration        = src_rep.acceleration;
        copy_rep.saved_velocity      = src_rep.saved_velocity;
        copy_rep.saved_acceleration  = src_rep.saved_acceleration;
        
        copy_rep.init_forward_dir = src_rep.init_forward_dir;
        copy_rep.forward_dir   = src_rep.forward_dir;
        copy_rep.center_offset = src_rep.center_offset;
        
        copy_rep.offset_align = src_rep.offset_align;
        
        copy_rep.transform = src_rep.transform;
        copy_rep.pose_transform_values = src_rep.pose_transform_values;
        //copy_rep.local_transform_values = src_rep.local_transform_values;
        
        //copy_rep.pose_transform_original = src_rep.pose_transform_original;
//        copy_rep.pose_transform_values_original = src_rep.pose_transform_values_original;
        
        for (auto c_it = copy_rep.colliders.begin(); c_it != copy_rep.colliders.end(); ++c_it) {
            Collider_remove((*c_it)->system, 0, *c_it);
            Collider_destroy((*c_it)->system, *c_it);
        }
        copy_rep.colliders.clear();
        
        if (thing->archetype_id == mtt::ARCHETYPE_NUMBER) {
            auto* old_value = mtt::access<float32>(thing, "value");
            auto* value = mtt::access<float32>(thing_copy, "value");
            *value = *old_value;
        }
        
        for (auto c_it = src_rep.colliders.begin(); c_it != src_rep.colliders.end(); ++c_it) {
            Collider* c_src = *c_it;
            Collider* c_cpy = Collider_copy(c_src, (void*)thing_copy->id);
            
            copy_rep.colliders.push_back(c_cpy);
            
            Collider_push(c_cpy);
        }
        
        copy_rep.render_data.is_shared = src_rep.render_data.is_shared;
        //copy_rep.render_data.layer = src_rep.render_data.layer;
        
        copy_rep.shader_id = src_rep.shader_id;
        
        //copy_rep.render_data.drawable_info_list.resize(src_rep.render_data.drawable_info_list.size());
        src_rep.render_data.on_copy(static_cast<void*>(thing_copy), copy_rep.render_data, (void*)-1);
        
        copy_rep.render_data.set_on_copy(src_rep.render_data.get_on_copy());
        
        auto* pos = mtt::access<vec3>(thing, "position");
        auto* pos_copy = mtt::access<vec3>(thing_copy, "position");
        if (pos != nullptr && pos_copy != nullptr) {
            *pos_copy = *pos;
        }
    }
    
    auto& thing_to_word = dt::DrawTalk::ctx()->lang_ctx.dictionary.thing_to_word;
    auto words = thing_to_word.find(thing->id);
    if (words != thing_to_word.end()) {
        //MTT_print("WORDS SIZE: %zu\n", words->second.size());
        for (auto w_it = words->second.begin(); w_it != words->second.end(); ++w_it) {
            dt::vis_word_derive_from(thing_copy, *w_it);
        }
    }
    
    dt::Thing_copy_own_attributes(thing, thing_copy);
    
    if (Thing_is_proxy(thing)) {
        ASSERT_MSG(false, "proxy should not be copied");
//        mtt::Thing_set_is_proxy(thing_copy, thing);
//        Thing_Proxy_Storage* proxies = nullptr;
//        mtt::map_try_get(&world->things.thing_to_proxy_map, thing->mapped_thing_id, &proxies);
//        ASSERT_MSG(proxies != nullptr, "if is a proxy, the list should already exist");
//        proxies->insert(thing_copy->id);
    }
    
    Thing_Archetype* arch;
    world->Thing_Archetype_get(thing_copy->archetype_id, &arch);
    arch->on_thing_copy(thing_copy, thing);
    
    if (p != nullptr) {
        mtt::connect_parent_to_child(world, p, thing);
    }
    
//    mtt::set_pose_transform(thing_copy, m::scale(copy_rep.pose_transform, vec3(0.5f, 0.5f, 1.0f)));
    thing_copy->mapped_thing_id = thing->mapped_thing_id;
    
    mtt::match_should_render(thing_copy, thing);
    
    return thing_copy;
}

Thing* Thing_copy(World* world, Thing_ID ID)
{
    Thing* thing = world->Thing_try_get(ID);
    if (thing == nullptr) {
        return nullptr;
    }
    
    return Thing_copy(thing);
}

Thing* Thing_copy_recursively(Thing* thing)
{
    mtt::World* world = mtt::world(thing);
    
    Thing* copy = Thing_copy(thing);
    
    auto saved_child_set = thing->child_id_set;
    for (auto c_it = saved_child_set.begin(); c_it != saved_child_set.end(); ++c_it) {
        Thing* child = thing->world()->Thing_try_get(*c_it);
        if (child == nullptr) {
            continue;
        }
        
        mtt::Thing* sub_copy = Thing_copy_recursively(child);
        
        mtt::connect_parent_to_child(world, copy, sub_copy);
        
//        copy->child_id_set.push_back(sub_copy->id);
//        sub_copy->parent_thing_id = copy->id;
        sub_copy->ecs_entity.add(thing->world()->ecs_world.lookup("ChildOfTransitive"), copy->ecs_entity);
    }
    
    return copy;
}

Thing* Thing_copy_recursively(World* world, Thing_ID ID)
{
    Thing* thing = world->Thing_try_get(ID);
    if (thing == nullptr) {
        return nullptr;
    }
    
    return Thing_copy_recursively(thing);
}

Thing* Thing_make_proxy(Thing* thing, const Thing_Make_Proxy_Args& args)
{
    ASSERT_MSG(!Thing_is_proxy(thing), "should not proxy a proxy!");
    
    mtt::World* world = mtt::world(thing);
    auto* p = get_parent(thing);
    if (p != nullptr) {
        mtt::disconnect_child_from_parent(world, thing);
    }
    
    Thing* thing_copy = mtt::Thing_make(world, thing->archetype_id);
    
    thing_copy->is_user_drawable = thing->is_user_drawable;
    thing_copy->logic = thing->logic;
    
    {
        cstring cstr = MTT_string_ref_to_cstring(thing->label);
        if (thing->label.id == 0) {
            mtt::Thing_set_label(thing_copy, "");
        } else {
            mtt::Thing_set_label(thing_copy, cstr);
        }
        
    }
    
    auto& src_rep = *mtt::rep(thing);
    auto& copy_rep = *mtt::rep(thing_copy);
    
    copy_properties(thing_copy, thing);
    
    {
        copy_rep.model_transform         = src_rep.model_transform;
        copy_rep.model_transform_inverse = src_rep.model_transform_inverse;
        
        
        copy_rep.hierarchy_model_transform         = src_rep.hierarchy_model_transform;
        //copy_rep.world_transform_inverse = src_rep.world_transform_inverse;
        
        copy_rep.pose_transform          = src_rep.pose_transform;
        copy_rep.offset                  = src_rep.offset;
        
        
        
        copy_rep.points                  = src_rep.points;
        copy_rep.points_alt              = src_rep.points_alt;
        copy_rep.radii                   = src_rep.radii;
        copy_rep.colors                  = src_rep.colors;
        copy_rep.representation_types    = src_rep.representation_types;
        
        copy_rep.velocity            = src_rep.velocity;
        copy_rep.acceleration        = src_rep.acceleration;
        copy_rep.saved_velocity      = src_rep.saved_velocity;
        copy_rep.saved_acceleration  = src_rep.saved_acceleration;
        
        copy_rep.init_forward_dir = src_rep.init_forward_dir;
        copy_rep.forward_dir   = src_rep.forward_dir;
        copy_rep.center_offset = src_rep.center_offset;
        
        copy_rep.offset_align = src_rep.offset_align;
        
        copy_rep.transform = src_rep.transform;
        copy_rep.pose_transform_values = src_rep.pose_transform_values;
        //copy_rep.local_transform_values = src_rep.local_transform_values;
        
        //copy_rep.pose_transform_original = src_rep.pose_transform_original;
//        copy_rep.pose_transform_values_original = src_rep.pose_transform_values_original;
        
        for (auto c_it = copy_rep.colliders.begin(); c_it != copy_rep.colliders.end(); ++c_it) {
            Collider_remove((*c_it)->system, 0, *c_it);
            Collider_destroy((*c_it)->system, *c_it);
        }
        copy_rep.colliders.clear();
        
        if (thing->archetype_id == mtt::ARCHETYPE_NUMBER) {
            auto* old_value = mtt::access<float32>(thing, "value");
            auto* value = mtt::access<float32>(thing_copy, "value");
            *value = *old_value;
        }
        
        for (auto c_it = src_rep.colliders.begin(); c_it != src_rep.colliders.end(); ++c_it) {
            Collider* c_src = *c_it;
            Collider* c_cpy = Collider_copy(c_src, (void*)thing_copy->id, args.collision_system);
            
            
            copy_rep.colliders.push_back(c_cpy);
            
            Collider_push(c_cpy);
        }
        
        copy_rep.render_data.is_shared = src_rep.render_data.is_shared;
        //copy_rep.render_data.layer = src_rep.render_data.layer;
        
        copy_rep.shader_id = src_rep.shader_id;

        //copy_rep.render_data.drawable_info_list.resize(src_rep.render_data.drawable_info_list.size());
        
        
        // TODO: change these since a proxy should be instanced
        
//        src_rep.render_data.on_copy(static_cast<void*>(thing_copy), copy_rep.render_data, (void*)args.renderer_layer_id);
//
        
        auto on_copy = [](void* ctx, Render_Data& dst, Render_Data& src, void* args) {
            //int64 layer_id = (int64)args;
            mtt::Thing* thing = static_cast<mtt::Thing*>(ctx);
            mtt::World* mtt_world = mtt::world(thing);
            
            //sd::Renderer* renderer = mtt_world->renderer;
            
            mtt::Rep& rep = *mtt::rep(thing);
            
            for (usize i = 0; i < src.drawable_info_list.size(); i += 1) {
                auto* info = mem::alloc_init<sd::Drawable_Info>(&mtt_world->drawable_pool.allocator);
                *info = *src.drawable_info_list[i];
                info->reference_count = 0;
                sd::set_transform(info, rep.hierarchy_model_transform);
                info->layer_id = src.drawable_info_list[i]->layer_id;
                dst.drawable_info_list.emplace_back(info);
                dst.drawable_info_list.back()->is_enabled = true;
            }
        };
        on_copy(static_cast<void*>(thing_copy), copy_rep.render_data, src_rep.render_data, (void*)(args.renderer_layer_id));
        
        copy_rep.render_data.set_on_copy(on_copy);
        
        
//        for (usize i = 0; i < src_rep.render_data.drawable_info_list.size(); i += 1) {
//            auto& dr_info = src_rep.render_Data.drawable_info_list[i];
//        }
        auto* pos = mtt::access<vec3>(thing, "position");
        auto* pos_copy = mtt::access<vec3>(thing_copy, "position");
        if (pos != nullptr && pos_copy != nullptr) {
            *pos_copy = *pos;
        }
    }
    
    auto& thing_to_word = dt::DrawTalk::ctx()->lang_ctx.dictionary.thing_to_word;
    auto words = thing_to_word.find(thing->id);
    if (words != thing_to_word.end()) {
        //MTT_print("WORDS SIZE: %zu\n", words->second.size());
        for (auto w_it = words->second.begin(); w_it != words->second.end(); ++w_it) {
            dt::vis_word_derive_from(thing_copy, *w_it);
        }
    }
    
    dt::Thing_copy_own_attributes(thing, thing_copy);
    
    
    
    Thing_Archetype* arch;
    world->Thing_Archetype_get(thing_copy->archetype_id, &arch);
    arch->on_thing_copy(thing_copy, thing);
    
    if (p != nullptr) {
        mtt::connect_parent_to_child(world, p, thing);
    }
    
    Thing_set_is_proxy(thing_copy, thing, args.scene_idx);


    return thing_copy;
}

Thing* Thing_make_proxy(World* world, Thing_ID ID_src, const Thing_Make_Proxy_Args& args)
{
    Thing* thing_src = mtt::Thing_try_get(world, ID_src);
    if (thing_src == nullptr) {
        return nullptr;
    }
    
    return Thing_make_proxy(thing_src, args);
}

void Thing_reinit(Thing* thing)
{
    ASSERT_MSG(false, "NOT DONE!!!\n");
    
    mtt::World* world = thing->world();
    Thing_Archetype& arch = world->archetypes.instances[thing->archetype_id];
    copy_field_list(world, field_allocator(world), &thing->field_descriptor, &arch.field_descriptor);
    
    
    thing->ports  = arch.port_descriptor;
}

void Thing_set_label(Thing* thing, const mtt::String& label)
{
    cstring as_cstr = label.c_str();
    auto new_label = MTT_string_add(0, as_cstr);
    if (thing->label.id != 0) {
        MTT_string_ref_release(&thing->label);
    }
    thing->label = new_label;
}
cstring Thing_cstring_from_label(Thing* thing)
{
    if (MTT_string_ref_is_valid(thing->label)) {
        return MTT_string_ref_to_cstring(thing->label);
    }
    return "";
}

mtt::String Thing_label(Thing* thing)
{
    return Thing_cstring_from_label(thing);
}

cstring search_id(Thing* thing)
{
    return thing->ecs_entity.name();
}

void Thing_set_locked(Thing* thing, bool lock_state)
{
    thing->is_locked = lock_state;
}

bool Thing_is_locked(Thing* thing)
{
    return thing->is_locked;
}


Thing_Ref::Thing_Ref(mtt::Thing* thing) : id(thing->id), world(thing->world()) {}
Thing_Ref::Thing_Ref(Thing_ID id, mtt::World* world) : id(id), world(world) {}
Thing_Ref::Thing_Ref(Thing_ID id) : id(id), world(mtt::ctx()) {}
bool Thing_Ref::try_get(mtt::Thing** thing)
{
    if (this->id == mtt::Thing_ID_INVALID) {
        return false;
    }
    
    return (this->world->Thing_try_get(this->id, thing));
}
mtt::Thing* Thing_Ref::try_get(void)
{
    if (this->id == mtt::Thing_ID_INVALID) {
        return nullptr;
    }
    mtt::Thing* out = nullptr;
    this->world->Thing_try_get(this->id, &out);
    return out;
}

void Thing_add_in_port(PORT_PARAM_LIST)
{
    Port_Descriptor* ports   = info.ports;
    Spatial_Alignment* align = &info.align;
    
    Port port = {};
    port.type           = (MTT_TYPE)info.type;
    port.contained_type = (MTT_TYPE)info.contained_type;
    port.index          = ports->in_ports.size();
    port.tag            = mtt::string(info.tag.c_str());
    port.is_in_port = true;
    port.is_active = false;
    //    port.get_and_update_position = nullptr;
    //    port.get_and_update_position = info.get_and_update_position;
    //    port.on_remove_connection = info.on_remove_connection;
    
    //ports->in_port_alignment.push_back(*align);
    ports->in_ports.push_back(port);
    ports->in_name_to_idx[info.tag] = port.index;
}

void Thing_add_in_port(mtt::Thing* thing, PORT_PARAM_LIST)
{
    info.field_desc = &thing->field_descriptor;
    info.ports      = &thing->ports;
    info.logic      = &thing->logic;
    
    
    Thing_add_in_port(info);
    
    
    thing->graph->incoming[thing->id].resize(thing->ports.in_ports.size());
}

void Thing_add_out_port(PORT_PARAM_LIST)
{
    Port_Descriptor* ports   = info.ports;
    Spatial_Alignment* align = &info.align;
    
    Port port = {};
    port.type           = (MTT_TYPE)info.type;
    port.contained_type = (MTT_TYPE)info.contained_type;
    port.index          = ports->out_ports.size();
    port.tag            = mtt::string(info.tag.c_str());
    port.is_in_port = false;
    port.is_active = false;
    //    port.get_and_update_position = nullptr;
    //    port.get_and_update_position = info.get_and_update_position;
    //    port.on_remove_connection = info.on_remove_connection;
    
    //ports->out_port_alignment.push_back(*align);
    ports->out_ports.push_back(port);
    ports->out_name_to_idx[info.tag] = port.index;
}

void Thing_add_out_port(mtt::Thing* thing, PORT_PARAM_LIST)
{
    info.field_desc = &thing->field_descriptor;
    info.ports      = &thing->ports;
    info.logic      = &thing->logic;
    
    Thing_add_out_port(info);
}

void Thing_remove_in_port(mtt::Thing* thing, const mtt::String& name)
{
    auto idx_find = thing->ports.in_name_to_idx.find(name);
    if (idx_find == thing->ports.in_name_to_idx.end()) {
        return;
    }
    
    usize idx = idx_find->second;
    thing->ports.in_ports[idx].is_active = false;
    mtt::string_free(thing->ports.in_ports[idx].tag);
    thing->ports.in_name_to_idx.erase(idx_find);
}
void Thing_remove_out_port(mtt::Thing* thing, const mtt::String& name)
{
    auto idx_find = thing->ports.out_name_to_idx.find(name);
    if (idx_find == thing->ports.out_name_to_idx.end()) {
        return;
    }
    
    usize idx = idx_find->second;
    thing->ports.out_ports[idx].is_active = false;
    mtt::string_free(thing->ports.out_ports[idx].tag);
    thing->ports.out_name_to_idx.erase(idx_find);
}

Result<vec3> Thing_in_port_update_position(World* world, Thing* thing, uint64 port_idx, void* user_data)
{
    ASSERT_MSG(false, "NOT IMPLEMENTED\n");
    return {};
}

Result<vec3> Thing_out_port_update_position(World* world, Thing* thing, uint64 port_idx, void* user_data)
{
    
    ASSERT_MSG(false, "NOT IMPLEMENTED\n");
    return {};
}

static void thing_make_noop(Thing*) {}
static void thing_copy_noop(Thing*, Thing*) {}
static void thing_destroy_noop(Thing*) {}

bool Thing_Archetype_make(World* world, Thing_Archetype** out, String tag)
{
    Thing_Archetype_ID id = world->archetypes.next_thing_archetype_id;
    world->archetypes.next_thing_archetype_id += 1;
    world->archetypes.instances.emplace(id, Thing_Archetype());
    Thing_Archetype* arch = &world->archetypes.instances[id];
    arch->id = id;
    arch->tag = tag;
    arch->field_descriptor.data.byte_offset = 0;
    arch->on_thing_make    = thing_make_noop;
    arch->on_thing_destroy = thing_destroy_noop;
    arch->on_thing_copy    = thing_copy_noop;
    arch->world_ = world;
    mtt::unset_is_actor(arch);
    //MTT_print("making archetype with name=[%s] id=[%llu]\n", arch->tag.c_str(), id);
    if (out != nullptr) {
        *out = arch;
    }
    return true;
}

void MTT_Field_print(Field_List* field_list, Field_List_Descriptor* desc, String* tag)
{
    auto result = desc->name_to_idx->find(*tag);
    if (result == desc->name_to_idx->end()) {
        MTT_print("[%s]=undefined", tag->c_str());
        return;
    }
    Field* field = &desc->fields[(result->second)];
    
    return MTT_Field_print(field_list, desc, tag, field);
}

void MTT_Field_print(Field_List* field_list, Field_List_Descriptor* desc, String* tag, Field* field)
{
    const char* const tag_str = tag->c_str();
    
    void* base = field_list->contents;
    
    switch (field->type) {
    case MTT_FLOAT: {
        MTT_print("[%s : %s]=%f", tag_str, meta[field->type].name.c_str(), *(float*)field->data_from(base));
        break;
    }
    case MTT_VECTOR2: {
        vec2* val = (vec2*)field->data_from(base);
        MTT_print("[%s : %s]=[%f,%f]", tag_str, meta[field->type].name.c_str(), val->x, val->y);
        break;
    }
    case MTT_VECTOR3: {
        vec3* val = (vec3*)field->data_from(base);
        MTT_print("[%s : %s]=[%f,%f,%f]", tag_str, meta[field->type].name.c_str(), val->x, val->y, val->z);
        break;
    }
    case MTT_VECTOR4: {
        vec4* val = (vec4*)field->data_from(base);
        MTT_print("[%s : %s]=[%f,%f,%f,%f]", tag_str, meta[field->type].name.c_str(), val->x, val->y, val->z, val->w);
        break;
    }
    case MTT_COLOR_RGBA: {
        vec4* val = (vec4*)field->data_from(base);
        MTT_print("[%s : %s]=[%f,%f,%f,%f]", tag_str, meta[field->type].name.c_str(), val->x, val->y, val->z, val->w);
        break;
    }
    case MTT_INT32: {
        int32 val = *(int32*)field->data_from(base);
        MTT_print("[%s : %s]=%d", tag_str, meta[field->type].name.c_str(), val);
        break;
    }
    case MTT_INT64: {
        int64 val = *(int64*)field->data_from(base);
        MTT_print("[%s : %s]=%lld", tag_str, meta[field->type].name.c_str(), val);
        break;
    }
    case MTT_BOOLEAN: {
        bool val = *(bool*)field->data_from(base);
        MTT_print("[%s : %s]=%s", tag_str, meta[field->type].name.c_str(), (val) ? "true" : "false");
        break;
    }
    case MTT_CHAR: {
        char val = *(char*)field->data_from(base);
        MTT_print("[%s : %s]=%c", tag_str, meta[field->type].name.c_str(), val);
        break;
    }
    case MTT_LIST: {
        MTT_TYPE sub = (MTT_TYPE)field->contained_type;
        
        switch (sub) {
        case MTT_FLOAT: {
            std::vector<float32>* list = (std::vector<float32>*)field->data_from(base);
            MTT_print("[%s : %s]{\n", tag_str, meta[field->type].name.c_str());
            for (usize i = 0; i < list->size(); i += 1) {
                MTT_print("%f,\n", (*list)[i]);
            }
            MTT_print("%s", "}\n");
            break;
        }
        case MTT_VECTOR2: {
            std::vector<vec2>* list = (std::vector<vec2>*)field->data_from(base);
            MTT_print("[%s : %s]{\n", tag_str, meta[field->type].name.c_str());
            for (usize i = 0; i < list->size(); i += 1) {
                MTT_print("[%f,%f],\n", (*list)[i].x, (*list)[i].y);
            }
            MTT_print("%s", "}\n");
            break;
        }
        case MTT_VECTOR3: {
            std::vector<vec3>* list = (std::vector<vec3>*)field->data_from(base);
            MTT_print("[%s : %s]{\n", tag_str, meta[field->type].name.c_str());
            for (usize i = 0; i < list->size(); i += 1) {
                MTT_print("[%f,%f,%f],\n", (*list)[i].x, (*list)[i].y, (*list)[i].z);
            }
            MTT_print("%s", "}\n");
            break;
        }
        case MTT_VECTOR4: {
            std::vector<vec4>* list = (std::vector<vec4>*)field->data_from(base);
            MTT_print("[%s : %s]{\n", tag_str, meta[field->type].name.c_str());
            for (usize i = 0; i < list->size(); i += 1) {
                MTT_print("[%f,%f,%f,%f],\n", (*list)[i].x, (*list)[i].y, (*list)[i].z, (*list)[i].w);
            }
            MTT_print("%s", "}\n");
            break;
        }
        case MTT_COLOR_RGBA: {
            std::vector<vec4>* list = (std::vector<vec4>*)field->data_from(base);
            MTT_print("[%s : %s]{\n", tag_str, meta[field->type].name.c_str());
            for (usize i = 0; i < list->size(); i += 1) {
                MTT_print("[%f,%f,%f,%f],\n", (*list)[i].x, (*list)[i].y, (*list)[i].z, (*list)[i].w);
            }
            MTT_print("%s", "}\n");
            break;
        }
        case MTT_INT32: {
            std::vector<int32>* list = (std::vector<int32>*)field->data_from(base);
            MTT_print("[%s : %s]{\n", tag_str, meta[field->type].name.c_str());
            for (usize i = 0; i < list->size(); i += 1) {
                MTT_print("%d,\n", (*list)[i]);
            }
            MTT_print("%s", "}\n");
            break;
        }
        case MTT_INT64: {
            std::vector<int64>* list = (std::vector<int64>*)field->data_from(base);
            MTT_print("[%s : %s]{\n", tag_str, meta[field->type].name.c_str());
            for (usize i = 0; i < list->size(); i += 1) {
                MTT_print("%lld,\n", (*list)[i]);
            }
            MTT_print("%s", "}\n");
            break;
        }
        case MTT_BOOLEAN: {
            std::vector<bool>* list = (std::vector<bool>*)field->data_from(base);
            MTT_print("[%s : %s]{\n", tag_str, meta[field->type].name.c_str());
            for (usize i = 0; i < list->size(); i += 1) {
                bool val = (*list)[i];
                MTT_print("%s,\n", (val) ? "true" : "false");
            }
            MTT_print("%s", "}\n");
            break;
        }
        case MTT_CHAR: {
            std::vector<char>* list = (std::vector<char>*)field->data_from(base);
            MTT_print("[%s : %s]{\n", tag_str, meta[field->type].name.c_str());
            for (usize i = 0; i < list->size(); i += 1) {
                MTT_print("%c,\n", (*list)[i]);
            }
            MTT_print("%s", "}\n");
            break;
        }
        case MTT_LIST: {
            // TODO:
            break;
        }
        case MTT_MAP: {
            // TODO:
            break;
        }
        case MTT_SET: {
            // TODO:
            break;
        }
        case MTT_ANY: {
            MTT_print("[%s : %s]", tag_str, meta[field->type].name.c_str());
            break;
        }
        default: { break; }
        }
        break;
    }
    case MTT_MAP: {
        MTT_print("[%s : %s]=%s", tag_str, meta[field->type].name.c_str(), "TODO");
        break;
    }
    case MTT_SET: {
        MTT_print("[%s : %s]=%s", tag_str, meta[field->type].name.c_str(), "TODO");
        break;
    }
    case MTT_POLYCURVE: {
        mtt::Polycurve* list = (mtt::Polycurve*)field->data_from(base);
        MTT_print("[%s : %s]{\n", tag_str, meta[field->type].name.c_str());
        for (usize i = 0; i < list->size(); i += 1) {
            auto* sub_list = &((*list)[i]);
            MTT_print("%s", "{\n");
            for (usize j = 0; j < sub_list->size(); j += 1) {
                MTT_print("[%f,%f,%f],\n", (*sub_list)[j][0], (*sub_list)[j][1], (*sub_list)[j][2]);
            }
            MTT_print("%s", "}\n");
        }
        MTT_print("%s", "}\n");
        
        break;
    }
    case MTT_TAG_LIST: {
        // TODO: //
        break;
    }
    case MTT_TEXT: {
        MTT_String_Ref* text = (MTT_String_Ref*)field->data_from(base);
        MTT_print("[%s : %s]=%s", tag_str, meta[field->type].name.c_str(), MTT_string_ref_to_cstring(*text));
        
        break;
    }
    case MTT_TAG: {
        MTT_String_Ref* text = (MTT_String_Ref*)field->data_from(base);
        MTT_print("[%s : %s]=%s", tag_str, meta[field->type].name.c_str(), MTT_string_ref_to_cstring(*text));
    }
    default: { break; }
    }
    MTT_print("%s", "\n");
}

void MTT_Field_List_print(Field_List* field_list, Field_List_Descriptor* desc)
{
    for (usize i = 0; i < desc->fields.size(); i += 1) {
        String* tag = &((*desc->idx_to_name)[i]);
        
        MTT_Field_print(field_list, desc, tag, &desc->fields[i]);
    }
}

void MTT_Port_print(Port_Descriptor* desc,MTT_String_Ref* tag, Port* port)
{
    cstring tag_str = string_get(*tag);
    if (port->is_in_port) {
        MTT_print("in_port=[%s : %s] active=[%d]", tag_str, meta[port->type].name.c_str(), port->is_active);
    } else {
        MTT_print("out_port=[%s : %s] active=[%d]", tag_str, meta[port->type].name.c_str(), port->is_active);
    }
}

void MTT_Port_Descriptor_print(Port_Descriptor* desc)
{
    for (usize i = 0; i < desc->in_ports.size(); i += 1) {
        MTT_Port_print(desc, &desc->in_ports[i].tag, &desc->in_ports[i]);
        MTT_print("%s", "\n");
    }
    for (usize i = 0; i < desc->out_ports.size(); i += 1) {
        MTT_Port_print(desc, &desc->out_ports[i].tag, &desc->out_ports[i]);
        MTT_print("%s", "\n");
    }
}


World World_make(void)
{
    MTT_print("%s", "You're making a thing world!\n");
    return World();
}


template <typename type>
void field_init_array_type(void* contents, void* val)
{
    auto* ptr = mem::allocate<mtt::Dynamic_Array<type>>(field_allocator(mtt::ctx()));
    *((mtt::Dynamic_Array<type>**)(contents)) = ptr;
    
    if (val != nullptr) {
        mtt::Dynamic_Array<type>* to_clone = (mtt::Dynamic_Array<type>*)val;
        if (to_clone->allocator == nullptr) {
            to_clone->allocator = mtt::buckets_allocator();
        }
        mtt::clone(ptr, to_clone);
    } else {
        init(ptr, *mtt::buckets_allocator());
    }
}
template <typename type>
void field_init_array_type(void* contents, void** val)
{
    mtt::Dynamic_Array<type>** actual_val = reinterpret_cast<mtt::Dynamic_Array<type>**>(val);
    field_init_array_type<type>(contents, (void*)(*actual_val));
}



template <typename type>
void field_deinit_array_type(mtt::Dynamic_Array<type>* array)
{
    mtt::deinit(array);
    mem::deallocate<mtt::Dynamic_Array<type>>(field_allocator(mtt::ctx()), array);
}

template <typename type>
void field_deinit_array_type(void* contents)
{
    auto* array = (mtt::Dynamic_Array<type>*)contents;
    field_deinit_array_type<type>(array);
}

template <typename type>
void field_deinit_array_type(void** contents)
{
    field_deinit_array_type<type>((void*)
                                  (*((mtt::Dynamic_Array<type>**)contents
                                     )));
}

template <typename type>
void array_pointer_make(mtt::Dynamic_Array<type>** out)
{
    auto* ptr = mem::allocate<mtt::Dynamic_Array<type>>(field_allocator(mtt::ctx()));
    init(ptr, *buckets_allocator(mtt::ctx()));
    *out = ptr;
}
template <typename type>
void array_pointer_destroy(mtt::Dynamic_Array<type>** array)
{
    mtt::deinit(*array);
    mem::deallocate<mtt::Dynamic_Array<type>>(field_allocator(mtt::ctx()), *array);
    *array = nullptr;
}

template <typename map_type>
void map_pointer_make(map_type** out)
{
    auto* ptr = mem::allocate<map_type>(field_allocator(mtt::ctx()));
    *out = ptr;
}
template <typename map_type>
void map_pointer_destroy(map_type** map)
{
    mem::deallocate<map_type>(field_allocator(mtt::ctx()), *map);
    *map = nullptr;
}

void add_field(Field_Create_Info info, Field_List_Builder& builder, void* value, FIELD_FLAG flags)
{
    builder.append([](Field_List_Builder::Init_State* init_state, Field_Create_Info info, void* value, usize byte_count, FIELD_FLAG flags) {
        add_field(info, value, flags);
    }, info, value, flags);
}

void add_field(Field_Create_Info info, Field_List_Builder& builder, const mtt::String& type_name, usize byte_size, void (*on_make)(void* data), void (*on_copy)(void* data_dst, void* data_src), void (*on_destroy)(void* data), FIELD_FLAG flags)
{
    builder.append([](Field_List_Builder::Init_State* init_state, Field_Create_Info info, void* value, usize byte_count, FIELD_FLAG flags) {
        
        Field_List_Descriptor* fields = info.field_desc;
        uint64 byte_offset = fields->data.byte_offset;
        
        auto* meta_info = &meta[info.type];
        
        ASSERT_MSG(fields->data.byte_offset + meta_info->alloc_byte_size <= fields->data.byte_count , "field initialization failed: requested %llu bytes, allocated %llu bytes\n", fields->data.byte_offset + meta_info->alloc_byte_size, fields->data.byte_count);
        
        
        
        fields->fields.emplace_back(Field());
        Field* field = &fields->fields.back();
        field->byte_offset = byte_offset;
        field->type = (MTT_TYPE)info.type;
        field->contained_type = (MTT_TYPE)info.contained_type;
        field->index = fields->fields.size() - 1;
        
        char* contents = ((char*)fields->data.contents) + fields->data.byte_offset;
        
        field->watchers = nullptr;
        field->flags = flags;
        
        
        field->byte_count = byte_count;
        
        byte_offset += byte_count;
        
        fields->data.byte_offset = byte_offset;
        (*fields->name_to_idx)[info.tag] = fields->fields.size() - 1;
        (*fields->idx_to_name)[fields->fields.size() - 1] = info.tag;
        
        field->on_make = init_state->on_make;
        field->on_copy = init_state->on_copy;
        field->on_destroy = init_state->on_destroy;
        
        
        field->on_make(contents);
        
        
        
        
    }, info, type_name, byte_size, on_make, on_copy, on_destroy, flags);
}

void add_field(Field_Create_Info info, void* value, FIELD_FLAG flags)
{
    Field_List_Descriptor* fields = info.field_desc;
    uint64 byte_offset = fields->data.byte_offset;
    
    auto* meta_info = &meta[info.type];
    
    ASSERT_MSG(fields->data.byte_offset + meta_info->alloc_byte_size <= fields->data.byte_count , "field initialization failed: requested %llu bytes, allocated %llu bytes\n", fields->data.byte_offset + meta_info->alloc_byte_size, fields->data.byte_count);
    
    
    
    fields->fields.emplace_back(Field());
    Field* field = &fields->fields.back();
    field->byte_offset = byte_offset;
    field->type = (MTT_TYPE)info.type;
    field->contained_type = (MTT_TYPE)info.contained_type;
    field->index = fields->fields.size() - 1;
    
    char* contents = ((char*)fields->data.contents) + fields->data.byte_offset;
    
    field->watchers = nullptr;
    field->flags = flags;
    
    
    field->byte_count = meta_info->alloc_byte_size;
    
    
    if (value == nullptr) {
        value = meta_info->def_val;
    }
    
    memcpy(contents, value, meta_info->actual_byte_size);
    
    byte_offset += meta_info->alloc_byte_size;
    
    fields->data.byte_offset = byte_offset;
    (*fields->name_to_idx)[info.tag] = fields->fields.size() - 1;
    (*fields->idx_to_name)[fields->fields.size() - 1] = info.tag;
    
    switch (field->type) {
    case MTT_POLYCURVE: {
        field_init_array_type<mtt::Dynamic_Array<vec3>>(contents, value);
        break;
    }
    case MTT_THING_LIST: {
        field_init_array_type<mtt::Thing_Ref>(contents, value);
        break;
    }
    case MTT_LIST: {
        field_init_array_type<char>(contents, value);
        break;
    }
    case MTT_TAG_LIST: {
        field_init_array_type<MTT_String_Ref>(contents, value);
        break;
    }
    case MTT_TEXT: {
        MTT_FALLTHROUGH;
    }
    case MTT_TAG: {
        MTT_string_ref_retain(*((MTT_String_Ref*)(contents)));
        break;
    }
    case MTT_CUSTOM: {
        field->on_make(contents);
    }
    default: {
        break;
    }
    }
}





void Thing_alloc_init_field_list(FIELD_PARAM_LIST, usize bytes_required)
{
    Field_List_Descriptor* fields = info.field_desc;
    Field_List* field_list = &fields->data;
    //field_list->byte_count = align_up(bytes_required, 16) + 16;
    
    if (bytes_required == 0) {
        field_list->contents = nullptr;
    } else {
        field_list->byte_count = align_up(bytes_required, 16);
        field_list->byte_offset = 0;
        
        field_list->contents = field_allocator(info.world)->do_allocate(field_list->byte_count);
        memset(field_list->contents, 0, field_list->byte_count);
    }
    
    
    fields->name_to_idx = mem::alloc_init<mtt::Map<String, uint64>>(
//                                                                    &info.world->allocator
                               buckets_allocator()                                     );
    fields->idx_to_name = mem::alloc_init<mtt::Map<uint64, String>>(
//                                                                    &info.world->allocator
                                                                    buckets_allocator()
                                                                    );
    
    fields->is_init = true;
}


void destroy_fields(mem::Allocator& allocator, Field_List_Descriptor& field_descriptor, Field_List& data)
{
    void* base = data.contents;

    for (usize i = 0; i < field_descriptor.fields.size(); i += 1) {
        mtt::Field* field = &field_descriptor.fields[i];
        void* raw_contents = field->data_from(base);
        switch (field->type) {
        case MTT_POLYCURVE: {
            field_deinit_array_type<mtt::Dynamic_Array<vec3>>((void**)raw_contents);
            break;
        }
        case MTT_THING_LIST: {
            field_deinit_array_type<mtt::Thing_Ref>((void**)raw_contents);
            break;
        }
        case MTT_TAG_LIST: {
            auto* list = *((mtt::Dynamic_Array<MTT_String_Ref>**)raw_contents);
            for (auto list_it = list->begin(); list_it != list->end(); ++list_it) {
                MTT_string_ref_release(list_it);
            }
            field_deinit_array_type<MTT_String_Ref>((void**)raw_contents);
            break;
        }
        case MTT_LIST: {
            field_deinit_array_type<char>((void**)raw_contents);
            break;
        }
        case MTT_TEXT: {
            MTT_FALLTHROUGH;
        }
        case MTT_TAG: {
            MTT_String_Ref* contents = (MTT_String_Ref*)raw_contents;
            MTT_string_ref_release(contents);
            break;
        }
        case MTT_CUSTOM: {
            field->on_destroy(raw_contents);
            break;
        }
        default: {
            break;
        }
            
        }
    }
    
    allocator.do_deallocate(data.contents, data.byte_count);
    
    data.contents = nullptr;
}
void clear_field_contents(Field_List_Descriptor& field_descriptor, Field_List& data)
{
    void* base = data.contents;
    for (usize i = 0; i < field_descriptor.fields.size(); i += 1) {
        mtt::Field* field = &field_descriptor.fields[i];
        void* raw_contents = field->data_from(base);
        switch (field->type) {
        case MTT_POLYCURVE: {
            field_deinit_array_type<mtt::Dynamic_Array<vec3>>((void**)raw_contents);
            break;
        }
        case MTT_THING_LIST: {
            field_deinit_array_type<mtt::Thing_Ref>((void**)raw_contents);
            break;
        }
        case MTT_TAG_LIST: {
            auto* list = *((mtt::Dynamic_Array<MTT_String_Ref>**)raw_contents);
            for (auto list_it = list->begin(); list_it != list->end(); ++list_it) {
                MTT_string_ref_release(list_it);
            }
            field_deinit_array_type<MTT_String_Ref>((void**)raw_contents);
            break;
        }
        case MTT_LIST: {
            field_deinit_array_type<char>((void**)raw_contents);
            break;
        }
        case MTT_TEXT: {
            MTT_FALLTHROUGH;
        }
        case MTT_TAG: {
            MTT_String_Ref* contents = (MTT_String_Ref*)raw_contents;
            MTT_string_ref_release(contents);
            break;
        }
        case MTT_CUSTOM: {
        }
        default: {
            break;
        }
            
        }
    }
    if (global_debug_proc != nullptr) {
        global_debug_proc();
    }
//    memset(base, 0, data.byte_count);
}

void init_field_list_from_source(mtt::World* world, mem::Allocator* allocator, Field_List* data, Field_List_Descriptor* src)
{
    if (src->data.contents == nullptr) {
        data->contents = nullptr;
        return;
    }
    
    //bool should_init = (data->contents == nullptr);
    auto* old_contents = data->contents;
    if (old_contents != nullptr) {
        allocator->deallocate(allocator, old_contents, src->data.byte_count);
    }
    
    *data = src->data;
    data->contents = allocator->allocate(allocator, src->data.byte_count);
    
    memcpy(data->contents, src->data.contents, src->data.byte_count);

    
    usize byte_offset = 0;
    for (usize i = 0; i < src->fields.size(); i += 1) {
        auto* field = &src->fields[i];
        byte_offset = field->byte_offset;
        auto* contents = field->data_from(data->contents);
        auto* src_contents = src->fields[i].data_from(src->data.contents);
        field->watchers = nullptr;
        
        switch (field->type) {
        case MTT_POLYCURVE: {
            field_init_array_type<mtt::Dynamic_Array<vec3>>(contents, (void**)src_contents);
            break;
        }
        case MTT_THING_LIST: {
            field_init_array_type<mtt::Thing_Ref>(contents, (void**)src_contents);
            break;
        }
        case MTT_LIST: {
            field_init_array_type<char>(contents, (void**)src_contents);
            break;
        }
        case MTT_TAG_LIST: {
            field_init_array_type<MTT_String_Ref>(contents, (void**)src_contents);
            break;
        }
        case MTT_TEXT: {
            MTT_FALLTHROUGH;
        }
        case MTT_TAG: {
            MTT_string_ref_retain(*((MTT_String_Ref*)(contents)));
            break;
        }
        case MTT_CUSTOM: {
            field->on_copy(contents, src_contents);
            break;
        }
        default: {
            break;
        }
        }
    }
    

}

void copy_field_list(mtt::World* world, mem::Allocator* allocator, Field_List_Descriptor* dst, Field_List_Descriptor* src)
{
    void* old_contents = dst->data.contents;
    if (old_contents != nullptr) {
        allocator->deallocate(allocator, old_contents, dst->data.byte_count);
    }
    
    *dst = *src;
    
    if (src->data.contents == nullptr) {
        dst->data.contents = nullptr;
        return;
    }
    
    dst->data.contents = allocator->allocate(allocator, src->data.byte_count);
    
    memcpy(dst->data.contents, src->data.contents, src->data.byte_count);
    
    usize byte_offset = 0;
    for (usize i = 0; i < dst->fields.size(); i += 1) {
        auto* field = &dst->fields[i];
        byte_offset = field->byte_offset;
        auto* contents = field->data_from(dst->data.contents);
        auto* src_contents = src->fields[i].data_from(src->data.contents);
        field->watchers = nullptr;
        
        switch (field->type) {
        case MTT_POLYCURVE: {
            field_init_array_type<mtt::Dynamic_Array<vec3>>(contents, (void**)src_contents);
            break;
        }
        case MTT_THING_LIST: {
            field_init_array_type<mtt::Thing_Ref>(contents, (void**)src_contents);
            break;
        }
        case MTT_LIST: {
            field_init_array_type<char>(contents, (void**)src_contents);
            break;
        }
        case MTT_TAG_LIST: {
            field_init_array_type<MTT_String_Ref>(contents, (void**)src_contents);
            break;
        }
        case MTT_TEXT: {
            MTT_FALLTHROUGH;
        }
        case MTT_TAG: {
            MTT_string_ref_retain(*((MTT_String_Ref*)(contents)));
            break;
        }
        case MTT_CUSTOM: {
            field->on_copy(contents, src_contents);
            break;
        }
        default: {
            break;
        }
        }
    }
}


void set_option(Thing* thing, uint64 option, uint64 flags)
{
    thing->logic.option       = option;
    thing->logic.option_flags = flags;
}

void init_flag(THING_FLAG* src)
{
    *src = THING_FLAG_EMPTY;
}

void set_flag(THING_FLAG* src, THING_FLAG flag)
{
    *src = (THING_FLAG)((uint64)*src | (uint64)flag);
}
void unset_flag(THING_FLAG* src, THING_FLAG flag)
{
    *src = (THING_FLAG)((uint64)*src & (~(uint64)flag));
}
bool flag_is_set(THING_FLAG* src, THING_FLAG flag)
{
    return ((uint64)*src & (uint64)flag) != 0;
}
void set_flag(THING_FLAG* src, THING_FLAG flag, bool state)
{
    ((state) ?  set_flag(src, flag) :
     unset_flag(src, flag));
}

void init_flag(mtt::Thing* thing)
{
    thing->flags = THING_FLAG_EMPTY;
}
void set_flag(Thing* thing, THING_FLAG flag)
{
    thing->flags = (THING_FLAG)((uint64)thing->flags | (uint64)flag);
}
void unset_flag(Thing* thing, THING_FLAG flag)
{
    thing->flags = (THING_FLAG)((uint64)thing->flags & (~(uint64)flag));
}
bool flag_is_set(Thing* thing, THING_FLAG flag)
{
    return ((uint64)thing->flags & (uint64)flag) != 0;
}
void set_flag(Thing* thing, THING_FLAG flag, bool state)
{
    ((state) ?  set_flag(thing, flag) :
     unset_flag(thing, flag));
}


//}




LOGIC_PROC_RETURN_TYPE no_operation_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE no_operation_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    return false;
}

LOGIC_PROC_RETURN_TYPE not_implemented_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE not_implemented_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    return false;
}

void clear_messages(Messages_Record* msgs)
{
    if (!msgs->from_entities.empty()) {
        msgs->from_entities.clear();
    }
    if (!msgs->from_tags.empty()) {
        msgs->from_tags.clear();
    }
}


bool actor_process_messages(mtt::Thing* thing, Messages_Record* msgs, void (*handler)(Message* msg))
{
    mtt::Thing_Archetype* arch = archetype_of(thing);
    bool received_message = false;
    while (!msgs->from_entities.empty()) {
        Message* msg = &msgs->from_entities.front();
        
        handler(msg);
        
        selector_invoke(arch, thing, msg);
        
        msgs->from_entities.pop_front();
        
        received_message = true;
    }
    
    return received_message;
}

// MARK: - begin_archetypes

MTT_DEFINE_LOGIC_PROCEDURE(freehand_sketch)
{
    Messages_Record* msgs = messages(&world->message_passer, thing->id);
    
    if (!thing_group_is_active(thing)) {
        clear_messages(msgs);
        return true;
    }
    
    
    bool received_message = actor_process_messages(thing, msgs, thing->message_handler);
    (void)received_message;
    
    vec3 vel_in = vec3(0.0f);
    vec3 acc_in = vec3(0.0f);
    vec3 pos_in = vec3(0.0f);
    vec3 scale_in = vec3(1.0f);
    float32 rotation_in = 0.0f;
    vec4 color_factor = vec4(1.0f);
    bool velocity_found = false;
    bool acceleration_found = false;
    bool position_found = false;
    bool scale_found = false;
    bool rotation_found = false;
    bool transform_found = false;
    
    mtt::Rep* rep;
    mtt::rep(thing, &rep);
    
    
    vec3 transform_vecs[3] = {
        *(vec3*)mtt::access(thing, (usize)FREEHAND_SKETCH_INDEX_VELOCITY),
        *(vec3*)mtt::access(thing, (usize)FREEHAND_SKETCH_INDEX_ACCELERATION),
        *(vec3*)mtt::access(thing, (usize)FREEHAND_SKETCH_INDEX_POSITION)
    };
    
    
    if (input != nullptr) {
        {
            auto result = get_in_port(world, thing, input, 1);
            if (result.status == mtt::PORT_STATUS_OK) {
                
                vel_in = result.value.out.Vector3;
                transform_vecs[0] = vel_in;
                
                
                velocity_found = true;
            }
        }
        
        
        {
            auto result = get_in_port(world, thing, input, 2);
            if (result.status == mtt::PORT_STATUS_OK) {
                
                acc_in = result.value.out.Vector3;
                transform_vecs[1] = acc_in;
                
                
                acceleration_found = true;
            }
        }
        
        
        
        {
            auto result = get_in_port(world, thing, input, 3);
            if (result.status == mtt::PORT_STATUS_OK) {
                
                pos_in = result.value.out.Vector3;
                transform_vecs[2] = pos_in;
                
                position_found = true;
            }
        }
        
        auto IN_destroy = get_in_port(world, thing, input, 6);
        if (IN_destroy.status == mtt::PORT_STATUS_OK) {
            Destroy_Command& destroy_command = *((Destroy_Command*)IN_destroy.value.out.Reference_Type);
            destroy_command.thing_id = thing->id;
            world->to_destroy.emplace_back(destroy_command);
        }
        
        {
            auto result = get_in_port(world, thing, input, 7);
            if (result.status == mtt::PORT_STATUS_OK) {
                switch (result.value.out.type) {
                case MTT_FLOAT: {
                    scale_in = vec3(result.value.out.Float, result.value.out.Float, 1.0f);
                    break;
                }
                case MTT_VECTOR3: {
                    scale_in = result.value.out.Vector3;
                    //ASSERT_MSG(false, "TODO");
                    break;
                }
                case MTT_VECTOR2: {
                    scale_in = result.value.out.Vector3;
                    break;
                }
                default: {
                    ASSERT_MSG(false, "INVALID");
                    break;
                }
                }
                scale_found = true;
            }
        }
        
        {
            auto result = get_in_port(world, thing, input, "rotation");
            if (result.status == mtt::PORT_STATUS_OK) {
                switch (result.value.out.type) {
                case MTT_FLOAT: {
                    rotation_in = result.value.out.Float;
                    break;
                }
                case MTT_VECTOR3: {
                    ASSERT_MSG(false, "TODO");
                    break;
                }
                case MTT_VECTOR2: {
                    ASSERT_MSG(false, "TODO");
                    break;
                }
                default: {
                    ASSERT_MSG(false, "INVALID");
                    break;
                }
                }
                rotation_found = true;
            }
        }
        
        
        if (scale_found || rotation_found) {
            mtt::set_pose_transform(thing, m::translate(rep->pose_transform_values.translation) * m::rotate(rotation_in, vec3(0.0f, 0.0f, 1.0f)) * m::rotate(mat4(1.0f), MTT_PI_32 * ( (rep->forward_dir.x < 0) ? 1.0f : 0.0f), {0.0f, 1.0f, 0.0f}) * m::scale(scale_in * rep->pose_transform_values.scale));
        }
        
        
        auto IN_color_factor = get_in_port(world, thing, input, "color_factor");
        if (IN_color_factor.status == mtt::PORT_STATUS_OK) {
            switch (IN_color_factor.value.out.type) {
            case MTT_FLOAT: {
                color_factor = vec4(IN_color_factor.value.out.Float);
                break;
            }
            case MTT_VECTOR3: {
                color_factor = vec4(IN_color_factor.value.out.Vector3, 1.0f);
                break;
            }
            case MTT_VECTOR4: {
                color_factor = IN_color_factor.value.out.Vector4;
                break;
            }
            }
            
            auto& drawables = rep->render_data.drawable_info_list;
            for (auto d_it = drawables.begin(); d_it != drawables.end(); ++d_it) {
                auto* drawable = (*d_it);
                drawable->set_color_factor(color_factor);
            }
        }
        
        auto result = get_in_port(world, thing, input, "transform");
        if (result.status == mtt::PORT_STATUS_OK) {
            mtt::set_pose_transform(thing, &result.value.out.Matrix4);
        }
        
    }
    
    get_out_port(thing, "transform").value.out.set_Matrix4(rep->pose_transform);
    
    
    
    
    //    auto option_flags = get_in_port(world, thing, input, "option:flags");
    //    if (option_flags.status == mtt::PORT_STATUS_OK) {
    //        thing->logic.option_flags = option_flags.value.out.Int64;
    //    }
    
    // MARK: adjust velocity
    {
        
        
        
        
        //        {
        //            auto* vel = (vec3*)thing->field_desc.fields[0].contents;//mtt::access<vec3>(thing, "velocity");
        //            // NOTE(Toby): now will need to set velocity and acceleration to 0
        //            // when a connection is removed because it won't automatically go to 0 anymore
        //            // when there is no input
        //            //if (velocity_found) {
        //                //MTT_print("velocity: [%f, %f, %f]\n", vel->x, vel->y, vel->z);
        //                *vel = vel_in;
        //            //}
        //
        //            auto* acc = (vec3*)thing->field_desc.fields[1].contents;//mtt::access<vec3>(thing, "acceleration");
        //            //if (acceleration_found) {
        //                //MTT_print("acceleration: [%f, %f, %f]\n", acc->x, acc->y, acc->z);
        //                *acc = acc_in;
        //            //}
        //
        //            auto* pos = (vec3*)thing->field_desc.fields[2].contents;//mtt::access<vec3>(thing, "position");
        //            //if (position_found) {
        //            *pos = pos_in;
        //        }
        //}
        
        //std::cout << m::to_string(*pos) << m::to_string(*vel) << m::to_string(*acc) << std::endl;
        {
            auto center_out = get_out_port(world, thing, "center");
            if (center_out.status == mtt::PORT_STATUS_OK) {
                
                if (rep->colliders.size() == 0) {
                    center_out.value.out.set_Vector3(vec3(0.0f, 0.0f, 0.0f));
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 center_position = vec3((br + tl) / 2.0f, 0.0f);
                    
                    center_out.value.out.set_Vector3(center_position);
                    
                    break;
                }
                default: {
                    center_out.value.out.set_Vector3(vec3(0.0f, 0.0f, 0.0f));
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        {
            auto OUT_rotation_radians = get_out_port(world, thing, "angle_radians");
            auto OUT_rotation_degrees = get_out_port(world, thing, "angle_degrees");
            if (OUT_rotation_radians.status == PORT_STATUS_OK || OUT_rotation_degrees.status == PORT_STATUS_OK) {
                vec3 scale;
                quat orientation;
                vec3 translation;
                vec3 skew;
                vec4 perspective;
                
                m::decompose(rep->pose_transform, scale, orientation, translation, skew, perspective);
                
                
                vec3 euler_angles = -m::eulerAngles(orientation);
                
                if (euler_angles.z < 0) {
                    euler_angles.z += 2 * MTT_PI_32;
                }
                
                OUT_rotation_radians.value.out.set_Float(euler_angles.z);
                OUT_rotation_degrees.value.out.set_Float((euler_angles.z) * (180.0f / MTT_PI_32));
            }
        }
        {
            auto OUT_position           = get_out_port(world, thing, "position");
            auto OUT_position_x         = get_out_port(world, thing, "position_x");
            auto OUT_position_y         = get_out_port(world, thing, "position_y");
            auto OUT_position_y_negated = get_out_port(world, thing, "position_y_negated");
            auto OUT_position_z         = get_out_port(world, thing, "position_z");
            
            auto* pos = mtt::access<vec3>(thing, "position");
            
            OUT_position.value.out.set_Vector3(*pos);
            OUT_position_x.value.out.set_Float(pos->x);
            OUT_position_y.value.out.set_Float(pos->y);
            OUT_position_y_negated.value.out.set_Float(-pos->y);
            OUT_position_z.value.out.set_Float(pos->z);
        }
    }
    
    
    return true;
}

LOGIC_PROC_RETURN_TYPE follower_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE follower_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    //MTT_print("Evaluating world: %s id=[%llu]\n", __func__, thing->eval_index);
    
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    float32 timestep = world->timestep;
    
    auto in_active = get_in_port(world, thing, input, "active");
    if (in_active.status == mtt::PORT_STATUS_OK) {
        if (in_active.value.out.Boolean == false) {
            auto* vel_current = mtt::access<vec3>(thing, "velocity current");
            if (vel_current) {
                *vel_current = vec3(0.0f, 0.0f, 0.0f);
            }
            
            return true;
        }
    }
    
    //    mtt::Thing_Ref& source = *mtt::access<mtt::Thing_Ref>(thing, "thing");
    //
    //    Thing* self = nullptr;
    //    if (!source.try_get(&self)) {
    //        source.id = thing->instance_id;
    //        source.try_get(&self);
    //    }
    
    switch (thing->logic.option) {
    default: {
        
        vec3 out_vel = vec3(0.0f);
        vec3 out_acc = vec3(0.0f);
        vec3 out_deacc = vec3(0.0f);
        vec3 out_vel_current = vec3(0.0f);
        
        auto* vel = mtt::access<vec3>(thing, "velocity");
        if (vel) {
            out_vel = *vel;
        }
        
        auto* acc = mtt::access<vec3>(thing, "acceleration");
        if (acc) {
            out_acc = *acc;
        }
        
        auto* deacc = mtt::access<vec3>(thing, "deceleration");
        if (deacc) {
            out_deacc = *deacc;
        }
        
        auto* vel_current = mtt::access<vec3>(thing, "velocity current");
        if (vel_current) {
            out_vel_current = *vel_current;
        }
        
        float32 interpolation_val = 0.0f;
        auto* interpolation = mtt::access<float32>(thing, "interpolation");
        if (interpolation) {
            interpolation_val = *interpolation;
        }
        
        float32 distance_traveled_current = 0.0f;
        auto* distance_traveled = mtt::access<float32>(thing, "distance traveled");
        if (distance_traveled) {
            distance_traveled_current = *distance_traveled;
        }
        
        float32 time_start_val = 0.0f;
        auto* time_start = mtt::access<float32>(thing, "time_start");
        if (time_start) {
            time_start_val = *time_start;
        }
        
        uint64 port_target_vector;
        
        auto out_done = get_out_port(world, thing, "is_done");
        out_done.value.out.set_Boolean(false);
        //if (name_to_in_port_id(world, thing->instance_id, "target:vector", &port_target_vector)) {
        //{
        auto in_target_path = get_in_port(world, thing, input, "target:path");
        if (in_target_path.status == mtt::PORT_STATUS_OK) {
            //result.value.out.Vector3 = out_vel_current;
            *vel_current = out_vel_current;
            
            //                auto option_flags = mtt::get_out_port(world, thing, "option:flags");
            //                if (option_flags.status == mtt::PORT_STATUS_OK) {
            //                    option_flags.value.out.Int64 = 1;
            //                }
            
            {
                
                //                    mtt::Polycurve& curve      = *(mtt::Polycurve*)in_target_path.value.out.Control_Curve.points;
                //                    mtt::Polycurve& curve_dist = *(mtt::Polycurve*)in_target_path.value.out.Control_Curve.distance_sum;
                
                // TODO: // temp only one curve
                vec3 out = {0.0f, 0.0f, 0.0f};
                //                    if (curve.size() > 0 && curve[0].size() > 0)
                {
                    out_vel_current += m::length(out_vel) * out_acc * timestep;
                    
                    float32 mag_current = m::length(out_vel_current);
                    float32 mag_max     = m::length(out_vel);
                    
                    if (mag_current > mag_max) {
                        out_vel_current *= mag_max / mag_current;
                    }
                    
                    *distance_traveled += (mtt::MOVEMENT_SCALE * m::length(out_vel_current) * timestep);
                    *vel_current = out_vel_current;
                    
                    {
                        Procedure_Input_Output in_out;
                        in_out.caller         = nullptr;
                        in_out.input_count    = 2;
                        
                        vec2 in_args          = {distance_traveled_current, interpolation_val};
                        in_out.input          = (void*)&in_args;
                        
                        in_target_path.value.out.Procedure(&in_out);
                        
                        out = in_out.output.Vector3;
                        
                        // done
                        if (in_out.output_flags == 2) {
                            out_done.value.out.set_Boolean(true);
                        } else {
                            out_done.value.out.set_Boolean(false);
                        }
                        
                        
                        for (mtt::Thing_Ref* source =
                             mtt::access<mtt::Thing_Ref>(thing, "thing");
                             source != nullptr;) {
                            
                            Thing* source_thing = nullptr;
                            if (source->try_get(&source_thing)) {
                                auto* src_pos = mtt::access<vec3>(source_thing, "position");
                                if (src_pos) {
                                    *src_pos = out;
                                    mtt::Thing_set_position(source_thing, out);
                                    
                                    {
                                        Message msg;
                                        msg.sender   = thing->id;
                                        msg.selector = string_ref_get("center");
                                        selector_invoke(source_thing, &msg);
                                        //MTT_print("FLAG BEFORE %llu\n", source_thing->logic.option_flags);
                                    }
                                    
                                    {
                                        Message msg;
                                        msg.selector = string_ref_get("set_movement_option");
                                        msg.input_value.type = MTT_INT64;
                                        msg.input_value.Int64 = OPTION_FLAGS_MOVEMENT_POSITIONAL;
                                        selector_invoke(source_thing, &msg);
                                        //MTT_print("FLAG AFTER %llu\n", source_thing->logic.option_flags);
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
                auto position = mtt::get_out_port(world, thing, "position");
                if (position.status == mtt::PORT_STATUS_OK) {
                    position.value.out.set_Vector3(out);
                    position.value.is_ignored = false;
                }
                
            }
            
            return true;
            
        }
        
        
        WITH_THING(source_thing, "thing", {
            mtt::Message msg;
            msg.selector = string_ref_get("set_movement_option");
            msg.input_value.type = MTT_INT64;
            msg.input_value.Int64 = OPTION_FLAGS_MOVEMENT_DEFAULT;
            selector_invoke(source_thing, &msg);
        })
        
        //MTT_print("v: [%f,%f,%f]\n", vel_current->x, vel_current->y, vel_current->z);
        auto in_vector_target = get_in_port(world, thing, input, "target:vector");
        if (in_vector_target.status == mtt::PORT_STATUS_OK) {
            auto result = get_out_port(world, thing, 0);
            if (result.status == mtt::PORT_STATUS_OK) {
                
                
                vec4 in = (in_vector_target.value.out.type == MTT_VECTOR4) ? in_vector_target.value.out.Vector4 : vec4(in_vector_target.value.out.Vector3, 1.0f);
                
                if (in[3] == 0.0f) {
                    // TODO easing in like with the mover?
                    vec3 out = vec3(
                                    in[0],
                                    in[1],
                                    in[2]
                                    );
                    result.value.out.set_Vector3(out);
                    
                } else {
                    
                    uint64 port_source_vector;
                    if (name_to_in_port_id(world, thing->id, "source:vector", &port_source_vector)) {
                        auto result_source_position = get_in_port(world, thing, input, port_source_vector);
                        if (result_source_position.status == mtt::PORT_STATUS_OK) {
                            
                            vec3 source      = result_source_position.value.out.Vector3;
                            vec3 destination = vec3(in[0], in[1], in[2]);
                            
                            if (source == destination) {
                                out_vel_current = vec3(0.0f, 0.0f, 0.0f);
                                result.value.out.set_Vector3(out_vel_current);
                                *vel_current = out_vel_current;
                                
                                
                                for (mtt::Thing_Ref* source = mtt::access<mtt::Thing_Ref>(thing, "thing"); source != nullptr;) {
                                    Thing* source_thing = nullptr;
                                    if (source->try_get(&source_thing)) {
                                        mtt::Message msg;
                                        msg.selector = string_ref_get("set_movement_option");
                                        msg.input_value.type = MTT_INT64;
                                        msg.input_value.Int64 = OPTION_FLAGS_MOVEMENT_POSITIONAL;
                                        selector_invoke(source_thing, &msg);
                                        
                                        auto* src_pos = mtt::access<vec3>(source_thing, "position");
                                        if (src_pos) {
                                            *src_pos = destination;
                                            mtt::Thing_set_position(source_thing, destination);
                                        }
                                    }
                                    break;
                                }
                                
                                auto option_flags = mtt::get_out_port(world, thing, "option:flags");
                                if (option_flags.status == mtt::PORT_STATUS_OK) {
                                    option_flags.value.out.set_Int64(1);
                                }
                                
                                auto position = mtt::get_out_port(world, thing, "position");
                                if (position.status == mtt::PORT_STATUS_OK) {
                                    position.value.out.set_Vector3(destination);
                                }
                            } else {
                                
                                //MTT_print("DST: [%f,%f,%f]\n", destination[0], destination[1], destination[2]);
                                
                                vec3 offset1 = destination - source;
//                                vec3 out_vel_current_begin = out_vel_current;
                                vec3 normalized = m::normalize(destination - source);
                                out_vel_current += m::length(out_vel) * out_acc * normalized;
                                
                                //                                MTT_print("\t\tsource[%f,%f,%f]dest[%f,%f,%f],out_vel[%f,%f,%f],curr=[%f,%f,%f]\n",
                                //                                          source.x, source.y, source.z,
                                //                                          destination.x,destination.y,destination.z,
                                //                                          out_vel.x,out_vel.y,out_vel.z,
                                //                                          out_vel_current.x,out_vel_current.y, out_vel_current.z);
                                {
                                    float32 mag_current = m::length(out_vel_current);
                                    float32 mag_max     = m::length(out_vel);
                                    
                                    if (mag_current > mag_max) {
                                        out_vel_current *= mag_max / mag_current;
                                    }
                                }
                                
                                // TODO: avoid double computation of position just to check for overshoot?
                                vec3 source_update = source + (mtt::MOVEMENT_SCALE * out_vel_current * timestep);
                                
                                vec3 offset2 = destination - source_update;
                                
                                // TODO: snap position to target if close enough
                                //MTT_print("src=[%f,%f,%f], dst=[%f,%f,%f]\n, dst2=[%f]", source_update.x, source_update.y, source_update.z, destination.x, destination.y, destination.z, m::dist2(source_update, destination));
                                if (m::dist(source_update, destination) < 100.0f ||  m::dot(m::normalize(offset1), m::normalize(offset2)) < 0) {
                                    // overshot
                                    out_vel_current = vec3(0.0f, 0.0f, 0.0f);
                                    result.value.out.set_Vector3(out_vel_current);
                                    *vel_current = out_vel_current;
                                    
                                    auto option_flags = mtt::get_out_port(world, thing, "option:flags");
                                    if (option_flags.status == mtt::PORT_STATUS_OK) {
                                        option_flags.value.out.set_Int64(1);
                                    }
                                    
                                    auto position = mtt::get_out_port(world, thing, "position");
                                    if (option_flags.status == mtt::PORT_STATUS_OK) {
                                        position.value.out.set_Vector3(destination);
                                    }
                                    
                                    
                                } else {
                                    result.value.out.set_Vector3(out_vel_current);
                                    *vel_current = out_vel_current;
                                    
                                    auto option_flags = mtt::get_out_port(world, thing, "option:flags");
                                    if (option_flags.status == mtt::PORT_STATUS_OK) {
                                        option_flags.value.out.set_Int64(0);
                                    }
                                    
                                }
                            }
                            
                        } else {
                            result.value.out.set_Vector3(out_vel_current);
                            *vel_current = out_vel_current;
                            
                            auto option_flags = mtt::get_out_port(world, thing, "option:flags");
                            if (option_flags.status == mtt::PORT_STATUS_OK) {
                                option_flags.value.out.set_Int64(0);
                            }
                            
                        }
                        
                    } else {
                        
                        result.value.out.set_Vector3(out_vel_current);
                        *vel_current = out_vel_current;
                        
                        auto option_flags = mtt::get_out_port(world, thing, "option:flags");
                        if (option_flags.status == mtt::PORT_STATUS_OK) {
                            option_flags.value.out.set_Int64(0);
                        }
                    }
                    
                }
                
                
            }
        } else {
            *vel_current = out_vel_current;
            
            auto option_flags = mtt::get_out_port(world, thing, "option:flags");
            if (option_flags.status == mtt::PORT_STATUS_OK) {
                option_flags.value.out.set_Int64(0);
            }
        }
        //}
        
        
        break;
    }
    }
    
    return true;
}




// MARK: MOVER
LOGIC_PROC_RETURN_TYPE mover_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE mover_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    //MTT_print("{\n");
    //MTT_print("Evaluating world: %s id=[%llu]\n", __func__, thing->eval_index);
    
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    float32 timestep = world->timestep;
    
    switch (thing->logic.option) {
    default: {
        vec3 out_vel = vec3(0.0f);
        vec3 out_acc = vec3(0.0f);
        vec3 out_deacc = vec3(0.0f);
        vec3 out_vel_current = vec3(0.0f);
        //        vec3 out_acc_current  = vec3(0.0f);
        
        auto* vel = mtt::access<vec3>(thing, "velocity");
        if (vel) {
            out_vel = *vel;
        }
        
        auto* acc = mtt::access<vec3>(thing, "acceleration");
        if (acc) {
            out_acc = *acc;
        }
        
        auto* deacc = mtt::access<vec3>(thing, "deceleration");
        if (deacc) {
            out_deacc = *deacc;
        }
        
        auto* vel_current = mtt::access<vec3>(thing, "velocity current");
        if (vel_current) {
            out_vel_current = *vel_current;
        }
        
        //        auto* acc_current = mtt::access<vec3>(thing, "acceleration current");
        //        if (acc_current) {
        //            out_acc_current = *acc_current;
        //        }
        
        
        
        // activate
        
        bool on_state = true;
        uint64 port_activate = 4;
        if (name_to_in_port_id(world, thing->id, "active", &port_activate)) {
            
            auto result_in = get_in_port(world, thing, input, port_activate);
            
            if (result_in.status == mtt::PORT_STATUS_OK) {
                
                switch (result_in.value.out.type) {
                case MTT_FLOAT: {
                    float32 activate = result_in.value.out.Float;
                    on_state = (bool)activate;
                    break;
                }
                case MTT_BOOLEAN: {
                    bool activate = result_in.value.out.Boolean;
                    on_state = activate;
                    //MTT_print("ON_STATE=[%s]\n", (on_state) ? "on" : "off");
                    
                    break;
                }
                default: { break; }
                }
            }
        }
        
        /*
         WITH_THING(moved_thing, {
         
         })
         */
        
        if (on_state) {
            {
                
                {
                    auto vel_in = mtt::get_in_port(thing, input, "velocity");
                    if (vel_in.status == PORT_STATUS_OK) {
                        out_vel_current = vel_in.value.out.Vector3;
                    }
                }
                {
                    auto acc_in = mtt::get_in_port(thing, input, "acceleration");
                    if (acc_in.status == PORT_STATUS_OK) {
                        out_acc = acc_in.value.out.Vector3;
                    }
                    
                    auto deac_in = mtt::get_in_port(thing, input, "deceleration");
                    if (acc_in.status == PORT_STATUS_OK) {
                        out_deacc = deac_in.value.out.Vector3;
                    }
                }
                auto result = get_out_port(world, thing, 0);
                if (result.status == mtt::PORT_STATUS_OK) {
                    //MTT_print("velocity TYPE = [%s], writing [%f,%f,%f]\n", meta[result.value.out.type].name.c_str(), out_vel.x, out_vel.y, out_vel.z);
                    
                    //MTT_print("%d\n", __LINE__);
                    
                    result.value.out.set_Vector3(out_vel_current);
                    
                } else {
                    result.value.out.set_Vector3(vec3(0.0f));
                }
            }
            {
                auto result = get_out_port(world, thing, 1);
                if (result.status == mtt::PORT_STATUS_OK) {
                    //MTT_print("acceleration TYPE = [%s], writing [%f,%f,%f]\n", meta[result.value.out.type].name.c_str(), out_acc.x, out_acc.y, out_acc.z);
                    
                    result.value.out.set_Vector3(out_acc);
                }
            }
            
            //if (out_vel - out_vel_current == vec3(0.0f))
            
            
            
            vec3 vel_current_signs = m::sign(out_vel_current);
            vec3 vel_signs = m::sign(out_vel);
            
            
            for (u8 i = 0; i < 3; i += 1) {
                bool diff_signs = (vel_current_signs[i] != 0.0) && (vel_current_signs[i] != vel_signs[i]);
                
                
                out_vel_current[i] += ((diff_signs) ? (((float32)diff_signs * out_deacc[i])) : (((float32)!diff_signs) * out_acc[i]))* out_vel[i] * timestep;
            }
            // velocity +=  (TARGET_SPEED / (1 - STRENGTH) - (TARGET_SPEED / (1 - DAMPENING))
            // (some number you choose to control acceleration) * (target_speed - current_speed)
            
            
            {
                float32 mag_current = m::length(out_vel_current);
                float32 mag_max     = m::length(out_vel);
                if (mag_current > mag_max) {
                    out_vel_current *= mag_max / mag_current;
                }
            }
            //MTT_print("v=[%f,%f,%f]\n", out_vel_current.x, out_vel_current.y, out_vel_current.z);
            *vel_current = out_vel_current;
        } else {
            //MTT_print("OFF\n");
            vec3 out_vel_current = *vel_current;
            
            auto result = get_out_port(world, thing, 0);
            
            if (result.status == mtt::PORT_STATUS_OK) {
                //MTT_print("velocity TYPE = [%s], writing [%f,%f,%f]\n", meta[result.value.out.type].name.c_str(), out_vel.x, out_vel.y, out_vel.z);
                
                //MTT_print("%d\n", __LINE__);
                
                result.value.out.set_Vector3(out_vel_current);
                
            }
            
            
            vec3 signs_before = m::sign(out_vel_current);
            
            out_vel_current -= out_deacc * out_vel * timestep;
            for (u8 i = 0; i < 3; i += 1) {
                if (out_deacc[i] == POSITIVE_INFINITY || (m::sign(out_vel_current[i]) != signs_before[i])) {
                    out_vel_current[i] = 0.0f;
                }
            }
            
            *vel_current = out_vel_current;
        }
        
        
        
        break;
    }
    }
    
    //MTT_print("}\n");
    
    return true;
}

mtt::Polycurve nil_control_curve = {};


Procedure_Return_Type builtin_procedure_curve_out_points(void* state, Procedure_Input_Output* args);
Procedure_Return_Type builtin_procedure_curve_out_points(void* state, Procedure_Input_Output* args)
{
    const auto* control_curve               = (Control_Curve_Info*)state;
    const mtt::Polycurve& curve             = *(mtt::Polycurve*)(control_curve->points);
    const mtt::Polycurve& curve_dist        = *(mtt::Polycurve*)(control_curve->distance_sum);
    
    const vec2* input = (vec2*)(args->input);
    const float32 distance_traveled_current = (*input)[0];
    const float32 dist = m::min(distance_traveled_current, curve_dist.back().back().x);
    
    vec3 out = {0.0, 0.0, 0.0};
    // arc length parameterization
    usize idx = 0;
    const auto& curve_last      = curve.back();
    const auto& curve_dist_last = curve_dist.back();
    
    for (usize i = 0; i < curve_dist_last.size(); i += 1) {
        if (curve_dist_last[i].x >= dist) {
            usize j = i;
            // FIXME: probably some problem with arc length sums that causes the final element to be the same as the second-to-last one
            while (j < curve_dist_last.size() - 1 && curve_dist_last[i].x == curve_dist_last[j].x) {
                j += 1;
            }
            idx = j;
            break;
        }
    }
    args->output_flags = 0; // FLAG STARTED (not done by default)
    if (idx == curve_dist_last.size() - 1) {
        out =  curve_last[idx];
        args->output_flags = 2; // FLAG DONE
    } else {
        if (idx > 0 && idx < curve_dist_last.size() - 1) {
            float32 t = (dist - curve_dist_last[idx - 1].x) / (curve_dist_last[idx].x - curve_dist_last[idx - 1].x);
            vec3 a = curve_last[idx - 1];
            vec3 b = curve_last[idx];
            
            out = m::lerp(a, b, t);
            
            args->output_flags = 1; // FLAG IN-BETWEEN
        } else if (idx == 0) {
            out = curve_last[0];
            args->output_flags = 0; // FLAG BEGINNING
        }
    }
    
    args->output.type    = MTT_VECTOR3;
    args->output.Vector3 = out;
    
    //std::cout << "[ " << m::to_string(out) << "]" << std::endl;
    
    return Procedure_Return_Type();
}

Procedure_Return_Type builtin_procedure_curve_out_parametric(void* state, Procedure_Input_Output* args);
Procedure_Return_Type builtin_procedure_curve_out_parametric(void* state, Procedure_Input_Output* args)
{
    return Procedure_Return_Type();
}

LOGIC_PROC_RETURN_TYPE control_curve_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    //MTT_print("Evaluating world: %s id=[%llu]\n", __func__, thing->eval_index);
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    switch (thing->logic.option) {
    case CONTROL_CURVE_OPTION_POINTS: {
        auto curve_out = get_out_port(world, thing, "curve");
        if (curve_out.status == mtt::PORT_STATUS_OK) {
            
            mtt::Rep* rep;
            mtt::rep(thing, &rep);
            
            {
                auto& points = rep->points;
                if (points.size() == 0 ||
                    points[0].size() == 0) {
                    curve_out.value.is_ignored = true;
                    return true;
                }
                auto& points_alt = rep->points_alt;
                
                Procedure& proc = curve_out.value.out.Procedure;
                proc.state = world->allocator_temporary().do_allocate(sizeof(Control_Curve_Info));
                proc.handler = builtin_procedure_curve_out_points;
                
                Control_Curve_Info* cci = (Control_Curve_Info*)proc.state;
                cci->points       = (uintptr)&points;
                cci->distance_sum = (uintptr)&points_alt;
                curve_out.value.out.set_Reference_Type((uintptr)cci);
                
                if (points_alt.empty()) {
                    curve_out.value.is_ignored = true;
                }
                
            }
        }
        break;
    }
    case CONTROL_CURVE_OPTION_PARAMETRIC: {
        const auto* first_point     = mtt::access<vec3>(thing, "first_point_anchor");
        const auto* last_point      = mtt::access<vec3>(thing, "last_point_anchor");
        const auto* do_regeneration = mtt::access<bool>(thing, "do_regeneration");
        const auto* procedure       = mtt::access<mtt::Procedure>(thing, "function");
        assert(first_point     != nullptr &&
               last_point      != nullptr &&
               do_regeneration != nullptr);
        
        auto curve_out = get_out_port(world, thing, "curve");
        if (curve_out.status == mtt::PORT_STATUS_OK) {
            
            Procedure& proc = curve_out.value.out.Procedure;
            proc.state = world->allocator_temporary().do_allocate(sizeof(Control_Curve_Info));
            // TODO: test
            //(void*)&(curve_out.value.out.Control_Curve);
            
            
            
            
            proc.handler = builtin_procedure_curve_out_parametric;
            curve_out.value.is_ignored = false;
        } else {
            curve_out.value.is_ignored = true;
        }
    }
    }
    
    return true;
}

LOGIC_PROC_RETURN_TYPE power_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE power_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    //MTT_print("Evaluating world: %s id=[%llu]\n", __func__, thing->eval_index);
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    auto* power = mtt::access<float32>(thing, "power");
    float32 out_val = 1.0f;
    if (power) {
        out_val = *power;
    }
    
    auto result = get_out_port(world, thing, 0);
    if (result.status == mtt::PORT_STATUS_OK) {
        result.value.out.set_Float(out_val);
    }
    
    auto result_bool = get_out_port(world, thing, 1);
    if (result_bool.status == mtt::PORT_STATUS_OK) {
        result_bool.value.out.set_Boolean(out_val >= 1.0f);
    }
    
    
    return true;
}

LOGIC_PROC_RETURN_TYPE vector_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE vector_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    
    
    //MTT_print("Evaluating world: %s id=[%llu]\n", __func__, thing->eval_index);
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    auto* vector = mtt::access<vec4>(thing, "vector");
    
    vec4 out_val = *vector;
    auto IN_x = get_in_port(world, thing, input, "x");
    if (IN_x.status == mtt::PORT_STATUS_OK) {
        out_val.x = IN_x.value.out.Float;
    }
    auto IN_y = get_in_port(world, thing, input, "y");
    if (IN_y.status == mtt::PORT_STATUS_OK) {
        out_val.y = IN_x.value.out.Float;
    }
    auto IN_z = get_in_port(world, thing, input, "z");
    if (IN_z.status == mtt::PORT_STATUS_OK) {
        out_val.z = IN_x.value.out.Float;
    }
    auto IN_w = get_in_port(world, thing, input, "w");
    if (IN_w.status == mtt::PORT_STATUS_OK) {
        out_val.w = IN_x.value.out.Float;
    }
    *vector = out_val;
    //MTT_print("out_val: [%f,%f,%f]\n", out_val[0], out_val[1], out_val[2]);
    
    auto result = get_out_port(world, thing, 0);
    if (result.status == mtt::PORT_STATUS_OK) {
        result.value.out.set_Vector4(out_val);
    }
    
    
    
    
    return true;
    
    
}

LOGIC_PROC_RETURN_TYPE destroyer_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE destroyer_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    auto IN_trigger           = get_in_port(world, thing, input, 0);
    //auto IN_affects_connected = get_in_port(world, thing, input, 1);
    
    // sends destruction signal to things
    auto OUT_affect_thing = get_out_port(world, thing, 0);
    // sends signal that destruction is happening
    auto OUT_trigger      = get_out_port(world, thing, 1);
    
    
    float32 time_delay = *mtt::access<float32>(thing, "time_delay");
    
    uint64 out_trigger_value      = 0x0;
    
    bool is_ready = true;
    if (IN_trigger.status == mtt::PORT_STATUS_OK) {
        switch (IN_trigger.value.out.type) {
        case MTT_FLOAT: {
            if (IN_trigger.value.out.Float < 1.0f) {
                is_ready = false;
            }
            break;
        }
        case MTT_BOOLEAN: {
            if (!IN_trigger.value.out.Boolean) {
                is_ready = false;
            }
            break;
        }
        default: {
            return true;
        }
        }
    }
    
    
    if (OUT_affect_thing.status == mtt::PORT_STATUS_OK) {
        if (is_ready) {
            world->to_destroy.emplace_back((Destroy_Command) {
                thing->id,
                true,
                0.0f,
            });
            Destroy_Command* ptr = (Destroy_Command*)world->allocator_temporary().do_allocate(sizeof(Destroy_Command));
            OUT_affect_thing.value.out.set_Reference_Type((uintptr)ptr);
            Destroy_Command& destroy =  *ptr;
            destroy.affects_connected = true;
            destroy.thing_id          = OUT_affect_thing.value.ID;
            destroy.time_remaining    = time_delay;
            destroy.time_delay        = time_delay;
            OUT_affect_thing.value.is_ignored = false;
            
            if (OUT_trigger.status == mtt::PORT_STATUS_OK) {
                out_trigger_value = DESTROY_IS_ACTIVE_MASK | DESTROY_CONNECTIONS_MASK;
                OUT_trigger.value.out.set_Int64((int64)(out_trigger_value));
            }
            
        } else {
            OUT_affect_thing.value.is_ignored = true;
            if (OUT_trigger.status == mtt::PORT_STATUS_OK) {
                OUT_trigger.value.out.set_Int64(0);
            }
        }
        
        
    } else {
        if (is_ready) {
            world->to_destroy.emplace_back((Destroy_Command) {
                thing->id,
                true,
                0.0f
            });
            
            if (OUT_trigger.status == mtt::PORT_STATUS_OK) {
                out_trigger_value = DESTROY_IS_ACTIVE_MASK | DESTROY_CONNECTIONS_MASK;
                OUT_trigger.value.out.set_Int64((int64)(out_trigger_value));
            }
            
        } else {
            if (OUT_trigger.status == mtt::PORT_STATUS_OK) {
                OUT_trigger.value.out.set_Int64(0);
            }
        }
    }
    
    
    return true;
}

LOGIC_PROC_RETURN_TYPE text_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE text_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    auto* arrows = mtt::arrow_links(world);
    auto& fwd = arrows->edges_forward;
    auto& rev = arrows->edges_reverse;
    auto find_incoming = rev.find(thing->id);
    auto find_outgoing = fwd.find(thing->id);
    
    mtt::String text = mtt::text_get_value(thing);
    if (find_incoming != rev.end() && find_outgoing != fwd.end()) {
        // FIXME: could save a bidrectional flag instead to avoid the recomputation
        for (const Arrow_Link& incoming : find_incoming->second) {
            mtt::Thing_try_get_then(world, incoming.id, [&](mtt::Thing* thing_src) {
                if (thing_src->archetype_id != mtt::ARCHETYPE_TEXT) {
                    return;
                }
                
                mtt::String text_src = mtt::text_get_value(thing_src);
                
                for (const Arrow_Link& outgoing : find_outgoing->second) {
                    mtt::Thing_try_get_then(world, outgoing.id, [&](mtt::Thing* thing_dst) {
                        if (thing_dst->archetype_id != mtt::ARCHETYPE_TEXT) {
                            return;
                        }
                        
                        mtt::String text_dst = mtt::text_get_value(thing_dst);
                        
                        dt::Word_Dictionary_Entry* entry = dt::verb_add(text);
                        
                        // TODO: this only works for bidirectional when src and tgt are the same (count 2)
                        
                        int find_src_in_dst = 0;
                        int find_dst_in_src = 0;
                        for (const Arrow_Link& incoming_rev : find_incoming->second) {
                            
                            mtt::Thing* in_rev_thing = mtt::Thing_try_get(world, incoming_rev.id);
                            
                            if (in_rev_thing != nullptr && mtt::text_get_value(in_rev_thing) == text_dst) {
                                find_dst_in_src += 1;
                                if (find_dst_in_src == 2) {
                                    break;
                                }
                            }
                        }
                        for (const Arrow_Link& outgoing_rev : find_outgoing->second) {
                            
                            mtt::Thing* out_rev_thing = mtt::Thing_try_get(world, outgoing_rev.id);
                            if (out_rev_thing != nullptr && mtt::text_get_value(out_rev_thing) == text_src) {
                                find_src_in_dst += 1;
                                if (find_src_in_dst == 2) {
                                    break;
                                }
                            }
                        }
                        
                        if (find_src_in_dst == 2 && find_dst_in_src == 2) {
                            entry->verb_is_bidirectional_by_default = true;
                        } else {
                            entry->verb_is_bidirectional_by_default = false;
                        }
                    });
                }
                
            });
        }
    }
    //    auto* position = mtt::access<vec3>(thing, "position");
    //
    //    auto* val_field = mtt::access<MTT_String_Ref>(thing, "value");
    
    
    //    char buffer[64];
    //    snprintf(buffer, 64, "%.2f", *value);
    //    //    char *p = strstr(buffer, ".00");
    //    //    if (p) *p = 0x0;
    //    mtt::String new_label = buffer;
    //
    //
    //
    //    if (!MTT_string_ref_is_equal_cstring(thing->label, new_label.c_str())) {
    //        mtt::Thing_set_label(thing, new_label);
    //
    //        mtt::Rep* rep;
    //        mtt::rep(thing, &rep);
    //        Collider* c = rep->colliders.front();
    //
    //    mtt:Collider_remove(c->system, 0, c);
    //
    //        auto* vg = nvgGetGlobalContext();
    //        nvgSave(vg);
    //
    //
    //        cstring cs = new_label.c_str();
    //
    //        struct Settings {
    //            float line_h;
    //        } font_settings;
    //        font_settings.line_h = 0;
    //        nvgFontSize(vg, 64.0f);
    //        nvgFontFace(vg, "sans-mono");
    //        nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
    //        nvgTextMetrics(vg, NULL, NULL, &font_settings.line_h);
    //
    //        {
    //            float bounds[4];
    //            nvgTextBounds(vg, 0, 0, cs, NULL, bounds);
    //
    //            vec2 extent = vec2(bounds[2], bounds[3]);
    //            vec2 half_extent = extent / 2.0f;
    //
    //            c->aabb.half_extent = half_extent;
    //            c->aabb.tl = vec2(c->center_anchor) - half_extent;
    //            c->aabb.br = vec2(c->center_anchor) + half_extent;
    //        }
    //
    //        nvgRestore(vg);
    //
    //
    //        mtt::push_AABB(c->system, c);
    //
    //        //auto* is_changed = mtt::access<bool>(thing, "is_changed");
    //        //*is_changed = true;
    //    } else {
    //        //auto* is_changed = mtt::access<bool>(thing, "is_changed");
    //        //*is_changed = false;
    //    }
    
    
    
    
    
    
    
    
    
    return true;
}

LOGIC_PROC_RETURN_TYPE timer_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE timer_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    // TODO: //
    // FIXME: use delta time instead of internal?
    auto* elapsed = mtt::access<float32>(thing, "elapsed");
    if (elapsed == nullptr) {
        return false;
    }
    float32 elapsed_value = *elapsed;
    
    auto* duration = mtt::access<float32>(thing, "duration");
    if (duration == nullptr) {
        return false;
    }
    float32 duration_value = *duration;
    
    bool* prev_reset = mtt::access<bool>(thing, "previous reset state");
    if (prev_reset == nullptr) {
        return false;
    }
    bool prev_reset_val = *prev_reset;
    
    auto* initial_time_ptr = mtt::access<float32>(thing, "initial_time");
    if (initial_time_ptr == nullptr) {
        return false;
    }
    const float32 initial_time = *initial_time_ptr;
    
    auto speed = *mtt::access<float32>(thing, "speed");
    
    
    auto IN_started  = get_in_port(world, thing, input, 0);
    auto IN_reset    = get_in_port(world, thing, input, 1);
    
    auto OUT_result  = get_out_port(world, thing, 0);
    
    if (IN_started.status == mtt::PORT_STATUS_OK) {
        if (IN_reset.status == mtt::PORT_STATUS_OK) {
            
            if (IN_reset.value.out.Boolean == true &&
                prev_reset_val == false) {
                
                *elapsed = initial_time;
                if (OUT_result.status == true) {
                    OUT_result.value.out.set_Float(initial_time);
                }
            }
            
            *prev_reset = IN_reset.value.out.Boolean;
        } else {
            
            if (IN_started.value.out.Float == 0.0) {
                if (OUT_result.status == true) {
                    OUT_result.value.out.set_Float(initial_time / duration_value);
                }
            } else {
                if (OUT_result.status == mtt::PORT_STATUS_OK) {
                    OUT_result.value.out.set_Float(((float64)(*elapsed)/ (float64)duration_value));
                }
                
                float32 next_result = m::clamp((float32)(elapsed_value + (MTT_TIMESTEP * speed)), 0.0f, duration_value);
                
                *elapsed = (next_result >= duration_value) ? duration_value : next_result;
            }
            
            *prev_reset = false;
        }
        
    } else {
        if (IN_reset.status == mtt::PORT_STATUS_OK) {
            
            if (IN_reset.value.out.Boolean == true &&
                prev_reset_val == false) {
                
                *elapsed = initial_time;
                if (OUT_result.status == mtt::PORT_STATUS_OK) {
                    OUT_result.value.out.set_Float(initial_time / duration_value);
                }
            }
            
            *prev_reset = IN_reset.value.out.Boolean;
        } else {
            
            if (OUT_result.status == mtt::PORT_STATUS_OK) {
                OUT_result.value.out.set_Float(((float64)(*elapsed) / (float64)duration_value));
            }
            
            float32 next_result =  m::clamp((float32)(elapsed_value + (MTT_TIMESTEP * speed)), 0.0f, duration_value);
            //MTT_print("elapsed before: %f\n", *elapsed);
            *elapsed = (next_result >= duration_value) ? duration_value : next_result;
            //MTT_print("elapsed after: %f\n", *elapsed);
            *prev_reset = false;
        }
    }
    
    
    return true;
}


LOGIC_PROC_RETURN_TYPE tag_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE tag_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    
    /*
     Thing_add_in_port({world, fields, ports, logic,  "active", MTT_FLOAT, 0, {ALIGN_UP + ALIGN_LEFT}, nullptr});
     Thing_add_in_port({world, fields, ports, logic,  "negate",      MTT_FLOAT,           0,       {ALIGN_LEFT}, nullptr});
     Thing_add_in_port({world, fields, ports, logic,  "input-signal-single", MTT_FLOAT,           0,       {ALIGN_LEFT}, nullptr});
     Thing_add_in_port({world, fields, ports, logic,  "input-thing", MTT_INT64,           0,       {ALIGN_LEFT}, nullptr});
     
     Thing_add_out_port({world, fields, ports, logic, "output-signal-single", MTT_FLOAT, 0,
     {ALIGN_RIGHT + (ALIGN_UP * 0.25f)}, nullptr
     });
     Thing_add_out_port({world, fields, ports, logic, "output-thing", MTT_INT64, 0,
     {ALIGN_RIGHT + (ALIGN_DOWN * 0.25f)}, nullptr
     });
     
     */
    
    return true;
}

LOGIC_PROC_RETURN_TYPE sensor_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE sensor_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    
    
    //mtt::Representation& rep = world->things.representation.find(thing->instance_id)->second;
    
    //    switch (thing->logic.option) {
    //    case LOGIC_OPTION_SENSOR_CIRCLE_BOUND:
    {
        //        world->message_passer
        
        Messages_Record* msgs = messages(&world->message_passer, thing->id);
        
        if (!thing_group_is_active(thing)) {
            if (!msgs->from_entities.empty()) {
                msgs->from_entities.clear();
            }
            if (!msgs->from_tags.empty()) {
                msgs->from_tags.clear();
            }
            return true;
        }
        
        //auto it_self = world->things.representation.find(thing->instance_id);
        
        usize received_message_count = 0;
        
        // TODO: support other types as well eventually
        
        
        usize required_count = *mtt::access<usize>(thing, "required_count");
        
        while (!msgs->from_tags.empty()) {
            Message* msg = &msgs->from_tags.front();
            
            msgs->from_tags.pop_front();
            
            received_message_count += 1;
        }
        
        // FIXME: return more than just boolean, return the type of the signal sent
        auto OUT_signal = get_out_port(world, thing, 0);
        OUT_signal.value.out.set_Boolean(received_message_count >= required_count);
        
        //        break;
    }
    //    case LOGIC_OPTION_SENSOR_RECTANGLE_BOUND: {
    //        break;
    //    }
    //    default: { break; }
    //    }
    
    return true;
}

LOGIC_PROC_RETURN_TYPE toggle_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE toggle_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    bool* const toggle_state_ptr = mtt::access<bool>(thing, "toggle-state");
    bool toggle_state = *toggle_state_ptr;
    
    bool* const prev_value_ptr = mtt::access<bool>(thing, "prev-value");
    const bool prev_value  = *prev_value_ptr;
    
    bool* const is_negated_ptr = mtt::access<bool>(thing, "is-negated");
    const bool is_negated = *is_negated_ptr;
    
    auto IN  = get_in_port(world, thing, input, 0);
    auto OUT  = get_out_port(world, thing, 0);
    
    bool current_value = false;
    
    
    if (IN.status != mtt::PORT_STATUS_OK) {
        OUT.value.out.set_Boolean(toggle_state);
        
        return true;
    }
    
    // TODO: negated version
    // FIXME: will need to do a version where the truth values are negated (inverse)
    switch (IN.value.out.type) {
    case MTT_INT32: {
        auto val = IN.value.out.Int32;
        
        if (prev_value == false && (bool)val == true) {
            toggle_state = !toggle_state;
            *toggle_state_ptr = toggle_state;
        }
        current_value = (bool)val;
        
        OUT.value.out.set_Int32((is_negated ^ toggle_state) ? val : 0);
        break;
    }
    case MTT_INT64: {
        auto val = IN.value.out.Int64;
        
        if (prev_value == false && (bool)val == true) {
            toggle_state = !toggle_state;
            *toggle_state_ptr = toggle_state;
        }
        current_value = (bool)val;
        
        OUT.value.out.set_Int64( (is_negated ^ toggle_state) ? val : 0);
        
        
        break;
    }
    case MTT_FLOAT: {
        auto val = IN.value.out.Float;
        
        if (prev_value == false && (bool)val == true) {
            toggle_state = !toggle_state;
            *toggle_state_ptr = toggle_state;
        }
        current_value = (bool)val;
        
        OUT.value.out.set_Float((float32)( (is_negated ^ toggle_state) ? val : 0.0));
        MTT_print("TOGGLE: is_negated %d toggle state %d val %f\n", is_negated, toggle_state, OUT.value.out.Float);
        
        
        
        break;
    }
    case MTT_BOOLEAN: {
        //std::cout << "IN=[[[" << IN_value.value.out.Boolean << "]]]" << std::endl;
        auto val = IN.value.out.Boolean;
        
        if (prev_value == false && (bool)val == true) {
            toggle_state = !toggle_state;
            *toggle_state_ptr = toggle_state;
        }
        current_value = (bool)val;
        
        
        // FIXME: need to cache values for other types
        OUT.value.out.set_Boolean((int64)(is_negated ^ toggle_state));
        
        
        break;
    }
    case MTT_VECTOR4: {
        auto val = IN.value.out.Vector4;
        
        bool truth_val = (bool)(val[0] + val[1] + val[2] + val[3]);
        if (prev_value == false && truth_val == true) {
            toggle_state = !toggle_state;
            *toggle_state_ptr = toggle_state;
        }
        current_value = (bool)truth_val;
        
        
        OUT.value.out.set_Vector4((is_negated ^ toggle_state) ? val : vec4(0.0));
        
        
        break;
    }
    case MTT_VECTOR3: {
        auto val = IN.value.out.Vector3;
        
        bool truth_val = (bool)(val[0] + val[1] + val[2]);
        if (prev_value == false && truth_val == true) {
            toggle_state = !toggle_state;
            *toggle_state_ptr = toggle_state;
        }
        current_value = (bool)truth_val;
        
        
        OUT.value.out.set_Vector3((is_negated ^ toggle_state) ? val : vec3(0.0));
        
        
        break;
    }
    case MTT_VECTOR2: {
        auto val = IN.value.out.Vector2;
        
        bool truth_val = (bool)(val[0] + val[1]);
        if (prev_value == false && truth_val == true) {
            toggle_state = !toggle_state;
            *toggle_state_ptr = toggle_state;
        }
        current_value = (bool)truth_val;
        
        
        OUT.value.out.set_Vector2((is_negated ^ toggle_state) ? val : vec2(0.0));
        
        
        break;
    }
    default: { MTT_error("%s", "Input type not covered!"); break; }
    }
    
    //std::cout << "OUT=[[[" << OUT_value.value.out.Boolean << "]]]" << std::endl;
    *prev_value_ptr = current_value;
    
    return true;
}


LOGIC_PROC_RETURN_TYPE group_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE group_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    bool parent_active = true;
    if (!thing_group_is_active(thing)) {
        parent_active = false;
    }
    
    auto IN_active = get_in_port(world, thing, input, 0);
    
    if (IN_active.status == mtt::PORT_STATUS_OK) {
        bool input_is_active = parent_active;
        //        switch (IN_active.value.out.type) {
        //        case MTT_FLOAT: {
        input_is_active = (IN_active.value.out.Boolean);
        //            break;
        //        }
        //        case MTT_BOOLEAN: {
        //            input_is_active = parent_active && IN_active.value.out.Boolean;
        //            break;
        //        }
        //        }
        
        mtt::set_is_active_group(thing, input_is_active);
    } else {
        mtt::set_is_active_group(thing, parent_active);
    }
    mtt::set_is_active_group(thing, mtt::is_active(thing));
    //MTT_print("GROUP IS_ACTIVE %llu, %s\n", thing->id, bool_str(thing->is_active_group));
    return true;
}

LOGIC_PROC_RETURN_TYPE selector_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE selector_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    
    auto* prev_cycle_state = mtt::access<bool>(thing, "prev_cycle_state");
    
    auto* index_count_field = mtt::access<usize>(thing, "index_count");
    usize index_count = 0;
    if (index_count_field) {
        index_count = *index_count_field;
    }
    auto* cycle_index_field = mtt::access<uint64>(thing, "cycle_index");
    uint64 cycle_index = 0;
    if (cycle_index_field) {
        cycle_index = *cycle_index_field;
    }
    
    // this decides whether to cycle which output is active
    auto IN_cycle_port = get_in_port(world, thing, input, "cycle");
    if (IN_cycle_port.status == PORT_STATUS_OK) {
        bool prev_cycle_state_local = *prev_cycle_state;
        bool in_val = IN_cycle_port.value.out.Boolean;
        
        if (in_val == true) {
            // check if there's been a change in the signal. If so, adjust
            if (prev_cycle_state_local == false) {
                prev_cycle_state_local = true;
                *prev_cycle_state = true;
                
                // there has been a change to on, so should cycle
                cycle_index = (cycle_index + 1) % index_count;
                *cycle_index_field = cycle_index;
            }
        } else {
            *prev_cycle_state = false;
        }
    } else {
        // no cycle input, so should be off
        *prev_cycle_state = false;
    }
    
    
    // this is the active output
    //auto IN_active_port = get_in_port(world, thing, input, cycle_index + IN_PORT_OFFSET);
    static const usize IN_PORT_OFFSET = 2;
    for (usize i = 0; i < index_count; i += 1) {
        usize idx = i + IN_PORT_OFFSET;
        auto IN_active_port = get_in_port(world, thing, input, idx);
        if (IN_active_port.status == PORT_STATUS_OK) {
            if (IN_active_port.value.out.Boolean == true) {
                cycle_index = i;
                *cycle_index_field = cycle_index;
                break;
            }
        }
    }
    
    {
        for (usize i = 0; i < cycle_index; i += 1) {
            auto OUT = get_out_port(world, thing, i);
            OUT.value.out.set_Boolean(false);
        }
        auto OUT_active_port = get_out_port(world, thing, cycle_index);
        OUT_active_port.value.out.set_Boolean(true);
        for (usize i = cycle_index + 1; i < index_count; i += 1) {
            auto OUT = get_out_port(world, thing, i);
            OUT.value.out.set_Boolean(false);
        }
    }
    
    return true;
}

LOGIC_PROC_RETURN_TYPE emitter_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE emitter_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    // TODO: //
    if (!thing_group_is_active(thing)) {
        return true;
    }
    return true;
}

LOGIC_PROC_RETURN_TYPE converter_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE converter_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    // TODO: //
    if (!thing_group_is_active(thing)) {
        return true;
    }
    return true;
}

LOGIC_PROC_RETURN_TYPE messenger_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE messenger_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    auto IN = get_in_port(world, thing, input, 0);
    if (IN.status == mtt::PORT_STATUS_OK &&
        IN.value.out.Boolean == true) {
        // TODO: broadcast
        //broadcast(&world->system_broadcast, {label, event, status, flags});
    }
    
    return true;
}

LOGIC_PROC_RETURN_TYPE receiver_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE receiver_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    {
        Messages_Record* msgs = messages(&world->message_passer, thing->id);
        
        if (!thing_group_is_active(thing)) {
            if (!msgs->from_entities.empty()) {
                msgs->from_entities.clear();
            }
            if (!msgs->from_tags.empty()) {
                msgs->from_tags.clear();
            }
            return true;
        }
        
        usize received_message_count = 0;
        
        auto OUT = get_out_port(world, thing, 0);
        if (OUT.status != mtt::PORT_STATUS_OK) {
            msgs->from_entities.clear();
            return true;
        }
        
        while (!msgs->from_entities.empty()) {
            Message* msg = &msgs->from_entities.front();
            
            
            // TODO: // finish
            
            // ??? msg->contents
            
            
            msgs->from_entities.pop_front();
            
            received_message_count += 1;
            
            
        }
        
        
    }
    
    return true;
}

#if 0
// NOTE: old
LOGIC_PROC_RETURN_TYPE jump_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE jump_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    auto* is_init = mtt::access<bool>(thing, "is_init");
    if (!thing_group_is_active(thing)) {
        *is_init = false;
        return true;
    }
    
    auto IN_active = mtt::get_in_port(thing, input, "active");
    if (IN_active.status != PORT_STATUS_OK) {
        auto result = mtt::get_out_port(thing, 0);
        if (result.status == PORT_STATUS_OK) {
            result.value.out.set_Boolean(false);
        }
    } else {
        if (IN_active.value.out.Boolean == false) {
            *is_init = false;
            auto result = mtt::get_out_port(thing, 0);
            if (result.status == PORT_STATUS_OK) {
                result.value.out.set_Boolean(false);
            }
            return true;
        }
    }
    
    auto* src      = mtt::access<mtt::Thing_Ref>(thing, "source");
    auto* dst      = mtt::access<mtt::Thing_Ref>(thing, "destination");
    auto* src_selector = mtt::access<MTT_String_Ref>(thing, "source_selector");
    auto* dst_selector = mtt::access<MTT_String_Ref>(thing, "destination_selector");
    
    // TODO: speed
    auto* speed           = mtt::access<float32>(thing, "speed");
    auto* jump_height_ref = mtt::access<float32>(thing, "jump_height");
    auto* cycle_count      = mtt::access<uint64>(thing, "cycle_count");
    
    auto* interpolation_ref = mtt::access<float32>(thing, "interpolation");
    
    auto IN_interpolation = mtt::get_in_port(thing, input, "interpolation");
    
    mtt::Thing_Ref* s_ref = src;
    
    
    mtt::Thing* src_thing = nullptr;
    if (!(s_ref->try_get(&src_thing))) {
        return true;
    }
    
    
    auto* init_src_ptr = mtt::access<vec3>(thing, "initial_position");
    auto* src_position_ptr = mtt::access<vec3>(src_thing, "position");
    if (src_position_ptr == nullptr) {
        MTT_error("%s", "position is required!\n");
        return true;
    }
    mtt::Thing_Ref* d_ref = dst;
    mtt::Thing* dst_thing = nullptr;
    if (!d_ref->try_get(&dst_thing)) {
        return true;
    }
    
    if (!(*is_init)) {
        
        {
            Message msg;
            msg.sender   = thing->id;
            msg.selector = *src_selector;
            selector_invoke(src_thing, &msg);
            ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
            *init_src_ptr = *src_position_ptr;
            
            thing->on_start(thing, src_thing, (void*)dst_thing);
            
        }
        *is_init = true;
        *interpolation_ref = 0.0f;
    }
    
    float32 jump_height = *jump_height_ref;
    
    
    float32 interpolation = 0.0f;
    if (IN_interpolation.status == mtt::PORT_STATUS_OK) {
        interpolation = IN_interpolation.value.out.Float;
        interpolation = m::clamp(interpolation, 0.0f, 1.0f);
        *interpolation_ref = interpolation;
    } else {
        interpolation = *interpolation_ref;
        interpolation = m::clamp(interpolation, 0.0f, 1.0f);
        *interpolation_ref += MTT_TIMESTEP * *speed;
    }
#define IS_DONE (0)
    auto result = mtt::get_out_port(thing, IS_DONE);
#undef IS_DONE
    if (result.status == mtt::PORT_STATUS_OK) {
        result.value.out.set_Boolean(interpolation >= 1.0f);
        
        if (result.value.out.Boolean == true) {
            for (usize idx = 0; idx < 1; idx += 1) {
                mtt::Thing_Ref* s_ref = src;
                
                
                mtt::Thing* src_thing = nullptr;
                
                if (s_ref->try_get(&src_thing)) {
                    Message msg;
                    msg.selector = string_ref_get("set_movement_option");
                    msg.input_value.type = MTT_INT64;
                    msg.input_value.Int64 = OPTION_FLAGS_MOVEMENT_DEFAULT;
                    selector_invoke(src_thing, &msg);
                }
            }
            
            
            
            return false;
        }
    } else if (interpolation >= 1.0f) {
        for (usize idx = 0; idx < 1; idx += 1) {
            mtt::Thing_Ref* s_ref = src;
            
            
            mtt::Thing* src_thing = nullptr;
            
            if (s_ref->try_get(&src_thing)) {
                Message msg;
                msg.selector = string_ref_get("set_movement_option");
                msg.input_value.type = MTT_INT64;
                msg.input_value.Int64 = OPTION_FLAGS_MOVEMENT_DEFAULT;
                selector_invoke(src_thing, &msg);
            }
        }
        
        
        return false;
    }
    
    
    
    
    
    
    
    
    {
        {
            
            
            
            vec3 src_pos = {0.0f, 0.0f, 0.0f};
            vec3 dst_pos = {0.0f, 0.0f, 0.0f};
            vec2 init_src = vec2(init_src_ptr->x, init_src_ptr->y);
            
            
            
            {
                Message msg;
                msg.sender   = thing->id;
                msg.selector = *src_selector;
                selector_invoke(src_thing, &msg);
                ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
                src_pos = msg.output_value.Vector3;
            }
            {
                Message msg;
                msg.sender   = src_thing->id;
                msg.selector = *dst_selector;
                selector_invoke(dst_thing, &msg);
                ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
                dst_pos = msg.output_value.Vector3;
            }
            {
                Message msg;
                msg.selector = string_ref_get("set_movement_option");
                msg.input_value.type = MTT_INT64;
                msg.input_value.Int64 = OPTION_FLAGS_MOVEMENT_POSITIONAL;
                selector_invoke(src_thing, &msg);
            }
            
            {
                
                vec2 cur = vec2(0.0f);
                {
                    const float32 TODO_CEILING_PARAM = NEGATIVE_INFINITY;
                    float32 ceiling = TODO_CEILING_PARAM;
                    
                    interpolation = m::inverse_smoothstep(interpolation);
                    
                    
                    vec2 dst = vec2(dst_pos.x, dst_pos.y);
                    cur.x = m::lerp(init_src.x, dst_pos.x, interpolation);
                    
                    
                    float32 sin_c = (dst.y - jump_height < ceiling) ? (init_src.y - ceiling) * 0.75 : jump_height;
                    
                    
                    cur.y = m::max(ceiling, m::lerp(init_src.y, dst_pos.y, interpolation) + m::sin(interpolation * -MTT_PI_32 * *cycle_count) * m::max(0.0f, sin_c));
                }
                
                //                std::cout << "src=[" << m::to_string(init_src) << "] cur=[" << m::to_string(cur) << "] dst=[" << m::to_string(dst_pos) << "]" << std::endl;
                vec3 original_position = *src_position_ptr;
                
                {
                    mtt::Rep* rep = mtt::rep(src_thing);
                    float32 x_diff = init_src_ptr->x - cur.x;
                    if (m::abs(x_diff) > 10) {
                        if (original_position.x < cur.x) {
                            if (rep->forward_dir.x < 0) {
                                mtt::flip_left_right(src_thing);
                            }
                        } else if (original_position.x > cur.x) {
                            if (rep->forward_dir.x > 0) {
                                mtt::flip_left_right(src_thing);
                            }
                        }
                    }
                }
                *src_position_ptr = vec3(cur.x, cur.y, src_position_ptr->z);
                mtt::Thing_set_position(src_thing, *src_position_ptr);
            }
        }
    }
    
    
    
    
    return true;
}
#endif

LOGIC_PROC_RETURN_TYPE run_group_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE run_group_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    return true;
}

LOGIC_PROC_RETURN_TYPE end_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE end_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    bool* prev_state = mtt::access<bool>(thing, "prev_state");
    
    auto out_signal = mtt::get_out_port(thing, "is_done");
    
    auto result = mtt::get_in_port(world, thing, input, "is_done");
    if (result.status != mtt::PORT_STATUS_OK) {
        if (out_signal.status == PORT_STATUS_OK) {
            out_signal.value.out.set_Boolean(false);
        }
        // no input
        
        *prev_state = false;
        
        
        return true;
    }
    
    
    if (result.value.out.Boolean != *mtt::access<bool>(thing, "expected_end_value")) {
        if (out_signal.status == PORT_STATUS_OK) {
            out_signal.value.out.set_Boolean(false);
        }
        
        *prev_state = false;
        
        // is not done
        return true;
    }
    
    
    if (*prev_state == true) {
        // don't re-trigger
        return true;
    }
    
    
    
    Messages_Record* msgs = mtt::messages(&world->message_passer, thing);
    
    while (!msgs->from_entities.empty()) {
        Message* msg = &msgs->from_entities.front();
        
        
        Procedure_Input_Output io = {};
        io.caller = thing;
        io.input = static_cast<void*>(msg);
        io.input_count = 1;
        msg->proc(&io);
        
        msgs->from_entities.pop_front();
    }
    
    if (out_signal.status == PORT_STATUS_OK) {
        out_signal.value.out.set_Boolean(true);
    }
    
    //    uint64* eval_ctx_id_ptr = mtt::access<uint64>(thing, "eval_ctx_id");
    //    if (eval_ctx_id_ptr == nullptr) {
    //        MTT_error("ERROR: missing evaluation context id!\n");
    //        return true;;
    //    }
    //
    //    uint64* program_id_ptr = mtt::access<Thing_ID>(thing, "program_id");
    //    if (program_id_ptr == nullptr) {
    //        MTT_error("ERROR: missing program id!\n");
    //        return true;;
    //    }
    //
    mtt::Procedure* proc = mtt::access<mtt::Procedure>(thing, "on_end");
    if (proc == nullptr) {
        
        
        *prev_state = true;
        
        return false;
    }
    
    
    Procedure_Input_Output io = {};
    
    io.input = nullptr;
    io.caller = thing;
    io.input_count = 0;
    io.input_flags = 0;
    
    (*proc)(&io);
    
    *prev_state = true;
    
    return false;
}

LOGIC_PROC_RETURN_TYPE distance_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE distance_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    
    // port version
    
    auto s_result   = get_in_port(thing, input, "source");
    auto d_result   = get_in_port(thing, input, "destination");
    auto out_result = get_out_port(thing, "distance");
    
    const vec3 scale = *mtt::access<vec3>(thing, "scale");
    
    if ((s_result.status == mtt::PORT_STATUS_OK &&
         d_result.status == mtt::PORT_STATUS_OK)) {
        
        out_result.value.out.set_Float(m::dist(scale * s_result.value.out.Vector3, scale * d_result.value.out.Vector3));
        
        
        return true;
    }
    
    // reference version
    
    mtt::Thing_Ref* s_ref = mtt::access<mtt::Thing_Ref>(thing, "source");
    mtt::Thing_Ref* d_ref = mtt::access<mtt::Thing_Ref>(thing, "destination");
    
    mtt::Thing* src_thing = nullptr;
    mtt::Thing* dst_thing = nullptr;
    if ((s_ref->try_get(&src_thing) && d_ref->try_get(&dst_thing))) {
        MTT_String_Ref* selector_ref = mtt::access<MTT_String_Ref>(thing, "selector");
        
        vec3 src_vec = vec3(0.0f);
        vec3 dst_vec = vec3(0.0f);
        {
            Message msg;
            msg.sender = src_thing->id;
            msg.selector = *selector_ref;
            selector_invoke(src_thing, &msg);
            src_vec = msg.output_value.Vector3;
        }
        {
            Message msg;
            msg.sender = dst_thing->id;
            msg.selector = *selector_ref;
            selector_invoke(dst_thing, &msg);
            dst_vec = msg.output_value.Vector3;
        }
        
        
        out_result.value.out.set_Float(m::dist(scale * src_vec, scale * dst_vec));
        
        
        return true;
    }
    
    out_result.value.out.set_Float(0.0f);
    
    
    return true;
}



LOGIC_PROC_RETURN_TYPE ui_element_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE ui_element_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    return true;
}

LOGIC_PROC_RETURN_TYPE reference_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE reference_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    auto* refs = mtt::access<mtt::Thing_List>(thing, "references");
    auto in = get_in_port(world, thing, input, "in");
    if (in.status == PORT_STATUS_OK) {
        *refs = *(mtt::Thing_List*)in.value.out.List;
    }
    
    
    auto out = get_out_port(world, thing, "out");
    if (out.status == PORT_STATUS_OK) {
        out.value.out.set_List((uintptr)refs, MTT_THING);
    }
    
    return true;
}

LOGIC_PROC_RETURN_TYPE random_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE random_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    // uint64 MTT_RNG_generate_bounded_range
    
    // float  stb_perlin_noise3( float x,
    //                           float y,
    //                           float z,
    //                           int   x_wrap=0,
    //                           int   y_wrap=0,
    //                           int   z_wrap=0)
    float  noise_out = stb_perlin_noise3(0,
                                         0,
                                         0,
                                         0,
                                         0,
                                         0);
    
    return true;
}

LOGIC_PROC_RETURN_TYPE query_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE query_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    
    
    return true;
}

LOGIC_PROC_RETURN_TYPE number_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE number_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    Field_Event ev = {};
    
    auto* position = mtt::access<vec3>(thing, "position");
    
    
    auto val_in = mtt::get_in_port(world, thing, input, "value");
    
    
        
    
    float32 old_value = 0;
    float32* value = mtt::access_and_modify<float32>(thing, "value", [&](mtt::Thing* thing, float32* val) {
        old_value = *val;
        if (val_in.status == PORT_STATUS_OK) {
            *val = val_in.value.out.Float;
            
            auto polarity_field = *mtt::access<float32>(thing, "polarity_constraint");
            if (polarity_field > 0) {
                *val = m::max(*val, 0.0f);
            } else if (polarity_field < 0) {
                *val = m::min(*val, 0.0f);
            }
            
        } else {
            float32 delay = 1.2f;
            auto& primary_input = thing->input_handlers.state[INPUT_MODALITY_FLAG_PEN];
            auto* delay_elapsed = mtt::access<float32>(thing, "interact_delay");
            if (primary_input.state != UI_TOUCH_STATE_NONE && primary_input.state != UI_TOUCH_STATE_ENDED) {

                DrawTalk_World* cw = static_cast<DrawTalk_World*>(mtt_core_ctx()->user_data);
                auto& view_transform = cw->cam.view_transform;
                
                vec2 canvas_position = view_transform * vec4(*position, 1.0f);
                
                auto dt = world->time_seconds - world->time_seconds_prev;
                
                auto delay_elapsed_now = *delay_elapsed + dt;
                
                float32 dist = m::dist(primary_input.prev_canvas_pos, canvas_position);
                
                vec2 v = primary_input.prev_canvas_pos - canvas_position;
                
                const float64 angle = m::atan2pos_64(v.y, v.x);
                const float32 section_count = 16;
                const usize section_idx = floor(angle / (MTT_TAU / section_count));
                const float32 dist_factor = 32.0f;
                const usize increase_force = (dist + (dist_factor * 0.5)) / dist_factor;
                
                while (delay_elapsed_now >= delay * (1.0f / (float32)(increase_force))) {
                    delay_elapsed_now -= delay * (1.0f / (float32)(increase_force));
                    
                    {
                        if (increase_force != 0) {
                            switch (section_idx) {
                            case 0: // right
                                MTT_FALLTHROUGH;
                            case 15:
                                if (*val == 0.0f) {
                                    *val = (1.0f/10.0f);
                                } else {
                                    *val *= (1.0f/10.0f);
                                }
                                break;
                                
                            case 3: // up
                                MTT_FALLTHROUGH;
                            case 4:
                                *val += (1.0f);
                                break;
                                
                            case 7: // left
                                MTT_FALLTHROUGH;
                            case 8:
                                if (*val == 0.0f) {
                                    *val = (10.0f);
                                } else  {
                                    *val *= (10.0f);
                                }
                                break;
                                
                            case 11: // down
                                MTT_FALLTHROUGH;
                            case 12:
                                *val -= (1.0f);
                                break;
                            default:
                                break;
                            }
                        }
                        
                    }
                }
                auto polarity_field = *mtt::access<float32>(thing, "polarity_constraint");
                if (polarity_field > 0) {
                    *val = m::max(*val, 0.0f);
                } else if (polarity_field < 0) {
                    *val = m::min(*val, 0.0f);
                }
                
                if (increase_force > 0) {
                    *delay_elapsed = delay_elapsed_now;
                }
            } else {
                *delay_elapsed = 0.0f;
            }
        }
    }, &ev);
    
    if (*value == -0.0f) {
        *value = 0.0f;
    }
    
    if (old_value < *value) {
        Thing_add_action_event_instantaneous(thing, dt::verb_add("increase"), nullptr, dt::VERB_EVENT_BEGIN);
    } else if (old_value > *value) {
        Thing_add_action_event_instantaneous(thing, dt::verb_add("decrease"), nullptr, dt::VERB_EVENT_BEGIN);
    }
    
    //Field_Event_print(&ev);
    
    auto val_out = mtt::get_out_port(world, thing, "value");
    if (val_out.status == PORT_STATUS_OK) {
        val_out.value.out.set_Float(*value);
    }
    

    
    //std::cout << m::to_string(*position) << std::endl;
    
    auto position_out = mtt::get_out_port(world, thing, "position");
    if (position_out.status == PORT_STATUS_OK) {
        position_out.value.out.set_Vector3(*position);
    }
    
    /*
     auto* vg = nvgGetGlobalContext();
     nvgSave(vg);
     {
     struct Settings {
     float line_h;
     } font_settings;
     font_settings.line_h = 0;
     nvgFontSize(vg, 64.0f);
     nvgFontFace(vg, "sans");
     nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
     nvgTextMetrics(vg, NULL, NULL, &font_settings.line_h);
     cstring cs = new_label.c_str();
     float bounds[4];
     nvgTextBounds(vg, 0, 0, cs, NULL, bounds);
     */
    
    
    
    char buffer[64];
    snprintf(buffer, 64, "%" MTT_number_thing_format_str, *value);
    //    char *p = strstr(buffer, ".00");
    //    if (p) *p = 0x0;
    mtt::String new_label = buffer;
    
    
    cstring as_cstr = new_label.c_str();
    assert(thing->label.id != 0);
    if (!MTT_string_ref_is_equal_cstring(thing->label, as_cstr)) {
        mtt::Thing_set_label(thing, new_label);
        
        mtt::Rep* rep;
        mtt::rep(thing, &rep);
        Collider* c = rep->colliders.front();
        
        mtt:Collider_remove(c->system, 0, c);
        
        auto* vg = nvgGetGlobalContext();
        nvgSave(vg);
        
        
        cstring cs = new_label.c_str();
        
        struct Settings {
            float line_h;
        } font_settings;
        font_settings.line_h = 0;
        nvgFontSize(vg, 64.0f);
        nvgFontFace(vg, "sans-mono");
        nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
        nvgTextMetrics(vg, NULL, NULL, &font_settings.line_h);
        
        {
            float bounds[4];
            nvgTextBounds(vg, 0, 0, cs, NULL, bounds);
            
            vec2 extent = vec2(bounds[2], bounds[3]);
            vec2 half_extent = extent / 2.0f;
            
            c->aabb.half_extent = half_extent;
            c->aabb.tl = vec2(c->center_anchor) - half_extent;
            c->aabb.br = vec2(c->center_anchor) + half_extent;
        }
        
        nvgRestore(vg);
        
        
        mtt::push_AABB(c->system, c);
        
        //auto* is_changed = mtt::access<bool>(thing, "is_changed");
        //*is_changed = true;
    } else {
        //auto* is_changed = mtt::access<bool>(thing, "is_changed");
        //*is_changed = false;
    }
    
    
    
    
    
    
    
    
    
    return true;
}

float32 number_get_value(mtt::Thing* thing)
{
    return *mtt::access<float32>(thing, "value");
}

void number_update_value(mtt::Thing* thing, float32 value)
{
    auto polarity_field = *mtt::access<float32>(thing, "polarity_constraint");
    if (polarity_field > 0) {
        value = m::max(value, 0.0f);
    } else if (polarity_field < 0) {
        value = m::min(value, 0.0f);
    }
    auto* val_field = mtt::access<float32>(thing, "value");
    float32 old_value = *val_field;
    
    *val_field = value;
    
    if (old_value < value) {
        Thing_add_action_event_instantaneous(thing, dt::verb_add("increase"), nullptr, dt::VERB_EVENT_BEGIN);
    } else if (old_value > value) {
        Thing_add_action_event_instantaneous(thing, dt::verb_add("decrease"), nullptr, dt::VERB_EVENT_BEGIN);
    }
    
    char buffer[64];
    snprintf(buffer, 64, "%" MTT_number_thing_format_str, value);
    //    char *p = strstr(buffer, ".00");
    //    if (p) *p = 0x0;
    mtt::String new_label = buffer;
    assert(thing->label.id != 0);
    mtt::Thing_set_label(thing, new_label);
    assert(thing->label.id != 0);
    mtt::Rep* rep;
    mtt::rep(thing, &rep);
    Collider* c = rep->colliders.front();
    
    mtt:Collider_remove(c->system, 0, c);
    
    auto* vg = nvgGetGlobalContext();
    nvgSave(vg);
    
    
    cstring cs = new_label.c_str();
    
    struct Settings {
        float line_h;
    } font_settings;
    font_settings.line_h = 0;
    nvgFontSize(vg, 64.0f);
    nvgFontFace(vg, "sans-mono");
    nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
    nvgTextMetrics(vg, NULL, NULL, &font_settings.line_h);
    
    {
        float bounds[4];
        nvgTextBounds(vg, 0, 0, cs, NULL, bounds);
        
        vec2 extent = vec2(bounds[2], bounds[3]);
        vec2 half_extent = extent / 2.0f;
        
        c->aabb.half_extent = half_extent;
        c->aabb.tl = vec2(c->center_anchor) - half_extent;
        c->aabb.br = vec2(c->center_anchor) + half_extent;
    }
    
    nvgRestore(vg);
    
    
    mtt::push_AABB(c->system, c);
}

cstring text_get_value(mtt::Thing* thing)
{
    auto* val_field = mtt::access<MTT_String_Ref>(thing, "value");
    return MTT_string_ref_to_cstring(*val_field);
}

void text_update_value(mtt::Thing* thing, mtt::String& value)
{
    cstring cs = value.c_str();
    auto* val_field = mtt::access<MTT_String_Ref>(thing, "value");
    MTT_string_ref_release(val_field);
    (*val_field) = MTT_string_add(0, cs);
    
    
    //    if (value.size() > 4096) {
    //        char* buffer = calloc(value.size(), 2 * sizeof(char));
    //    } else {
    //        char buffer[4096];
    //        snprintf(buffer, value.size(), "%s", as_cstr);
    //    }
    
    //    char *p = strstr(buffer, ".00");
    //    if (p) *p = 0x0;
    mtt::String new_label = value;
    
    mtt::Thing_set_label(thing, new_label);
    
    mtt::Rep* rep;
    mtt::rep(thing, &rep);
    Collider* c = rep->colliders.front();
    
    mtt:Collider_remove(c->system, 0, c);
    
    auto* vg = nvgGetGlobalContext();
    nvgSave(vg);
    
    
    
    
    struct Settings {
        float line_h;
    } font_settings;
    font_settings.line_h = 0;
    nvgFontSize(vg, 64.0f);
    nvgFontFace(vg, "sans");
    nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
    nvgTextMetrics(vg, NULL, NULL, &font_settings.line_h);
    
    {
        float bounds[4];
        nvgTextBounds(vg, 0, 0, cs, NULL, bounds);
        
        vec2 extent = vec2(bounds[2], bounds[3]);
        vec2 half_extent = extent / 2.0f;
        
        c->aabb.half_extent = half_extent;
        c->aabb.tl = vec2(c->center_anchor) - half_extent;
        c->aabb.br = vec2(c->center_anchor) + half_extent;
    }
    
    nvgRestore(vg);
    
    
    mtt::push_AABB(c->system, c);
}

LOGIC_PROC_RETURN_TYPE slider_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE slider_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    auto* value = mtt::access<float32>(thing, "value");
    //    auto val_in = mtt::get_in_port(world, thing, input, "value");
    //    if (val_in.status == PORT_STATUS_OK) {
    //        *value = val_in.value.out.Float;
    //    }
    
    
    auto val_out = mtt::get_out_port(world, thing, "value");
    if (val_out.status == PORT_STATUS_OK) {
        val_out.value.out.set_Float(*value);
    }
    
    
    return true;
}

LOGIC_PROC_RETURN_TYPE difference_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE difference_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    auto* value = mtt::access<mtt::Any>(thing, "value");
    
    auto val_in = mtt::get_in_port(world, thing, input, "value");
    if (val_in.status == PORT_STATUS_OK) {
        
        
        switch (val_in.value.out.type) {
        case MTT_FLOAT: {
            auto val_out = mtt::get_out_port(world, thing, "value");
            
            float32 prev_value = (*value).Float;
            mtt::Any new_val = mtt::Any::from_Float(val_in.value.out.Float);
            *value = new_val;
            
            auto diff = new_val.Float - prev_value;
            if (val_out.status == PORT_STATUS_OK) {
                val_out.value.out.set_Float(diff);
            }
            
            break;
        }
        case MTT_VECTOR2: {
            auto val_out = mtt::get_out_port(world, thing, "value_vector");
            
            vec2 prev_value = (*value).Vector2;
            mtt::Any new_val  = mtt::Any::from_Vector2(val_in.value.out.Vector2);
            *value = new_val;
            
            auto diff = new_val.Vector2 - prev_value;
            if (val_out.status == PORT_STATUS_OK) {
                val_out.value.out.set_Vector4(vec4(diff, 0.0f, 0.0f));
                
            }
            break;
        }
        case MTT_VECTOR3: {
            auto val_out = mtt::get_out_port(world, thing, "value_vector");
            
            vec3 prev_value = (*value).Vector3;
            mtt::Any new_val  = mtt::Any::from_Vector3(val_in.value.out.Vector3);
            *value = new_val;
            
            auto diff = new_val.Vector3 - prev_value;
            if (val_out.status == PORT_STATUS_OK) {
                val_out.value.out.set_Vector4(vec4(diff, 0.0f));
                
            }
            break;
        }
        case MTT_VECTOR4: {
            auto val_out = mtt::get_out_port(world, thing, "value_vector");
            
            vec4 prev_value = (*value).Vector4;
            mtt::Any new_val = mtt::Any::from_Vector4(val_in.value.out.Vector4);
            *value = new_val;
            
            auto diff = new_val.Vector4 - prev_value;
            if (val_out.status == PORT_STATUS_OK) {
                val_out.value.out.set_Vector4(diff);
                
            }
            break;
        }
        }
    }
    
    return true;
}

LOGIC_PROC_RETURN_TYPE sign_procedure(LOGIC_PROCEDURE_PARAM_LIST) {
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    auto val_in = mtt::get_in_port(world, thing, input, "value");
    if (val_in.status == PORT_STATUS_OK) {
        auto val_out = mtt::get_out_port(world, thing, "value");
        if (val_out.status != PORT_STATUS_OK) {
            return true;
        }
        
        switch (val_in.value.out.type) {
        case MTT_INT32: {
            auto val = val_in.value.out.Int32;
            val_out.value.out.set_Int32(m::sign(val));
            break;
        }
        case MTT_INT64: {
            auto val = val_in.value.out.Int64;
            val_out.value.out.set_Int64(m::sign(val));
            break;
        }
        case MTT_FLOAT: {
            auto val = val_in.value.out.Float;
            val_out.value.out.set_Float(m::sign(val));
            break;
        }
        case MTT_VECTOR2: {
            auto val = val_in.value.out.Vector2;
            val_out.value.out.set_Vector2(m::sign(val));
            break;
        }
        case MTT_VECTOR3: {
            auto val = val_in.value.out.Vector3;
            val_out.value.out.set_Vector3(m::sign(val));
            break;
        }
        case MTT_VECTOR4: {
            auto val = val_in.value.out.Vector4;
            val_out.value.out.set_Vector4(m::sign(val));
            break;
        }
        }
        
    }
    /*
     
     */
    
    return true;
}

LOGIC_PROC_RETURN_TYPE value_procedure(LOGIC_PROCEDURE_PARAM_LIST) {
    
    auto* value = mtt::access<Any>(thing, "value");
    auto val_in = mtt::get_in_port(world, thing, input, "value");
    
    if (val_in.status == PORT_STATUS_OK) {
        *value = val_in.value.out;
    }
    
    auto val_out = mtt::get_out_port(world, thing, "value");
    val_out.value.out = *value;
    
    return true;
}

LOGIC_PROC_RETURN_TYPE comparison_procedure(LOGIC_PROCEDURE_PARAM_LIST) {
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    
    auto* comp = mtt::access<float32>(thing, "value");
    
    auto val0_in = mtt::get_in_port(world, thing, input, "first");
    auto val1_in = mtt::get_in_port(world, thing, input, "second");
    if (val0_in.status == PORT_STATUS_OK) {
        if (val1_in.status == PORT_STATUS_OK) {
            auto comp_local = val0_in.value.out.Float - val1_in.value.out.Float;
            *comp = comp_local;
            
            auto val_out = mtt::get_out_port(world, thing, "value");
            if (val_out.status == PORT_STATUS_OK) {
                val_out.value.out.set_Float(comp_local);
            }
        } else {
            auto comp_local = val0_in.value.out.Float;
            *comp = comp_local;
            
            auto val_out = mtt::get_out_port(world, thing, "value");
            if (val_out.status == PORT_STATUS_OK) {
                val_out.value.out.set_Float(comp_local);
            }
        }
    } else if (val1_in.status == PORT_STATUS_OK) {
        
        auto comp_local = -val1_in.value.out.Float;
        *comp = comp_local;
        
        auto val_out = mtt::get_out_port(world, thing, "value");
        if (val_out.status == PORT_STATUS_OK) {
            
            auto val_out = mtt::get_out_port(world, thing, "value");
            if (val_out.status == PORT_STATUS_OK) {
                val_out.value.out.set_Float(comp_local);
            }
        }
    }
    
    
    return true;
}




//Thing_add_in_port({world, fields, ports, logic,  "value0", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
//Thing_add_in_port({world, fields, ports, logic,  "value1", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
//Thing_add_out_port({world, fields, ports, logic,  "value", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });

LOGIC_PROC_RETURN_TYPE and_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE and_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    
    
    
    auto out_value = get_out_port(thing, "value");
    
    
    
    
    if (out_value.status == PORT_STATUS_OK) {
        usize max_count = *mtt::access<uint64>(thing, "max_count");
        bool value = true;
        for (usize i = 0; i < max_count; i += 1) {
            auto in_value = get_in_port(thing, input, i);
            if (in_value.value.out.Boolean == false) {
                value = false;
                break;
            }
        }
        out_value.value.out.set_Boolean(value);
    }
    
    //MTT_print("[AND][%s:%s]\n", bool_str(in_value0.value.out.Boolean), bool_str(in_value1.value.out.Boolean));
    
    return true;
}

LOGIC_PROC_RETURN_TYPE or_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE or_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    
    
    
    auto out_value = get_out_port(thing, "value");
    
    
    
    
    if (out_value.status == PORT_STATUS_OK) {
        usize max_count = *mtt::access<uint64>(thing, "max_count");
        bool value = false;
        for (usize i = 0; i < max_count; i += 1) {
            auto in_value = get_in_port(thing, input, i);
            if (in_value.value.out.Boolean == true) {
                value = true;
                break;
            }
        }
        out_value.value.out.set_Boolean(value);
    }
    
    //MTT_print("[AND][%s:%s]\n", bool_str(in_value0.value.out.Boolean), bool_str(in_value1.value.out.Boolean));
    
    return true;
}

MTT_DEFINE_LOGIC_PROCEDURE(xor)
{
    return LOGIC_PROC_RETURN_TYPE();
}


LOGIC_PROC_RETURN_TYPE not_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE not_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    auto in_value = get_in_port(thing, input, "value");
    
    bool value = false;
    
    if (in_value.status == PORT_STATUS_OK) {
        value = in_value.value.out.Boolean;
    }
    auto out_value = get_out_port(thing, "value");
    if (out_value.status == PORT_STATUS_OK) {
        out_value.value.out.set_Boolean(!value);
    }
    
    //MTT_print("[NOT][%s]\n", bool_str(in_value.value.out.Boolean));
    
    return true;
}

LOGIC_PROC_RETURN_TYPE counter_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE counter_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    auto* prev_state = mtt::access<bool>(thing, "prev_state");
    if (!thing_group_is_active(thing)) {
        *prev_state = false;
        return true;
    }
    
    auto in_active = get_in_port(thing, input, "active");
    if (in_active.status == PORT_STATUS_OK && in_active.value.out.Boolean == false) {
        *prev_state = false;
        return true;
    }
    
    auto* count = mtt::access<uint64>(thing, "count");
    auto* max_count = mtt::access<uint64>(thing, "max_count");
    
    
    auto in_increment = get_in_port(thing, input, "increment");
    if (in_increment.status != PORT_STATUS_OK) {
        *prev_state = false;
    } else {
        if (in_increment.value.out.Boolean == true) {
            if (*prev_state == false) {
                *count = m::min(*max_count, *count + 1);
                *prev_state = true;;
                MTT_print("counter value changed: %llu\n", *count);
                
            }
        } else {
            *prev_state = false;
        }
    }
    
    
    if (*count == *max_count) {
        auto out_result = get_out_port(thing, "result");
        if (out_result.status == PORT_STATUS_OK) {
            out_result.value.out.set_Boolean(true);
        }
        auto out_result_analogue = get_out_port(thing, "result_analogue");
        if (out_result_analogue.status == PORT_STATUS_OK) {
            out_result.value.out.set_Float(1.0f);
        }
        
    } else {
        auto out_result = get_out_port(thing, "result");
        if (out_result.status == PORT_STATUS_OK) {
            out_result.value.out.set_Boolean(false);
        }
        auto out_result_analogue = get_out_port(thing, "result_analogue");
        if (out_result_analogue.status == PORT_STATUS_OK) {
            out_result.value.out.set_Float(((float32)*count)/ ((float32)*max_count));
        }
    }
    
    
    // TODO: reset
    
    return true;
    
    
    
    /*
     Thing_add_field({
     world, fields, ports, logic, "prev_state", MTT_BOOLEAN, 0
     });
     Thing_add_field({
     world, fields, ports, logic, "count", MTT_INT64, 0
     });
     Thing_add_field({
     world, fields, ports, logic, "max_count", MTT_INT64, 0
     });
     
     Thing_add_in_port({world, fields, ports, logic,  "active", MTT_BOOLEAN, 0, {ALIGN_DOWN}, nullptr});
     Thing_add_in_port({world, fields, ports, logic,  "increment", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
     
     Thing_add_in_port({world, fields, ports, logic,  "reset", MTT_BOOLEAN, 0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
     Thing_add_out_port({world, fields, ports, logic, "result", MTT_FLOAT, 0, {{0.0f, 0.5f, 0.0f}}, nullptr });
     */
}



LOGIC_PROC_RETURN_TYPE variable_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE variable_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    return true;
}
LOGIC_PROC_RETURN_TYPE parameter_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE parameter_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    return true;
}

LOGIC_PROC_RETURN_TYPE conditional_brancher_procedure(LOGIC_PROCEDURE_PARAM_LIST);
LOGIC_PROC_RETURN_TYPE conditional_brancher_procedure(LOGIC_PROCEDURE_PARAM_LIST)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    
    auto* prev_cycle_state = mtt::access<bool>(thing, "prev_cycle_state");
    
    auto* index_count_field = mtt::access<usize>(thing, "index_count");
    usize index_count = 0;
    if (index_count_field) {
        index_count = *index_count_field;
    }
    auto* cycle_index_field = mtt::access<uint64>(thing, "cycle_index");
    uint64 cycle_index = 0;
    if (cycle_index_field) {
        cycle_index = *cycle_index_field;
    }
    
    // this decides whether to cycle which output is active
    auto IN_cycle_port = get_in_port(world, thing, input, "cycle");
    if (IN_cycle_port.status == PORT_STATUS_OK) {
        bool prev_cycle_state_local = *prev_cycle_state;
        bool in_val = IN_cycle_port.value.out.Boolean;
        
        if (in_val == true) {
            // check if there's been a change in the signal. If so, adjust
            if (prev_cycle_state_local == false) {
                prev_cycle_state_local = true;
                *prev_cycle_state = true;
                
                // there has been a change to on, so should cycle
                cycle_index = (cycle_index + 1) % index_count;
                *cycle_index_field = cycle_index;
            }
        } else {
            *prev_cycle_state = false;
        }
    } else {
        // no cycle input, so should be off
        *prev_cycle_state = false;
    }
    
    
    // this is the active output
    //auto IN_active_port = get_in_port(world, thing, input, cycle_index + IN_PORT_OFFSET);
    static const usize IN_PORT_OFFSET = 2;
    for (usize i = 0; i < index_count; i += 1) {
        usize idx = i + IN_PORT_OFFSET;
        auto IN_active_port = get_in_port(world, thing, input, idx);
        if (IN_active_port.status == PORT_STATUS_OK) {
            if (IN_active_port.value.out.Boolean == true) {
                cycle_index = i;
                *cycle_index_field = cycle_index;
                break;
            }
        }
    }
    
    {
        for (usize i = 0; i < cycle_index; i += 1) {
            auto OUT = get_out_port(world, thing, i);
            OUT.value.out.set_Boolean(false);
        }
        auto OUT_active_port = get_out_port(world, thing, cycle_index);
        OUT_active_port.value.out.set_Boolean(true);
        for (usize i = cycle_index + 1; i < index_count; i += 1) {
            auto OUT = get_out_port(world, thing, i);
            OUT.value.out.set_Boolean(false);
        }
    }
    
    return true;
}

MTT_DEFINE_LOGIC_PROCEDURE(interpreter)
{
    return LOGIC_PROC_RETURN_TYPE();
}

MTT_DEFINE_LOGIC_PROCEDURE(set_run_prop)
{
    return LOGIC_PROC_RETURN_TYPE();
}

MTT_DEFINE_LOGIC_PROCEDURE(get_run_prop)
{
    return LOGIC_PROC_RETURN_TYPE();
}

MTT_DEFINE_LOGIC_PROCEDURE(bolt)
{
    return LOGIC_PROC_RETURN_TYPE();
}

MTT_DEFINE_LOGIC_PROCEDURE(run)
{
    return LOGIC_PROC_RETURN_TYPE();
}

MTT_DEFINE_LOGIC_PROCEDURE(re_init)
{
    return LOGIC_PROC_RETURN_TYPE();
}



MTT_DEFINE_LOGIC_PROCEDURE(if)
{
    return LOGIC_PROC_RETURN_TYPE();
}




// MARK: Thing archetype initializers



void freehand_sketch_init(ARCHETYPE_PARAM_LIST)
{
    mtt::set_is_actor(archetype);
    
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0}, meta[MTT_VECTOR3].alloc_byte_size + meta[MTT_VECTOR3].alloc_byte_size + meta[MTT_VECTOR3].alloc_byte_size);
    
    add_field({world, fields, ports, logic, "velocity",     MTT_VECTOR3, 0}, builder);
    add_field({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0}, builder);
    add_field({world, fields, ports, logic, "position",     MTT_VECTOR3, 0}, builder);
    
    MTT_ADD_FIELD("acceleration_local", MTT_VECTOR3, MTT_NONE);
    MTT_ADD_FIELD("velocity_local",     MTT_VECTOR3, MTT_NONE);
    
    
    Thing_add_in_port({world, fields, ports, logic,  "color",    MTT_COLOR_RGBA, 0, {{-0.5f, -0.25f, 0.0f}}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "velocity", MTT_VECTOR3, 0, {{-0.5f, 0.0f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "acceleration", MTT_VECTOR3, 0, {{-0.5f, 0.25f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "position", MTT_VECTOR3, 0, {{-0.5f, -0.25f, 0.0f}}, nullptr });
    
    Thing_add_in_port({world, fields, ports, logic,  "option", MTT_INT64, 0, {{-0.5f, -0.25f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "option:flags", MTT_INT64, 0, {{-0.5f, -0.25f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "destroy", MTT_ANY, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    
    Thing_add_in_port({world, fields, ports, logic,  "scale", MTT_VECTOR3, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "color_factor", MTT_VECTOR4, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "color_addition", MTT_VECTOR4, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    
    Thing_add_in_port({world, fields, ports, logic,  "rotation", MTT_VECTOR3, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "transform", MTT_MATRIX4, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic,  "angle_radians", MTT_FLOAT, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic,  "angle_degrees", MTT_FLOAT, 0, {{ALIGN_LEFT + ALIGN_DOWN}}, nullptr });
    
    
    Thing_add_out_port({world, fields, ports, logic, "top",    MTT_VECTOR3, 0, {{0.0f, -0.5f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "right",  MTT_VECTOR3, 0, {{0.5f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "bottom", MTT_VECTOR3, 0, {{0.0f, 0.5f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "left",   MTT_VECTOR3, 0, {{-0.5f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "center", MTT_VECTOR3, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    
    Thing_add_out_port({world, fields, ports, logic, "position", MTT_VECTOR3, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "position_x", MTT_FLOAT, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "position_y", MTT_FLOAT, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "position_y_negated", MTT_FLOAT, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "position_z", MTT_FLOAT, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    
    Thing_add_out_port({world, fields, ports, logic, "scale", MTT_VECTOR3, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    
    Thing_add_out_port({world, fields, ports, logic, "transform", MTT_MATRIX4, 0, {{0.0f, 0.0f, 0.0f}}, nullptr });
    
    
    
    
    auto set_position = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        Message* msg = (Message*)state;
        
        *mtt::access<vec3>(args->caller, "position") = *(vec3*)(args->input);
        
        return Procedure_Return_Type();
    });
    auto set_velocity = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        Message* msg = (Message*)state;
        
        *mtt::access<vec3>(args->caller, "velocity") = *(vec3*)(args->input);
        
        return Procedure_Return_Type();
    });
    auto stop_moving = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        
        *mtt::access<vec3>(args->caller, "velocity") = vec3(0.0f);
        
        return Procedure_Return_Type();
    });
    auto get_center = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);

            
            Thing* thing = args->caller;
            World* world = thing->world();
            args->output.type = MTT_VECTOR3;

            
            auto* center_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *center_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 center_position = vec3((br + tl) / 2.0f, 0.0f);
                    
                    *center_out = center_position;
                    
                    break;
                }
                default: {
                    *center_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    auto get_top_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;
            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.y -= half_extent.y;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, tl.y, 0.0f);
                    
                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    auto get_bottom_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;
            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.y += half_extent.y * mult;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, br.y, 0.0f);
                    
                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    auto get_beside_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        
        mtt::Message* msg = static_cast<mtt::Message*>(args->input);
        
        args->output.type = MTT_VECTOR3;

        Thing* thing = args->caller;

        World* world = thing->world();
        
        Thing* sender = nullptr;
        if (world->Thing_try_get(msg->sender, &sender)) {
            
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;

            
            auto* tgt_pos = mtt::access<vec3>(thing, "position");
            auto* src_pos = mtt::access<vec3>(sender, "position");
            
            // adopt some specification for l / r offset (1 << 1, 1 << 2 ?)
//            if (msg->flags == ???) {
//                
//            }
            
            if (src_pos->x < tgt_pos->x) {
                msg->selector = mtt::string("left_with_offset");
                msg->input_value.set_Float(mult);
                selector_invoke(thing, msg);
                args->output.set_Vector3(msg->output_value.Vector3);
            } else {
                msg->selector = mtt::string("right_with_offset");
                msg->input_value.set_Float(mult);
                selector_invoke(thing, msg);
                args->output.set_Vector3(msg->output_value.Vector3);
            }
            MTT_BP();
        }
        
        
        return true;
    });
    
    auto get_over_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        
        mtt::Message* msg = static_cast<mtt::Message*>(args->input);
        
        
        args->output.type = MTT_VECTOR3;

        Thing* thing = args->caller;

        World* world = thing->world();
        
        Thing* sender = nullptr;
        if (world->Thing_try_get(msg->sender, &sender)) {
            
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;

            
            auto* tgt_pos = mtt::access<vec3>(thing, "position");
            auto* src_pos = mtt::access<vec3>(sender, "position");
            
            mtt::Message* msg_next = msg->next;
            auto* src_pos_init = &msg_next->input_value.Vector3;
            
            if (src_pos_init->x > tgt_pos->x) {
                msg->selector = mtt::string("left_with_offset");
                msg->input_value.set_Float(mult);
                selector_invoke(thing, msg);
                args->output.set_Vector3(msg->output_value.Vector3);
            } else {
                msg->selector = mtt::string("right_with_offset");
                msg->input_value.set_Float(mult);
                selector_invoke(thing, msg);
                args->output.set_Vector3(msg->output_value.Vector3);
            }
            MTT_BP();
        }
        
        
        return true;
    });
    
    auto get_left_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;
            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.x -= 2 * half_extent.x;
                        offset.y += half_extent.y;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, (tl.y + br.y) / 2.0f, 0.0f);
                    
                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    auto get_right_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;

            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.x += 2 * half_extent.x;
                        offset.y += half_extent.y;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, (tl.y + br.y) / 2.0f, 0.0f);

                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    
    
    add_selector(archetype, "position", set_position);
    add_selector(archetype, "velocity", set_velocity);
    add_selector(archetype, "stop",     stop_moving);
    add_selector(archetype, "center",   get_center);
    add_selector(archetype, "top_with_offset",     get_top_with_offset);
    add_selector(archetype, "bottom_with_offset",  get_bottom_with_offset);
    add_selector(archetype, "left_with_offset",    get_left_with_offset);
    add_selector(archetype, "right_with_offset",   get_right_with_offset);
    add_selector(archetype, "beside_with_offset",   get_beside_with_offset);
    add_selector(archetype, "over_with_offset",   get_over_with_offset);
    //    add_selector(archetype, "set_movement_option", MTT_PROC(
    //        Thing* thing = args->caller;
    //        World* world = thing->world();
    //        args->output.type = MTT_NONE;
    //
    //        Message* msg = (Message*)args->input;
    //        auto movement_flag = (uint64)msg->input_value.Int64;
    //        thing->logic.option_flags = modify_bit(thing->logic.option_flags, 0, movement_flag);
    //
    //        return Procedure_Return_Type();
    //    ));
    MTT_add_selector(archetype, "set_movement_option", {
        Thing* thing = args->caller;
        World* world = thing->world();
        args->output.type = MTT_NONE;
        
        Message* msg = (Message*)args->input;
        auto movement_flag = (uint64)msg->input_value.Int64;
        thing->logic.option_flags = modify_bit(thing->logic.option_flags, 0, movement_flag);
        // TODO: switch to using
        //mtt::set_movement_mode(thing, movement_flag);
        
        return Procedure_Return_Type();
    }, nullptr);
    
    archetype->message_handler = [](auto* msg) {
        switch (msg->type) {
        default: {
            break;
        }
        }
    };
    
    archetype->logic.proc = freehand_sketch_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->eval_priority = 1;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        
        
    };
}
void mover_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_VECTOR3].alloc_byte_size +
                                meta[MTT_VECTOR3].alloc_byte_size +
                                meta[MTT_VECTOR3].alloc_byte_size +
                                meta[MTT_VECTOR3].alloc_byte_size +
                                meta[MTT_VECTOR3].alloc_byte_size +
                                meta[MTT_BOOLEAN].alloc_byte_size +
                                meta[MTT_THING].alloc_byte_size
                                );
    add_field({world, fields, ports, logic, "velocity",     MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "deceleration", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "velocity current",     MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "acceleration current", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "active state",         MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "thing", MTT_THING, 0});
    
    
    Thing_add_in_port({world, fields, ports, logic,  "active", MTT_BOOLEAN, 0, {ALIGN_UP + ALIGN_RIGHT}, nullptr});
    
    Thing_add_in_port({world, fields, ports, logic, "velocity", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "deceleration", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    Thing_add_out_port({world, fields, ports, logic, "velocity", MTT_VECTOR3, 0,
        {ALIGN_RIGHT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_out_port({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_out_port({world, fields, ports, logic, "deceleration", MTT_VECTOR3, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    archetype->logic.proc = mover_procedure;
    
}

void follower_init(ARCHETYPE_PARAM_LIST)
{
    
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_VECTOR3].alloc_byte_size + meta[MTT_VECTOR3].alloc_byte_size + meta[MTT_VECTOR3].alloc_byte_size + meta[MTT_VECTOR3].alloc_byte_size + meta[MTT_VECTOR3].alloc_byte_size + meta[MTT_BOOLEAN].alloc_byte_size + meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_THING].alloc_byte_size + meta[MTT_BOOLEAN].alloc_byte_size
                                );
    add_field({world, fields, ports, logic, "velocity",     MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "deceleration", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "velocity current",     MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "acceleration current", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "active state",         MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "interpolation",        MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "time_start",           MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "distance traveled",           MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "thing", MTT_THING, 0});
    
    add_field({world, fields, ports, logic, "loop_follow", MTT_BOOLEAN, 0}, temp_mem<bool>(false));
    
    
    Thing_add_in_port({world, fields, ports, logic,  "active", MTT_BOOLEAN, 0, {ALIGN_UP + ALIGN_RIGHT}, nullptr});
    
    Thing_add_in_port({world, fields, ports, logic,  "target:tag",      MTT_TAG,           0,       {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "target:tag-list", MTT_TAG_LIST,      MTT_TAG, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "target:path",     MTT_POLYCURVE,     0,       {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "target:point",    MTT_VECTOR3,       0,       {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "target:vector",   MTT_VECTOR4,       0,       {ALIGN_LEFT}, nullptr});
    
    Thing_add_in_port({world, fields, ports, logic,  "source:vector",   MTT_VECTOR4,       0,       {ALIGN_RIGHT}, nullptr});
    
    
    Thing_add_out_port({world, fields, ports, logic, "velocity", MTT_VECTOR3, 0,
        {ALIGN_RIGHT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_out_port({world, fields, ports, logic, "acceleration", MTT_VECTOR3, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_out_port({world, fields, ports, logic, "deceleration", MTT_VECTOR3, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    Thing_add_out_port({world, fields, ports, logic,  "option:flags", MTT_INT64, 0, {{-0.5f, -0.25f, 0.0f}}, nullptr });
    
    Thing_add_out_port({world, fields, ports, logic, "position", MTT_VECTOR3, 0,
        {ALIGN_RIGHT + (ALIGN_UP * 0.25f)}, nullptr
    });
    
    Thing_add_out_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    archetype->logic.proc = follower_procedure;
    
    archetype->logic.option = FOLLOWER_OPTION_CHASE;
    archetype->logic.option_flags = 0;
}
void control_curve_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_POLYCURVE].alloc_byte_size +
                                meta[MTT_POLYCURVE].alloc_byte_size +
                                meta[MTT_POLYCURVE].alloc_byte_size +
                                meta[MTT_VECTOR3].alloc_byte_size +
                                meta[MTT_VECTOR3].alloc_byte_size +
                                meta[MTT_BOOLEAN].alloc_byte_size +
                                meta[MTT_PROCEDURE].alloc_byte_size +
                                meta[MTT_POINTER].alloc_byte_size);
    
    
    add_field({
        world, fields, ports, logic, "polycurve", MTT_POLYCURVE, MTT_VECTOR3
    });
    add_field({
        world, fields, ports, logic, "distance_sum_polycurve", MTT_POLYCURVE, MTT_VECTOR3
    });
    add_field({
        world, fields, ports, logic, "key_points", MTT_POLYCURVE, MTT_VECTOR3
    });
    
    add_field({
        world, fields, ports, logic, "first_point_anchor", MTT_VECTOR3, 0
    });
    add_field({
        world, fields, ports, logic, "last_point_anchor", MTT_VECTOR3, 0
    });
    add_field({
        world, fields, ports, logic, "do_regeneration", MTT_BOOLEAN, 0
    });
    add_field({
        world, fields, ports, logic, "function", MTT_PROCEDURE, 0
    });
    
    
    //    {
    //        Field_Create_Info info = {};
    //        info.world = world;
    //        info.field_desc = fields;
    //        info.ports = ports;
    //        info.logic = logic;
    //        info.tag = "function_state";
    //        info.type = MTT_POINTER;
    //        info.contained_type = 0;
    //
    //        Thing_add_field(info);
    //    }
    
    Thing_add_in_port({world, fields, ports, logic,  "first_point", MTT_VECTOR3, 0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "last point", MTT_VECTOR3, 0, {{0.5f, 0.5f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "reset", MTT_FLOAT, 0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    
    // TODO: might consider using a callback for the port position since tthe position can change
    Thing_add_in_port({world, fields, ports, logic,  "max_y_point", MTT_VECTOR3, 0, {{0.0f, -0.5f, 0.0f}}, nullptr });
    
    Thing_add_out_port({world, fields, ports, logic, "curve", MTT_POLYCURVE, 0, {{0.0f, 0.5f, 0.0f}}, nullptr });
    
    archetype->logic.option = mtt::CONTROL_CURVE_OPTION_POINTS;
    
    archetype->logic.proc = control_curve_procedure;
    
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        //
        //        mtt::Curve_Procedure** function_state = (mtt::Curve_Procedure**)mtt::access<void*>(thing, "function_state");
        //        assert(function_state != nullptr);
        //
        //        mtt::Curve_Procedure* proc = (mtt::Curve_Procedure*)mem::alloc_init<mtt::Curve_Procedure>(&curve_allocation.allocator);
        //
        //        assert(proc != nullptr);
        //
        //        //proc->state = CatmullRom();
        //        //auto& ref = std::get<CatmullRom>(proc->state);
        //
        //        *function_state = proc;
    };
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
        //
        //        mtt::Curve_Procedure** function_state = (mtt::Curve_Procedure**)mtt::access<void*>(thing, "function_state");
        //        if (function_state == nullptr) {
        //            return;
        //        }
        //
        //        mem::deallocate<mtt::Curve_Procedure>(&curve_allocation.allocator, *function_state);
    };
}

void tag_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_TEXT].alloc_byte_size + meta[MTT_COLOR_RGBA].alloc_byte_size +
                                meta[MTT_BOOLEAN].alloc_byte_size + meta[MTT_BOOLEAN].alloc_byte_size +
                                meta[MTT_VECTOR4].alloc_byte_size
                                );
    
    
    add_field({world, fields, ports, logic, "label", MTT_TEXT, 0});
    add_field({world, fields, ports, logic, "color", MTT_COLOR_RGBA, 0});
    add_field({world, fields, ports, logic, "is-active", MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "is-negated", MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "value", MTT_VECTOR4, 0});
    
    
    Thing_add_in_port({world, fields, ports, logic,  "active", MTT_FLOAT, 0, {ALIGN_UP + ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "negate",      MTT_FLOAT,           0,       {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "input-signal-single", MTT_FLOAT,           0,       {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "input-thing", MTT_INT64,           0,       {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "output-signal-single", MTT_FLOAT, 0,
        {ALIGN_RIGHT + (ALIGN_UP * 0.25f)}, nullptr
    });
    Thing_add_out_port({world, fields, ports, logic, "output-thing", MTT_INT64, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.25f)}, nullptr
    });
    
    
    archetype->logic.proc = tag_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
}

void sensor_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_TEXT].alloc_byte_size + meta[MTT_COLOR_RGBA].alloc_byte_size +
                                meta[MTT_BOOLEAN].alloc_byte_size + meta[MTT_BOOLEAN].alloc_byte_size +
                                meta[MTT_VECTOR4].alloc_byte_size + meta[MTT_INT32].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "label", MTT_TEXT, 0});
    add_field({world, fields, ports, logic, "color", MTT_COLOR_RGBA, 0});
    add_field({world, fields, ports, logic, "is-active", MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "is-negated", MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "value", MTT_VECTOR4, 0});
    add_field({world, fields, ports, logic, "required_count", MTT_INT64, 0}, temp_mem<uint64>(1));
    
    Thing_add_out_port({world, fields, ports, logic, "output-signal", MTT_BOOLEAN, 0,
        {ALIGN_RIGHT}, nullptr
    });
    
    
    
    archetype->logic.proc = sensor_procedure;
    
    archetype->logic.option = LOGIC_OPTION_SENSOR_CIRCLE_BOUND;
    
    
    // TODO: option_flags = 1 would return true if collision does does happen
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        thing->ecs_entity.set<mtt::Sensor>({
            .thing_id = thing->id
        });
    };
}

void bolt_init(ARCHETYPE_PARAM_LIST) {MTT_print("TODO! file=[%s] [%s]\n", __FILE__, __PRETTY_FUNCTION__);}

MTT_DEFINE_INITIALIZER(gravity)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0}, meta[MTT_FLOAT].alloc_byte_size + meta[MTT_THING].alloc_byte_size);
    
    add_field({world, fields, ports, logic, "strength", MTT_FLOAT, 0}, temp_mem<float32>(9.8));
    add_field({world, fields, ports, logic, "target", MTT_THING, 0});
    
    Thing_add_in_port({world, fields, ports, logic, "strength", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "target",   MTT_THING, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "active",   MTT_THING, 0, {ALIGN_DOWN}, nullptr});
}
MTT_DEFINE_LOGIC_PROCEDURE(gravity)
{
    if (!is_active_group(thing)) {
        return LOGIC_PROC_RETURN_TYPE(true);
    }
    
    //auto IN_stength = get_in_port(thing, input, 0);
    float32* strength = mtt::access<float32>(thing, "strength");
    auto IN_target  = get_in_port(thing, input, 1);
    auto IN_active  = get_in_port(thing, input, 2);
    
    return LOGIC_PROC_RETURN_TYPE(true);
}

void and_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0}, meta[MTT_INT64].alloc_byte_size);
    
    add_field({
        world, fields, ports, logic, "max_count", MTT_INT64, 0
    });
    
    Thing_add_in_port({world, fields, ports, logic,  "0", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "1", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    for (usize i = 2; i < 16; i += 1) {
        Thing_add_in_port({world, fields, ports, logic, std::to_string(i), MTT_BOOLEAN, 0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    }
    Thing_add_out_port({world, fields, ports, logic,  "value", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    
    archetype->logic.proc = and_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        auto* max_count = mtt::access<uint64>(thing, "max_count");
        *max_count = 2;
    };
}


void or_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0}, meta[MTT_INT64].alloc_byte_size);
    
    add_field({
        world, fields, ports, logic, "max_count", MTT_INT64, 0
    });
    
    Thing_add_in_port({world, fields, ports, logic,  "0", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "1", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    for (usize i = 2; i < 16; i += 1) {
        Thing_add_in_port({world, fields, ports, logic, std::to_string(i), MTT_BOOLEAN, 0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    }
    Thing_add_out_port({world, fields, ports, logic,  "value", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    
    archetype->logic.proc = or_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        auto* max_count = mtt::access<uint64>(thing, "max_count");
        *max_count = 2;
    };
}
void xor_init(ARCHETYPE_PARAM_LIST) {MTT_print("TODO! file=[%s] [%s]\n", __FILE__, __PRETTY_FUNCTION__);}


void not_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0}, 0);
    
    
    Thing_add_in_port({world, fields, ports, logic,  "value", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic,  "value", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    
    archetype->logic.proc = not_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
}

void timer_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_BOOLEAN].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size);
    
    add_field({
        world, fields, ports, logic, "previous reset state", MTT_BOOLEAN, 0
    });
    add_field({
        world, fields, ports, logic, "elapsed", MTT_FLOAT, 0
    });
    add_field({
        world, fields, ports, logic, "duration", MTT_FLOAT, 0
    });
    add_field({
        world, fields, ports, logic, "initial_time", MTT_FLOAT, 0
    });
    add_field({
        world, fields, ports, logic, "speed", MTT_FLOAT, 0
    });
    
    Thing_add_in_port({world, fields, ports, logic,  "start", MTT_FLOAT,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    Thing_add_in_port({world, fields, ports, logic,  "reset", MTT_BOOLEAN, 0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "result", MTT_FLOAT, 0, {{0.0f, 0.5f, 0.0f}}, nullptr });
    
    archetype->logic.proc = timer_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        *mtt::access<float32>(thing, "speed") = 1.0f;
    };
}
void text_init(ARCHETYPE_PARAM_LIST)
{
    set_is_actor(archetype);
    
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_TEXT].alloc_byte_size +
                                meta[MTT_TEXT].alloc_byte_size +
                                meta[MTT_VECTOR3].alloc_byte_size +
                                meta[MTT_BOOLEAN].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "value",    MTT_TEXT, 0});
    add_field({world, fields, ports, logic, "attrib",   MTT_TEXT, 0});
    add_field({world, fields, ports, logic, "position", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "is_changed", MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "interact_delay", MTT_FLOAT, 0});
    //Thing_add_field({world, fields, ports, logic, "value_text", MTT_TEXT, 0});
    
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "position", MTT_VECTOR3, 0, {ALIGN_RIGHT}, nullptr});
    
    archetype->logic.proc = text_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        thing->do_evaluation        = true;
        thing->is_user_drawable     = false;
        thing->is_user_destructible = true;
        thing->is_user_movable      = true;
        thing->lock_to_canvas       = false;
        auto* value      = mtt::access<MTT_String_Ref>(thing, "value");
        MTT_string_ref_release(value);
        (*value) = MTT_string_add(0, text_thing_empty_string);
        
        
        mtt::Thing_set_label(thing, text_thing_empty_string);
        
        mtt::Rep* rep = mtt::rep(thing);
        
        
        mtt::Collider* c = mtt::Collider_make_aabb(&thing->world()->collision_system);
        c->user_data = (void*)((thing)->id);
        rep->colliders.push_back(c);
        
        auto* vg = nvgGetGlobalContext();
        nvgSave(vg);
        
        
        cstring cs = text_thing_empty_string;
        
        struct Settings {
            float line_h;
        } font_settings;
        font_settings.line_h = 0;
        nvgFontSize(vg, 64.0f);
        nvgFontFace(vg, "sans");
        nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
        nvgTextMetrics(vg, NULL, NULL, &font_settings.line_h);
        
        {
            float bounds[4];
            nvgTextBounds(vg, 0, 0, cs, NULL, bounds);
            
            vec2 extent = vec2(bounds[2], bounds[3]);
            vec2 half_extent = extent / 2.0f;
            
            c->aabb.half_extent = half_extent;
            c->aabb.tl = vec2(c->center_anchor) - half_extent;
            c->aabb.br = vec2(c->center_anchor) + half_extent;
        }
        
        nvgRestore(vg);
        
        c->transform = mat4(1.0f);
        
        push_AABB(c->system, c);
    };
    
    archetype->on_thing_copy = [](mtt::Thing* dst, mtt::Thing* src) {
        auto* val_field = mtt::access<MTT_String_Ref>(src, "value");
        cstring val = MTT_string_ref_to_cstring(*val_field);
        
        {
            assert(strcmp(MTT_string_ref_to_cstring(*mtt::access<MTT_String_Ref>(src, "value")), val) == 0);
        }
        
        auto str_val = mtt::String(val);
        text_update_value(dst, str_val);
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
        //mem::deallocate<Query>(&thing->world->allocator, *mtt::access_pointer_to_pointer<Query*>(thing, "state"));
    };
    
    auto get_center = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            Thing* thing = args->caller;
            World* world = thing->world();
            args->output.type = MTT_VECTOR3;
            
            auto* center_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *center_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 center_position = vec3((br + tl) / 2.0f, 0.0f);
                    
                    *center_out = center_position;
                    
                    break;
                }
                default: {
                    *center_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    add_selector(archetype, "center", get_center);
    
    auto get_top_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.y -= half_extent.y;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, tl.y, 0.0f);
                    
                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    auto get_bottom_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            float32 mult = (msg->input_value.type == MTT_FLOAT) ? msg->input_value.Float : 1;
            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.y += half_extent.y;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, br.y, 0.0f);
                    
                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    add_selector(archetype, "center",   get_center);
    add_selector(archetype, "top_with_offset",    get_top_with_offset);
    add_selector(archetype, "bottom_with_offset", get_bottom_with_offset);
    add_selector(archetype, "set_movement_option", Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        
        
        Thing* thing = args->caller;
        args->output.type = MTT_NONE;
        
        Message* msg = (Message*)args->input;
        auto movement_flag = (uint64)msg->input_value.Int64;
        thing->logic.option_flags = modify_bit(thing->logic.option_flags, 0, movement_flag);
        
        return Procedure_Return_Type();
    }));
    
    archetype->message_handler = [](auto* msg) {
        switch (msg->type) {
        default: {
            break;
        }
        }
    };
    
    archetype->input_handlers.on_pen_input_began = [](Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> Input_Handler_Return {
        
        dt::DrawTalk* DT = dt::DrawTalk::ctx();
        for (auto it = DT->scn_ctx.selected_things.begin(); it != DT->scn_ctx.selected_things.end(); ++it) {
            mtt::Thing_ID other_id = *it;
            if (other_id == mtt::Thing_ID_INVALID || other_id == thing->id) {
                continue;
            }
            
            mtt::Thing* other_thing = thing->world()->Thing_try_get(other_id);
            if (other_thing == nullptr) {
                continue;
            }
            
            if (other_thing->archetype_id == mtt::ARCHETYPE_TEXT) {
                
                
                auto* first_val_field = mtt::access<MTT_String_Ref>(other_thing, "value");
                cstring prefix = MTT_string_ref_to_cstring(*first_val_field);
                
                auto* second_val_field = mtt::access<MTT_String_Ref>(thing, "value");
                cstring suffix = MTT_string_ref_to_cstring(*second_val_field);
                
                mtt::String new_label = mtt::String(prefix) + " " + mtt::String(suffix);
                
                text_update_value(other_thing, new_label);
                
                mtt::Thing_destroy(thing);
                break;
            } else if (other_thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH || other_thing->archetype_id == mtt::ARCHETYPE_NUMBER) {
                auto it_words = DT->lang_ctx.dictionary.thing_to_word.find(other_thing->id);
                if (it_words != DT->lang_ctx.dictionary.thing_to_word.end()) {
                    if (it_words->second.size() == 1 && other_thing->archetype_id != mtt::ARCHETYPE_NUMBER) {
                        auto* second_val_field = mtt::access<MTT_String_Ref>(thing, "value");
                        cstring word = MTT_string_ref_to_cstring(*second_val_field);
                        
                        auto* entry = dt::noun_add(word);
                        dt::vis_word_derive_from(other_thing, entry);
                    } else {
                        auto* second_val_field = mtt::access<MTT_String_Ref>(thing, "value");
                        cstring word = MTT_string_ref_to_cstring(*second_val_field);
                        for (auto _ = it_words->second.begin(); _ != it_words->second.end(); ++_) {
                            auto* entry = *_;
                            if (strcmp(word, entry->name.c_str()) == 0) {
                                dt::vis_word_underive_from(other_thing, entry);
                                break;
                            }
                        }
                    }
                } else {
                    auto* second_val_field = mtt::access<MTT_String_Ref>(thing, "value");
                    cstring word = MTT_string_ref_to_cstring(*second_val_field);
                    auto* entry = dt::noun_add(word);
                    dt::vis_word_derive_from(other_thing, entry);
                }
            }
            
        }
        
        return true;
    };
}

void power_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_FLOAT].alloc_byte_size
                                );
    add_field({world, fields, ports, logic, "power", MTT_FLOAT, 0});
    
    Thing_add_out_port({world, fields, ports, logic, "power", MTT_FLOAT, 0,
        {ALIGN_RIGHT}, nullptr
    });
    Thing_add_out_port({world, fields, ports, logic, "power_digital", MTT_BOOLEAN, 0,
        {ALIGN_RIGHT}, nullptr
    });
    
    archetype->logic.proc = power_procedure;
}

void group_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                0
                                );
    
    archetype->logic.proc = group_procedure;
    
    Thing_add_in_port({world, fields, ports, logic, "active", MTT_BOOLEAN, 0,
        {ALIGN_LEFT}, nullptr
    });
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](Thing* thing) {
        MTT_print("%s", "GROUP!\n");
    };
}

void vector_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_VECTOR4].alloc_byte_size + meta[MTT_INT64].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "vector", MTT_VECTOR4, 0});
    add_field({world, fields, ports, logic, "option", MTT_INT64, 0});
    
    Thing_add_in_port({world, fields, ports, logic, "x", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "y", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "z", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "w", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "vector", MTT_VECTOR4, 0, {ALIGN_RIGHT}, nullptr});
    
    archetype->logic.proc = vector_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
}

void destroyer_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_BOOLEAN].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_BOOLEAN].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "affects_connected", MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "time_delay", MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "fade_effect", MTT_BOOLEAN, 0});
    
    
    Thing_add_in_port({world, fields, ports, logic, "trigger", MTT_ANY, 0, {ALIGN_RIGHT}, nullptr});
    
    Thing_add_in_port({world, fields, ports, logic, "affects connected", MTT_BOOLEAN, 0, {ALIGN_RIGHT + (ALIGN_DOWN * 0.25f)}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "target", MTT_ANY, 0, {ALIGN_RIGHT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "on trigger", MTT_BOOLEAN, 0, {ALIGN_RIGHT}, nullptr});
    
    archetype->logic.proc = destroyer_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
}

void toggle_init(ARCHETYPE_PARAM_LIST)
{
    
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_BOOLEAN].alloc_byte_size + meta[MTT_BOOLEAN].alloc_byte_size
                                + meta[MTT_BOOLEAN].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "toggle-state",      MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "prev-value", MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "is-negated", MTT_BOOLEAN, 0}, temp_mem<bool>(false));
    
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_BOOLEAN, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_BOOLEAN, 0, {ALIGN_RIGHT}, nullptr});
    
    archetype->logic.proc = toggle_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
}

void selector_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_INT64].alloc_byte_size + meta[MTT_INT64].alloc_byte_size + meta[MTT_BOOLEAN].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "cycle_index", MTT_INT64, 0});
    
    
    // TODO: support a default value
    add_field({world, fields, ports, logic, "index_count", MTT_INT64, 0}, temp_mem<uint64>(30));
    add_field({world, fields, ports, logic, "prev_cycle_state", MTT_BOOLEAN, 0}, temp_mem<bool>(false));
    
    
    
    Thing_add_in_port({world, fields, ports, logic, "cycle", MTT_INT64, 0, {ALIGN_DOWN}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "active", MTT_BOOLEAN, 0, {ALIGN_DOWN}, nullptr});
    for (usize idx = 0; idx < 30; idx += 1) {
        Thing_add_in_port({world, fields, ports, logic, std::to_string(idx), MTT_BOOLEAN, 0, {ALIGN_LEFT + ALIGN_DOWN * (idx * 0.15f)}, nullptr});
        
        Thing_add_out_port({world, fields, ports, logic, std::to_string(idx), MTT_BOOLEAN, 0, {ALIGN_LEFT + ALIGN_DOWN * (idx * 0.15f)}, nullptr});
    }
    
    
    
    archetype->logic.proc = selector_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](Thing* thing) {
        auto* init_cycle_state = mtt::access<bool>(thing, "prev_cycle_state");
        *init_cycle_state = false;
    };
}

void emitter_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_TAG].alloc_byte_size //+ meta[MTT_BOOLEAN].type_size
                                );
    
    add_field({world, fields, ports, logic, "label", MTT_TAG, 0});
    
    
    Thing_add_in_port({world, fields, ports, logic, "trigger", MTT_BOOLEAN, 0, {ALIGN_DOWN}, nullptr});
    
    archetype->logic.proc = emitter_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
}


void converter_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                0
                                );
    
    
    Thing_add_in_port({world, fields, ports, logic, "value_in", MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_in_port({world, fields, ports, logic, "value_out", MTT_ANY, 0, {ALIGN_RIGHT}, nullptr});
    
    archetype->logic.proc = converter_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
}

constexpr const uint64 MESSENGER_OPTION_SYSTEM    = 0;
constexpr const uint64 MESSENGER_OPTION_TO_ENTITY = 0;

void messenger_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0}, meta[MTT_PROCEDURE].alloc_byte_size + meta[MTT_TAG].alloc_byte_size * 3);
    
    Thing_add_in_port({world, fields, ports, logic, "value_in", MTT_BOOLEAN, 0, {ALIGN_LEFT}, nullptr});
    
    add_field({world, fields, ports, logic, "sender", MTT_TAG, 0});
    add_field({world, fields, ports, logic, "event", MTT_TAG, 0});
    add_field({world, fields, ports, logic, "status", MTT_TAG, 0});
    
    add_field({world, fields, ports, logic, "on_end", MTT_PROCEDURE, 0});
    
    archetype->logic.proc = messenger_procedure;
    
    archetype->logic.option = MESSENGER_OPTION_SYSTEM;
    archetype->logic.option_flags = 0;
}


void receiver_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0}, 0);
    
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_ANY, 0, {ALIGN_RIGHT}, nullptr});
    
    archetype->logic.proc = receiver_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
}

#if 0
// NOTE: OLD
void jump_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                (meta[MTT_BOOLEAN].alloc_byte_size) +
                                
                                (meta[MTT_THING].alloc_byte_size) +
                                (meta[MTT_THING].alloc_byte_size) +
                                
                                (meta[MTT_VECTOR3].alloc_byte_size) +
                                (meta[MTT_THING].alloc_byte_size) +
                                (meta[MTT_FLOAT].alloc_byte_size) +
                                (meta[MTT_VECTOR3].alloc_byte_size) +
                                (meta[MTT_FLOAT].alloc_byte_size) +
                                (meta[MTT_FLOAT].alloc_byte_size) +
                                (meta[MTT_FLOAT].alloc_byte_size) +
                                (meta[MTT_TEXT].alloc_byte_size) +
                                (meta[MTT_TEXT].alloc_byte_size) +
                                (meta[MTT_TEXT].alloc_byte_size) +
                                (meta[MTT_VECTOR3].alloc_byte_size) +
                                (meta[MTT_INT64].alloc_byte_size)
                                );
    
    
    add_field({world, fields, ports, logic, "is_init" ,             MTT_BOOLEAN, 0});
    
    add_field({world, fields, ports, logic, "source",       MTT_THING, 0});
    add_field({world, fields, ports, logic, "destination",  MTT_THING, 0});
    
    
    add_field({world, fields, ports, logic, "position_destination", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "thing_ceiling",        MTT_THING, 0, });
    
    add_field({world, fields, ports, logic, "position_ceiling", MTT_FLOAT, 0}, temp_mem<float32>(POSITIVE_INFINITY));
    add_field({world, fields, ports, logic, "destination_offset", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "speed",              MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "interpolation",      MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "jump_height",      MTT_FLOAT, 0});
    
    
    add_field({world, fields, ports, logic, "source_selector", MTT_TEXT, 0});
    add_field({world, fields, ports, logic, "destination_selector", MTT_TEXT, 0});
    add_field({world, fields, ports, logic, "ceiling_selectors", MTT_TEXT, 0});
    add_field({world, fields, ports, logic,
        "initial_position", MTT_VECTOR3, 0});
    
    
    add_field({world, fields, ports, logic, "cycle_count",      MTT_INT64, 0});
    
    
    Thing_add_in_port({world, fields, ports, logic,  "active", MTT_BOOLEAN, 0, {ALIGN_DOWN}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "interpolation", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_out_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    
    archetype->logic.proc = jump_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](Thing* thing) {
        
        auto* src_selector = mtt::access<MTT_String_Ref>(thing, "source_selector");
        MTT_string_ref_release(src_selector);
        (*src_selector) = MTT_string_add(0, "center");
        auto* dst_selector = mtt::access<MTT_String_Ref>(thing, "destination_selector");
        MTT_string_ref_release(dst_selector);
        (*dst_selector) = MTT_string_add(0, "top_with_offset");
        
        
        auto* is_init = mtt::access<bool>(thing, "is_init");
        *is_init = false;
        
        auto* speed = mtt::access<float32>(thing, "speed");
        *speed = 1.0f;
        
        auto* cycle_count = mtt::access<uint64>(thing, "cycle_count");
        *cycle_count = 1;
    };
    
    archetype->on_thing_destroy = [](Thing* thing) {
        //        Message msg;
        //        msg.selector = "set_movement_option";
        //        msg.input_value.type = MTT_INT64;
        //        msg.input_value.Int64 = OPTION_FLAGS_MOVEMENT_DEFAULT;
        //        msg.sender = thing->id;
        //
        //        auto* src = mtt::access<mtt::Thing_List>(thing, "source");
        //
        //
        //        for (usize idx = 0, count = src->size(); idx < count; idx += 1) {
        //            mtt::Thing_Ref* s_ref = &((*src)[idx]);
        //
        //
        //            mtt::Thing* src_thing = nullptr;
        //
        //
        //
        //            if (!(s_ref->try_get(&src_thing))) {
        //                continue;
        //            }
        //
        //            selector_invoke(src_thing, &msg);
        //        }
    };
}
#endif



// TODO: This should temporary enable groups, run all sub-hierarchies, then turn them off again
void run_group_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_THING_LIST].alloc_byte_size +
                                meta[MTT_TAG_LIST].alloc_byte_size);
    
    add_field({world, fields, ports, logic, "things",     MTT_THING_LIST, 0});
    add_field({world, fields, ports, logic, "selections", MTT_TAG_LIST,   0});
    
    Thing_add_in_port({world, fields, ports, logic, "active", MTT_BOOLEAN, 0,
        {(ALIGN_LEFT * 0.5f)}, nullptr
    });
    
    archetype->logic.proc = run_group_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
}

void end_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_INT64].alloc_byte_size +
                                meta[MTT_THING].alloc_byte_size +
                                meta[MTT_PROCEDURE].alloc_byte_size + meta[MTT_BOOLEAN].alloc_byte_size + meta[MTT_BOOLEAN].alloc_byte_size);
    
    add_field({world, fields, ports, logic, "eval_ctx_id",  MTT_INT64, 0});
    add_field({world, fields, ports, logic, "program_id", MTT_THING, 0});
    add_field({world, fields, ports, logic, "on_end", MTT_PROCEDURE, 0});
    add_field({world, fields, ports, logic, "expected_end_value", MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "prev_state", MTT_BOOLEAN, 0}, temp_mem<bool>(false));
    
    Thing_add_in_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "is_done_analogue", MTT_FLOAT, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    
    Thing_add_out_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    
    archetype->logic.proc =  end_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](Thing* thing) {
        bool* state = mtt::access<bool>(thing, "prev_state");
        *state = false;
    };
}

void distance_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_THING].alloc_byte_size +
                                meta[MTT_THING].alloc_byte_size +
                                meta[MTT_VECTOR3].alloc_byte_size +
                                meta[MTT_TEXT].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "source",      MTT_THING,   0});
    add_field({world, fields, ports, logic, "destination", MTT_THING,   0});
    add_field({world, fields, ports, logic, "scale",       MTT_VECTOR3, 0}, temp_mem<vec3>(vec3(1.0f)));
    add_field({world, fields, ports, logic, "selector",    MTT_TEXT, 0}, temp_mem<MTT_String_Ref>(string("center")));
    
    
    Thing_add_in_port({world, fields, ports, logic, "source", MTT_VECTOR3, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "destination", MTT_VECTOR3, 0,
        {ALIGN_RIGHT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_out_port({world, fields, ports, logic, "distance", MTT_FLOAT, 0,
        {ALIGN_UP}, nullptr
    });
    
    
    archetype->logic.proc =  distance_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](Thing* thing) {
        //        vec3* scale = mtt::access<vec3>(thing, "scale");
        //        *scale = vec3(1.0f);
        
        //        MTT_String_Ref* selector = mtt::access<MTT_String_Ref>(thing, "selector");
        //        *selector = string("center");
    };
}




void ui_element_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_POINTER].alloc_byte_size +
                                meta[MTT_ANY].alloc_byte_size +
                                meta[MTT_ANY].alloc_byte_size +
                                meta[MTT_ANY].alloc_byte_size +
                                meta[MTT_ANY].alloc_byte_size +
                                meta[MTT_TEXT].alloc_byte_size +
                                meta[MTT_VECTOR3].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "ctx_ptr", MTT_POINTER, 0});
    add_field({world, fields, ports, logic, "state0", MTT_ANY, 0});
    add_field({world, fields, ports, logic, "state1", MTT_ANY, 0});
    add_field({world, fields, ports, logic, "state2", MTT_ANY, 0});
    add_field({world, fields, ports, logic, "state3", MTT_ANY, 0});
    add_field({world, fields, ports, logic, "typename", MTT_TEXT, 0});
    add_field({world, fields, ports, logic, "position", MTT_VECTOR3, 0});
    
    Thing_add_in_port({world, fields, ports, logic, "state0_in", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "state1_in", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "state2_in", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "state3_in", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    Thing_add_out_port({world, fields, ports, logic, "out", MTT_ANY, 0,
        {ALIGN_LEFT + (ALIGN_DOWN * 0.5f)}, nullptr
    });
    
    
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        thing->do_evaluation        = false;
        thing->is_user_drawable     = false;
        thing->is_user_destructible = false;
        thing->is_user_movable      = false;
        thing->lock_to_canvas       = true;
        
        //        mtt::Any* state[] = {
        //            mtt::access<mtt::Any>(thing, "state0"),
        //            mtt::access<mtt::Any>(thing, "state1"),
        //            mtt::access<mtt::Any>(thing, "state2"),
        //            mtt::access<mtt::Any>(thing, "state3"),
        //        };
        //
        //        for (usize i = 0; i < 4; i += 1) {
        //            ASSERT_MSG(state[i] != nullptr, "field should exist!\n");
        //        }
    };
    
    archetype->logic.proc = ui_element_procedure;
}

void reference_init(ARCHETYPE_PARAM_LIST);
void reference_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_THING_LIST].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "references", MTT_THING_LIST, 0});
    
    Thing_add_in_port({world, fields, ports, logic, "in", MTT_THING_LIST, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "out", MTT_THING_LIST, 0, {ALIGN_RIGHT}, nullptr});
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = reference_procedure;
}


void random_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_INT64].alloc_byte_size +
                                meta[MTT_INT64].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "minimum", MTT_INT64, 0});
    add_field({world, fields, ports, logic, "maximum", MTT_INT64, 0});
    
    Thing_add_in_port({world, fields, ports, logic, "in", MTT_LIST, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "out", MTT_LIST, 0, {ALIGN_RIGHT}, nullptr});
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = random_procedure;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
        
    };
}

void query_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_POINTER].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "state", MTT_POINTER, 0});
    
    Thing_add_out_port({world, fields, ports, logic, "out", MTT_THING_LIST, 0, {ALIGN_RIGHT}, nullptr});
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = query_procedure;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        *mtt::access_pointer_to_pointer<Query_Rule*>(thing, "state") = mem::alloc_init<Query_Rule>(&thing->world()->allocator);
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
        mem::deallocate<Query_Rule>(&thing->world()->allocator, *mtt::access_pointer_to_pointer<Query_Rule*>(thing, "state"));
    };
}

void number_init(ARCHETYPE_PARAM_LIST)
{
    mtt::set_is_actor(archetype);
    
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_FLOAT].alloc_byte_size
                                + meta[MTT_VECTOR3].alloc_byte_size +
                                meta[MTT_BOOLEAN].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size
                                );
    
    
    add_field({world, fields, ports, logic, "value",    MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "position", MTT_VECTOR3, 0});
    add_field({world, fields, ports, logic, "is_changed", MTT_BOOLEAN, 0});
    add_field({world, fields, ports, logic, "interact_delay", MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "polarity_constraint", MTT_FLOAT, 0});
    
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "position", MTT_VECTOR3, 0, {ALIGN_RIGHT}, nullptr});
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = number_procedure;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        thing->do_evaluation        = true;
        thing->is_user_drawable     = false;
        thing->is_user_destructible = true;
        thing->is_user_movable      = true;
        thing->lock_to_canvas       = false;
        auto* value      = mtt::access<float32>(thing, "value");
        *value = 0.0f;
        
        
        mtt::Thing_set_label(thing, "0.0");
        
        mtt::Rep* rep = mtt::rep(thing);
        
        
        mtt::Collider* c = mtt::Collider_make_aabb(&thing->world()->collision_system);
        c->user_data = (void*)((thing)->id);
        rep->colliders.push_back(c);
        
        auto* vg = nvgGetGlobalContext();
        nvgSave(vg);
        
        
        cstring cs = "0.0";
        
        struct Settings {
            float line_h;
        } font_settings;
        font_settings.line_h = 0;
        nvgFontSize(vg, 64.0f);
        nvgFontFace(vg, "sans-mono");
        nvgTextAlign(vg, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
        nvgTextMetrics(vg, NULL, NULL, &font_settings.line_h);
        
        {
            float bounds[4];
            nvgTextBounds(vg, 0, 0, cs, NULL, bounds);
            
            vec2 extent = vec2(bounds[2], bounds[3]);
            vec2 half_extent = extent / 2.0f;
            
            c->aabb.half_extent = half_extent;
            c->aabb.tl = vec2(c->center_anchor) - half_extent;
            c->aabb.br = vec2(c->center_anchor) + half_extent;
        }
        
        nvgRestore(vg);
        
        c->transform        = mat4(1.0f);
        
        push_AABB(c->system, c);
        
        //        {
        //            auto* val = mtt::access<float32>(thing, "value");
        //            *val = (float32)thing->id;
        //        }
        
        
        thing->input_handlers.on_pen_input_began = [](Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) -> Input_Handler_Return {
            
            dt::DrawTalk* DT = dt::DrawTalk::ctx();
            for (auto it = DT->scn_ctx.selected_things.begin(); it != DT->scn_ctx.selected_things.end(); ++it) {
                mtt::Thing_ID other_id = *it;
                if (other_id == mtt::Thing_ID_INVALID || other_id == thing->id) {
                    continue;
                }
                
                mtt::Thing* other_thing = thing->world()->Thing_try_get(other_id);
                if (other_thing == nullptr) {
                    continue;
                }
                
                if (other_thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH) {
                    if (mtt::in_port_is_active(thing, "value")) {
                        mtt::remove_connection_at_dst(thing, "value");
                    } else {
                        auto& named_things = dt::word_dict()->thing_to_word;
                        auto& self_names = named_things[thing->id];
                        if (self_names.empty()) {
                            mtt::add_connection(thing->world(), other_thing, "angle_degrees", thing, "value");
                        } else {
                            for (auto names = self_names.begin(); names != self_names.end(); ++names) {
                                auto& name = (*names)->name;
                                if (dt::numeric_value_words.find(name) != dt::numeric_value_words.end()) {
                                    
                                    if (name == "angle" || name == "degree" || name == "rotation") {
                                        mtt::add_connection(thing->world(), other_thing, "angle_degrees", thing, "value");
                                    } else if (name == "radian") {
                                        mtt::add_connection(thing->world(), other_thing, "angle_radians", thing, "value");
                                    }
                                    
                                    break;
                                }
                            }
                        }
                        
                        
                        
                    }
                } else if (other_thing->archetype_id == mtt::ARCHETYPE_NUMBER) {
                    
                    
                    /*
                     auto& named_things = dt->lang_ctx.dictionary.thing_to_word;
                     for (auto it = named_things.begin(); it != named_things.end(); ++it) {
                     mtt::Thing_ID thing_id = it->first;
                     */
                    
                    //            auto* other_val = mtt::access<float32>(other_thing, "value");
                    //            auto* self_val  = mtt::access<float32>(thing, "value");
                    //            *self_val = *other_val;
                    
                    if (mtt::in_port_is_active(thing, "value")) {
                        //mtt::remove_all_connections(thing->world, thing->id);
                        mtt::remove_connection_at_dst(thing, "value");
                    } else {
                        mtt::add_connection(thing->world(), other_thing, "value", thing, "value");
                    }
                    
//                    MTT_print("%s", "SOURCE\n");
//                    MTT_Port_Descriptor_print(&other_thing->ports);
//                    MTT_print("%s", "TARGET\n");
//                    MTT_Port_Descriptor_print(&thing->ports);
                    
                }
                
            }
            
            return true;
        };
        
        
        thing->input_handlers.on_pen_input_ended = [](Input_Handlers* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags) {
            
            dt::DrawTalk* DT = dt::DrawTalk::ctx();
            for (auto it = DT->scn_ctx.selected_things.begin(); it != DT->scn_ctx.selected_things.end(); ++it) {
                mtt::Thing_ID other_id = *it;
                if (other_id == mtt::Thing_ID_INVALID || other_id == thing->id) {
                    continue;
                }
                
                mtt::Thing* other_thing = thing->world()->Thing_try_get(other_id);
                if (other_thing == nullptr) {
                    continue;
                }
                
                if (other_thing->archetype_id == mtt::ARCHETYPE_FREEHAND_SKETCH) {
//                    if (mtt::in_port_is_active(thing, "value")) {
//                        mtt::remove_connection_at_dst(thing, "value");
//                    } else {
//                        //mtt::add_connection(thing->world(), other_thing, "angle_degrees", thing, "value");
//
//                        auto& named_things = dt::word_dict()->thing_to_word;
//                        auto& self_names = named_things[thing->id];
//                        if (self_names.empty()) {
//                            mtt::add_connection(thing->world(), other_thing, "angle_degrees", thing, "value");
//                        } else {
//                            for (auto names = self_names.begin(); names != self_names.end(); ++names) {
//                                auto& name = (*names)->name;
//                                if (dt::numeric_value_words.find(name) != dt::numeric_value_words.end()) {
//
//                                    if (name == "angle" || name == "degree" || name == "rotation") {
//                                        mtt::add_connection(thing->world(), other_thing, "angle_degrees", thing, "value");
//                                    } else if (name == "radian") {
//                                        mtt::add_connection(thing->world(), other_thing, "angle_radians", thing, "value");
//                                    }
//
//                                    break;
//                                }
//                            }
//                        }
//
//
//
//                    }
                } else if (other_thing->archetype_id == mtt::ARCHETYPE_NUMBER) {
                    
                    
                    /*
                     auto& named_things = dt->lang_ctx.dictionary.thing_to_word;
                     for (auto it = named_things.begin(); it != named_things.end(); ++it) {
                     mtt::Thing_ID thing_id = it->first;
                     */
                    
                    //            auto* other_val = mtt::access<float32>(other_thing, "value");
                    //            auto* self_val  = mtt::access<float32>(thing, "value");
                    //            *self_val = *other_val;
                    //
//                    if (mtt::in_port_is_active(thing, "value")) {
//                        //mtt::remove_all_connections(thing->world, thing->id);
//                        //mtt::remove_connection_at_dst(thing, "value");
//                    } else {
//                        //mtt::add_connection(thing->world(), other_thing, "value", thing, "value");
//                    }
//
//                    MTT_print("SOURCE\n");
//                    MTT_Port_Descriptor_print(&other_thing->ports);
//                    MTT_print("TARGET\n");
//                    MTT_Port_Descriptor_print(&thing->ports);
                    
                }
                
            }
        };
        
        //*mtt::access_pointer_to_pointer<Query*>(thing, "state") = mem::alloc_init<Query>(&thing->world->allocator);
        
        //mtt::World* world = thing->world;
        //        sd::Renderer* renderer = world->renderer;
        //        sd::save(renderer);
        //        sd::restore(renderer);
    };
    
    archetype->on_thing_copy = [](mtt::Thing* dst, mtt::Thing* src) {
        auto* src_val = mtt::access<float32>(src, "value");
        assert(src_val != nullptr);
        float32 src_val_local = *src_val;
        mtt::number_update_value(dst, src_val_local);
        assert(src_val_local == *mtt::access<float32>(dst, "value"));
    };
    
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
        //mem::deallocate<Query>(&thing->world->allocator, *mtt::access_pointer_to_pointer<Query*>(thing, "state"));
    };
    
    auto get_center = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            Thing* thing = args->caller;
            World* world = thing->world();
            args->output.type = MTT_VECTOR3;
            
            auto* center_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *center_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 center_position = vec3((br + tl) / 2.0f, 0.0f);
                    
                    *center_out = center_position;
                    
                    break;
                }
                default: {
                    *center_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    add_selector(archetype, "center", get_center);
    
    auto get_top_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.y -= half_extent.y;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, tl.y, 0.0f);
                    
                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    auto get_bottom_with_offset = Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        {
            mtt::Message* msg = static_cast<mtt::Message*>(args->input);
            
            
            Thing* thing = args->caller;
            World* world = thing->world();
            
            Thing* getter = nullptr;
            
            vec3 offset = vec3(0.0f);
            mtt::Thing* sender;
            if (world->Thing_try_get(msg->sender, &sender)) {
                Representation* rep = mtt::rep(sender);
                if (rep->colliders.size() != 0) {
                    
                    Collider* c = rep->colliders[0];
                    switch (c->type) {
                    case COLLIDER_TYPE_AABB: {
                        AABB* aabb = &c->aabb;
                        
                        vec2& tl = aabb->tl;
                        vec2& br = aabb->br;
                        
                        vec2 half_extent = vec2((br - tl) / 2.0f);
                        offset.y += half_extent.y;
                        
                        break;
                    }
                    }
                }
            }
            
            
            args->output.type = MTT_VECTOR3;
            
            auto* top_out = &args->output.Vector3;
            {
                Representation* rep = mtt::rep(thing);
                if (rep->colliders.size() == 0) {
                    *top_out = vec3(0.0);
                    return true;
                }
                
                Collider* c = rep->colliders[0];
                switch (c->type) {
                case COLLIDER_TYPE_AABB: {
                    AABB* aabb = &c->aabb;
                    
                    vec2& tl = aabb->tl;
                    vec2& br = aabb->br;
                    
                    vec3 top_position = vec3((tl.x + br.x) / 2.0f, br.y, 0.0f);
                    
                    *top_out = top_position + offset;
                    
                    break;
                }
                default: {
                    *top_out = vec3(0.0);
                    return true;
                }
                }
                
                
                return true;
                
            }
        }
        return Procedure_Return_Type();
    });
    
    add_selector(archetype, "center",   get_center);
    add_selector(archetype, "top_with_offset",    get_top_with_offset);
    add_selector(archetype, "bottom_with_offset", get_bottom_with_offset);
    add_selector(archetype, "set_movement_option", Procedure_make([](void* state, Procedure_Input_Output* args) -> Procedure_Return_Type {
        
        
        Thing* thing = args->caller;
        args->output.type = MTT_NONE;
        
        Message* msg = (Message*)args->input;
        auto movement_flag = (uint64)msg->input_value.Int64;
        thing->logic.option_flags = modify_bit(thing->logic.option_flags, 0, movement_flag);
        
        return Procedure_Return_Type();
    }));
    
    archetype->message_handler = [](auto* msg) {
        switch (msg->type) {
        default: {
            break;
        }
        }
    };
}

void slider_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_FLOAT].alloc_byte_size +
                                meta[MTT_BOOLEAN].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "value", MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "lower", MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "upper", MTT_FLOAT, 0});
    add_field({world, fields, ports, logic, "show_labels", MTT_BOOLEAN, 0});
    
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = slider_procedure;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        thing->do_evaluation        = true;
        thing->is_user_drawable     = false;
        thing->is_user_destructible = true;
        thing->is_user_movable      = true;
        thing->lock_to_canvas       = false;
        
        //        mtt::World* world = thing->world;
        //        sd::Renderer* renderer = world->renderer;
        //        sd::save(renderer);
        //        {
        //            mtt::Rep* rep = mtt::rep(thing);
        //            mtt::Collider* collider = mtt::Collider_make_aabb(&world->collision_system);
        //            rep->colliders.push_back(collider);
        //        }
        //        sd::restore(renderer);
        
        //*mtt::access_pointer_to_pointer<dt::UI_Slider*>(thing, "state") = mem::alloc_init<dt::UI_Slider>(&thing->world->allocator);
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
        //mem::deallocate<Query>(&thing->world->allocator, *mtt::access_pointer_to_pointer<dt::UI_Slider*>(thing, "state"));
    };
}

void difference_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_ANY].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "value", MTT_ANY, 0});
    
    
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value_vector", MTT_VECTOR4, 0, {ALIGN_RIGHT}, nullptr});
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = difference_procedure;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        thing->do_evaluation        = true;
        //        thing->is_user_drawable     = false;
        //        thing->is_user_destructible = true;
        //        thing->is_user_movable      = true;
        //        thing->lock_to_canvas       = false;
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
        
        //mem::deallocate<Query>(&thing->world->allocator, *mtt::access_pointer_to_pointer<dt::UI_Slider*>(thing, "state"));
    };
}



void counter_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_BOOLEAN].alloc_byte_size +
                                meta[MTT_INT64].alloc_byte_size +
                                meta[MTT_INT64].alloc_byte_size
                                );
    
    
    add_field({
        world, fields, ports, logic, "prev_state", MTT_BOOLEAN, 0
    });
    add_field({
        world, fields, ports, logic, "count", MTT_INT64, 0
    });
    add_field({
        world, fields, ports, logic, "max_count", MTT_INT64, 0
    });
    
    Thing_add_in_port({world, fields, ports, logic,  "active", MTT_BOOLEAN, 0, {ALIGN_DOWN}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "increment", MTT_BOOLEAN,   0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    
    Thing_add_in_port({world, fields, ports, logic,  "reset", MTT_BOOLEAN, 0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    
    Thing_add_out_port({world, fields, ports, logic, "result",         MTT_BOOLEAN, 0, {{-0.5f, 0.5f, 0.0f}}, nullptr });
    Thing_add_out_port({world, fields, ports, logic, "result_analogue", MTT_FLOAT, 0, {{0.0f, 0.5f, 0.0f}}, nullptr });
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = counter_procedure;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        auto* state = mtt::access<bool>(thing, "prev_state");
        *state = false;
    };
}

void sign_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_ANY].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "value", MTT_ANY, 0});
    
    Thing_add_in_port({world, fields, ports, logic, "value",  MTT_ANY, 0, {ALIGN_LEFT},   nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_ANY, 0, {ALIGN_RIGHT}, nullptr});
    
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = sign_procedure;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        thing->do_evaluation = true;
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
    };
}

void value_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0}, meta[MTT_ANY].alloc_byte_size);
    
    
    add_field({world, fields, ports, logic, "value", MTT_ANY, 0});
    
    
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_ANY, 0, {ALIGN_RIGHT}, nullptr});
    
    
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = value_procedure;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        thing->do_evaluation = true;
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
    };
}

void comparison_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0}, meta[MTT_FLOAT].alloc_byte_size);
    
    add_field({world, fields, ports, logic, "value", MTT_FLOAT, 0});
    
    
    Thing_add_in_port({world, fields, ports, logic, "first", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "second", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    
    
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = comparison_procedure;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        thing->do_evaluation = true;
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
    };
}

void set_field_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_THING].alloc_byte_size +
                                meta[MTT_INT64].alloc_byte_size +
                                meta[MTT_ANY].alloc_byte_size
                                );
    
    
//    add_field({world, fields, ports, logic, "target",    MTT_THING, 0});
    add_field({world, fields, ports, logic, "type",      MTT_INT64, 0});
    add_field({world, fields, ports, logic, "value",     MTT_ANY, 0});
    
    Thing_add_in_port({world, fields, ports, logic, "target", MTT_THING, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "key", MTT_STRING, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "target", MTT_THING, 0, {ALIGN_RIGHT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = set_field_procedure;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        thing->do_evaluation = true;
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
    };
}

MTT_DEFINE_LOGIC_PROCEDURE(set_field)
{
    if (!is_active_group(thing)) {
        return true;
    }
    
    auto IN_target = get_in_port(thing, input, 0);
    auto IN_key    = get_in_port(thing, input, 1);
    auto IN_value  = get_in_port(thing, input, 2);
    
    return true;
}


void get_field_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_THING].alloc_byte_size +
                                meta[MTT_INT64].alloc_byte_size +
                                meta[MTT_ANY].alloc_byte_size
                                );
    
    
    add_field({world, fields, ports, logic, "target",    MTT_THING, 0});
    add_field({world, fields, ports, logic, "type", MTT_INT64, 0});
    add_field({world, fields, ports, logic, "value",     MTT_ANY, 0});
    
    Thing_add_in_port({world, fields, ports, logic, "target", MTT_THING, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "key", MTT_STRING, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "target", MTT_THING, 0, {ALIGN_RIGHT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->logic.proc = get_field_procedure;
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        thing->do_evaluation = true;
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
    };
}

MTT_DEFINE_LOGIC_PROCEDURE(get_field)
{
    return true;
}

void set_active_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                0
                                );
    
    Thing_add_in_port({world, fields, ports, logic, "target", MTT_THING, 0, {ALIGN_LEFT}, nullptr});
}
MTT_DEFINE_LOGIC_PROCEDURE(set_active)
{
    auto IN_target = get_in_port(thing, input, 0);
    if (IN_target.status == PORT_STATUS_OK) {
        mtt::Thing* target;
        if (world->Thing_try_get(IN_target.value.out.thing_id, &target)) {
            mtt::set_is_active(target);
        }
    }
    return true;
}

void set_inactive_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                0
                                );
    
    Thing_add_in_port({world, fields, ports, logic, "target", MTT_THING, 0, {ALIGN_LEFT}, nullptr});
}

MTT_DEFINE_LOGIC_PROCEDURE(set_inactive)
{
    auto IN_target = get_in_port(thing, input, 0);
    if (IN_target.status == PORT_STATUS_OK) {
        mtt::Thing* target;
        if (world->Thing_try_get(IN_target.value.out.thing_id, &target)) {
            mtt::unset_is_active(target);
        }
    }
    return true;
}

/*
 [ARCHETYPE_INTERPRETER] = interpreter_init,
 
 [ARCHETYPE_CALL] = call_init,
 [ARCHETYPE_RUN] = run_init,
 [ARCHETYPE_RE_INIT] = re_init_init,
 [ARCHETYPE_SUSPEND] = suspend_init,
 [ARCHETYPE_IF] = if_init,
 */
void set_run_prop_init(ARCHETYPE_PARAM_LIST)
{
    
}
void get_run_prop_init(ARCHETYPE_PARAM_LIST)
{
    
}
void interpreter_init(ARCHETYPE_PARAM_LIST)
{
    
}

void run_init(ARCHETYPE_PARAM_LIST)
{
    
}
void re_init_init(ARCHETYPE_PARAM_LIST)
{
    
}
void suspend_init(ARCHETYPE_PARAM_LIST)
{
    
}
MTT_DEFINE_LOGIC_PROCEDURE(suspend)
{
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED_SUSPEND;
}

void if_init(ARCHETYPE_PARAM_LIST)
{
    
}


void cosine_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                0
                                );
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
}


MTT_DEFINE_LOGIC_PROCEDURE(cosine)
{
    
    
    auto OUT_val = get_out_port(thing, 0);
    if (OUT_val.status == PORT_STATUS_OK) {
        OUT_val.value.out.set_Float(m::cos(get_in_port_or<float32>(thing, input, 0, MTT_PI / 2.0f)));
    }
    
    return true;
}

void cosine01_init(ARCHETYPE_PARAM_LIST)
{
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
    //                                0
    //                                );
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
}

MTT_DEFINE_LOGIC_PROCEDURE(cosine01)
{
    
    
    auto OUT_val = get_out_port(thing, 0);
    if (OUT_val.status == PORT_STATUS_OK) {
        OUT_val.value.out.set_Float(m::cos01(get_in_port_or<float32>(thing, input, 0, MTT_PI / 2.0f)));
    }
    
    return true;
}

void sine_init(ARCHETYPE_PARAM_LIST)
{
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
    //                                0
    //                                );
    
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
}

MTT_DEFINE_LOGIC_PROCEDURE(sine)
{
    
    
    auto OUT_val = get_out_port(thing, 0);
    if (OUT_val.status == PORT_STATUS_OK) {
        OUT_val.value.out.set_Float(m::sin(get_in_port_or<MTT_FLOAT_Type>(thing, input, 0, 0.0f)));
    }
    
    return true;
}
MTT_DEFINE_INITIALIZER(sine01)
{
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
    //                                0
    //                                );
    
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
}

MTT_DEFINE_LOGIC_PROCEDURE(sine01)
{
    
    
    auto OUT_val = get_out_port(thing, 0);
    if (OUT_val.status == PORT_STATUS_OK) {
        OUT_val.value.out.set_Float(m::sin01(get_in_port_or<MTT_FLOAT_Type>(thing, input, 0, 0.0f)));
    }
    
    return true;
}


void variable_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_ANY].alloc_byte_size +
                                meta[MTT_STRING].alloc_byte_size
                                );
    
    
    add_field({world, fields, ports, logic, "value", MTT_ANY, 0});
    add_field({world, fields, ports, logic, "label", MTT_STRING, 0});
    
    Thing_add_in_port({world, fields, ports, logic,   "input",   MTT_ANY, 0, {ALIGN_DOWN}, nullptr});
    Thing_add_out_port({world, fields, ports, logic,  "value", MTT_ANY, 0, {ALIGN_DOWN}, nullptr});
    
    archetype->logic.proc = variable_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](Thing* thing) {
        
    };
}
void parameter_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_ANY].alloc_byte_size
                                );
    
    
    add_field({world, fields, ports, logic, "value", MTT_ANY, 0});
    
    Thing_add_in_port({world, fields, ports, logic,   "input",   MTT_ANY, 0, {ALIGN_DOWN}, nullptr});
    Thing_add_out_port({world, fields, ports, logic,  "value", MTT_ANY, 0, {ALIGN_DOWN}, nullptr});
    
    archetype->logic.proc = parameter_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](Thing* thing) {
        
    };
}

void conditional_brancher_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_INT64].alloc_byte_size + meta[MTT_INT64].alloc_byte_size + meta[MTT_BOOLEAN].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "cycle_index", MTT_INT64, 0});
    
    add_field({world, fields, ports, logic, "index_count", MTT_INT64, 0}, temp_mem<uint64>(30));
    add_field({world, fields, ports, logic, "prev_cycle_state", MTT_BOOLEAN, 0}, temp_mem<bool>(false));
    
    Thing_add_in_port({world, fields, ports, logic, "cycle", MTT_INT64, 0, {ALIGN_DOWN}, nullptr});
    Thing_add_in_port({world, fields, ports, logic,  "active", MTT_BOOLEAN, 0, {ALIGN_DOWN}, nullptr});
    for (usize idx = 0; idx < 30; idx += 1) {
        Thing_add_in_port({world, fields, ports, logic, std::to_string(idx), MTT_BOOLEAN, 0, {ALIGN_LEFT + ALIGN_DOWN * (idx * 0.15f)}, nullptr});
        
        Thing_add_out_port({world, fields, ports, logic, std::to_string(idx), MTT_BOOLEAN, 0, {ALIGN_LEFT + ALIGN_DOWN * (idx * 0.15f)}, nullptr});
    }
    
    
    
    archetype->logic.proc = conditional_brancher_procedure;
    
    archetype->logic.option = 0;
    archetype->logic.option_flags = 0;
    
    archetype->on_thing_make = [](Thing* thing) {
        auto* init_cycle_state = mtt::access<bool>(thing, "prev_cycle_state");
        *init_cycle_state = false;
    };
}

void time_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                0
                                );
    
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
}

MTT_DEFINE_LOGIC_PROCEDURE(time)
{
    get_out_port(thing, 0).value.out.set_Float(world->time_seconds);
    return true;
}

void float32_value_init(ARCHETYPE_PARAM_LIST)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                meta[MTT_FLOAT].alloc_byte_size
                                );
    
    add_field({world, fields, ports, logic, "value", MTT_FLOAT, 0});
    
    
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
}

MTT_DEFINE_LOGIC_PROCEDURE(float32_value)
{
    auto* value = mtt::access<float32>(thing, "value");
    auto val_in = mtt::get_in_port(world, thing, input, "value");
    if (val_in.status == PORT_STATUS_OK) {
        *value = val_in.value.out.Float;
    }
    auto val_out = mtt::get_out_port(world, thing, "value");
    val_out.value.out.set_Float(*value);
    return true;
}

void label_init(ARCHETYPE_PARAM_LIST)
{
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
    //                                meta[MTT_STRING].alloc_byte_size
    //                                );
    
    add_field({world, fields, ports, logic, "value", MTT_STRING, 0}, builder);
}

MTT_DEFINE_LOGIC_PROCEDURE(label)
{
    return true;
}

MTT_DEFINE_INITIALIZER(matrix_matrix_multiply)
{
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
    //                                0
    //                                );
    
    Thing_add_in_port({world, fields, ports, logic, "left", MTT_MATRIX4, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "right", MTT_MATRIX4, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "result", MTT_MATRIX4, 0, {ALIGN_RIGHT}, nullptr});
}

MTT_DEFINE_LOGIC_PROCEDURE(matrix_matrix_multiply)
{
    auto result = get_in_port_or(thing, input, 0, Mat4(1.0f)) *
    get_in_port_or(thing, input, 1, Mat4(1.0f));
    get_out_port(thing, 0).value.out.set_Matrix4(result);
    return true;
}

MTT_DEFINE_INITIALIZER(multiply)
{
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
    //                                0
    //                                );
    Thing_add_in_port({world, fields, ports, logic, "left", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "right", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "result", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
}
MTT_DEFINE_LOGIC_PROCEDURE(multiply)
{
    auto result = get_in_port_or<MTT_FLOAT_Type>(thing, input, 0, 1.0f) *
    get_in_port_or<MTT_FLOAT_Type>(thing, input, 1, 1.0f);
    get_out_port(thing, 0).value.out.set_Float(result);
    
    return true;
}

MTT_DEFINE_INITIALIZER(add)
{
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
    //                                0
    //                                );
    Thing_add_in_port({world, fields, ports, logic, "left", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "right", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "result", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
}
MTT_DEFINE_LOGIC_PROCEDURE(add)
{
    auto result = get_in_port_or<MTT_FLOAT_Type>(thing, input, 0, 1.0f) +
    get_in_port_or<MTT_FLOAT_Type>(thing, input, 1, 1.0f);
    get_out_port(thing, 0).value.out.set_Float(result);
    
    return true;
}

MTT_DEFINE_INITIALIZER(subtract)
{
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
    //                                0
    //                                );
    Thing_add_in_port({world, fields, ports, logic, "left", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "right", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "result", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
}
MTT_DEFINE_LOGIC_PROCEDURE(subtract)
{
    auto result = get_in_port_or<MTT_FLOAT_Type>(thing, input, 0, 1.0f) -
    get_in_port_or<MTT_FLOAT_Type>(thing, input, 1, 1.0f);
    get_out_port(thing, 0).value.out.set_Float(result);
    
    return true;
}

MTT_DEFINE_INITIALIZER(divide)
{
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
    //                                0
    //                                );
    Thing_add_in_port({world, fields, ports, logic, "left", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "right", MTT_FLOAT, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "result", MTT_FLOAT, 0, {ALIGN_RIGHT}, nullptr});
}
MTT_DEFINE_LOGIC_PROCEDURE(divide)
{
    auto right = get_in_port_or<MTT_FLOAT_Type>(thing, input, 1, 1.0f);
    if (right == 0.0f) {
        get_out_port(thing, 0).value.out.set_Float(NEGATIVE_INFINITY);
        return true;
    }
    
    auto result = get_in_port_or<MTT_FLOAT_Type>(thing, input, 0, 1.0f) /
    right;
    get_out_port(thing, 0).value.out.set_Float(result);
    
    return true;
}

MTT_DEFINE_INITIALIZER(group_block_begin)
{
    //    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
    //                                0
    //                                );
    MTT_ADD_FIELD_T("state", Group_Block_Begin);
}
MTT_DEFINE_LOGIC_PROCEDURE(group_block_begin)
{
    auto* begin = mtt::access<Group_Block_Begin>(thing, "state");
    
    auto* end = Thing_try_get(world, begin->matching_end);
    ASSERT_MSG(end != nullptr, "???");
    auto* END = mtt::access<Group_Block_End>(end, "state");
    END->has_exited = false;
    if (begin->should_push_new_context) {
        {
            Script_Contexts* new_ctxs = nullptr;
            for (usize i = 0; i < s->contexts.size(); i += 1) {
                if (s->contexts[i].is_done) {
                    new_ctxs = &s->contexts[i];
                    s->contexts[i].is_done = false;
                    break;
                }
            }
            
            if (new_ctxs == nullptr) {
                s->contexts.push_back((Script_Contexts){});
                new_ctxs = &s->contexts.back();
            }
            Script_Context ctx = {};
            ctx.first_idx = thing->eval_index;
            ctx.instruction_idx = ctx.first_idx;
            new_ctxs->ctx_stack.push_back(ctx);
            
            
            Script_Context next_ctx   = new_ctxs->ctx_stack.back();
            next_ctx.instruction_idx  = thing->eval_index + 1;
            next_ctx.first_idx        = next_ctx.instruction_idx;
            next_ctx.continuation_idx = end->eval_index + 1;
            next_ctx.local_ops.clear();
            new_ctxs->ctx_stack.push_back(next_ctx);
            
            {//
                usize begin_index = next_ctx.first_idx + 1;
                usize end_index = end->eval_index;
                ASSERT_MSG(begin_index <= end_index, "overlapping entries in group block is disallowed");
                init_script_instance_instructions_range(s, begin_index, end_index);
            }
        }
        
        auto& Q = Script_current_queue(s);
        Script_Operation_List op = {};
        op.is_waiting_on_subscope = true;
        op.subscope = end;
        
        Q.push_back(op);
        
        (*s_ctx)->instruction_idx = end->eval_index + 1;
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP;
    } else {
        
        Script_context_push(s, end->eval_index + 1, end->eval_index);
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ENTER_SCOPE;
    }
}



// end_block)
// group_end)
MTT_DEFINE_INITIALIZER(group_block_end)
{
    MTT_ADD_FIELD_T("state", Group_Block_End);
}
MTT_DEFINE_LOGIC_PROCEDURE(group_block_end)
{
    auto* end = mtt::access<Group_Block_End>(thing, "state");
    auto* begin = mtt::access<Group_Block_Begin>(Thing_get(world, end->matching_begin), "state");
    
//    auto required_time_elapsed = [&]() -> bool {
//        if (end->last_checked_time == -1) {
//            end->last_checked_time = world->time_seconds;
//        } else {
//            end->time_remaining -= (world->time_seconds - end->last_checked_time);
//            end->last_checked_time = world->time_seconds;
//        }
//#ifndef NDEBUG
//        MTT_print("Last checked time: %f, Time remaining: %f, Time interval: %f", end->last_checked_time, end->time_remaining, end->time_interval);
//#endif
//        return (end->time_remaining <= 0.0f);
//    };
    

    
//    if ((false) && end->has_count && end->has_time_interval) {
//        if (!required_time_elapsed()) {
//            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
//        } else {
//            // reset clock
//            if (end->time_remaining < 0.0f) {
//                end->time_remaining = end->time_interval + end->time_remaining;
//            } else {
//                end->time_remaining = end->time_interval;
//            }
//            end->last_checked_time = -1.0f;
//
//            if (end->count == mtt::FOREVER) {
//                return jump_to_beginning(s, s_ctx, thing->eval_index, end->reset_upon_jump);
//            } else {
//                end->count -= 1;
//                if (end->count != 0) {
//                    return jump_to_beginning(s, s_ctx, thing->eval_index, end->reset_upon_jump);
//                } else {
//                    goto LABEL_SHOULD_EXIT;
//                }
//            }
//        }
//    } else
    if (end->has_count) {
        
        if (end->count == mtt::FOREVER) {
            return jump_to_beginning(s, s_ctx, thing->eval_index, end->reset_upon_jump);
        } else {
            end->count -= 1;
            if (end->count != 0) {
                return jump_to_beginning(s, s_ctx, thing->eval_index, end->reset_upon_jump);
            } else {
                goto LABEL_SHOULD_EXIT;
            }
        }
    }
//    else if ((false) && end->has_time_interval) {
//        if (!required_time_elapsed()) {
//            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
//        } else {
//            // reset clock
//            if (end->time_remaining < 0.0f) {
//                end->time_remaining = end->time_interval + end->time_remaining;
//            } else {
//                end->time_remaining = end->time_interval;
//            }
//            end->last_checked_time = -1.0f;
//
//            goto LABEL_SHOULD_EXIT;
//        }
//    }
    
LABEL_SHOULD_EXIT:;
    
    Script_context_pop(s);
    
    if (end->is_matched_with_root) {
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
    } else if (begin->should_push_new_context) {
        end->has_exited = true;
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
    }
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_EXIT_SCOPE;
}



mtt::Thing* code_procedure_make(Logic_Procedure&& proc, void* args)
{
    auto* thing = mtt::Thing_make(mtt::ctx(), mtt::ARCHETYPE_CODE_PROCEDURE);
    Logic_Procedure* proc_ptr = access<Logic_Procedure>(thing, "procedure");
    *proc_ptr = proc;
    
    auto** _ = mtt::access<void*>(thing, "args");
    *_ = args;
    return thing;
}

mtt::Thing* code_procedure_make_thing_in(void (*proc)(mtt::World* world, Script_Instance* script, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg, bool is_valid, mtt::Thing* thing))
{
    return mtt::code_procedure_make([](LOGIC_PROCEDURE_PARAM_LIST) -> mtt::Logic_Procedure_Return_Status {
        auto IN_arg     = mtt::get_in_port(thing, input, "in");
        auto IN_source  = mtt::get_in_port(thing, input, "in_source");
        auto OUT_arg    = mtt::get_out_port(thing, "out");
        
        if (IN_source.status == mtt::PORT_STATUS_OK) {
            mtt::Any& val = IN_arg.value.out;
            MTT_UNUSED(val);
            mtt::Any& src = IN_source.value.out;
            
            mtt::Thing_ID thing_src = src.thing_id;
            
            using PROC_TYPE = void (*)(mtt::World* world, Script_Instance* script, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_arg, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& IN_source, mtt::Result<mtt::Evaluation_Output_Entry_Port&>& OUT_arg, bool is_valid, mtt::Thing* thing);
            if (mtt::Thing* src = mtt::Thing_try_get(world, thing_src)) {
                PROC_TYPE proc = *(PROC_TYPE*)mtt::access<void*>(thing, "args");
                proc(world, s, IN_arg, IN_source, OUT_arg, (src != nullptr), src);
            }
        }
              
        return mtt::LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    }, (void*)proc);
}

MTT_DEFINE_INITIALIZER(code_procedure)
{
    MTT_ADD_FIELD_T("procedure", Logic_Procedure);
    MTT_ADD_FIELD("args", MTT_POINTER, MTT_NONE);
    
    Thing_add_in_port({world, fields, ports, logic, "in",   MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "in_source",   MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "out", MTT_ANY, 0, {ALIGN_RIGHT}, nullptr});
    
    
    
    archetype->on_thing_make = [](mtt::Thing* thing)
    {
        Logic_Procedure* proc = access<Logic_Procedure>(thing, "procedure");
        *proc = [](LOGIC_PROCEDURE_PARAM_LIST) -> Logic_Procedure_Return_Status {
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
        };
    };
}

MTT_DEFINE_LOGIC_PROCEDURE(code_procedure)
{
    Logic_Procedure& proc = *mtt::access<Logic_Procedure>(thing, "procedure");
    
    return proc(LOGIC_PROCEDURE_PARAM_LIST_ARGS);
}

MTT_DEFINE_INITIALIZER(call)
{
//    MTT_ADD_FIELD_T_EX("state", Node_Graph_State,
//                       [](void* data) {
//        new (data) Node_Graph_State();
//    },
//                       [](void* data_dst, void* data_src) {
//        (*reinterpret_cast<Node_Graph_State*>(data_dst)) = (*reinterpret_cast<Node_Graph_State*>(data_src));
//    },
//                       [](void* data) {
//        reinterpret_cast<Node_Graph_State*>(data)->~Node_Graph_State();
//    },
//                       mtt::MTT_FIELD_FLAG_NONE
//                       );
auto on_init_call_field = [](void* data) {
    new (data) Call_Descriptor();
};
auto on_copy_call_field = [](void* data_dst, void* data_src) {
    using T = Call_Descriptor;
    new (data_dst) T(*((T*)data_src));
};
MTT_ADD_FIELD_T_EX("state", Call_Descriptor,
on_init_call_field,
on_copy_call_field
,
[](void* data) {
    reinterpret_cast<Call_Descriptor*>(data)->~Call_Descriptor();
},
    mtt::MTT_FIELD_FLAG_NONE
);
    //MTT_ADD_FIELD_T("params", Call_Param_List);
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        Call_Descriptor* call = mtt::access<Call_Descriptor>(thing, "state");
        call->call_instruction = thing->id;
    };
    
    archetype->on_thing_copy = [](mtt::Thing* to, mtt::Thing* from) {
        Call_Descriptor* call = mtt::access<Call_Descriptor>(to, "state");
        for (auto& param_list : call->params().param_lists) {
            for (auto& param : param_list.params) {
                param.query_param.query_rule.is_copy = true;
            }
        }
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
        Call_Descriptor* call = mtt::access<Call_Descriptor>(thing, "state");
        for (auto& param_list : call->params().param_lists) {
            for (auto& param : param_list.params) {
                Query_Rule_destroy(&param.query_param.query_rule);
            }
        }
    };
}

void Call_cancel_stopped_sources(Call_Descriptor* call, Script_Instance* s)
{
    auto& calls_all_things_to_stop = Runtime::ctx()->sources_to_stop_for_call;
    auto& things_to_stop = calls_all_things_to_stop[call->call_instruction];
    if (things_to_stop.empty()) {
        return;
    }
    
    // TODO: maybe want the creation time of the top-most root script
    uint64 root_creation_time = s->creation_time;
    uint64 actual_root_creation_time = Script_Instance_root_creation_time(s);
    for (usize p_i = 0; p_i < call->params().param_list_count(); p_i += 1) {
        auto& plist_i = call->params().param_list(p_i);
        auto& lookup = plist_i.lookup;
        for (usize p_idx = 0; auto& p : plist_i.params) {
            switch (p.type) {
                    //MTT_print("param: [%s]\n", mtt::Call_Param::TYPE_strings[(unsigned int)p.type]);
                case mtt::Call_Param::TYPE::FIXED_THINGS: {
                    if (p.property_lookup_mapping.target_key == ARG_source) {

                        auto& entry = lookup[p.property_lookup_mapping.source_key];
                        for (auto& el : p.prop_list) {
                            auto& list = entry[el.scope];
                            for (auto it = list.begin(); it != list.end();) {
                                auto find_contains = things_to_stop.find(it->value.thing_id);
                                if (find_contains != things_to_stop.end() && root_creation_time <= find_contains->second) {
                                    //auto c_time = find_contains->second;
                                    it = list.erase(it);
                                } else {
                                    ++it;
                                }
                            }
                        }
                        for (auto it = p.prop_list.begin(); it != p.prop_list.end(); ) {
                            auto find_contains = things_to_stop.find(it->value.thing_id);
                            if (find_contains != things_to_stop.end()) {
                                //auto c_time = find_contains->second;
                                auto against = find_contains->second;
                                if (root_creation_time <= against) {
                                    it = p.prop_list.erase(it);
                                } else {
                                    ++it;
                                }
                                
                            } else {
                                ++it;
                            }
                        }
                    } else {
                        break;
                    }
                }
                default: { break; }
            }
        }
    }
    
    things_to_stop.clear();
    calls_all_things_to_stop.erase(call->call_instruction);
}

Logic_Procedure_Return_Status handle_call(Call_Descriptor* call, mtt::Thing* thing, Port_Input_List* input, Script_Instance* s, void (*on_new_script)(Script_Instance* si, void* args), void* args);
Logic_Procedure_Return_Status handle_call(Call_Descriptor* call, Script_Instance* s, void (*on_new_script)(Script_Instance* si, void* args), void* args);

Logic_Procedure_Return_Status handle_call(Call_Descriptor* call, Script_Instance* s, void (*on_new_script)(Script_Instance* si, void* args), void* args)
{
    mtt::Thing* thing = mtt::Thing_try_get(mtt::ctx(), call->call_instruction);
    if (thing == nullptr) {
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
    }
    Port_Input_List* input = nullptr;
    map_get(curr_graph(mtt::ctx())->incoming, thing->id, &input);
    
    return handle_call(call, thing, input, s, on_new_script, args);
}

static mtt::Set_Stable<mtt::String> non_referrable_actions = {
    "reference",
};

bool referrents_handled(mtt::World* w, Call_Descriptor* call_desc, Script_Property_Lookup& entry, Script_Property& el)
{
    if (non_referrable_actions.contains(call_desc->label)) {
        return false;
    }
    auto* refs = thing_referrents(w, el.value.thing_id);

    if (refs == nullptr) {
        return false;
    }
  
    auto* entry_scope = &entry[el.scope];
    auto el_referrent = el;
    
    mtt::Set<mtt::Thing_ID> unresolved = {};
    mtt::Set<mtt::Thing_ID> seen = {};
    
    for (auto referrent_id : *refs) {
        mtt::Thing* tgt = mtt::Thing_try_get(w, referrent_id);
        if (tgt != nullptr) {
            unresolved.insert(referrent_id);
        }
    }
    
    if (unresolved.empty()) {
        return false;
    }
    
    while (!unresolved.empty()) {
        mtt::Thing_ID ref_id = *unresolved.begin();
        unresolved.erase(unresolved.begin());
        seen.insert(ref_id);
        
        auto* refs = thing_referrents(w, ref_id);
        if (refs != nullptr) {
            bool sub = false;
            for (auto referrent_id : *refs) {
                
                mtt::Thing* tgt = mtt::Thing_try_get(w, referrent_id);
                if (tgt == nullptr) {
                    continue;
                }
                
                sub = true;
                if (seen.contains(referrent_id)) {
                    el_referrent.value.thing_id = referrent_id;
                    entry_scope->push_back(el_referrent);
                } else {
                    unresolved.insert(referrent_id);
                }
            }
            if (!sub) {
                el_referrent.value.thing_id = ref_id;
                entry_scope->push_back(el_referrent);
            }
        } else {
            el_referrent.value.thing_id = ref_id;
            entry_scope->push_back(el_referrent);
        }
    }
    
    return true;
}

Logic_Procedure_Return_Status handle_call(Call_Descriptor* call, mtt::Thing* thing, Port_Input_List* input, Script_Instance* s, void (*on_new_script)(Script_Instance* si, void* args), void* args)
{
//#ifndef NDEBUG
//    MTT_log_debug("CALL TEST_FLAG: %llu\n", call->TEST_FLAG);
//#endif
    if (call->on_invoke != nullptr) {

        {
            //auto& param_list = call->curr_param_list();
            
//            auto& param0 = param_list.params[0];
//            auto& param1 = param_list.params[1];
//            ASSERT_MSG(param_list.params.size() >= 2, "???");
//            ASSERT_MSG(param0.property_lookup_mapping.source_key == "AGENT", "???");
//            ASSERT_MSG(param0.property_lookup_mapping.target_key == mtt::ARG_source, "???");
            //MTT_print("begin plist\n");
            
            Call_cancel_stopped_sources(call, s);
            
            ASSERT_MSG(call->params().param_lists.size() > 0, "???");

            
            //usize param_list_count = call->params().param_list_count();
            for (usize i = 0; i < call->params().param_list_count(); i += 1) {
                auto& plist_i = call->params().param_list(i);
                plist_i.lookup.clear();
                //plist_i.print();
                
                // TODO: fill
                std::vector<usize> deferred = {};
                for (usize p_idx = 0; auto& p : plist_i.params) {
                    
                    switch (p.type) {
                        //MTT_print("param: [%s]\n", mtt::Call_Param::TYPE_strings[(unsigned int)p.type]);
                        case mtt::Call_Param::TYPE::FIXED_THINGS: {
//                            if (p.is_ref) {
//                                auto* r_call = p.ref.call_desc;
//                                auto& r_plist = r_call->param_list(i);
//                                auto& r_p = r_plist.params[p_idx];
//                                if (r_p.did_early_context_done) {
//                                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
//                                }
//                                p.prop_list = r_p.prop_list;
//                                break;
//                            } else {
//                                int bla = 0;
//                            }
                            break;
                        }
                        case mtt::Call_Param::TYPE::TYPES: {
                            break;
                        }
                        case mtt::Call_Param::TYPE::QUERY: {
                            MTT_BP();
                            {
                                p.prop_list.clear();
                                
                                p.did_early_context_done = false;
                                
                                if (p.is_ref) {
                                    Call_Descriptor* r_call = Call_Descriptor_from_Thing(mtt::world(thing), p.ref.call_id);
                                    
                                    bool check_return = r_call->do_check_return_value;
                                    
                                    ASSERT_MSG(r_call != nullptr, "%s\n", "Should exist!");
//                                    assert(r_call->is_source);
//                                    assert(call->is_source);
                                    if (r_call->params_saved().param_list_count() <= p.ref.param_list_index) {
                                        break;
                                    }
                                    auto& r_plist = r_call->params_saved().param_list(p.ref.param_list_index);
                                    
                                    
                                    auto& r_p = r_plist.params[p.ref.param_list_entry_index];
                                    if (r_p.did_early_context_done) {
                                        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                                    }
                                    
                                    p.prop_list.resize(r_p.prop_list.size());
                                    
                                    if (r_p.query_param.rule_handles.empty()) {
                                        for (usize r_p_i = 0; r_p_i < s->return_value.size(); r_p_i += 1) {
                                            p.prop_list[r_p_i] = {
                                                .value = s->return_value[r_p_i],
                                                .label = p.label,
                                                .selector_name = p.selector_name,
                                                p.selector_value,
                                                .scope = p.label
                                            };
                                        }
                                        
                                        
                                        break;
                                    }
                                    
                                    for (usize r_p_i = 0; r_p_i < r_p.prop_list.size(); r_p_i += 1) {
                                        p.prop_list[r_p_i] = {
                                            .value = r_p.prop_list[r_p_i].value,
                                            .label = r_p.query_param.rule_handles.back().name,
                                            .selector_name = p.selector_name,
                                            p.selector_value,
                                            .scope = p.label
                                        };
                                    }
                                    break;
                                }
                                
                                if (p.allow_prev_seen_thing_props) {
                                    p.prev_seen.clear();
                                }
                                
                                usize count_required = p.count_constraint;
                                usize count_found = 0;
                                Query_Rule_results_for_var(&p.query_param.rule(), p.query_param.var(), [&](auto* result) {
                                    //MTT_print("Thing: id=[%llu]\n", result->id);
                                    //MTT_print("rule: %s\n", p.query_param.rule().string_rep.c_str());
                                    if (!p.allow_prev_seen_thing_props) {
                                        if (p.prev_seen.contains(result->id)) {
                                            return true;
                                        } else {
                                            p.prev_seen.insert(result->id);
                                        }
                                    }
                                        
                                    p.prop_list.push_back({.value = mtt::Any::from_Thing_ID(result->id),
                                            .label = p.query_param.rule_handles.back().name,
                                            .selector_name = p.selector_name,
                                            p.selector_value,
                                            .scope = p.label
                                    });
                                    
                                    count_found += 1;
                                    if (!p.allow_prev_seen_thing_props) {
                                        if (count_required <= count_found && count_found != ULLONG_MAX && count_required != ULLONG_MAX) {
                                            return false;
                                        }
                                    }
                                    
                                    return true;
                                });
                                //if (count_required > count_found && count_found != ULLONG_MAX) {
                                if (count_found == 0) {
                                    p.did_early_context_done = true;
                                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                                }
                                
                                if (p.allow_prev_seen_thing_props) {
                                    
                                    auto prev_first_val = p.prev_first_property.value.thing_id;
                                    if (count_found > count_required) {
                                        for (usize e_idx = 0; e_idx < p.prop_list.size(); e_idx += 1) {
                                            if (p.prop_list[e_idx].value.thing_id == prev_first_val) {
                                                swap_and_pop(p.prop_list, e_idx);
                                                count_found -= 1;
                                                break;
                                            }
                                        }
                                        
                                        while (count_found > count_required) {
                                            auto to_remove_idx = MTT_Random_range(0, p.prop_list.size());
                                            p.prop_list.erase(p.prop_list.begin() + to_remove_idx);
                                            count_found -= 1;
                                        }
                                    } else {
                                        auto first_val = p.prop_list[0].value.thing_id;
                                        if (first_val == prev_first_val) {
                                            std::swap(p.prop_list[0], p.prop_list[p.prop_list.size() - 1]);
                                        }
                                    }
                                    
                                    p.prev_first_property = p.prop_list[0];
                                } else {
                                    if (count_found > count_required) {
                                        while (count_found > count_required) {
                                            auto to_remove_idx = MTT_Random_range(0, p.prop_list.size());
                                            p.prop_list.erase(p.prop_list.begin() + to_remove_idx);
                                            count_found -= 1;
                                        }
                                    }
                                    p.prev_first_property = p.prop_list[0];
                                }
                            }
                            break;
                        }
                        case mtt::Call_Param::TYPE::RULE_RESULT: {
                            auto& rule_vars_all = s->rule_vars;
                            auto& p_rule_vars = p.rule_vars;
                            p.prop_list.clear();
                            if (p_rule_vars.size() == 0) {
                                return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                            }
                            
                            for (usize r_idx = 0; r_idx < p_rule_vars.size(); r_idx += 1) {
                                auto& var_to_find = p_rule_vars[r_idx];
                                var_to_find.value = mtt::Any::from_Thing_ID(mtt::Thing_ID_INVALID);
                                
                                bool is_found_var = false;
                                for (usize in_r_idx = 0; in_r_idx < rule_vars_all.size(); in_r_idx += 1) {
                                    auto& var_to_check = rule_vars_all[in_r_idx];
                                    if (Rule_Var_Record_One_Result_is_equal(var_to_find, rule_vars_all[in_r_idx])) {
                                        is_found_var = true;
                                        var_to_find.value = var_to_check.value;
#ifndef NDEBUG
                                        {
                                            mtt::Thing_ID thing_id = var_to_check.value.thing_id;
                                        }
#endif
                                        
                                        
                                        p.prop_list.push_back({
                                            .value = var_to_check.value,
//                                            .label = r_p.query_param.rule_handles.back().name,
                                            .selector_name = p.selector_name,
                                            p.selector_value,
                                            .scope = p.label
                                        });
                                        
                                        
                                        break;
                                    }
                                }
                                if (!is_found_var) {
                                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
                                }
                            }
                            break;
                        }
                        case mtt::Call_Param::TYPE::ARG_THINGS: {
                            break;
                        }
                        case mtt::Call_Param::TYPE::LOCATION: {
                            
                            break;
                        }
                        case mtt::Call_Param::TYPE::SELECTOR: {
                            
                            break;
                        }
                        case mtt::Call_Param::TYPE::SET_FIELD: {
                            
                            break;
                        }
                        case mtt::Call_Param::TYPE::VALUE: {
                            break;
                        }
                        case mtt::Call_Param::TYPE::OP_SWAP_ARGS: {
                            deferred.push_back(p_idx);

                            break;
                        }
                    }
                    p_idx += 1;
                }
                
                call->save_params();
                
                for (auto& idx : deferred) {
                    auto& deferred_param = plist_i.params[idx];
                    switch (deferred_param.type) {
                        case Call_Param::TYPE::OP_SWAP_ARGS: {
                            auto& from = deferred_param.swap_args_from;
                            auto& to   = deferred_param.swap_args_to;
                            for (usize p_idx_ = 0; p_idx_ < plist_i.params.size(); p_idx_ += 1) {
                                auto& at = plist_i.params[p_idx_];
                                if (at.property_lookup_mapping.target_key == from.target_key) {
                                    at.property_lookup_mapping = to;
                                } else if (at.property_lookup_mapping.target_key == to.target_key) {
                                    at.property_lookup_mapping = from;
                                }
                            }
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                }
                
                
                
                auto& lookup = plist_i.lookup;
                for (usize p_idx = 0; auto& p : plist_i.params) {
                    
                    switch (p.type) {
                        //MTT_print("param: [%s]\n", mtt::Call_Param::TYPE_strings[(unsigned int)p.type]);
                        case mtt::Call_Param::TYPE::FIXED_THINGS: {
                            
                            auto* w = mtt::world(thing);
                            for (auto it = p.prop_list.begin(); it != p.prop_list.end();) {
                                if (!mtt::Thing_try_get(w, it->value.thing_id)) {
                                    it = p.prop_list.erase(it);
                                } else {
                                    ++it;
                                }
                            }
                            
                            auto& entry = lookup[p.property_lookup_mapping.target_key];
                            for (auto& el : p.prop_list) {
                                if (!referrents_handled(w, call, entry, el)) {
                                    entry[el.scope].push_back(el);
                                }
                            }
                            break;
                        }
                        case mtt::Call_Param::TYPE::TYPES: {
                            auto& entry = lookup[p.property_lookup_mapping.target_key];
                            for (auto& el : p.prop_list) {
                                entry[el.scope].push_back(el);
                            }
                            break;
                        }
                        case mtt::Call_Param::TYPE::QUERY: {
                            auto* w = mtt::world(thing);
                            auto& entry = lookup[p.property_lookup_mapping.target_key];
                            for (auto& el : p.prop_list) {
                                if (!referrents_handled(w, call, entry, el)) {
                                    entry[el.scope].push_back(el);
                                }
                            }
                            break;
                        }
                        case mtt::Call_Param::TYPE::RULE_RESULT: {
                            auto& entry = lookup[p.property_lookup_mapping.target_key];
                            for (auto& el : p.prop_list) {
                                entry[el.scope].push_back(el);
                            }
                            break;
                        }
                        case mtt::Call_Param::TYPE::ARG_THINGS: {
                            
                            
                            //Script_Lookup_print(s->lookup());
                            //MTT_print("param target_key:[%s]\n", p.property_lookup_mapping.target_key.c_str());
                            
                            Script_Instance* s_parent = s->parent;
                            Script_Instance* s_parent_2 = s_parent->parent;
                            bool do_override_selectors = false;
                            if (s_parent_2 == nullptr) {
                                
                                auto* plist_ptr = Script_Lookup_get_var_with_key(s, p.property_lookup_mapping.target_key, DEFAULT_LOOKUP_SCOPE);
                                if (!plist_ptr || plist_ptr->empty()) {
                                    
                                    do_override_selectors = true;
//                                    static const mtt::String ARG_source     = "ARG_source";
//                                    static const mtt::String ARG_target     = "ARG_target";
//                                    static const mtt::String ARG_object     = "ARG_object";
//                                    static const mtt::String ARG_object_aux = "ARG_object_aux";

                                    
                                    
                                    {
                                        const auto* target_key_ptr = &p.property_lookup_mapping.target_key;
                                        const auto lookup_scope = DEFAULT_LOOKUP_SCOPE;
                                        
                                        const mtt::String** fallbacks = nullptr;
                                        usize COUNT = 0;
                                        
                                        if (*target_key_ptr == ARG_source) {
                                            target_key_ptr = &ARG_source;
                                            
//                                            static const mtt::String* fallbacks_for_ARG_source[] = {
//                                                &ARG_target, &ARG_object
//                                            };
                                            
                                            //fallbacks = fallbacks_for_ARG_source;
                                            COUNT = 0//std::size(fallbacks_for_ARG_source)
                                            ;
                                            
                                        } else if (*target_key_ptr == ARG_target) {
                                            target_key_ptr = &ARG_target;
                                            
                                            static const mtt::String* fallbacks_for_ARG_target[] = {
                                                &ARG_object,
                                            };
                                            
                                            fallbacks = fallbacks_for_ARG_target;
                                            COUNT = std::size(fallbacks_for_ARG_target);
                                            
                                        } else if (*target_key_ptr == ARG_object) {
                                            target_key_ptr = &ARG_object;
                                            
                                            static const mtt::String* fallbacks_for_ARG_object[] = {
                                                &ARG_target
                                            };
                                            
                                            fallbacks = fallbacks_for_ARG_object;
                                            COUNT = std::size(fallbacks_for_ARG_object);
                                        } else if (*target_key_ptr == ARG_object_aux) {
                                            target_key_ptr = &ARG_object_aux;
                                            
                                            static const mtt::String* fallbacks_for_ARG_object_aux[] = {
                                               &ARG_object, &ARG_target
                                            };
                                            
                                            fallbacks = fallbacks_for_ARG_object_aux;
                                            COUNT = std::size(fallbacks_for_ARG_object_aux);
                                            
                                            auto& entry0 = lookup[ARG_object];
                                            auto& entry1 = lookup[ARG_target];
                                            
                                            
                                            // special case
                                            {
                                                for (usize fb_idx = 0; fb_idx < COUNT; fb_idx += 1) {
                                                    auto* key = fallbacks[fb_idx];
                                                    auto* ptr = Script_Lookup_get_var_with_key(s, *key, lookup_scope);
                                                    if (ptr != nullptr && !ptr->empty()) {
                                                        for (auto& el : *ptr) {
                                                            entry0[el.scope].push_back(el);
                                                            entry1[el.scope].push_back(el);
                                                        }
                                                    }
                                                }
                                                
                                                auto& plist = *plist_ptr;
                                                
                                                auto* w = mtt::world(thing);
                                                for (auto it = p.prop_list.begin(); it != p.prop_list.end();) {
                                                    if (!mtt::Thing_try_get(w, it->value.thing_id)) {
                                                        it = p.prop_list.erase(it);
                                                    } else {
                                                        ++it;
                                                    }
                                                }
                                            
  

                                            }
                                            
                                            
                                            break;
                                        }
                                        
                                         
                                        if (fallbacks == nullptr) {
                                            break;
                                        }
                                        
                                        for (usize fb_idx = 0; fb_idx < COUNT; fb_idx += 1) {
                                            auto* key = fallbacks[fb_idx];
                                            auto* ptr = Script_Lookup_get_var_with_key(s, *key, lookup_scope);
                                            if (ptr != nullptr && !ptr->empty()) {
                                                plist_ptr = ptr;
                                                target_key_ptr = key;
                                                break;
                                            }
                                        }
                                    }
                                    if (plist_ptr == nullptr) {
                                        break;
                                    }

                                }
                                auto& plist = *plist_ptr;
                                
                                auto* w = mtt::world(thing);
                                for (auto it = p.prop_list.begin(); it != p.prop_list.end();) {
                                    if (!mtt::Thing_try_get(w, it->value.thing_id)) {
                                        it = p.prop_list.erase(it);
                                    } else {
                                        ++it;
                                    }
                                }
                                
                                auto& entry = lookup[p.property_lookup_mapping.target_key];
                                //Script_Lookup_print(lookup);
                                if (do_override_selectors) {
                                    if (!plist.empty()) {
                                        for (auto& el : plist) {
                                            auto& entry_w_scope = entry[el.scope];
                                            entry_w_scope.push_back(el);
                                            auto& back_el = entry_w_scope.back();
                                            back_el.selector_name = p.selector_name;
                                            back_el.selector_value = p.selector_value;
                                        }
                                    } else {
                                        MTT_log_error("%s\n", "empty");
                                    }
                                } else {
                                    if (!plist.empty()) {
                                        for (auto& el : plist) {
                                            entry[el.scope].push_back(el);
                                        }
                                    } else {
                                        MTT_log_error("%s\n", "empty");
                                    }
                                }
                                
                                //Script_Lookup_print(lookup);
                                
                                
                            } else {
                                
                                auto* plist_ptr = Script_Lookup_get_var_with_key(s_parent, p.property_lookup_mapping.target_key, DEFAULT_LOOKUP_SCOPE);
                                
                                if (!plist_ptr || plist_ptr->empty()) {
                                    plist_ptr = Script_Lookup_get_var_with_key(s_parent_2, p.property_lookup_mapping.target_key, DEFAULT_LOOKUP_SCOPE);
                                }
                                
                                if (!plist_ptr || plist_ptr->empty()) {
                                    
                                    do_override_selectors = true;
//                                    static const mtt::String ARG_source     = "ARG_source";
//                                    static const mtt::String ARG_target     = "ARG_target";
//                                    static const mtt::String ARG_object     = "ARG_object";
//                                    static const mtt::String ARG_object_aux = "ARG_object_aux";

                                    
                                    
                                    {
                                        const auto* target_key_ptr = &p.property_lookup_mapping.target_key;
                                        const auto lookup_scope = DEFAULT_LOOKUP_SCOPE;
                                        
                                        const mtt::String** fallbacks = nullptr;
                                        usize COUNT = 0;
                                        
                                        if (*target_key_ptr == ARG_source) {
                                            target_key_ptr = &ARG_source;
                                            
//                                            static const mtt::String* fallbacks_for_ARG_source[] = {
//                                                &ARG_target, &ARG_object
//                                            };
                                            
                                            //fallbacks = fallbacks_for_ARG_source;
                                            COUNT = 0//std::size(fallbacks_for_ARG_source)
                                            ;
                                            
                                        } else if (*target_key_ptr == ARG_target) {
                                            target_key_ptr = &ARG_target;
                                            
                                            static const mtt::String* fallbacks_for_ARG_target[] = {
                                                &ARG_object,
                                            };
                                            
                                            fallbacks = fallbacks_for_ARG_target;
                                            COUNT = std::size(fallbacks_for_ARG_target);
                                            
                                        } else if (*target_key_ptr == ARG_object) {
                                            target_key_ptr = &ARG_object;
                                            
                                            static const mtt::String* fallbacks_for_ARG_object[] = {
                                                &ARG_target
                                            };
                                            
                                            fallbacks = fallbacks_for_ARG_object;
                                            COUNT = std::size(fallbacks_for_ARG_object);
                                        } else if (*target_key_ptr == ARG_object_aux) {
                                            target_key_ptr = &ARG_object_aux;
                                            
                                            static const mtt::String* fallbacks_for_ARG_object_aux[] = {
                                               &ARG_object, &ARG_target
                                            };
                                            
                                            fallbacks = fallbacks_for_ARG_object_aux;
                                            COUNT = std::size(fallbacks_for_ARG_object_aux);
                                            
                                            auto& entry0 = lookup[ARG_object];
                                            auto& entry1 = lookup[ARG_target];
                                            
                                            
                                            // special case
                                            {
                                                for (usize fb_idx = 0; fb_idx < COUNT; fb_idx += 1) {
                                                    auto* key = fallbacks[fb_idx];
                                                    auto* ptr = Script_Lookup_get_var_with_key(s_parent_2, *key, lookup_scope);
                                                    if (ptr != nullptr && !ptr->empty()) {
                                                        for (auto& el : *ptr) {
                                                            entry0[el.scope].push_back(el);
                                                            entry1[el.scope].push_back(el);
                                                        }
                                                    }
                                                }
                                                
                                                auto& plist = *plist_ptr;
                                                
                                                auto* w = mtt::world(thing);
                                                for (auto it = p.prop_list.begin(); it != p.prop_list.end();) {
                                                    if (!mtt::Thing_try_get(w, it->value.thing_id)) {
                                                        it = p.prop_list.erase(it);
                                                    } else {
                                                        ++it;
                                                    }
                                                }
                                            
  

                                            }
                                            
                                            
                                            break;
                                        }
                                        
                                         
                                        if (fallbacks == nullptr) {
                                            break;
                                        }
                                        
                                        for (usize fb_idx = 0; fb_idx < COUNT; fb_idx += 1) {
                                            auto* key = fallbacks[fb_idx];
                                            auto* ptr = Script_Lookup_get_var_with_key(s_parent_2, *key, lookup_scope);
                                            if (ptr != nullptr && !ptr->empty()) {
                                                plist_ptr = ptr;
                                                target_key_ptr = key;
                                                break;
                                            }
                                        }
                                    }
                                    if (plist_ptr == nullptr) {
                                        break;
                                    }

                                }
                                
                                auto& plist = *plist_ptr;
                                
                                
                                if (!plist.empty()) {
                                    auto& entry = lookup[p.property_lookup_mapping.target_key];
                                    if (do_override_selectors) {
                                        for (auto& el : plist) {
                                            auto& entry_w_scope = entry[el.scope];
                                            entry_w_scope.push_back(el);
                                            auto& back_el = entry_w_scope.back();
                                            back_el.selector_name = p.selector_name;
                                            back_el.selector_value = p.selector_value;
                                        }
                                    } else {
                                        
                                        for (auto& el : plist) {
                                            entry[el.scope].push_back(el);
                                        }
                                    }
                                }
                                
                                //Script_Lookup_print(lookup);
                                
                                //int BP2_ = 0;
                                
                                
                            }
                            
                            //ASSERT_MSG(false, "%s\n", "TODO");
                            
                            break;
                        }
                        case mtt::Call_Param::TYPE::LOCATION: {
                            break;
                        }
                        case mtt::Call_Param::TYPE::SELECTOR: {
                            
                            
                            
                            lookup[p.selector_name][DEFAULT_LOOKUP_SCOPE].push_back({.value = mtt::Any::from_String(p.selector_value.c_str())});
                            break;
                        }
                        case mtt::Call_Param::TYPE::SET_FIELD: {
                            break;
                        }
                        case mtt::Call_Param::TYPE::VALUE: {
                            break;
                        }
                        case mtt::Call_Param::TYPE::OP_SWAP_ARGS: {
                            break;
                        }
                    }
                    p_idx += 1;
                }
                
                static mtt::Set<Script_Property*> found = {};
                for (auto& [key, val] : lookup) {
                    found.clear();
                    for (auto& [entry_key, entry_val] : val) {
                        if (entry_key == DEFAULT_LOOKUP_SCOPE) {
                            continue;
                        }
                        for (auto& el : entry_val) {
                            found.insert(&el);
                        }
                    }
                    for (auto& f_entry : found) {
                        val[DEFAULT_LOOKUP_SCOPE].push_back(*f_entry);
                    }
                }
                //Script_Lookup_print(lookup);
                //MTT_BP();
            }

           // MTT_print("end plist\n");
            
            //MTT_BP();
        }
        ASSERT_MSG(call->params().param_lists.size() > 0, "???");
        auto ret = call->on_invoke(call, s, on_new_script, args);
        //MTT_print("Calling: %s\n", call->label.c_str());
        if (ret.type == LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED) {
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
        } else if (ret.type == LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE) {
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
        } else if (ret.type == LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE) {
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
        } else if (ret.type == LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR) {
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE;
        } else {
            ASSERT_MSG(false, "???");
        }
    } else {
//
//
//        if (call->source_script_id == Script_ID_INVALID) {
//            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
//        }
//
//        Script* callee_script = Script_lookup(call->source_script_id);
//        if (callee_script == nullptr) {
//            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_ERROR;
//        }
//
//        auto inputs = get_active_inputs(thing, input);
//        for (auto& input : inputs) {
//            mtt::Any& value = input.value;
//            mtt::String& name = input.name;
//        }
//
////        if (call->is_sequence) {
////            switch (call->calling_convention) {
////                case SCRIPT_CALLING_CONVENTION::ONE_TO_ONE: {
////
////                    break;
////                }
////                case SCRIPT_CALLING_CONVENTION::ONE_TO_MANY: {
////
////                    break;
////                }
////                case SCRIPT_CALLING_CONVENTION::MANY_TO_ONE: {
////
////                    break;
////                }
////                case SCRIPT_CALLING_CONVENTION::MANY_TO_MANY: {
////
////                    break;
////                }
////            }
////        }
//        Script_Instance* script_instance = Script_Instance_call_from_script(callee_script, s);
//        if (on_new_script) {
//            on_new_script(script_instance, nullptr);
//        }
//
//        auto& Q = Script_current_queue(s);
//        Q.push_back({
//            .list = {},
//            .call_info = *call,
//        });
//
//        Q.back().list.push_back({script_instance, 0});
//        mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, script_instance);
    }
    
    
    
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}

MTT_DEFINE_LOGIC_PROCEDURE(call)
{
    //MTT_log_debug("%s\n", "handling a call!");
    Call_Descriptor* call = mtt::access<Call_Descriptor>(thing, "state");
    
    if (call->is_active) {
#ifndef NDEBUG
        //MTT_print(">>>(call addr %p)\n", call);
#endif
    } else {
//#ifndef NDEBUG
//        MTT_print(">>>((DISABLED)call addr %p)\n", call);
//#endif
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
    }

    return handle_call(call, thing, input, s, nullptr, nullptr);
}

MTT_DEFINE_INITIALIZER(call_for_each)
{
    MTT_ADD_FIELD_T("state", Call_Descriptor);
    MTT_ADD_FIELD_T("loop", For_Each_Loop_Begin);
    //MTT_ADD_FIELD_T("params", Call_Param_List);
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        Call_Descriptor* call = mtt::access<Call_Descriptor>(thing, "state");
        call->call_instruction = thing->id;
    };
}

MTT_DEFINE_LOGIC_PROCEDURE(call_for_each)
{
    auto* call = mtt::access<Call_Descriptor>(thing, "state");
    auto* loop = mtt::access<For_Each_Loop_Begin>(thing, "loop");
    
    
    
    if (!loop->is_stateless) {
        ASSERT_MSG(false, "UNSUPPORTED");
    }
    
    
    
//    if (loop->index == 0) {
//        loop->is_started = true;
//        s->contexts.push_back((Script_Contexts){});
//        Script_Context ctx = {};
//        ctx.first_idx = thing->eval_index;
//        ctx.instruction_idx = ctx.first_idx;
//        s->contexts.back().ctx_stack.push_back(ctx);
//
//        auto& parentQ = Script_current_queue(s);
//
//        Script_set_current_context(s, s->contexts.size() - 1);
//
//        auto& Q = Script_current_queue(s);
//
//        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND;
//    } else if (loop->index >= loop->count) {
//        // MARK: DONE
//
//        loop->is_started = false;
//
//        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE_REMOVE_CONTEXT;
//    } else if (loop->is_sequence) {
//        // MARK: WAIT
//    } else {
//
//    }
    
    
    
    
    
    
    if (loop->wait_for_iteration_to_end) {
        
    }
    
    
    return true;
}


//MTT_T_X(ATTACH_TO_PARENT, attach_to_parent) \
//MTT_T_X(DETATCH_FROM_PARENT, detatch_from_parent)
MTT_DEFINE_INITIALIZER(attach_to_parent)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                0
                                );
    
    Thing_add_in_port({world, fields, ports, logic, "is_active", MTT_BOOLEAN, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "child", MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "parent", MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
}
MTT_DEFINE_LOGIC_PROCEDURE(attach_to_parent)
{
    auto IN_is_active = get_in_port(thing, input, "is_active");
    if (IN_is_active.status != PORT_STATUS_OK || IN_is_active.value.out.Boolean != true) {
        return true;
    }
    
    auto IN_child = get_in_port(thing, input, "child");
    auto IN_parent = get_in_port(thing, input, "parent");
    if (IN_child.status != PORT_STATUS_OK || IN_parent.status != PORT_STATUS_OK) {
        return true;
    }
    
    
    mtt::Thing_ID child_id = IN_child.value.out.thing_id;
    mtt::Thing* child = mtt::Thing_try_get(world, child_id);
    if (child == nullptr) {
        return true;
    }
    
    mtt::Thing_ID parent_id = IN_parent.value.out.thing_id;
    mtt::Thing* parent = mtt::Thing_try_get(world, parent_id);
    if (parent == nullptr) {
        return true;
    }
    
    Message msg;
    msg.sender   = child->id;
    msg.selector = mtt::string("center");
    selector_invoke(parent, &msg);
    ASSERT_MSG(msg.output_value.type == MTT_VECTOR3, "Incorrect type, expected MTT_VECTOR3, got %s\n", meta[msg.output_value.type].name.c_str());
    mtt::Thing_set_position(child, msg.output_value.Vector3);
    
    
    mtt::connect_parent_to_child(world, parent, child);
    
    
    return true;
}

MTT_DEFINE_INITIALIZER(detach_from_parent)
{
    Thing_alloc_init_field_list({world, fields, ports, logic, "", 0, 0},
                                0
                                );
    
    Thing_add_in_port({world, fields, ports, logic, "is_active", MTT_BOOLEAN, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_in_port({world, fields, ports, logic, "child", MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
}

MTT_DEFINE_LOGIC_PROCEDURE(detach_from_parent)
{
    auto IN_is_active = get_in_port(thing, input, "is_active");
    if (IN_is_active.status != PORT_STATUS_OK || IN_is_active.value.out.Boolean != true) {
        return true;
    }
    
    auto IN_child = get_in_port(thing, input, "child");
    if (IN_child.status != PORT_STATUS_OK) {
        return true;
    }
        
    mtt::Thing_ID child_id = IN_child.value.out.thing_id;
    mtt::Thing* child = mtt::Thing_try_get(world, child_id);
    if (child == nullptr) {
        return true;
    }
    
    mtt::disconnect_child_from_parent(world, child);

    return true;
}


MTT_DEFINE_INITIALIZER(particle_system)
{
    set_is_actor(archetype);
    
    MTT_ADD_FIELD("velocity",     MTT_VECTOR3, 0);
    MTT_ADD_FIELD("acceleration", MTT_VECTOR3, 0);
    MTT_ADD_FIELD("position",     MTT_VECTOR3, 0);
    MTT_ADD_FIELD("particle_state_list", MTT_POINTER, 0);
    MTT_ADD_FIELD("particle_drawable_list", MTT_POINTER, 0);
    MTT_ADD_FIELD_T("source", sd::Drawable_Info*);
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        auto** ptr = mtt::access_pointer_to_pointer<mtt::Dynamic_Array<Particle_System_State>*>(thing, "particle_state_list");
        
        array_pointer_make<Particle_System_State>(ptr);
        sd::Drawable_Info** src = mtt::access<sd::Drawable_Info*>(thing, "source");
        *src = nullptr;
        
        auto** ptr_dr = mtt::access_pointer_to_pointer<mtt::Dynamic_Array<sd::Drawable_Info>*>(thing, "particle_drawable_list");
        
        array_pointer_make<sd::Drawable_Info>(ptr_dr);
        
        int BP = 0;
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
        auto** ptr = mtt::access_pointer_to_pointer<mtt::Dynamic_Array<Particle_System_State>*>(thing, "particle_state_list");
        
        array_pointer_destroy<Particle_System_State>(ptr);
    };
    
    archetype->on_thing_copy = [](mtt::Thing* to, mtt::Thing* from)
    {
        //        auto** ptr_from = mtt::access_pointer_to_pointer<mtt::Dynamic_Array<Particle_State>*>(from, "particle_state_list");
        //
        //        auto** ptr_to = mtt::access_pointer_to_pointer<mtt::Dynamic_Array<Particle_State>*>(to, "particle_state_list");
        //
        //        clone(*ptr_to, *ptr_from);
        field_copy_array<Particle_System_State>(to, from, "particle_state_list");
    };
}
MTT_DEFINE_LOGIC_PROCEDURE(particle_system)
{
    return true;
}


MTT_DEFINE_INITIALIZER(path_follower)
{
    
}
MTT_DEFINE_LOGIC_PROCEDURE(path_follower)
{
    return true;
}

MTT_DEFINE_INITIALIZER(node_graph)
{
    mtt::set_is_actor(archetype);
    
    
    MTT_ADD_FIELD_T_EX("state", Node_Graph_State,
                       [](void* data) {
        new (data) Node_Graph_State();
    },
    [](void* data_dst, void* data_src) {
        using T = Node_Graph_State;
        new (data_dst) T(*((T*)data_src));
    },
    [](void* data) {
        reinterpret_cast<Node_Graph_State*>(data)->~Node_Graph_State();
    },
    mtt::MTT_FIELD_FLAG_NONE
    );
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
    };
    
    archetype->on_thing_destroy = [](mtt::Thing* thing) {
    };
    
    archetype->on_thing_copy = [](mtt::Thing* to, mtt::Thing* from)
    {
    };
    
    /*
     MTT_ADD_FIELD("things",  MTT_POINTER, 0);
     
     archetype->on_thing_make = [](mtt::Thing* thing) {
     //        auto** ptr = mtt::access_pointer_to_pointer<map_type*>(thing, "graph");
     //        map_pointer_make<map_type>(ptr);
     auto* map = mtt::access<Thing_To_Thing_List_Map>(thing, "graph");
     ASSERT_MSG(map != nullptr, "???");
     (*map)[0] = {1};
     int x = 0;
     
     //        mtt::Thing_List* list = mtt::access<mtt::Thing_List>(thing, "things");
     //        ASSERT_MSG(list != nullptr, "???");
     //        auto& l_val = *list;
     
     auto** ptr_dr = mtt::access_pointer_to_pointer<mtt::Thing_List*>(thing, "things");
     ASSERT_MSG(ptr_dr != nullptr, "???");
     array_pointer_make<mtt::Thing_Ref>(ptr_dr);
     };
     
     archetype->on_thing_destroy = [](mtt::Thing* thing) {
     auto** ptr = mtt::access_pointer_to_pointer<mtt::Thing_List*>(thing, "things");
     
     array_pointer_destroy<mtt::Thing_Ref>(ptr);
     };
     
     archetype->on_thing_copy = [](mtt::Thing* to, mtt::Thing* from)
     {
     //        auto** ptr_from = mtt::access_pointer_to_pointer<mtt::Dynamic_Array<Particle_State>*>(from, "particle_state_list");
     //
     //        auto** ptr_to = mtt::access_pointer_to_pointer<mtt::Dynamic_Array<Particle_State>*>(to, "particle_state_list");
     //
     //        clone(*ptr_to, *ptr_from);
     field_copy_array<mtt::Thing_Ref>(to, from, "things");
     };
     */
    
    
}
MTT_DEFINE_LOGIC_PROCEDURE(node_graph)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    auto& ng_state = *mtt::access<Node_Graph_State>(thing, "state");
    
#define NDEBUG_LINES_HERE
#ifndef NDEBUG_LINES_HERE
    sd::save(world->renderer);
    sd::set_render_layer(world->renderer, LAYER_LABEL_PER_FRAME_WORLD);
    sd::set_color_rgba_v4(world->renderer, {0.0f, 1.0f, 0.0f, 1.0f});
#endif
    for (auto it = ng_state.src_tgts.begin(); it != ng_state.src_tgts.end();) {
        mtt::Thing* src = world->Thing_try_get(it->first);
        if (src == nullptr) {
            it = ng_state.src_tgts.erase(it);
            continue;
        } else {
            
            switch (src->archetype_id) {
            case ARCHETYPE_PARTICLE_SYSTEM: {
                
                
                mtt::access_array<Particle_System_State>(src, "particle_state_list", [&](auto& array) {
#ifndef NDEBUG_LINES_HERE
                    sd::begin_path(world->renderer);
                    
                    sd::path_pixel_radius(world->renderer, 128);
#endif
                    
                    for (usize i = 0; i < ng_state.nodes.size();) {
                        mtt::Thing* tgt = world->Thing_try_get(ng_state.nodes[i]);
                        if (tgt != nullptr) {
                            i += 1;
                        } else {
                            node_graph_remove_graph_node_with_callback(&ng_state, ng_state.nodes[i], [&](const auto removed_target, Thing_Edge_List& out_edges, Thing_Edge_List& in_edges) {
                                usize node_count = ng_state.nodes.size();
                                
                                for (usize i_e = 0; i_e < in_edges.size(); i_e += 1) {
                                    for (usize o_e = 0; o_e < out_edges.size(); o_e += 1) {
                                        mtt::Thing_ID in_thing = edge_id(in_edges[i_e]);
                                        mtt::Thing_ID out_thing = edge_id(out_edges[o_e]);
                                        
                                        if (world->Thing_try_get(in_thing) != nullptr && world->Thing_try_get(out_thing) != nullptr) {
                                            node_graph_add_graph_edge(&ng_state, in_thing, out_thing);
                                        }
                                    }
                                }
                                usize which_idx = 0;
                                usize edge_out_count = out_edges.size();
                                if (edge_out_count != 0) {
                                    if (ng_state.flags == NODE_GRAPH_FOLLOW_FLAG_ANY) {
                                        for (usize j = 0; j < array.size(); j += 1) {
                                            if (array[j].target == removed_target) {
                                                which_idx = MTT_Random_range(0, node_count);
                                                array[j].target = ng_state.nodes[which_idx];
                                            }
                                        }
                                    } else if (ng_state.flags == NODE_GRAPH_FOLLOW_FLAG_FORWARD_NEIGHBORS) {
                                        
                                        for (usize j = 0; j < array.size(); j += 1) {
                                            if (array[j].target == removed_target) {
                                                which_idx = MTT_Random_range(0, edge_out_count);
                                                auto& edge = out_edges[which_idx];
                                                array[j].target = edge_id(edge);
                                                if (edge.flags == THING_EDGE_FLAG_TELEPORT_TO_DST) {
                                                    mtt::Thing* thing = world->Thing_try_get(array[j].target);
                                                    if (thing == nullptr) {
                                                        array[j].target = mtt::Thing_ID_INVALID;
                                                    } else {
                                                        mtt::Rep* t_thing_rep = mtt::rep(thing);
                                                        array[j].position = vec3(vec2(t_thing_rep->transform.translation), array[j].position.z);
                                                    }
                                                }
                                            }
                                        }
                                        
                                    }
                                    
                                } else {
                                    if (ng_state.flags == NODE_GRAPH_FOLLOW_FLAG_ANY) {
                                        for (usize j = 0; j < array.size(); j += 1) {
                                            if (array[j].target == removed_target) {
                                                which_idx = MTT_Random_range(0, node_count);
                                                array[j].target = ng_state.nodes[which_idx];
                                            }
                                        }
                                    } else if (ng_state.flags == NODE_GRAPH_FOLLOW_FLAG_FORWARD_NEIGHBORS) {
                                        
                                        for (usize j = 0; j < array.size(); j += 1) {
                                            if (array[j].target == removed_target) {
                                                array[j].target = mtt::Thing_ID_INVALID;
                                            }
                                        }
                                        
                                    }
                                }
                            });
                        }
                    }
                    
                    const usize count = array.size();
                    for (usize i = 0; i < count; ) {
                        auto* p = &array[i];
                        mtt::Thing* tgt = world->Thing_try_get(p->target);
                        if (tgt == nullptr) {
                            i += 1;
                            
                            continue;
                        }
                        
                        mtt::Rep* target_rep = mtt::rep(tgt);
                        vec3 tgt_pos = vec3(vec2(target_rep->transform.translation), p->position.z);
                        
                        
                        const float32 velocity_factor = p->max_velocity_magnitude;
                        
                        if (tgt_pos != p->position) {
                            p->velocity += m::normalize(tgt_pos - p->position) * velocity_factor;
                        }
                        
                        auto v_len = m::abs(m::length(p->position - tgt_pos));
                        if (v_len > velocity_factor) {
                            p->velocity = m::normalize(p->velocity) * velocity_factor;
                            //BUFFER_DISTANCE = velocity_factor;
                        } else {
                            //BUFFER_DISTANCE = v_len;
                        }
                        
                        float32 BUFFER_DISTANCE = m::length(p->velocity + vec3(1, 1, 1));//p->scale;
                        
                        if (v_len <= BUFFER_DISTANCE) {
                            usize which_idx = 0;
                            if (ng_state.flags == NODE_GRAPH_FOLLOW_FLAG_ANY) {
                                which_idx = MTT_Random_range(0, ng_state.nodes.size());
                                p->target = ng_state.nodes[which_idx];
                            } else if (ng_state.flags == NODE_GRAPH_FOLLOW_FLAG_FORWARD_NEIGHBORS) {
                                auto* edge_list = node_graph_get_graph_edge_list(&ng_state, p->target);
                                if (edge_list == nullptr) {
                                    array[i].target = mtt::Thing_ID_INVALID;
                                } else {
                                    which_idx = MTT_Random_range(0, edge_list->size());
                                    auto& edge = (*edge_list)[which_idx];
                                    array[i].target = edge_id(edge);
                                    if (edge.flags == THING_EDGE_FLAG_TELEPORT_TO_DST) {
                                        mtt::Thing* t_thing = world->Thing_try_get(array[i].target);
                                        if (t_thing == nullptr) {
                                            array[i].target = mtt::Thing_ID_INVALID;
                                        } else {
                                            mtt::Rep* t_thing_rep = mtt::rep(t_thing);
#ifndef NDEBUG_LINES_HERE
                                            {
                                                sd::path_vertex_v3(world->renderer, p->position);
                                                sd::path_vertex_v3(world->renderer, t_thing_rep->transform.position);
                                                
                                                sd::break_path(world->renderer);
                                            }
#endif
                                            
                                            array[i].position = vec3(vec2(t_thing_rep->transform.translation), array[i].position.z);
                                        }
                                    }
                                }
                            }
                            
                            
                            // try again with the new target
                            continue;
                            
                            
                            //                            target_rep = mtt::rep(tgt);
                            //                            tgt_pos = target_rep->transform.position;
                            
                            
                            //                            array[i].velocity = m::normalize(tgt_pos - p->position) * m::length(p->velocity) * (float32)MTT_TIMESTEP;
                        }
                        
#ifndef NDEBUG_LINES_HERE
                        if (i < 128) {
                            sd::path_vertex_v3(world->renderer, p->position);
                            sd::path_vertex_v3(world->renderer, tgt_pos);
                            
                            sd::break_path(world->renderer);
                        }
#endif
                        
                        i += 1;
                        
                    }
#ifndef NDEBUG_LINES_HERE
                    sd::end_path(world->renderer);
#endif
                });
                
                
                
                break;
            }
            default: {
                mtt::Thing* dst = world->Thing_try_get(it->second.tgt);
                if (dst == nullptr) {
                    it->second.tgt = mtt::Thing_ID_INVALID;
                    break;
                }

                break;
            }
            }
            
            ++it;
        }
    }
#ifndef NDEBUG_LINES_HERE
    sd::restore(world->renderer);
#endif
#undef NDEBUG_LINES_HERE
    
    
    return true;
}

MTT_DEFINE_INITIALIZER(wait)
{
    MTT_ADD_FIELD_T("state", Wait_Block);
}
MTT_DEFINE_LOGIC_PROCEDURE(wait)
{
    
    auto required_time_elapsed = [](mtt::World* world, Call_Descriptor* info) -> bool {
        if (info->last_checked_time == -1) {
            info->last_checked_time = world->time_seconds;
        } else {
            info->time_remaining -= (world->time_seconds - info->last_checked_time);
            info->last_checked_time = world->time_seconds;
        }
//#ifndef NDEBUG
//        MTT_print("World time: %f, Last checked time: %f, Time remaining: %f, Time interval: %f\n", world->time_seconds, info->last_checked_time, info->time_remaining, info->time_interval);
//#endif
        // adjust timer for overshoot
        {
            Call_Descriptor* call = info;
            bool is_done = call->time_remaining <= 0.0f;
            if (call->time_remaining < 0.0f) {
                call->time_remaining = call->time_interval + call->time_remaining;
                call->last_checked_time = -1.0f;
            } else if (call->time_remaining == 0.0f) {
                call->time_remaining = call->time_interval;
                call->last_checked_time = -1.0f;
            }
            
            return (is_done);
        }
        
    };
    
    auto& Q = Script_current_queue(s);
    usize initial_count_pending = Q.size();
    usize count_pending = initial_count_pending;
    //MTT_print("COUNT_PENDING: %llu\n", count_pending);
    for (isize i = Q.size() - 1; i >= 0; i -= 1) {
        auto& task = Q[i];
        if (task.is_waiting_on_subscope) {
            if (mtt::access<Group_Block_End>(task.subscope, "state")->has_exited) {
                std::swap(Q[i], Q[Q.size() - 1]);
                Q.pop_back();
                count_pending -= 1;
            }
            continue;
        }
        auto* task_script = task.list[task.call_info.params().current_index].script;
        switch (task_script->status) {
            case SCRIPT_STATUS_NOT_STARTED: {
                break;
            }
            case SCRIPT_STATUS_STARTED: {
                auto* call = &task.call_info;
                if (call->has_time_interval) {
                    if (!call->should_timeout_if_not_done && !task_script->source_script->is_infinite) {
                        break;
                    }
                    
                    if (call->count == mtt::FOREVER || task_script->source_script->is_infinite) {
                        if (!required_time_elapsed(world, call)) {
                            break;
                        }
                    }

                    task.call_info.params().current_index += 1;
                    if (task.call_info.params().current_index < task.list.size()) {
                        task_script->status = SCRIPT_STATUS_CANCELED;
                        auto* next_script = task.list[task.call_info.params().current_index].script;
                        if (next_script->source_script->preserve_lookup) {
                            next_script->is_own_lookup = task_script->is_own_lookup;
                            if (next_script->is_own_lookup) {
                                next_script->set_lookup_copy(&task_script->lookup());
                            } else {
                                next_script->set_shared_lookup(task_script->shared_lookup());
                            }
                        }
                        mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, next_script);
                    } else {
                        task.call_info.params().current_index = 0;
                        
                        
                        Call_Descriptor* call = &task.call_info;
                        

                        if (call->count == mtt::FOREVER) {
                            handle_call(call, s, [](Script_Instance* new_script, void* args) {
                                Script_Instance* task_script = (Script_Instance*)args;
                                if (new_script->source_script->preserve_lookup) {
                                    new_script->is_own_lookup = task_script->is_own_lookup;
                                    if (new_script->is_own_lookup) {
                                        new_script->set_lookup_copy(&task_script->lookup());
                                    } else {
                                        new_script->set_shared_lookup(task_script->shared_lookup());
                                    }
                                }
                            }, (void*)task_script);
                            
                            task_script->status = SCRIPT_STATUS_CANCELED;
                        } else {
                            call->count_remaining -= 1;
                            if (call->count_remaining != 0) {
                                handle_call(call, s, [](Script_Instance* new_script, void* args) {
                                    Script_Instance* task_script = (Script_Instance*)args;
                                    if (new_script->source_script->preserve_lookup) {
                                        new_script->is_own_lookup = task_script->is_own_lookup;
                                        if (new_script->is_own_lookup) {
                                            new_script->set_lookup_copy(&task_script->lookup());
                                        } else {
                                            new_script->set_shared_lookup(task_script->shared_lookup());
                                        }
                                    }
                                }, (void*)task_script);
                            } else { // count is 0
                                count_pending -= 1;
                            }
                            task_script->status = SCRIPT_STATUS_CANCELED;
                        }
                        
                        std::swap(Q[i], Q[Q.size() - 1]);
                        Q.pop_back();
                        
                    }
                }
                break;
            }
            case SCRIPT_STATUS_CANCELED: {
                auto* call = &task.call_info;
                call->count = 1;
                for (usize i = 0; i < task.list.size(); i += 1) {
                    task.list[i].script->status = SCRIPT_STATUS_CANCELED;
                }
                task.list.clear();
                std::swap(Q[i], Q[Q.size() - 1]);
                Q.pop_back();
                count_pending -= 1;
                break;
            }
            case SCRIPT_STATUS_DONE_SHOULD_TERMINATE: {
                //task_script->status = SCRIPT_STATUS_DONE;
                //auto* call = &task.call_info;
                {
                    task_script->status = SCRIPT_STATUS_DONE;
                    task.list.clear();
//                    Q.clear();
//                    Script_context_pop(s);
//                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP_IMMEDIATE_SUSPEND;
                }
//                task.list.clear();
//                std::swap(Q[i], Q[Q.size() - 1]);
//                Q.pop_back();
//                count_pending -= 1;
                break;
            }
            case SCRIPT_STATUS_DONE: {
                auto* call = &task.call_info;
                if (call->has_time_interval) {
                    if (!required_time_elapsed(world, call)) {
                        task_script->status = SCRIPT_STATUS_STARTED;
                        break;
                    } else {
                        task_script->status = SCRIPT_STATUS_DONE;
                    }
                }
                task.call_info.params().current_index += 1;
                if (task.call_info.params().current_index < task.list.size()) {
                    auto* next_script = task.list[task.call_info.params().current_index].script;
                    if (next_script->source_script->preserve_lookup) {
                        next_script->is_own_lookup = task_script->is_own_lookup;
                        if (next_script->is_own_lookup) {
                            next_script->set_lookup_copy(&task_script->lookup());
                        } else {
                            next_script->set_shared_lookup(task_script->shared_lookup());
                        }
                    }
                    mtt::push_script_task(&mtt::Runtime::ctx()->script_tasks, next_script);
                } else {
                    task.call_info.params().current_index = 0;

                    
                    if (call->count == mtt::FOREVER) {

                        handle_call(call, s, [](Script_Instance* new_script, void* args) {
                            Script_Instance* task_script = (Script_Instance*)args;
                            if (new_script->source_script->preserve_lookup) {
                                new_script->is_own_lookup = task_script->is_own_lookup;
                                if (new_script->is_own_lookup) {
                                    new_script->set_lookup_copy(&task_script->lookup());
                                } else {
                                    new_script->set_shared_lookup(task_script->shared_lookup());
                                }
                            }
                        }, (void*)task_script);
                    } else {
                        call->count_remaining -= 1;
                        if (call->count_remaining != 0) {
                            handle_call(call, s, [](Script_Instance* new_script, void* args) {
                                Script_Instance* task_script = (Script_Instance*)args;
                                if (new_script->source_script->preserve_lookup) {
                                    new_script->is_own_lookup = task_script->is_own_lookup;
                                    if (new_script->is_own_lookup) {
                                        new_script->set_lookup_copy(&task_script->lookup());
                                    } else {
                                        new_script->set_shared_lookup(task_script->shared_lookup());
                                    }
                                }
                            }, (void*)task_script);
                        } else { // count is 0
                            count_pending -= 1;
                        }
                    }
                    
                    std::swap(Q[i], Q[Q.size() - 1]);
                    Q.pop_back();
                    
                }
                break;
            }
            case SCRIPT_STATUS_TERMINATED: {
                {
                    //task_script->status = SCRIPT_STATUS_DONE;
                    task.list.clear();
                    Q.clear();
                    Script_context_pop(s);
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                }
            }
            case SCRIPT_STATUS_STOPPED: {
                
                task_script->status = SCRIPT_STATUS_DONE;

                auto* call = &task.call_info;
                mtt::Thing* ins = mtt::Thing_try_get(world, call->call_instruction);
                ASSERT_MSG(ins != nullptr, "call thing should exist\n");
                Call_Descriptor* call_src = mtt::access<Call_Descriptor>(ins, "state");
                //call_src->TEST_FLAG = 1;
                
                // TODO: no, should not disable the entire call -- remove the agent parameter instead;
                
                //call_src->is_active = false;
#if 1

                
                //mtt::clear_args_to_stop(task_script);
#endif
                Call_Descriptor_stop_for_source(call, task_script->agent, mtt_time_nanoseconds());
                
                task.list.clear();

                ASSERT_MSG(count_pending > 0, "pending count incorrect\n");
                count_pending -= 1;
                std::swap(Q[i], Q[Q.size() - 1]);
                Q.pop_back();
                
                break;
            }
            case SCRIPT_STATUS_ERROR: {
                
                if (count_pending > 1) {
                    
                    task_script->status = SCRIPT_STATUS_DONE;
                    
                    auto* call = &task.call_info;
                    mtt::Thing* ins = mtt::Thing_try_get(world, call->call_instruction);
                    ASSERT_MSG(ins != nullptr, "call thing should exist\n");
                    Call_Descriptor* call_src = mtt::access<Call_Descriptor>(ins, "state");
                    
                    call_src->is_active = false;
                    
                    Call_Descriptor_stop_for_source(call, task_script->agent, mtt_time_nanoseconds());
                    
                    task.list.clear();
                    
                    ASSERT_MSG(count_pending > 0, "pending count incorrect\n");
                    count_pending -= 1;
                    std::swap(Q[i], Q[Q.size() - 1]);
                    Q.pop_back();
                    break;
                } else {
                    task_script->status = SCRIPT_STATUS_DONE;
                    task.list.clear();
                    Q.clear();
                    Script_context_pop(s);
                    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_DONE_SHOULD_TERMINATE;
                }
            }
        }
        
    }
    return (count_pending != 0) ? LOGIC_PROCEDURE_RETURN_STATUS_TYPE_IMMEDIATE_SUSPEND : LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}




MTT_DEFINE_INITIALIZER(look_at)
{
    MTT_ADD_FIELD("initial_v",      MTT_VECTOR3, 0);
    MTT_ADD_FIELD("source",         MTT_THING,   0);
    MTT_ADD_FIELD("target",         MTT_THING,   0);
    
    Thing_add_in_port({world, fields, ports, logic, "source", MTT_THING, 0,
        {(ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "target", MTT_THING, 0,
        {(ALIGN_DOWN * 0.5f)}, nullptr
    });
    Thing_add_in_port({world, fields, ports, logic, "active", MTT_BOOLEAN, 0,
        {(ALIGN_DOWN * 0.5f)}, nullptr
    });
}
MTT_DEFINE_LOGIC_PROCEDURE(look_at)
{
    if (!thing_group_is_active(thing)) {
        return true;
    }
    
    return true;
}


MTT_DEFINE_INITIALIZER(save_physics)
{
}

MTT_DEFINE_LOGIC_PROCEDURE(save_physics)
{
    mtt::Rep* rep = mtt::rep(thing);
    rep->saved_velocity     = rep->velocity;
    rep->saved_acceleration = rep->saved_acceleration;
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}

MTT_DEFINE_INITIALIZER(restore_physics)
{
}

MTT_DEFINE_LOGIC_PROCEDURE(restore_physics)
{
    mtt::Rep* rep = mtt::rep(thing);
    rep->velocity     = rep->saved_velocity;
    rep->acceleration = rep->saved_acceleration;
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}

MTT_DEFINE_INITIALIZER(destroy_thing_and_connected)
{
}

MTT_DEFINE_LOGIC_PROCEDURE(destroy_thing_and_connected)
{
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}

MTT_DEFINE_INITIALIZER(print)
{
}

MTT_DEFINE_LOGIC_PROCEDURE(print)
{
    cstring str = Thing_cstring_from_label(thing);
    MTT_print("[%s]\n", str);
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}


MTT_DEFINE_INITIALIZER(for_each_begin)
{
    MTT_ADD_FIELD_T("state", For_Each_Loop_Begin);
}
MTT_DEFINE_LOGIC_PROCEDURE(for_each_begin)
{
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}

MTT_DEFINE_INITIALIZER(for_each_end)
{
    MTT_ADD_FIELD_T("state", For_Each_Loop_End);
}
MTT_DEFINE_LOGIC_PROCEDURE(for_each_end)
{
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}


MTT_DEFINE_INITIALIZER(for_each_persistent_begin)
{
    MTT_ADD_FIELD_T("state", For_Each_Loop_Begin);
}
MTT_DEFINE_LOGIC_PROCEDURE(for_each_persistent_begin)
{
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}

MTT_DEFINE_INITIALIZER(for_each_persistent_end)
{
    MTT_ADD_FIELD_T("state", For_Each_Loop_End);
}
MTT_DEFINE_LOGIC_PROCEDURE(for_each_persistent_end)
{
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}

MTT_DEFINE_INITIALIZER(break)
{
    Thing_add_in_port({world, fields, ports, logic, "is_active", MTT_BOOLEAN, 0,
        {(ALIGN_DOWN * 0.5f)}, nullptr
    });
    MTT_ADD_FIELD_T("state", Break_Statement);
}
MTT_DEFINE_LOGIC_PROCEDURE(break)
{
    auto IN_on = get_in_port(thing, input, "is_active");
    if (IN_on.status == mtt::PORT_STATUS_OK) {
        if (IN_on.value.out.type == MTT_BOOLEAN && IN_on.value.out.Boolean != 0) {
            Script_context_pop(s);
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP_IMMEDIATE_SUSPEND;
        } else {
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED_SUSPEND;
        }
    } else {
        Script_context_pop(s);
        return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP_IMMEDIATE_SUSPEND;
        
    }
}


MTT_DEFINE_INITIALIZER(return)
{
    Thing_add_in_port({world, fields, ports, logic, "is_active", MTT_BOOLEAN, 0,
        {(ALIGN_DOWN * 0.5f)}, nullptr
    });
    MTT_ADD_FIELD_T("state", Return_Statement);
}
MTT_DEFINE_LOGIC_PROCEDURE(return)
{
    auto IN_on = get_in_port(thing, input, "is_active");
    if (IN_on.status == mtt::PORT_STATUS_OK) {
        if (IN_on.value.out.type == MTT_BOOLEAN && IN_on.value.out.Boolean != 0) {
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
        } else {
            return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED_SUSPEND;
        }
    }
    // TODO: return something
    //s->return_value.push_back();
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_CONTEXT_DONE;
}

MTT_DEFINE_INITIALIZER(continue)
{
    MTT_ADD_FIELD_T("state", Continue_Statement);
    
}
MTT_DEFINE_LOGIC_PROCEDURE(continue)
{
    Continue_Statement* continue_statement = mtt::access<Continue_Statement>(thing, "state");
    return jump_to_beginning(s, s_ctx, thing->eval_index, continue_statement->should_reset_on_jump);
}

MTT_DEFINE_INITIALIZER(goto)
{
    MTT_ADD_FIELD_T("state", Goto_Statement);
}
MTT_DEFINE_LOGIC_PROCEDURE(goto)
{
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_JUMP;
}



MTT_DEFINE_INITIALIZER(set_property)
{
    //MTT_ADD_FIELD_T("state", Property_Param);
    MTT_ADD_FIELD_T("state", Property_Access);
    Thing_add_in_port({world, fields, ports, logic, "value", MTT_ANY, 0, {ALIGN_LEFT}, nullptr});
}
MTT_DEFINE_LOGIC_PROCEDURE(set_property)
{
    Property_Access* a = mtt::access<Property_Access>(thing, "state");
    a->update_in_ports(thing, s);
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}
MTT_DEFINE_INITIALIZER(get_property)
{
    MTT_ADD_FIELD_T("state", Property_Access);
    Thing_add_in_port({world, fields, ports, logic, "idx", MTT_INT32, 0, {ALIGN_LEFT}, nullptr});

    Thing_add_in_port({world, fields, ports, logic, "increment_index", MTT_BOOLEAN, 0, {ALIGN_LEFT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "is_done", MTT_BOOLEAN, 0, {ALIGN_LEFT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "value_properties", MTT_LIST, MTT_SCRIPT_PROPERTY, {ALIGN_RIGHT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "value_single", MTT_ANY, MTT_NONE, {ALIGN_RIGHT}, nullptr});
    Thing_add_out_port({world, fields, ports, logic, "selector", MTT_TEXT, MTT_NONE, {ALIGN_RIGHT}, nullptr});
    
    Thing_add_out_port({world, fields, ports, logic, "value_property_single", MTT_SCRIPT_PROPERTY, MTT_NONE, {ALIGN_RIGHT}, nullptr});
    
}
MTT_DEFINE_LOGIC_PROCEDURE(get_property)
{
    //if (auto out = get_out_port(thing, 0); out.status == mtt::PORT_STATUS_OK) {
        Property_Access* a = mtt::access<Property_Access>(thing, "state");
        a->update_out_ports(thing, input, s);
    //}
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}

MTT_DEFINE_INITIALIZER(set_call_parameter)
{
    MTT_ADD_FIELD_T("state", Call_Param);
    
    Thing_add_out_port({world, fields, ports, logic, "value", MTT_POINTER, 0, {ALIGN_RIGHT}, nullptr});
}
MTT_DEFINE_LOGIC_PROCEDURE(set_call_parameter)
{
    auto* val = mtt::access<Call_Param>(thing, "state");
    
    if (auto out = get_out_port(thing, 0); out.status == mtt::PORT_STATUS_OK) {
        out.value.out.set_Reference_Type((void*)val);
    }
    
    
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}



//struct Debug__Delay {
//    float32 time_interval  = 0.0;
//    float32 remaining_time = 0.0;
//    usize   count = 1;
//    bool    has_time_interval = false;
//};

MTT_DEFINE_INITIALIZER(debug__)
{
    
    //MTT_ADD_FIELD_T("state", Debug__Delay);
    MTT_ADD_FIELD_PRIMITIVE_WITH_DEFAULT("state", MTT_INT64, uint64, 0);
}

MTT_DEFINE_LOGIC_PROCEDURE(debug__)
{
    uint64* state = mtt::access<uint64>(thing, "state");
    *state += 1;
    MTT_print("%s VAL: %llu\n", __PRETTY_FUNCTION__, *state);
    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
}

MTT_DEFINE_INITIALIZER(return_status_control)
{
    MTT_ADD_FIELD_T("status", Logic_Procedure_Return_Status);
    
    archetype->on_thing_make = [](mtt::Thing* thing) {
        ASSERT_MSG(mtt::access<Logic_Procedure_Return_Status>(thing, "status")->type == LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED, "must be PROCEED");
    };
}
MTT_DEFINE_LOGIC_PROCEDURE(return_status_control)
{
    return *mtt::access<LOGIC_PROCEDURE_RETURN_STATUS_TYPE>(thing, "status");
}


//MTT_DEFINE_INITIALIZER(set_to_velocity_movement)
//{
//
//}
//MTT_DEFINE_LOGIC_PROCEDURE(set_to_velocity_movement)
//{
//    mtt::set_movement_mode(thing, mtt::OPTION_FLAGS_MOVEMENT_DEFAULT);
//
//    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
//}
//
//MTT_DEFINE_INITIALIZER(set_to_positional_movement)
//{
//
//}
//MTT_DEFINE_LOGIC_PROCEDURE(set_to_positional_movement)
//{
//    mtt::set_movement_mode(thing, mtt::OPTION_FLAGS_MOVEMENT_POSITIONAL);
//
//    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
//}
//
//MTT_DEFINE_INITIALIZER(push_movement_type)
//{
//
//}
//MTT_DEFINE_LOGIC_PROCEDURE(push_movement_type)
//{
//    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
//}
//
//
//MTT_DEFINE_INITIALIZER(pop_movement_type)
//{
//
//}
//MTT_DEFINE_LOGIC_PROCEDURE(pop_movement_type)
//{
//    return LOGIC_PROCEDURE_RETURN_STATUS_TYPE_PROCEED;
//}





// MARK: - end_archetypes

//static bool is_destroying = false;
//static void on_destroy(flecs::entity e, mtt::Thing_Info& info)
//{
//    if (is_destroying) {
//        return;
//    }
//
//    mtt::Thing_destroy(info.world, info.thing_id);
//}
void init_meta(void)
{
    for (usize i = 0; i < MTT_TYPE_COUNT; i += 1) {
        meta[i].def_val = raw_value_address(&meta[i].default_value);
    }
}

void World_init(World* world, const World_Descriptor& desc)
{
    MTT_print("%s", "You're making a thing world!\n");
    
    mtt::set_ctx(world);
    
    init_meta();
    
    world->step_count = 0;
    
    world->allocator        = *desc.allocator;
    
    mem::Buckets_Allocation_init(&world->buckets, world->allocator, mtt_core_ctx()->page_size);
    mem::Buckets_Allocation_init(&world->message_allocation, world->allocator, mtt_core_ctx()->page_size);
    mem::Buckets_Allocation_init(&world->arg_allocation, world->allocator, mtt_core_ctx()->page_size);
    
    //    struct Script_Evaluation_Context {
    //        char placeholder;
    //        static mem::Allocator allocator;
    //
    //        static Script_Evaluation_Context* make(void);
    //    };
    
    for (usize i = 0; i < ALLOCATOR_TEMPORARY_COUNT; i += 1) {
        mem::Arena_init(&world->memory_temporary_[i], world->allocator, sizeof(char) * 2048, 16);
        world->allocator_temporary_[i] = mem::Arena_Allocator(&world->memory_temporary_[i]);
    }
    
    world->per_frame_reset_counter_ = 0;
    
    world->renderer = desc.renderer;
    
    //Pool_Allocation_init(&world->instancing.drawable_pool, world->allocator, 1024, sizeof(sd::Drawable_Info), 16);
    
    Pool_Allocation_init(&world->drawable_pool, world->allocator, 4, sizeof(sd::Drawable_Info), 16);
    Pool_Allocation_init(&Script_Instance::pool, world->allocator, 1024, sizeof(Script_Instance), 16);
    
    
    world->collision_system.user_data = nullptr;
    world->collision_system.world = world;
    
    mtt::init(&world->things.things, world->allocator, 1, 1);
    world->things.count = 0;
    
    
    world->things.next_thing_id = 1;
    world->things.next_thing_group_id = 2;
    
    {
        strpool_config_t conf = strpool_default_config;
        strpool_init(&world->string_pool, &conf);
        
        MTT_set_string_pool(&world->string_pool, 0);
        
    }
    
    
//    auto it = world->things.groups.emplace(1, Thing_Group());
//    it.first->second.group_id = 1;
//    it.first->second.parent_group_id = 0;
    
    world->archetypes.next_thing_archetype_id = 0;
    
    //Pool_Allocation_init(&control_curve_pool, world->allocator, 64, sizeof(Control_Curve_Info), 16);
    
    
    //mtt::Array_init(&world->things.proxy_scenes, world->allocator, 0, 2);
    
    
    World_Graph_init(root_graph(world), &world->allocator);
    
    meta[MTT_TEXT].default_value.String   = mtt::string("");
    meta[MTT_TAG].default_value.String    = mtt::string("");
    meta[MTT_STRING].default_value.String = mtt::string("");
    
    //    mtt::init(&meta[MTT_THING_LIST].default_value.Thing_List, world->allocator);
    //    mtt::init(&meta[MTT_TAG_LIST].default_value.String_List, world->allocator);
    //    mtt::init(&meta[MTT_POLYCURVE].default_value.Polycurve, world->allocator);
    
    usize max_arch_byte_count = 0;
    
    Field_List_Builder field_list_builder;
    
    for (usize i = 0; i < THING_TYPE_COUNT; i += 1) {
        
        Thing_Archetype* arch;
        if (Thing_Archetype_make(world, &arch, ARCHETYPE_NAMES[i])) {
            mtt::set_is_active(arch);
            mtt::set_is_active_group(arch);
            arch->logic.option       = 0;
            arch->logic.option_flags = 0;
            arch->message_handler = message_handler_noop;
            
            field_list_builder.init(arch, world, &arch->field_descriptor, &arch->port_descriptor, &arch->logic);
            
            ARCHETYPE_INITIALIZERS[i](arch, world, &arch->field_descriptor, &arch->port_descriptor, &arch->logic, field_list_builder);
            
            arch->logic.proc = ARCHETYPE_LOGIC_PROCEDURES[(ARCHETYPE)i];
            
            field_list_builder.build();
            
            
            //            Thing_add_in_port({world, &arch->field_descriptor, &arch->port_descriptor, &arch->logic, "execution_order", MTT_NONE, 0, {}, nullptr});
            //            Thing_add_out_port({world, &arch->field_descriptor, &arch->port_descriptor, &arch->logic, "execution_order", MTT_NONE, 0, {}, nullptr});
            
            
            
            //
            //
            max_arch_byte_count = (arch->field_descriptor.data.byte_count > max_arch_byte_count) ? arch->field_descriptor.data.byte_count : max_arch_byte_count;
            
#define PRINT_TYPE_MAKE (0)
#if PRINT_TYPE_MAKE
            MTT_print("Created type: name[%s] id[%llu]\n", ARCHETYPE_NAMES[i], arch->id);
            MTT_print("MAX ARCHETYPE FIELD LIST BYTE SIZE: %llu\n", max_arch_byte_count);
#endif
        } else {
            MTT_error("%s", "ERROR: failed to initialize archetype fields and ports\n");
        }
        
        
    }
    
    Message_Passer_init(&world->message_passer, &world->allocator);
    world->message_passer.world = world;
    
    
    world->on_thing_make = nullptr;
    
    world->ecs_world.set_context(world);
    
    flecs::component<Thing_Info>(world->ecs_world, "Thing_Info");
    flecs::component<Sensor>(world->ecs_world, "Sensor");
    flecs::component<mtt::Any>(world->ecs_world, "Property");
    
    world->sensor_query = world->ecs_world.query<mtt::Thing_Info, mtt::Sensor>();
    
    
    //Pool_Allocation_init(&curve_allocation, world->allocator, 64, sizeof(Curve_Procedure), 16);
    
    world->archetype_drawables.init(*buckets_allocator(world));
    
    world->collisions_to_remove_count = 0;
    
    // box2d-support eventually
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity.y = -9.81;
//    worldDef.workerCount = 2;//settings.workerCount;
//    worldDef.enqueueTask = [](b2TaskCallback* task, int32_t itemCount, int32_t minRange, void* taskContext, void* userContext) -> void* {
//        task(0, itemCount, 0, taskContext);
//        return NULL;
//    };
//    worldDef.finishTask = [](void* userTask, void* userContext) -> void {};
    worldDef.userTaskContext = world;
    worldDef.enableSleep = true;
    worldDef.enableContinuous = true;
    world->physics_world = b2CreateWorld(&worldDef);
    
    world->log = os_log_create("make_the_thing.eval_world_post", "eval_world_post");

    auto child_of_transitive = flecs::entity(world->ecs_world, "ChildOfTransitive");
    child_of_transitive.add(flecs::Transitive);
}

void World_deinit(mtt::World* world)
{
    b2DestroyWorld(world->physics_world);
    strpool_term(&world->string_pool);
}


const char* message_string(mtt::World* world, cstring str, usize length)
{
    char* ptr = (char*)world->allocator_temporary().do_allocate(length);
    if (ptr == NULL) {
        return NULL;
    }
    
    memcpy(ptr, str, length);
    
    return ptr;
}

const char* message_string(mtt::World* world, mtt::String& str)
{
    return message_string(world, str.c_str(), str.size());
}


Thing* Thing_from_ID(World* world, Thing_ID id)
{
    return world->Thing_try_get(id);
}


Thing_Archetype* Thing_Archetype_from_ID(World* world, Thing_Archetype_ID id)
{
    return &world->archetypes.instances[id];
}


void Thing_destroy(World* world, Thing_ID id)
{
    mtt::Thing* thing = world->Thing_try_get(id);
    if (thing == nullptr) {
        return;
    }
    Thing_destroy(world, thing);
}

void Thing_destroy(mtt::World* world, mtt::Thing* thing)
{
    Thing_destroy(thing);
}
void Thing_destroy(mtt::Thing* thing)
{
    mtt::World* world = mtt::world(thing);
    
    mtt::Thing_ID id = mtt::thing_id(thing);
    
    terminate_rule_if_dependent_on_thing(id);
    
    if (world->no_deletion_zone_on) {
        Destroy_Command cmd;
        cmd.affects_connected = false;
        cmd.do_fade_animation = false;
        cmd.thing_id = id;
        cmd.time_delay = 0;
        cmd.time_remaining = 0;
        world->to_destroy.push_back(cmd);
        thing->should_defer_destruction = false;
        return;
    } else if (thing->should_defer_destruction) {
        world->to_destroy_end.push_back(id);
        return;
    }
    
    
    
    
    thing->on_destroy(thing);
    
    if (thing->graph != nullptr) {
        thing->graph->is_modified = true;
    }
    
    if (Thing_is_proxy(thing)) {
        
        Thing_Proxy_Scene_remove_proxy(world, thing);
        
        Thing_Proxy_Storage* proxies = nullptr;
        mtt::map_try_get(&world->things.thing_to_proxy_map, thing->mapped_thing_id, &proxies);
        ASSERT_MSG(proxies != nullptr, "if is a proxy, the list should already exist");
        auto find_it = proxies->find(thing->id);
        ASSERT_MSG(find_it != proxies->end(), "if is a proxy, should be found");
        proxies->erase(find_it);
        if (proxies->empty()) {
            world->things.thing_to_proxy_map.erase(thing->mapped_thing_id);
        }
        
        

    } else {
        Thing_Proxy_Storage* proxies = nullptr;
        mtt::map_try_get(&world->things.thing_to_proxy_map, thing->id, &proxies);
        if (proxies != nullptr) {
            
            
            for (usize s_i = 0; s_i < world->things.proxy_scenes.size(); s_i += 1) {
                auto* proxy_scene_map = &world->things.proxy_scenes[s_i].thing_to_proxy_map;
                auto find_it_in_proxy_scene = proxy_scene_map->find(thing->id);
                if (find_it_in_proxy_scene != proxy_scene_map->end()) {
                    proxy_scene_map->erase(find_it_in_proxy_scene);
                }
            }
            
            
            auto& proxies_ref = *proxies;
            static std::vector<mtt::Thing_ID> ids = {};
            ids.clear();
            for (auto proxy_id : proxies_ref) {
                ids.push_back(proxy_id);
            }
            for (usize i = 0; i < ids.size(); i += 1) {
                mtt::Thing_destroy(world, ids[i]);
            }
            proxies_ref.clear();
            world->things.thing_to_proxy_map.erase(thing->id);
            
        }
        
        thing_remove_referrents(thing);
    }
    
    Script_Command_Sequence::for_thing.erase(id);
    
    
    deinit_properties(thing);
    
    {
        auto find_own_selectors = world->things.own_selector_to_proc.find(id);
        if (find_own_selectors != world->things.own_selector_to_proc.end()) {
            world->things.own_selector_to_proc.erase(find_own_selectors);
        }
    }
    
    
    for (auto f_it = thing->field_descriptor.fields.begin(); f_it != thing->field_descriptor.fields.end(); ++f_it) {
        auto* watchers = (*f_it).watchers;
        if (watchers == nullptr) {
            continue;
        }
        std::vector<Watcher>* watchers_ptr = (std::vector<Watcher>*)watchers;
        for (auto w_it = watchers_ptr->begin(); w_it != watchers_ptr->end(); ++w_it) {
            (*w_it).on_watched_destroy_proc(thing);
        }
        watchers_ptr->clear();
        
        mem::deallocate<std::vector<Watcher>>(&world->allocator, watchers_ptr);
    }
    
    arrow_edge_remove_all_with_source(thing);
    
    mtt::remove_thing_from_hierarchy(world, thing);
//t
    //Thing_print_all(world);
    
    
    constexpr const bool ENABLE_SELF_CLEANUP = false;
    {
        {
            mtt::Map<uint64, Collision_Record>& curr_collisions = world->collision_system.curr_collisions;
            
            auto& ecs_entity = thing->ecs_entity;
            {
                for (auto it = curr_collisions.begin(); it != curr_collisions.end();) {
                    auto& record = it->second;
                    flecs::entity other_entity;
                    
                    
                    
                    if (ecs_entity == record.a) {
                        other_entity = record.b;
                    } else if (ecs_entity == record.b) {
                        other_entity = record.a;
                    } else {
                        ++it;
                        continue;
                    }
                    
                    if (!(other_entity.is_valid() && other_entity.is_alive())) {
                        ++it;
                        continue;
                    }
                    
                    if (other_entity.has(world->collide_begin_tag, ecs_entity)) {
                        other_entity.remove(world->collide_begin_tag, ecs_entity);
                    }
                    if (other_entity.has(world->collide_tag, ecs_entity)) {
                        other_entity.remove(world->collide_tag, ecs_entity);
                    }
                    if (other_entity.has(world->collide_end_tag, ecs_entity)) {
                        other_entity.remove(world->collide_end_tag, ecs_entity);
                    }
                    
                    it = curr_collisions.erase(it);
                }
            }
            
            mtt::Representation* rep = mtt::rep(thing);
            for (usize i = 0; i < rep->colliders.size(); i += 1) {
                mtt::Collider* collider = rep->colliders[i];
                
                mtt::Collider_remove(collider->system, collider->layer, collider);
                
                mtt::Collider_destroy(collider->system, collider);
            }
            rep->colliders.clear();
            rep->colors.clear();
            rep->points.clear();
            
            {
                auto& drawable_info_list = rep->render_data.drawable_info_list;
                const usize drawable_info_list_count = drawable_info_list.size();
                if (!Thing_is_proxy(thing)) {
                    for (usize i = 0; i < drawable_info_list_count; i += 1) {
                        auto* dr_info = drawable_info_list[i];
                        dr_info->is_enabled = false;
                        sd::Drawable_Info_release(world->renderer, dr_info);
                        //                if ((rep->render_data.drawable_info_list[i]->flags & DRAWABLE_INFO_SHARED_FLAG_BITMASK) != 0) {
                        //                    world->archetype_drawables.arch_to_drawable[thing->archetype_id].destroy_instance(rep->render_data.drawable_info_list[i]);
                        //                }
                        
                    }
                } else {
                    for (usize i = 0; i < drawable_info_list_count; i += 1) {
                        auto* dr_info = drawable_info_list[i];
                        
                        dr_info->is_enabled = false;

                        mem::deallocate<sd::Drawable_Info>(&world->drawable_pool.allocator, dr_info);
                    }
                }
                rep->render_data.drawable_info_list.clear();
            }
            
            //            if (!rep->render_data.is_shared && rep->render_data.layer != nullptr) {
            //                sd::Renderer* renderer = world->renderer;
            //                sd::deallocate_staging_layer_with_allocator(renderer, &rep->render_data.layer);
            //            }
        }
    }
    //    {
    //        auto result = world->things.logic.find(id);
    //        if (result != world->things.logic.end()) {
    //            world->things.logic.erase(result);
    //        }
    //    }
    {
        
        //Thing_Group_ID group_id = thing->group_id;
        
 
        destroy_fields(*field_allocator(world), thing->field_descriptor, thing->field_descriptor.data);
        

        
        world->archetypes.instances[thing->archetype_id].instance_ids.erase(id);
        
        if (thing->graph != nullptr) {
            remove_all_connections(world, id);
            auto g_found = thing->graph->incoming.find(id);
            thing->graph->incoming.erase(g_found);
            auto g_out_found = thing->graph->outgoing.find(id);
            thing->graph->outgoing.erase(g_out_found);
            
            delete_execution_connections(thing);
        }
        
        dt::Thing_remove_all_own_attributes(thing);
        {
            
            
            {
                // relations from this Thing to other Things
                auto& map = dt::word_dict()->source_to_target_relation;
                
                auto& words = map[id];
                
                if constexpr ((ENABLE_SELF_CLEANUP)) {
                    
                    for (auto it = words.begin(); it != words.end(); ++it) {
                        auto& val = *it;
                        for (auto mp_it = val.second.begin(); mp_it != val.second.end(); ++mp_it) {
                            auto& entry = *mp_it;
                            if (entry.target == mtt::Thing_ID_INVALID) {
                                thing->ecs_entity.remove(entry.dict_entry->typename_desc);
                            } else {
                                mtt::Thing* target = world->Thing_try_get(entry.target);
                                if (target != nullptr) {
                                    thing->ecs_entity.remove(entry.dict_entry->typename_desc, target->ecs_entity);
                                }
                            }
                        }
                    }
                }
                words.clear();
                
                map.erase(id);
            }
            
            
            
            {
                // relations from other things to this Thing
                auto& map = dt::word_dict()->target_to_source_relation;
                
                auto& words = map[id];
                
                if constexpr ((ENABLE_SELF_CLEANUP)) {
                    for (auto it = words.begin(); it != words.end(); ++it) {
                        auto& val = *it;
                        for (auto mp_it = val.second.begin(); mp_it != val.second.end(); ++mp_it) {
                            auto& entry = *mp_it;
                            ASSERT_MSG(entry.target == id, "The target should be this\n");
                            {
                                mtt::Thing* source = world->Thing_try_get(entry.source);
                                if (source != nullptr) {
                                    source->ecs_entity.remove(entry.dict_entry->typename_desc, thing->ecs_entity);
                                }
                            }
                        }
                    }
                }
                words.clear();
                
                map.erase(id);
            }
        }
        
        //        auto find_thing = world->things.instances.find(id);
        //        if (find_thing != world->things.instances.end()) {
        //
        
        if (thing->ecs_entity.is_alive() && thing->ecs_entity.is_valid()) {
//            cstring str = Thing_cstring_from_label(thing);
//            MTT_print("%s", str);
            thing->ecs_entity.destruct();
        } else {
            ASSERT_MSG(false, "Should be valid!\n");
        }
        //(world->ecs_world.c_ptr(), thing->ecs_entity);
        
//        {
//            Thing_Group* group = nullptr;
//            if (mtt::map_try_get(&world->things.groups, group_id, &group)) {
//                assert(group->group_id != 0);
//
//                group->things.erase(id);
//
//                // FIXME: decide if should automatically remove an empty group. (Probably not)
//            }
//        }
        
        
        {
            auto* DT = dt::DrawTalk::ctx();
            auto* dict = &DT->lang_ctx.dictionary;
            auto it_dict = dict->thing_to_word.find(id);
            if (it_dict != dict->thing_to_word.end()) {
                auto& entries = it_dict->second;
                
                for (auto w_it = entries.begin();
                     w_it != entries.end();
                     ++w_it) {
                    auto& entry = *w_it;
                    auto found_it_id = entry->things.find(id);
                    if (found_it_id  != entry->things.end()) {
                        entry->things.erase(found_it_id);
                    }
                }
                dict->thing_to_word.erase(it_dict);
            }
            
            
            
            auto find_selected = DT->scn_ctx.selected_things.find(id);
            if (find_selected != DT->scn_ctx.selected_things.end()) {
                DT->scn_ctx.selected_things.erase(find_selected);
            }
            
            if (DT->scn_ctx.thing_selected_drawable == id) {
                DT->scn_ctx.thing_selected_drawable = mtt::Thing_ID_INVALID;
            }
            if (DT->scn_ctx.thing_most_recently_selected_with_touch == id) {
                //                float64 max_timestamp = 0.0f;
                mtt::Thing_ID most_recent = mtt::Thing_ID_INVALID;
                //                for (auto sel = DT->selection_map.begin(); sel !=  DT->selection_map.end(); ++sel) {
                //                    if (sel->second.thing == thing->id) {
                //                        sel = DT->selection_map.erase(sel);
                //                        continue;
                //                    }
                //
                //                    mtt::Thing* candidate = world->Thing_try_get(sel->second.thing);
                //                    if (candidate == nullptr || !mtt::is_user_movable(candidate)) {
                //                        continue;
                //                    }
                //
                //                    if (sel->second.time >= max_timestamp) {
                //                        max_timestamp = sel->second.time;
                //                        most_recent   = sel->second.thing;
                //                    }
                //                }
                
                mtt::set_thing_most_recently_selected_with_touch(world, most_recent, __LINE__, __FILE__);
            }
            
            if (DT->scn_ctx.thing_selected_with_pen == id) {
                DT->scn_ctx.thing_selected_with_pen = mtt::Thing_ID_INVALID;
            }
            
        }
        
        MTT_string_ref_release(&thing->label);
        thing->id = mtt::Thing_ID_INVALID;
        thing->mapped_thing_id = mtt::Thing_ID_INVALID;
        world->things.instances.erase(id);
    }
    world->things.count -= 1;

//#ifndef NDEBUG
//    MTT_print("Remove Done, Current Thing count %zu\n", world->things.instances.size());
//#endif
}

void render(World* world, sd::Renderer* renderer)
{
    
}

//Thing_Group* Thing_Group_from_ID(Make_The_Thing_World* world, Thing_Group_ID id)
//{
//    return &world->things.groups[id];
//}

void Thing_print(World* world, Thing* thing)
{
    MTT_print("Thing: label=[%s], id=[%llu], type={id=[%llu], name=[%s]}\n", MTT_string_ref_to_cstring(thing->label), thing->id, thing->archetype_id, world->archetypes.instances[thing->archetype_id].tag.c_str());
}
void Thing_print(Thing* thing)
{
    Thing_print(mtt::world(thing), thing);
}

void Thing_print(World* world, Thing_ID id)
{
    mtt::Thing* thing = Thing_try_get(world, id);
    if (thing == nullptr) {
        return;
    }
    
    Thing_print(world, thing);
}

void Thing_print_verbose(World* world, Thing* thing)
{
    Thing_print(world, thing);
    MTT_Field_List_print(&thing->active_fields, &thing->field_descriptor);
    MTT_Port_Descriptor_print(&thing->ports);
}
void Thing_print_verbose(Thing* thing)
{
    Thing_print_verbose(thing->world(), thing);
}
void Thing_print_verbose(World* world, Thing_ID id)
{
    Thing* thing = nullptr;
    if (world->Thing_try_get(id, &thing)) {
        Thing_print_verbose(world, thing);
    } else {
        assert(false);
    }
}


void Thing_print_all(World* world)
{
    MTT_print("PRINT_ALL{count=%zu\n", world->things.instances.size());
    for (auto it = world->things.instances.begin(); it != world->things.instances.end(); ++it) {
        Thing* thing = &it->second;
        Thing_Archetype* arch = &world->archetypes.instances[thing->archetype_id];
        MTT_print("Thing: {\n"
                  "id=[%llu] arch_id=[%llu] arch_tag=[%s]\n",
                  thing->id, thing->archetype_id, arch->tag.c_str());
        
        MTT_Port_Descriptor_print(&thing->ports);
        MTT_Field_List_print(&thing->active_fields, &thing->field_descriptor);
        MTT_print("%s", "\n}\n");
    }
    MTT_print("%s", "}\n");
}


mtt::Thing_Storage* Thing_collection(World* world)
{
    return &world->things.instances;
}

// MARK: - access fields

static Field_Handle* handle_most_recent = nullptr;

void* access(World* world, Thing_ID id, Field_Handle handle)
{
    Thing* thing = world->Thing_try_get(id);
    if (thing == nullptr) {
        return nullptr;
    }
    
    return thing->field_descriptor.fields[handle.index].data_from(field_contents(thing));
}

void* access(Thing* thing, Field_Handle handle)
{
    return thing->field_descriptor.fields[handle.index].data_from(field_contents(thing));
}



template <typename T, typename Proc>
T* access_and_modify(Thing* thing, const mtt::String& tag, Proc&& proc, Field_Event* ev_out)
{
    auto handle = lookup<false>(thing, tag);
    auto* info = &thing->field_descriptor.fields[handle.index];
    T* data = ((T*)info->data_from(field_contents(thing)));
    T data_before = *data;
    proc(thing, data);
    
    Field_Event ev = {};
    ev.type = BUILTIN_FIELD_EVENT_UNCHANGED_OR_INVALID;
    
    if constexpr (std::is_arithmetic_v<T>) {
        if (*data > data_before) {
            ev.type = BUILTIN_FIELD_EVENT_INCREASED;
            
        } else if (*data < data_before) {
            ev.type = BUILTIN_FIELD_EVENT_DECREASED;
        }
        
        ev.value = *data - data_before;
    }
    
    *ev_out = ev;
    
    if (info->watchers != nullptr) {
        std::vector<Watcher>* watchers = (std::vector<Watcher>*)info->watchers;
        for (usize i = 0; i < watchers->size(); i += 1) {
            (*watchers)[i].proc(thing, ev);
        }
    }
    
    return data;
}


void* access(Thing* thing, usize index)
{
    return thing->field_descriptor.fields[index].data_from(field_contents(thing));
}

void* access(Thing* thing, Field_Handle handle, MTT_TYPE type)
{
    if (!( type == handle.field_desc->fields[handle.index].type )) {
        return nullptr;
    }
    
    return thing->field_descriptor.fields[handle.index].data_from(field_contents(thing));
}
template <>
float32* access<float32>(Thing* thing, const String& tag)
{
    return (float32*)mtt::access(thing, tag, MTT_FLOAT);
}
template <>
vec2* access<vec2>(Thing* thing, const String& tag)
{
    return (vec2*)mtt::access(thing, tag, MTT_VECTOR2);
}
template <>
vec3* access<vec3>(Thing* thing, const String& tag)
{
    return (vec3*)mtt::access(thing, tag, MTT_VECTOR3);
}
template <>
vec4* access<vec4>(Thing* thing, const String& tag)
{
    return (vec4*)mtt::access(thing, tag, MTT_VECTOR4);
}
template <>
color::rgba* access<color::rgba>(Thing* thing, const String& tag)
{
    return (color::rgba*)mtt::access(thing, tag, MTT_COLOR_RGBA);
}

template <>
bool* access<bool>(Thing* thing, const String& tag)
{
    return (bool*)mtt::access(thing, tag, MTT_BOOLEAN);
}

template <>
mtt::String* access<mtt::String>(Thing* thing, const String& tag)
{
    ASSERT_MSG(false, "Should use String_Handle instead\n");
    return nullptr;
}
template <>
MTT_String_Ref_Modify* access<MTT_String_Ref_Modify>(Thing* thing, const String& tag)
{
    auto* ref = (MTT_String_Ref*)mtt::access(thing, tag, MTT_TEXT);
    if (ref == nullptr) {
        ref = (MTT_String_Ref*)mtt::access(thing, tag, MTT_TAG);
        if (ref == nullptr) {
            ref = (MTT_String_Ref*)mtt::access(thing, tag, MTT_STRING);
            if (ref == nullptr) {
                ASSERT_MSG(false, "this is bad\n");
            }
        }
    }
    
    //    MTT_print("Accessed Tag:[%s], string:[%s] with ref count=[%u]\n", tag.c_str(), MTT_string_ref_to_cstring(*ref), MTT_string_ref_get_ref_count(*ref));
    
    MTT_String_Ref_Modify* handle;
    mtt::make_temp(&handle);
    handle->ref = *ref;
    return handle;
}

template <>
MTT_String_Ref* access<MTT_String_Ref>(Thing* thing, const String& tag)
{
    auto* ref = (MTT_String_Ref*)mtt::access(thing, tag, MTT_TEXT);
    if (ref == nullptr) {
        ref = (MTT_String_Ref*)mtt::access(thing, tag, MTT_TAG);
        if (ref == nullptr) {
            ref = (MTT_String_Ref*)mtt::access(thing, tag, MTT_STRING);
            if (ref == nullptr) {
                ASSERT_MSG(false, "this is bad\n");
            }
        }
    }
    
    return ref;
}

template <>
const char* access<const char>(Thing* thing, const String& tag)
{
    auto* ref = (MTT_String_Ref*)mtt::access(thing, tag, MTT_TEXT);
    if (ref == nullptr) {
        ref = (MTT_String_Ref*)mtt::access(thing, tag, MTT_TAG);
        if (ref == nullptr) {
            ref = (MTT_String_Ref*)mtt::access(thing, tag, MTT_STRING);
            if (ref == nullptr) {
                ASSERT_MSG(false, "this is bad\n");
            }
        }
    }
    
    //    MTT_print("Accessed Tag:[%s], string:[%s] with ref count=[%u]\n", tag.c_str(), MTT_string_ref_to_cstring(*ref), MTT_string_ref_get_ref_count(*ref));
    
    return MTT_string_ref_to_cstring(*ref);
}

template <>
uint8* access<uint8>(Thing* thing, const String& tag)
{
    return (uint8*)mtt::access(thing, tag, MTT_INT32);
}

template <>
int8* access<int8>(Thing* thing, const String& tag)
{
    return (int8*)mtt::access(thing, tag, MTT_INT32);
}

template <>
uint16* access<uint16>(Thing* thing, const String& tag)
{
    return (uint16*)mtt::access(thing, tag, MTT_INT32);
}

template <>
int16* access<int16>(Thing* thing, const String& tag)
{
    return (int16*)mtt::access(thing, tag, MTT_INT32);
}

template <>
uint32* access<uint32>(Thing* thing, const String& tag)
{
    return (uint32*)mtt::access(thing, tag, MTT_INT32);
}

template <>
int32* access<int32>(Thing* thing, const String& tag)
{
    return (int32*)mtt::access(thing, tag, MTT_INT32);
}

template <>
uint64* access<uint64>(Thing* thing, const String& tag)
{
    return (uint64*)mtt::access(thing, tag, MTT_INT64);
}

template <>
int64* access<int64>(Thing* thing, const String& tag)
{
    return (int64*)mtt::access(thing, tag, MTT_INT64);
}

template <>
Any* access<Any>(Thing* thing, const String& tag)
{
    return (Any*)mtt::access(thing, tag, MTT_ANY);
}

template <>
mtt::Procedure* access<mtt::Procedure>(Thing* thing, const String& tag)
{
    return (mtt::Procedure*)mtt::access(thing, tag, MTT_PROCEDURE);
}

template <>
void** access<void*>(Thing* thing, const String& tag)
{
    return (void**)mtt::access(thing, tag, MTT_POINTER);
}

template <>
Thing_Ref* access<Thing_Ref>(Thing* thing, const String& tag)
{
    return (Thing_Ref*)mtt::access(thing, tag, MTT_THING);
}

template <>
Thing_List* access<Thing_List>(Thing* thing, const String& tag)
{
    return *static_cast<Thing_List**>(mtt::access(thing, tag, MTT_THING_LIST));
}

template <>
String_List* access<String_List>(Thing* thing, const String& tag)
{
    return *static_cast<String_List**>(mtt::access(thing, tag, MTT_TAG_LIST));
}


void* access(Thing* thing, const String& tag, MTT_TYPE type)
{
    auto handle = lookup<
#ifdef NDEBUG
    false
#else
    true
#endif
    >(thing, tag);
    
    if (!handle.is_valid) {
        return nullptr;
    }
    if (!( type == handle.field_desc->fields[handle.index].type )) {
        return nullptr;
    }
    
    return thing->field_descriptor.fields[handle.index].data_from(field_contents(thing));
}

// MARK: - logic


inline bool name_to_port_id(Map<String, uint64>& map, const String& tag, uint64* out)
{
    auto result = map.find(tag);
    if (result == map.end()) {
        MTT_print("map query failed: %s\n", tag.c_str());
        for (auto it = map.begin(); it != map.end(); ++it) {
            MTT_print("map entry [%s,%llu] ", it->first.c_str(), it->second);
        }
        MTT_print("%s", "\n");
        ASSERT_MSG(false, "");
        return false;
    }
    
    *out = result->second;
    
    return true;
}

bool name_to_out_port_id(World* world, Thing_ID thing_id, const String& tag, uint64* out)
{
    Thing* thing = world->Thing_get(thing_id);
    return name_to_port_id(thing->ports.out_name_to_idx, tag, out);
}
bool name_to_in_port_id(World* world, Thing_ID thing_id, const String& tag, uint64* out)
{
    Thing* thing = world->Thing_get(thing_id);
    return name_to_port_id(thing->ports.in_name_to_idx, tag, out);
}



bool add_connection(World* world, mtt::Thing* src, uint64 src_port, mtt::Thing* dst, uint64 dst_port)
{
    //    Connection_Pool* connections = &world->graph.connection_pool;
    //    if (connections->count == MAX_CONNECTIONS) {
    //        MTT_error("ERROR: maximum connections reached\n");
    //        return false;
    //    }
    
    
    
    if (src_port >= src->ports.out_ports.size()) {
        MTT_error("ERROR: source port out-of-bounds: attempted to connect to port %llu, but the maximum port index is %lu\n", src_port, src->ports.out_ports.size() - 1);
        return false;
    }
    if (dst_port >= dst->ports.in_ports.size()) {
        MTT_error("ERROR: destination port out-of-bounds: attempted to connect to port %llu, but the maximum port index is %lu\n", dst_port, dst->ports.in_ports.size() - 1);
        return false;
    }
    
    Connection connection;
    memset(&connection, 0, sizeof(Connection));
    connection.src_thingref = src->id;
    connection.dst_thingref = dst->id;
    connection.header.src_port_ref = src_port;
    connection.header.dst_port_ref = dst_port;
    
    auto find = src->graph->incoming.find(dst->id);
    //    assert(find != world->graph.incoming.end());
    //    assert(find->second.size() > dst_port);
    
    
    if (find->second[dst_port].is_same_connection(connection)) {
        return false;
    }
    
    find->second[dst_port] = connection;
    
    
    src->ports.out_ports[src_port].is_active       = true;
    dst->ports.in_ports[dst_port].is_active        = true;
    
    // TODO: temporarily just going with connections stored by the objects themselves
    //connections->connections[connections->count] = connection;
    //connections->count += 1;
    
    
    auto& record = src->graph->outgoing[src->id][src_port];
    record.emplace_back(connection);
    
    src->graph->is_modified = true;
    
    
    return true;
}

bool add_connection(World* world, Thing_ID src, const String& src_tag, Thing_ID dst, const String& dst_tag, Thing** src_thing_out, Thing** dst_thing_out)
{
    uint64 src_port = 0;
    uint64 dst_port = 0;
    
    if (!(name_to_out_port_id(world, src, src_tag, &src_port) &&
          name_to_in_port_id(world, dst, dst_tag, &dst_port))) {
        return false;
    }
    
    Thing* src_thing = nullptr;
    Thing* dst_thing = nullptr;
    if (!(world->Thing_try_get(src, &src_thing) &&
          world->Thing_try_get(dst, &dst_thing))) {
        MTT_error("%s", "ERROR: cannot find things\n");
        return false;
    }
    
    if (add_connection(world, src_thing, src_port, dst_thing, dst_port)) {
        *src_thing_out = src_thing;
        *dst_thing_out = dst_thing;
        return true;
    }
    
    return false;
}

bool add_connection(World* world, Thing* src, const String& src_tag, Thing* dst, const String& dst_tag)
{
    uint64 src_port = 0;
    uint64 dst_port = 0;
    
    if (!(name_to_out_port_id(world, src->id, src_tag, &src_port) &&
          name_to_in_port_id(world, dst->id, dst_tag, &dst_port))) {
        ASSERT_MSG(false, "Should not happen");
        return false;
    }
    
    return add_connection(world, src, src_port, dst, dst_port);
}

bool add_connection(World* world, Thing_ID src, const String& src_tag, Thing_ID dst, const String& dst_tag, my::Function<void(World*, Thing*, Thing*, void*)> callback, void* state, Thing** src_thing_out, Thing** dst_thing_out) {
    
    Thing* src_thing = nullptr;
    Thing* dst_thing = nullptr;
    if (add_connection(world, src, src_tag, dst, dst_tag, &src_thing, &dst_thing)) {
        
        callback(world, src_thing, dst_thing, state);
        if (src_thing_out && dst_thing_out) {
            *src_thing_out = src_thing;
            *dst_thing_out = dst_thing;
        }
        
        return true;
    }
    
    return false;
}
bool add_connection(World* world, Thing* src, const String& src_tag, Thing* dst, const String& dst_tag, my::Function<void(World*, Thing*, Thing*, void*)> callback, void* state)
{
    if (add_connection(world, src, src_tag, dst, dst_tag)) {
        
        callback(world, src, dst, state);
        return true;
    }
    
    return false;
}

void add_execution_connection(mtt::Thing* thing_src, mtt::Thing* thing_dst)
{
    thing_src->graph->outgoing_execution[thing_src->id].insert(thing_dst->id);
    thing_src->graph->incoming_execution[thing_dst->id].insert(thing_src->id);
}
void remove_execution_connection(mtt::Thing* thing_src, mtt::Thing* thing_dst)
{
    thing_src->graph->outgoing_execution[thing_src->id].erase(thing_dst->id);
    thing_src->graph->incoming_execution[thing_dst->id].erase(thing_src->id);
}
void remove_incoming_execution_connections(mtt::Thing* thing)
{
    auto in_find = thing->graph->incoming_execution.find(thing->id);
    if (in_find == thing->graph->incoming_execution.end()) {
        return;
    }
    
    auto& in = in_find->second;
    for (auto src_it = in.begin(); src_it != in.end(); ++src_it) {
        auto out = thing->graph->outgoing_execution.find((*src_it));
        if (out != thing->graph->outgoing_execution.end()) {
            out->second.erase(thing->id);
        }
    }
    thing->graph->incoming_execution[thing->id].clear();
}
void remove_outgoing_execution_connections(mtt::Thing* thing)
{
    auto out_find = thing->graph->outgoing_execution.find(thing->id);
    if (out_find == thing->graph->outgoing_execution.end()) {
        return;
    }
    
    auto& out = out_find->second;
    for (auto src_it = out.begin(); src_it != out.end(); ++src_it) {
        auto in = thing->graph->incoming_execution.find((*src_it));
        if (in != thing->graph->incoming_execution.end()) {
            in->second.erase(thing->id);
        }
    }
    thing->graph->outgoing_execution[thing->id].clear();
}
void remove_all_execution_connections(mtt::Thing* thing)
{
    remove_incoming_execution_connections(thing);
    remove_outgoing_execution_connections(thing);
}
void delete_execution_connections(mtt::Thing* thing)
{
    remove_incoming_execution_connections(thing);
    remove_outgoing_execution_connections(thing);
    thing->graph->incoming_execution.erase(thing->id);
    thing->graph->outgoing_execution.erase(thing->id);
}



bool remove_connection_at_dst(mtt::Thing* dst, const mtt::String& dst_port_name)
{
    uint64 idx = 0;
    mtt::World* world = dst->world();
    if (name_to_in_port_id(world, dst->id, dst_port_name, &idx)) {
        bool success = remove_connection_at_dst_port(world, dst->id, idx);
        ASSERT_MSG(success, "Should not happen");
        return success;
    }
    
    return false;
}

bool remove_connection(World* world, Connection& connection)
{
    if (!connection.is_valid()) {
        return false;
    }
    
    Thing* src_thing = nullptr;
    Thing* dst_thing = nullptr;
    
    
    
    if (!world->Thing_try_get(connection.dst_thingref, &dst_thing)) {
        MTT_error("ERROR: dst Thing does not exist %d\n", __LINE__);
        return false;
    }
    
    if (connection.header.dst_port_ref >= dst_thing->ports.in_ports.size()) {
        return false;
    }
    
    
    
    auto dst_result = dst_thing->graph->incoming.find(connection.dst_thingref);
    if (!connection.is_same_connection(dst_result->second[connection.header.dst_port_ref])) {
        MTT_error("ERROR: stale connection %d\n", __LINE__);
        return false;
    }
    
    dst_thing->graph->is_modified = true;
    
    if (world->Thing_try_get(connection.src_thingref, &src_thing) && connection.header.src_port_ref < src_thing->ports.out_ports.size()) {
        auto& out_port = src_thing->graph->outgoing[connection.src_thingref];
        ASSERT_MSG(out_port.size() > 0, "should have a port connected before reducing number!\n");
        
        auto& out = out_port[connection.header.src_port_ref];
        for (auto it = out.begin(); it != out.end(); ++it) {
            Connection& src_conn = (*it);
            if (connection.is_same_connection(src_conn)) {
                out.erase(it);
                break;
            }
        }
        
        if (out.empty()) {
            src_thing->ports.out_ports[connection.header.src_port_ref].is_active = false;
        }
    }

    
    {
        memset(&dst_result->second[connection.header.dst_port_ref], 0, sizeof(Connection));
        dst_thing->ports.in_ports[connection.header.dst_port_ref].is_active = false;
    }
    
    MTT_print("%s", "Successfully removed connection\n");
    
    return true;
}

bool remove_connection_at_dst_port(World* world, Thing_ID thing_id, uint64 port_idx)
{
    auto* thing = Thing_try_get(world, thing_id);
    if (thing == nullptr) {
        return false;
    }
    
    auto result = thing->graph->incoming.find(thing_id);
    Connection& connection = result->second[port_idx];
    
    return remove_connection(world, connection);
}

bool remove_all_connections(World* world, Thing_ID thing_id)
{
    
    Thing* thing_to_remove = nullptr;
    if (!world->Thing_try_get(thing_id, &thing_to_remove)) {
        MTT_error("ERROR: could not find thing to remove %llu %d\n", thing_id, __LINE__);
        return false;
    }
    
    bool removed_a_connection = false;
    {
        auto it_incoming = thing_to_remove->graph->incoming.find(thing_id);
        
        auto& c = (*it_incoming).second;
        for (isize i = c.size() - 1; i >= 0; i -= 1) {
            removed_a_connection = removed_a_connection || remove_connection(world, c[i]);
        }
    }
    
    
    
    return removed_a_connection;
}

//static Evaluation_Output_Entry entries[Evaluation_Output_Entry_count_cap];
void World_Graph_init(Eval_Connection_Graph* G, mem::Allocator* allocator, usize count)
{
    //G->output.buffer = entries;//(void*)allocator->allocate(allocator, sizeof(Evaluation_Output_Entry) * 2048);
    //    G->output.byte_count = 0;
    //    G->output.count = 0;
    //    G->output.cap = Evaluation_Output_Entry_count_cap;
    //    G->output.allocator = allocator;
    //G->output.list = std::vector<Evaluation_Output>(2048);
    if (count != 0) {
        G->output.list.resize(count);
    }
    
    
    G->is_modified = false;
    //G->output.list.reserve(Evaluation_Output_Entry_count_cap);
    
    
    //mtt::init(&G->sorted_things_direct, *allocator, 0, 256);
    
    
    
    //Connections_init(&G->connection_pool, allocator);
    
}
void Connections_init(Connection_Pool* connection_pool, mem::Allocator* allocator)
{
    memset(connection_pool, 0, MAX_CONNECTIONS * sizeof(Connection));
    connection_pool->count = 0;
}

void sort_things_internal(World* world, Eval_Connection_Graph& G, Thing* dst)
{
    //    if (dst->instance_id == 0) {
    //
    //        return;
    //    }
    
    //MTT_print("visiting: %llu\n", dst->id);
    
    auto incoming_check = G.incoming.find(dst->id);
    if (incoming_check != G.incoming.end()) {
        
        Port_Input_List& incoming = incoming_check->second;
        for (usize in_idx = 0; in_idx < incoming.size(); in_idx += 1) {
            if ((in_idx >= dst->ports.in_ports.size()) || (!dst->ports.in_ports[in_idx].is_active) || (incoming[in_idx].src_thingref == mtt::Thing_ID_INVALID)) {
                continue;
            }
            
            mtt::Thing* src = world->Thing_try_get(incoming[in_idx].src_thingref);
            if (src == nullptr) {
                remove_connection(world, incoming[in_idx]);
                continue;
            }
            
            
            MTT_print("src instance id[%llu]\n", src->id);
            if (src->is_visited) {
                continue;
            }
            
            src->is_visited = true;
            
            sort_things_internal(world, G, src);
        }
        
    }
    
    //G.sorted_things[G.sorted_things_count] = dst->instance_id;
    //MTT_print("[SORT: %llu]\n", dst->instance_id);
    // TODO check if this is safe
    
    G.sorted_things_direct.push_back(dst);
    //G.sorted_things_direct.count += 1;
    //dst->eval_index = G.sorted_things_count;
    
}


void sort_things_by_execution_internal(World* world, Eval_Connection_Graph& G, mtt::Thing* dst);
void sort_things_by_execution_internal(World* world, Eval_Connection_Graph& G, mtt::Thing* dst)
{
    auto incoming_check = G.incoming_execution.find(dst->id);
    if (incoming_check != G.incoming_execution.end()) {
        auto& incoming = incoming_check->second;
        for (auto it = incoming.begin(); it != incoming.end(); ++it) {
            mtt::Thing* src = world->Thing_try_get(*it);
            if (src == nullptr) {
                continue;
            }
            
            if (src->is_visited) {
                continue;
            }
            
            src->is_visited = true;
            
            sort_things_by_execution_internal(world, G, src);
        }
        
    }
    
    G.sorted_things_direct.push_back(dst);
    //G.sorted_things_direct.count += 1;
}

void sort_things_by_execution(World* world, Eval_Connection_Graph& G)
{
    for (auto it = G.incoming_execution.begin(); it != G.incoming_execution.end(); ++it) {
        mtt::Thing_ID dst = (*it).first;
        mtt::Thing* dst_thing = world->Thing_try_get(dst);
        if (dst_thing->is_visited) {
            continue;
        }
        
        dst_thing->is_visited = true;
        
        sort_things_by_execution_internal(world, G, dst_thing);
    }
}





void sort_things_in_root_ctx(World* world, Thing_Storage& things, Eval_Connection_Graph& G);

void sort_things_in_root_ctx(World* world, Thing_Storage& things, Eval_Connection_Graph& G)
{
    ASSERT_MSG(&G == root_graph(world), "Should be root graph");
    
    G.sorted_things_direct.clear();
    G.sorted_things_direct.reserve(things.size());
    
    sort_things_by_execution(world, G);
    
    for (auto it = things.begin(); it != things.end(); ++it) {
        Thing* thing = &it->second;
        // guard
        if (thing->is_visited || thing->ctx_id != Context_ID_DEFAULT) {
            continue;
        }
        thing->is_visited = true;
        
        sort_things_internal(world, *root_graph(world), thing);
    }
    
    std::stable_sort(G.sorted_things_direct.begin(), G.sorted_things_direct.end(), Thing_compare_by_eval_priority());
    
    for (usize i = 0; i < G.sorted_things_direct.size(); i += 1) {
        G.sorted_things_direct[i]->eval_index = i;
    }
    //    for (usize i = 1; i < G.sorted_things_count; i += 1) {
    //        MTT_print("%llu\n", G.sorted_things_direct[i]->eval_priority);
    //        assert(G.sorted_things_direct[i]->eval_priority >= G.sorted_things_direct[i]->eval_priority);
    //    }
    
    Things_mark_unvisited(world);
    
    G.output.list.resize(root_graph(world)->sorted_things_direct.size());
}

void sort_connections(World* world, Thing_Storage& things, Eval_Connection_Graph& G);
void sort_connections(World* world, Thing_Storage& things, Eval_Connection_Graph& G)
{
    // FIXME: no need to do this now
    return;
//    Connection_Compare_dst_id comparison_proc;
//    
//    
//    std::sort(G.connection_pool.connections, G.connection_pool.connections + G.connection_pool.count, comparison_proc);
    
}

static Evaluation_Output_Entry_Port nil_port_entry;
Result<Evaluation_Output_Entry_Port&> get_in_port(World* world, Thing* thing, Port_Input_List* input_list, uint64 port_idx)
{
    if (port_idx >= thing->ports.in_ports.size() || !thing->ports.in_ports[port_idx].is_active) {
        return {false, nil_port_entry};
    }
    
    Thing* in_thing = nullptr;
    // TODO: instead of using a map, switch to a sparse/dense array allocated as one
    // 2-compartment memory region, one for sparse, one for dense
    if (!world->Thing_try_get((*input_list)[port_idx].src_thingref, &in_thing)) {
        thing->ports.in_ports[port_idx].is_active = false;
        return {false, nil_port_entry};
    }
    
    // if the input thing is deactivated,
    // treat as nonexistent-input
    if (!mtt::is_active(in_thing) || !mtt::is_active_group(in_thing)) {
        return {false, nil_port_entry};
    }
    
    
    Connection& connection = (*input_list)[port_idx];
    
    // TODO: caching if the graph has not changed?
    auto& output = *thing->eval_out;
    
    Evaluation_Output_Entry_Port& port_entry = output.port_entries()[output.list[in_thing->eval_index].first_port_index + connection.header.src_port_ref];
    
    if (port_entry.is_ignored) {
        return {false, nil_port_entry};
    }
    
    return {true, port_entry};
}

//struct Port_Input_Info {
//    Port_Array* ports = nullptr;
//    mtt::Array_Slice<Evaluation_Output_Entry_Port*> values = {};
//};


mtt::Dynamic_Array<Value_Entry> get_active_inputs(Thing* thing, Port_Input_List* input_list)
{
    World* world = mtt::world(thing);
    
    auto& tmp_allocator = world->allocator_temporary();
    
    mtt::Dynamic_Array<Value_Entry> out = {};
    mtt::init(&out, tmp_allocator);
    
    usize count = thing->ports.in_ports.size();
    if (count == 0) {
        return out;
    }
    auto& in_ports = thing->ports.in_ports;
    for (usize port_idx = 0; port_idx < count; port_idx += 1){
        mtt::Thing* in_thing = nullptr;
        if (!Thing_try_get(world, (*input_list)[port_idx].src_thingref, &in_thing)) {
            in_ports[port_idx].is_active = false;
            continue;
        }
        
        // if the input thing is deactivated,
        // treat as nonexistent-input
        if (!mtt::is_active(in_thing) || !mtt::is_active_group(in_thing)) {
            continue;
        }
        
        
        Connection& connection = (*input_list)[port_idx];
        
        // TODO: caching if the graph has not changed?
        auto& output = *thing->eval_out;
        
        Evaluation_Output_Entry_Port& port_entry = output.port_entries()[output.list[in_thing->eval_index].first_port_index + connection.header.src_port_ref];
        
        if (port_entry.is_ignored) {
            continue;
        }
        
        out.push_back((Value_Entry){
            .value = port_entry.out,
            .name = mtt::String(string_get(thing->ports.in_ports[port_idx].tag))
        });
    }
    
    return out;
}

Result<Evaluation_Output_Entry_Port&> get_in_port(World* world, Thing* thing, Port_Input_List* input_list, const String& tag)
{
    uint64 idx = 0;
    if (name_to_in_port_id(world, thing->id, tag, &idx)) {
        return get_in_port(world, thing, input_list, idx);
    }
    
    return {false, nil_port_entry};
}

bool in_port_is_active(Thing* thing, uint64 port_idx)
{
    return !(port_idx >= thing->ports.in_ports.size() || !thing->ports.in_ports[port_idx].is_active);
}
bool in_port_is_active(Thing* thing, const String& tag)
{
    uint64 idx = 0;
    if (name_to_in_port_id(thing->world(), thing->id, tag, &idx)) {
        return in_port_is_active(thing, idx);
    }
    
    return false;
}
bool out_port_is_active(Thing* thing, uint64 port_idx)
{
    return !(port_idx >= thing->ports.out_ports.size() || !thing->ports.out_ports[port_idx].is_active);
}
bool out_port_is_active(Thing* thing, const String& tag)
{
    uint64 idx = 0;
    if (name_to_out_port_id(thing->world(), thing->id, tag, &idx)) {
        return out_port_is_active(thing, idx);
    }
    
    return false;
}


Result<Evaluation_Output_Entry_Port&> get_in_port(Thing* thing, Port_Input_List* input_list, uint64 port_idx)
{
    return get_in_port(thing->world(), thing, input_list, port_idx);
}
Result<Evaluation_Output_Entry_Port&> get_in_port(Thing* thing, Port_Input_List* input_list, const String& tag)
{
    return get_in_port(thing->world(), thing, input_list, tag);
}

template <typename T>
T get_in_port_or(Thing* thing, Port_Input_List* input_list, uint64 port_idx, const T alt)
{
    auto out = get_in_port(thing->world(), thing, input_list, port_idx);
    if (out.status == PORT_STATUS_OK) {
        return *(T*)raw_value_address(&out.value.out);
    } else {
        return alt;
    }
}
template <typename T>
T get_in_port_or(Thing* thing, Port_Input_List* input_list, const String& tag, const T alt)
{
    auto out = get_in_port(thing->world(), thing, input_list, tag);
    if (out.status == PORT_STATUS_OK) {
        return *(T*)raw_value_address(&out.value.out);
    } else {
        return alt;
    }
}

template <typename T>
typename T::CONCRETE_TYPE get_in_port_or(Thing* thing, Port_Input_List* input_list, uint64 port_idx, const typename T::CONCRETE_TYPE alt)
{
    auto out = get_in_port(thing->world(), thing, input_list, port_idx);
    if (out.status == PORT_STATUS_OK) {
        return T::value(&out.value.out);
    } else {
        return alt;
    }
}
template <typename T>
typename T::CONCRETE_TYPE get_in_port_or(Thing* thing, Port_Input_List* input_list, const String& tag, const typename T::CONCRETE_TYPE alt)
{
    auto out = get_in_port(thing->world(), thing, input_list, tag);
    if (out.status == PORT_STATUS_OK) {
        return T::value(&out.value.out);
    } else {
        return alt;
    }
}


Result<Evaluation_Output_Entry_Port&> get_out_port(Thing* thing, uint64 port_idx)
{
    if (port_idx >= thing->ports.out_ports.size() || !thing->ports.out_ports[port_idx].is_active) {
        return {false, nil_port_entry};
    }
    
    auto& output = *thing->eval_out;
    ASSERT_MSG(!output.list.empty(), "???");
    Evaluation_Output_Entry_Port& out = output.port_entries()[output.list[thing->eval_index].first_port_index + port_idx];
    
    return {true, out};
}

Result<Evaluation_Output_Entry_Port&> get_out_port(mtt::World* world, Thing* thing, uint64 port_idx) {
    return get_out_port(thing, port_idx);
}

Result<Evaluation_Output_Entry_Port&> get_out_port(Thing* thing, const String& tag)
{
    uint64 idx = 0;
    mtt::World* world = thing->world();
    if (name_to_out_port_id(world, thing->id, tag, &idx)) {
        return get_out_port(world, thing, idx);
    }
    
    return {false, nil_port_entry};
}

Result<Evaluation_Output_Entry_Port&> get_out_port(World* world, Thing* thing, const String& tag)
{
    return get_out_port(thing, tag);
}


void on_end_noop(mtt::Thing* thing)
{
    return;
}

void process_things(World* world, /*Thing_Storage& things,*/ Eval_Connection_Graph& G);

void process_things(World* world, /*Thing_Storage& things,*/Eval_Connection_Graph& G)
{
    Evaluation_Output& eval_out = G.output;
    
    ASSERT_MSG(&G == mtt::root_graph(world), "Incorrect connection graph!");
    
    //eval_out.offset = 0;
    //eval_out.index  = 0;
    
    //Thing_ID* sorted_things   = G.sorted_things;
    usize sorted_things_count = G.sorted_things_direct.size();
    if (sorted_things_count == 0) {
        return;
    }
    
    
    auto* prev_graph = mtt::curr_graph(world);
    mtt::set_graph_to_root(world);
    
    Thing** sorted_things = &G.sorted_things_direct[0];
    usize port_index = 0;
    for (usize thing_idx = 0; thing_idx < sorted_things_count; thing_idx += 1) {
        
        //eval_out.index = thing_idx;
        
        Thing* const thing = sorted_things[thing_idx];
        Thing_ID const thing_id = thing->id;
        
        {
            //eval_out.list[thing_idx].port_entries.resize(thing->ports.out_ports.size());
            eval_out.list[thing_idx].first_port_index = port_index;
            
            auto* const out_ports = &thing->ports.out_ports;
            const usize port_count = out_ports->size();
            usize next_port_index = port_index + port_count;
            while (eval_out.port_entries().size() < next_port_index) {
                eval_out.port_entries().push_back({});
            }
            for (usize i = 0; i < port_count; i += 1) {
                auto* const entry = (&eval_out.port_entries()[port_index + i]);
                entry->ID                 = thing_id;
                entry->out.type           = (*out_ports)[i].type;
                entry->out.contained_type = (*out_ports)[i].contained_type;
                
                entry->is_ignored = false;
            }
            port_index += port_count;
        }
    }
    
    for (usize thing_idx = 0; thing_idx < sorted_things_count; thing_idx += 1) {
        Thing* const thing = sorted_things[thing_idx];
        Thing_ID const thing_id = thing->id;
        
        
        Port_Input_List* input_list = nullptr;
        map_get(G.incoming, thing_id, &input_list);
        auto return_status = thing->logic.proc(world, thing, input_list, NULL, NULL, NULL);
        (void)return_status;
    }
    
    mtt::set_graph(world, prev_graph);
}




void evaluate_world_pre(World* world)
{
    if (world->thing_buffer.size() < world->things.instances.size()) {
        world->thing_buffer.resize(world->things.instances.size());
    }
    
    collisions_pre(&world->collision_system);
    
    while (!world->message_passer.system_messages_deferred.empty()) {
        auto& msg = world->message_passer.system_messages_deferred.front();
        
        Procedure_Input_Output io = {};
        
        Thing* sender = nullptr;
        world->Thing_try_get(msg.sender, &sender);
        io.caller = sender;
        io.input = (void*)&msg;
        msg.proc(&io);
        
        world->message_passer.system_messages_deferred.pop_front();
    }
    
    if (!world->to_destroy.empty()) {
        
        for (auto it = world->to_destroy.begin(); it != world->to_destroy.end();) {
            Destroy_Command& cmd = (*it);
            mtt::Thing* th;
            if (!mtt::Thing_try_get(world, cmd.thing_id, &th)) {
                it = world->to_destroy.erase(it);
                continue;
            } else if (cmd.time_remaining > 0.0f) {
                //if (cmd.do_fade_animation)
                {
                    mtt::Rep* rep;
                    mtt::rep(th, &rep);
                    for (auto it_r = rep->render_data.drawable_info_list.begin(); it_r != rep->render_data.drawable_info_list.end(); ++it_r) {
                        (*it_r)->set_color_factor(vec4(cmd.time_remaining / cmd.time_delay));
                        //(*it)->color_addition *= cmd.time_remaining / cmd.time_delay;
                    }
                }
                cmd.time_remaining -= MTT_TIMESTEP;
                ++it;
                continue;
            } else if (cmd.frame_delay > 0) {
                cmd.frame_delay -= 1;
                ++it;
                continue;
            } else {
                if (cmd.affects_connected) {
                    Thing_destroy_self_and_connected(world, cmd.thing_id);
                } else {
                    Thing_destroy(world, cmd.thing_id);
                }
                
                it = world->to_destroy.erase(it);
            }
            
            
        }
        
        // no need to mark as unvisited since these things will be destroyed
    }
    
    
    if (!world->to_enable.empty()) {
        do {
            Thing_ID thing_id = world->to_enable.front();
            world->to_enable.pop_front();
            
            mtt::Thing_apply_self_and_children(world, thing_id, [](mtt::World* world, mtt::Thing* thing, void* args) {
                mtt::set_is_active(thing);
            }, nullptr);
            
        } while (!world->to_enable.empty());
        
        Things_mark_unvisited(world);
    }
    if (!world->to_disable.empty()) {
        do {
            Thing_ID thing_id = world->to_disable.front();
            world->to_disable.pop_front();
            
            mtt::Thing_apply_self_and_children(world, thing_id, [](mtt::World* world, mtt::Thing* thing, void* args) {
                mtt::unset_is_active(thing);
            }, nullptr);
            
        } while (!world->to_disable.empty());
        
        Things_mark_unvisited(world);
    }
}

void evaluate_world(World* world)
{
    
#define eval_world_print(...) // MTT_print(__VA_ARGS__)
    
    //eval_world_print("Evaluate World begin:{\n");
    //    world->time_seconds
    //    world->timestep;
    
    
    //    for (auto it = world->things.instances.begin(); it != world->things.instances.end(); ++it) {
    //        Thing_print_verbose(world, it->first);
    //    }
    
    
    
    //    MTT_print("begin sensor loop\n");
    
    //    MTT_print("Sensor collision layer cell count before: %llu\n", collision_layer_active_count(world->collision_system, 0));
    
    world->no_deletion_zone_on = true;
    
    
    //    world->sensor_query.iter([](flecs::iter it, mtt::Thing_Info* info, mtt::Sensor* sensor) {
    //
    //        mtt::World* mtt_world = static_cast<World*>(it.world().get_context());
    //
    //        Collision_System* collision_system = &mtt_world->collision_system;
    //
    //        for (auto i : it) {
    //            auto* thing_info = &info[i];
    //            //Thing_print_verbose(mtt_world, thing_info->thing_id);
    ////                p[i].x += v[i].x;
    ////                p[i].y += v[i].y;
    ////
    ////                std::cout << "Moved " << it.entity(i).name() << " to {" <<
    ////                p[i].x << ", " << p[i].y << "}" << std::endl;
    //
    //#define SENSOR_COLLIDER_INTERACTION_IDX (0)
    //#define SENSOR_COLLIDER_BOUNDARY_IDX_OFFSET (1)
    //
    //            Thing_ID thing_id = thing_info->thing_id;
    //            mtt::Thing* sensor_thing;
    //            if (!mtt_world->Thing_try_get(thing_id, &sensor_thing)) {
    //                continue;
    //            }
    //            auto& colliders = mtt::rep(sensor_thing)->colliders;
    //
    //            //Collider_print(&colliders[SENSOR_COLLIDER_BOUNDARY_IDX_OFFSET]);
    //
    //            Collider_remove(collision_system, 0, colliders[SENSOR_COLLIDER_BOUNDARY_IDX_OFFSET]);
    //
    //            push_circles(collision_system, colliders[SENSOR_COLLIDER_BOUNDARY_IDX_OFFSET], 1, 0);
    //        }
    //    });
    
    //    MTT_print("Sensor collision layer cell count after: %llu\n", collision_layer_active_count(world->collision_system, 0));
    
    
    sd::rewind_layer(world->renderer, LAYER_LABEL_COLLISION_DEBUG);
    
    b2World_Step(world->physics_world, world->timestep, 4);
    
    broad_phase(&world->collision_system, 0);
    //    narrow_phase(&world->collision_system, 0);
    
    
    
    
    //    MTT_print("end sensor loop\n");
    
    if (root_graph(world)->is_modified) {
        root_graph(world)->is_modified = false;
        
        //        MTT_print("World graph modified: \n");
        
        sort_things_in_root_ctx(world, world->things.instances, *root_graph(world));
        
        //sort_connections(world, world->things.instances, *root_graph(world));
    }
    
//    eval_world_print("Thing count: %llu\n", world->things.instances.size());
//    eval_world_print("sorted things: count: %llu{\n", world->graph.sorted_things_count);
//    for (usize i = 0; i < root_graph(world)->sorted_things_direct.size(); i += 1) {
//        eval_world_print("id=[%llu]\n", world->graph.sorted_things[i]);
//    }
//    eval_world_print("}\n");
    
    //eval_world_print("sorted connections:{\n");
    //for (usize i = 0; i < world->graph.connection_pool.count; i += 1) {
    //Connection_print(&world->graph.connection_pool.connections[i]);
    //}
    
    //eval_world_print("}\n");
    
    
    auto* rtime = Runtime::ctx();
    Signal_Mailbox_clear(&rtime->signal_mailbox);
    
    
    {
        std::stable_sort(
                         rtime->script_tasks.rules_list.begin(),
                         rtime->script_tasks.rules_list.end(),
                         Script_Instance_compare__ID_and_Priority());
        
        auto& task_list = rtime->script_tasks.rules_list;
        for (isize i = task_list.size() - 1; i >= 0; i -= 1) {
            
            auto* task = task_list[i];
            
            switch (task->status) {
                case SCRIPT_STATUS_CANCELED: {
                    Script_Instance_cancel(task);
                    MTT_FALLTHROUGH
                }
                case SCRIPT_STATUS_DONE_SHOULD_TERMINATE:
                case SCRIPT_STATUS_TERMINATED:
                case SCRIPT_STATUS_DONE: {
                    Script_Instance_terminate(task);
                    rtime->id_to_rule_script.erase(task->id);
                    Script_Instance_destroy(task);
                    std::swap(task_list[i], task_list[task_list.size() - 1]);
                    
                    task_list.pop_back();
                    continue;
                }
                default: { break; }
            }
            
            process_script(world, task);
        }
    }
    
    while (!world->message_passer.system_messages_deferred_before_scripts.empty()) {
        // TODO: do something
        auto& msg = world->message_passer.system_messages_deferred_before_scripts.front();
        
        Procedure_Input_Output io = {};
        
        Thing* sender = nullptr;
        world->Thing_try_get(msg.sender, &sender);
        io.caller = sender;
        io.input = (void*)&msg;
        msg.proc(&io);
        
        world->message_passer.system_messages_deferred_before_scripts.pop_front();
    }
    
    Script_shared_eval_pre();
    
    std::stable_sort(
                     rtime->script_tasks.list.begin(),
                     rtime->script_tasks.list.end(),
                     Script_Instance_compare__ID_and_Priority());

    auto& task_list = rtime->script_tasks.list;
    //MTT_print("TASK COUNT: %lu\n", task_list.size());
    for (isize i = task_list.size() - 1; i >= 0; i -= 1) {
        
        auto* task = task_list[i];
        
        switch (task->status) {
            case SCRIPT_STATUS_CANCELED: {
                Script_Instance_cancel(task);
                MTT_FALLTHROUGH
            }
            case SCRIPT_STATUS_DONE_SHOULD_TERMINATE:
            case SCRIPT_STATUS_TERMINATED:
            case SCRIPT_STATUS_DONE: {
                Script_Instance_terminate(task);
                Script_Instance_destroy(task);
                std::swap(task_list[i], task_list[task_list.size() - 1]);
                
                task_list.pop_back();
                continue;
            }
            default: { break; }
        }
        
        process_script(world, task);
    }
    
    while (!world->message_passer.system_messages_deferred_after_scripts.empty()) {
        auto& msg = world->message_passer.system_messages_deferred_after_scripts.front();
        
        Procedure_Input_Output io = {};
        
        Thing* sender = nullptr;
        world->Thing_try_get(msg.sender, &sender);
        io.caller = sender;
        io.input = (void*)&msg;
        msg.proc(&io);
        
        world->message_passer.system_messages_deferred_after_scripts.pop_front();
    }
    
    Script_shared_eval_post();
    
    process_things(world, /*world->things.instances,*/ *root_graph(world));
    
    
    
    
    //eval_world_print("Evaluate World end:\n}\n");
    
    
    
    
    
#undef eval_world_print
}


void evaluate_world_post(World* world)
{
    while (!world->message_passer.system_messages.empty()) {
        // TODO: do something
        auto& msg = world->message_passer.system_messages.front();
        
        Procedure_Input_Output io = {};
        
        
        Thing* sender = nullptr;
        if (msg.sender != Thing_ID_INVALID) {
            world->Thing_get(msg.sender, &sender);
        }
        io.caller = sender;
        io.input = (void*)&msg;
        msg.proc(&io);
        
        world->message_passer.system_messages.pop_front();
    }
    
    //    if (false) {
    //        auto col_btwn = world->collision_between->typename_desc;
    //        const usize count = world->collisions_to_remove_count;
    //        auto& collisions_to_remove = world->collisions_to_remove;
    //        for (usize i = 0; i < count; i += 1) {
    //            auto& to_remove = collisions_to_remove[i];
    //            flecs::entity a = to_remove.a;
    //            flecs::entity b = to_remove.b;
    //
    //            a.remove(col_btwn, b);
    //            b.remove(col_btwn, a);
    //        }
    //        //MTT_print("%llu\n", world->collisions_to_remove_count);
    //        //printf("collisions_to_remove %llu\n", world->collisions_to_remove_count);
    //        world->collisions_to_remove_count = 0;        //world->collisions_to_remove.clear();
    //    }
    
    world->no_deletion_zone_on = false;
    
    //world->ecs_world.progress();
    //printf("%llu\n", world->collision_system.found_collisions.size());
    //world->collision_system.found_collisions.clear();
    
    for (usize i = 0; i < world->to_clear.size(); i += 1) {
        
    }
    world->to_clear.clear();
    
    auto* rtime = Runtime::ctx();
    auto& task_list = rtime->script_tasks.list;
    for (isize i = task_list.size() - 1; i >= 0; i -= 1) {
        auto* script = task_list[i];
        if (script->source_script->on_end_frame != nullptr) {
            auto res = script->on_end_frame(world, script, nullptr);
            (void)res;
            //auto [result_type, result_continuation, result_value] = res;
        }
    }
    
    
    
    Collision_System_end_step(&world->collision_system);
    
    if (!world->to_destroy_end.empty()) {
        usize destroy_count = world->to_destroy_end.size();
        do {
            destroy_count -= 1;
            mtt::Thing_ID thing_id = world->to_destroy_end.front();
            world->to_destroy_end.pop_front();
            
            Thing_destroy_self_and_connected(world, thing_id);
        } while (destroy_count > 0);
    }
    
    world->step_count += 1;
    
}


void logic_print_internal(World* world, Thing* thing)
{
    return;
    
    // TODO: - implement
    //    Port_Input_List* connections = &thing->in_connnections;
    //
    //    for (auto it = connections->begin(); it != connections->end(); ++it) {
    //        Connection* connection = &*it;
    //        // BUG: creates default entry, which is undesirable
    //        Thing* dst = &world->things.instances[connection->src_ref];
    //
    //        if (dst->is_visited) {
    //            continue;
    //        }
    //        dst->is_visited = true;
    //
    //        logic_print_internal(world, dst);
    //    }
    //
    //
    //    Thing_print(world, thing->instance_id);
}

void logic_print(World* world)
{
    MTT_print("{LOGIC PRINT thing_count=[%zu]\n", world->things.instances.size());
    auto* things = &world->things.instances;
    
    for (auto it = things->begin(); it != things->end(); ++it) {
        Thing* thing = &it->second;
        
        if (thing->is_visited) {
            continue;
        }
        thing->is_visited = true;
        
        logic_print_internal(world, thing);
    }
    
    for (auto it = things->begin(); it != things->end(); ++it) {
        Thing* thing = &it->second;
        
        thing->is_visited = false;
    }
    
    MTT_print("%s", "}\n");
}





//static void new_attr(Make_The_Thing_World* mtt_world, const char *name, usize size, ecs_entity_t* ent)
//{
//    ecs_world_t * world = mtt_world->ecs_world.c_ptr();
//
//    ecs_entity_t component = ecs_new_component(world, *ent, name, size, align_up(size, 16));
//
//    mtt_world->attributes.tag_to_attribute.insert({std::string(name), component});
//}
//
//static const void* get_attr(Make_The_Thing_World* mtt_world, ecs_entity_t e, ecs_entity_t component_id)
//{
//    ecs_world_t * world = mtt_world->ecs_world.c_ptr();
//
//    return ecs_get_w_entity(world, e, component_id);
//}
//
//static const Result<Attribute_Result> get_attr(Make_The_Thing_World* mtt_world, ecs_entity_t e, const char *name)
//{
//    auto result = mtt_world->attributes.tag_to_attribute.find(name);
//    if (result == mtt_world->attributes.tag_to_attribute.end()) {
//        // TODO: // search for related attributes to resolve
//
//        // temp
//        return {.status = false};
//    }
//
//    auto* attr = get_attr(mtt_world, e, result->second);
//    if (attr == NULL) {
//        return {.status = false};
//    }
//    return {.status = true, {.attribute = attr, .is_exact_match = true}};
//}


// old
//    ecs_type_t type = ecs_get_type(world, e);
//    int32_t count = ecs_vector_count(type);
//    ecs_entity_t comp_id = 0;
//
//    int32_t i = 0;
//    for (; i < count; i += 1) {
//        comp_id = (ecs_entity_t)ecs_vector_get(type, ecs_entity_t, i);
//        if (!strcmp(name, ecs_get_name(world, comp_id))) {
//            break;
//        }
//    }
//
//    if (i == count) {
//        return NULL;
//    } else {
//        return ecs_get_w_entity(world, e, comp_id);
//    }

void Connection_print(Connection* connection, World* world)
{
    Thing* src = nullptr;
    Thing* dst = nullptr;
    
    world->Thing_try_get(connection->src_thingref, &src);
    world->Thing_try_get(connection->dst_thingref, &dst);
    
    if (src == nullptr) {
        MTT_error("ERROR: could not find source thing with id=[%llu] %d\n", connection->src_thingref, __LINE__);
        return;
    }
    if (dst == nullptr) {
        MTT_error("ERROR: could not find destination thing with id=[%llu] %d\n", connection->dst_thingref, __LINE__);
        return;
    }
    
    cstring src_port_name = mtt::string_get(src->ports.out_ports[connection->header.src_port_ref].tag);
    cstring dst_port_name = mtt::string_get(dst->ports.in_ports[connection->header.dst_port_ref].tag);
    
    MTT_print("Connection {\n"
              "(id:[%llu] arch:[%s] port:[%llu] name:[%s]) ->\n\t(id:[%llu] arch[%s], port:[%llu] name:[%s])\n"
              "}\n",
              connection->src_thingref, ARCHETYPE_NAMES[src->archetype_id], connection->header.src_port_ref, src_port_name,
              connection->dst_thingref, ARCHETYPE_NAMES[dst->archetype_id], connection->header.dst_port_ref, dst_port_name
              );
}

Collision_Handler_Result collision_handler_Tag_Sensor_default(Broad_Phase_Collision_Record* collisions, Collision_System* system)
{
    
    mtt::World* world = system->world;
    usize count =collisions->colliders_against.size();
    Thing_ID thing_id = (Thing_ID)collisions->collider_primary->user_data;
    
    //Collider_print(collisions->collider_primary);
    
    
    Thing* thing = nullptr;
    if (!world->Thing_try_get(thing_id, &thing)) {
        MTT_error("ERROR: could not find Thing in %s\n", __PRETTY_FUNCTION__);
        return Collision_Handler_Result(true);
    }
    
    //mtt::Representation& rep = world->things.representation[thing_id];
    //Collider_List& colliders = rep.colliders;
    
    auto* primary_label = mtt::access<MTT_String_Ref>(thing, "label");
    
    Messages_Record* msgs = messages(&world->message_passer, thing_id);
    
    //Thing_print_verbose(world, thing_id);
    
    switch (collisions->collider_primary->type) {
    case COLLIDER_TYPE_CIRCLE: {
        for (auto it = collisions->colliders_against.cbegin(); it != collisions->colliders_against.cend(); ++it) {
            Collider* const c_against = *it;
            
            Thing_ID against_id = (Thing_ID)c_against->user_data;
            
            if (against_id == 0) {
                MTT_error("%s", "WARNING: collision with unknown thing! Ignored");
                continue;
            }
            
            Thing* against_thing = nullptr;
            if (!world->Thing_try_get(against_id, &against_thing)) {
                continue;
            }
            
            auto* against_label = mtt::access<MTT_String_Ref>(against_thing, "label");
            
            if (!against_label ||
                (!MTT_string_ref_is_equal(*primary_label, *against_label))) {
                continue;
            }
            
            switch (against_thing->archetype_id) {
            default: {
                switch (c_against->type) {
                case COLLIDER_TYPE_CIRCLE: {
                    
                    float32 sqr_dist = 0.0f;
                    if (Circle_Circle_intersection(&collisions->collider_primary->circle, &c_against->circle, &sqr_dist)) {
                        
                        send_message_to_sensor_from_tag(&world->message_passer, (Thing_ID)c_against->user_data, nullptr, msgs);
                    }
                    break;
                }
                case COLLIDER_TYPE_AABB: {
                    // TODO: ... distance
                    
                    if (AABB_Circle_intersection(&c_against->aabb, &collisions->collider_primary->circle)) {
                        send_message_to_sensor_from_tag(&world->message_passer, (Thing_ID)c_against->user_data, nullptr, msgs);
                    }
                    
                    break;
                }
                    // TODO:
                default: { break; }
                }
                
                break;
            }
            }
        }
        break;
    }
    case COLLIDER_TYPE_AABB: {
        // TODO: ...
        break;
    }
    default: { MTT_print("%s", "TODO\n"); break; }
    }
    
    //MTT_print("}\n");
    
    return Collision_Handler_Result(true);
}

void Message_Passer_init(Message_Passer* message_passer, mem::Allocator* allocator)
{
    //mtt::init(&message_passer->messages, *allocator, 0, 128);
}

Messages_Record* messages(Message_Passer* message_passer, Thing* to_thing)
{
    return messages(message_passer, to_thing->id);
}

Messages_Record* messages(Message_Passer* message_passer, Thing_ID to_thing)
{
    if (message_passer->world->things.instances.find(to_thing) ==
        message_passer->world->things.instances.end()) {
        return nullptr;
    }
    
    auto it = message_passer->message_map.find(to_thing);
    if (it == message_passer->message_map.end()) {
        Messages_Record* msgs = &message_passer->message_map.emplace(to_thing, Messages_Record()).first->second;
        msgs->recipient = to_thing;
        
        return msgs;
    }
    return &it->second;
}

void send_message_to_sensor_from_tag(Message_Passer* message_passer, Thing_ID from_tag, Thing_ID to, void* with_contents)
{
    Messages_Record* msgs = mtt::messages(message_passer, to);
    if (msgs == nullptr) {
        return;
    }
    
    return send_message_to_sensor_from_tag(message_passer, from_tag, with_contents, msgs);
}
void send_message_to_sensor_from_entity(Message_Passer* message_passer, Thing_ID from_entity, Thing_ID to, void* with_contents)
{
    Messages_Record* msgs = mtt::messages(message_passer, to);
    if (msgs == nullptr) {
        return;
    }
    
    return send_message_to_sensor_from_entity(message_passer, from_entity, with_contents, msgs);
}

void send_message_to_sensor_from_tag(Message_Passer* message_passer, Thing_ID from_tag, void* with_contents, Messages_Record* record)
{
    record->from_tags.emplace_back((mtt::Message){
        .type = mtt::MESSAGE_TYPE_FROM_TAG,
        .sender = from_tag,
        .contents = with_contents,
    });
}
void send_message_to_sensor_from_entity(Message_Passer* message_passer, Thing_ID from_entity, void* with_contents, Messages_Record* record)
{
    record->from_entities.emplace_back((mtt::Message){
        .type = mtt::MESSAGE_TYPE_FROM_ENTITY,
        .sender = from_entity,
        .contents = with_contents,
    });
}

void send_system_message(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents)
{
    message_passer->system_messages.emplace_back((Message) {
        .sender = from_entity,
        .type = type,
        .contents = with_contents,
        .proc = {}
    });
}

void send_system_message(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents, mtt::Procedure with_callback)
{
    message_passer->system_messages.emplace_back((Message) {
        .sender = from_entity,
        .type = type,
        .contents = with_contents,
        .proc = with_callback
    });
}

void send_message_to_entity(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, Thing_ID to_entity, void* with_contents)
{
    
    Messages_Record* record = messages(message_passer, to_entity);
    if (record == nullptr) {
        return;
    }
    
    record->from_entities.emplace_back((Message){
        .sender = from_entity,
        .type = type,
        .contents = with_contents,
        .proc = {}
    });
}
void send_message_to_entity(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, Thing_ID to_entity, void* with_contents, mtt::Procedure with_callback)
{
    Messages_Record* record = messages(message_passer, to_entity);
    if (record == nullptr) {
        return;
    }
    
    record->from_entities.emplace_back((Message){
        .sender = from_entity,
        .type = type,
        .contents = with_contents,
        .proc = with_callback
    });
}


void send_system_message_deferred(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents)
{
    message_passer->system_messages_deferred.emplace_back((Message) {
        .sender = from_entity,
        .type = type,
        .contents = with_contents,
        .proc = {},
    });
}

void send_system_message_deferred(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents, mtt::Procedure with_callback)
{
    message_passer->system_messages_deferred.emplace_back((Message) {
        .sender = from_entity,
        .type = type,
        .contents = with_contents,
        .proc = with_callback,
    });
}

void send_system_message_deferred_before_script_eval(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents, mtt::Procedure with_callback)
{
    message_passer->system_messages_deferred_before_scripts.emplace_back((Message) {
        .sender = from_entity,
        .type = type,
        .contents = with_contents,
        .proc = with_callback,
    });
}

void send_system_message_deferred_after_script_eval(Message_Passer* message_passer, uint64 type, Thing_ID from_entity, void* with_contents, mtt::Procedure with_callback)
{
    message_passer->system_messages_deferred_after_scripts.emplace_back((Message) {
        .sender = from_entity,
        .type = type,
        .contents = with_contents,
        .proc = with_callback,
    });
}


template <typename Procedure>
void Thing_apply_self_and_attached(mtt::World* world, mtt::Thing_ID thing_id, Procedure proc, void* args)
{
    auto& q = world->traversal_queue;
    
    
    mtt::Thing* th = world->Thing_try_get(thing_id);
    if (th == nullptr) {
        return;
    }
    
    
    thing_id = mtt::get_root(th)->id;
    
    q.emplace_back(thing_id);
    
    while (!q.empty()) {
        mtt::Thing_ID id = q.front();
        q.pop_front();
        
        mtt::Thing* thing = nullptr;
        if (world->Thing_try_get(id, &thing)) {
            //            if (thing->is_visited) {
            //                return;
            //            }
            
            //thing->is_visited = true;
            
            proc(world, thing, args);
            
            for (auto it_children = thing->child_id_set.begin();
                 it_children != thing->child_id_set.end();
                 ++it_children) {
                
                q.emplace_back(*it_children);
            }
        }
    }
}

template <typename Procedure>
void Thing_apply_self_and_attached(mtt::World* world, mtt::Thing* thing, Procedure proc, void* args)
{
    Thing_apply_self_and_attached(world, thing->id, proc, args);
}




template <typename Procedure>
void breadth_first_traverse_thing_hierarchy(mtt::World* world, mtt::Thing_ID thing_id, Procedure proc, void* args)
{
    auto& q = world->traversal_queue;
    q.emplace_back(thing_id);
    
    while (!q.empty()) {
        mtt::Thing_ID id = q.front();
        q.pop_front();
        
        mtt::Thing* thing = nullptr;
        if (world->Thing_try_get(id, &thing)) {
            //            if (thing->is_visited) {
            //                return;
            //            }
            //
            //            thing->is_visited = true;
            //
            proc(world, thing, args);
            
            for (auto it_children = thing->child_id_set.begin();
                 it_children != thing->child_id_set.end();
                 ++it_children) {
                
                q.emplace_back(*it_children);
            }
        }
    }
}

void breadth_first_traverse_thing_hierarchy_w_args(mtt::World* world, mtt::Thing_ID thing_id, void (*proc)(mtt::World* world, mtt::Thing* thing, void* args), void* args)
{
    auto& q = world->traversal_queue;
    q.emplace_back(thing_id);
    
    while (!q.empty()) {
        mtt::Thing_ID id = q.front();
        q.pop_front();
        
        mtt::Thing* thing = nullptr;
        if (world->Thing_try_get(id, &thing)) {
            //            if (thing->is_visited) {
            //                return;
            //            }
            //
            //            thing->is_visited = true;
            //
            proc(world, thing, args);
            
            for (auto it_children = thing->child_id_set.begin();
                 it_children != thing->child_id_set.end();
                 ++it_children) {
                
                q.emplace_back(*it_children);
            }
        }
    }
}

void breadth_first_traverse_thing_arrows_w_args(mtt::World* world, mtt::Thing_ID thing_id, void (*proc)(mtt::World* world, mtt::Thing* thing, void* args), void* args)
{
    auto& q = world->traversal_queue;
    q.emplace_back(thing_id);
    
    auto* arrows = mtt::arrow_links(world);
    
    mtt::Set<mtt::Thing_ID> visited = {};
    
    while (!q.empty()) {
        mtt::Thing_ID id = q.front();
        q.pop_front();
        
        visited.insert(id);
        
        mtt::Thing* thing = nullptr;
        if (world->Thing_try_get(id, &thing)) {
            //            if (thing->is_visited) {
            //                return;
            //            }
            //
            //            thing->is_visited = true;
            //
            
            proc(world, thing, args);
            
            auto fwd_edges = arrows->edges_forward.find(id);
            if (fwd_edges != arrows->edges_forward.end()) {
                auto& children = fwd_edges->second;
                for (auto it_children = children.begin();
                     it_children != children.end();
                     ++it_children) {
                    
                    mtt::Thing_ID child_ID = it_children->id;
                    if (!visited.contains(child_ID)) {
                        q.emplace_back(child_ID);
                    }
                }
            }
        }
    }
}


template <typename Procedure>
mtt::Thing* breadth_first_traverse_thing_hierarchy_with_stopping_condition(mtt::World* world, mtt::Thing_ID thing_id, Procedure proc, void* args)
{
    auto& q = world->traversal_queue;
    q.emplace_back(thing_id);
    
    while (!q.empty()) {
        mtt::Thing_ID id = q.front();
        q.pop_front();
        
        mtt::Thing* thing = nullptr;
        if (world->Thing_try_get(id, &thing)) {
            //            if (thing->is_visited) {
            //                return;
            //            }
            //
            //            thing->is_visited = true;
            //
            if (proc(world, thing, args)) {
                return thing;
            }
            
            for (auto it_children = thing->child_id_set.begin();
                 it_children != thing->child_id_set.end();
                 ++it_children) {
                
                q.emplace_back(*it_children);
            }
        }
    }
    
    return nullptr;
}

void Things_mark_unvisited(mtt::World* world)
{
    auto& things = world->things.instances;
    for (auto it = things.begin(); it != things.end(); ++it) {
        Thing* thing = &it->second;
        
        thing->is_visited = false;
    }
}

void Things_mark_unvisited(mtt::World* world, std::vector<mtt::Thing*>& things)
{
    for (auto it = things.begin(); it != things.end(); ++it) {
        Thing* thing = *it;
        
        thing->is_visited = false;
    }
}

void Things_mark_unvisited(Thing* const things, const usize count)
{
    for (usize i = 0; i < count; i += 1) {
        things[i].is_visited = false;
    }
}


void Thing_destroy_self_and_connected(mtt::World* world, Thing_ID thing_id)
{
    world->thing_buffer.clear();
    Thing_apply_self_and_attached(world, thing_id, [](mtt::World* world, Thing* thing, void* args) {
        world->thing_buffer.push_back(thing);
    }, nullptr);
    
    if (world->no_deletion_zone_on) {
        
        for (usize i = 0; i < world->thing_buffer.size();
             i += 1) {
            
            
            world->to_destroy.emplace_back((Destroy_Command) {
                world->thing_buffer[i]->id,
                true,
                0.0f,
            });
        }
        
    } else {
        for (usize i = 0; i < world->thing_buffer.size();
             i += 1) {
            mtt::Thing_destroy(world->thing_buffer[i]);
        }
    }
    
    world->thing_buffer.clear();
};

void Thing_destroy_if_predicate_true(mtt::World* world, Thing_ID thing_id, bool (*destroy)(mtt::Thing*))
{
    usize el_idx = 0;
    Thing_apply_self_and_attached(world, thing_id, [](mtt::World* world, Thing* thing, void* args) {
        usize* el_idx = static_cast<usize*>(args);
        world->thing_buffer[*el_idx] = thing;
        *el_idx += 1;
    }, &el_idx);
    
    for (usize i = 0; i < el_idx;
         i += 1) {
        
        if (destroy(world->thing_buffer[i])) {
            Thing_destroy(world, world->thing_buffer[i]);
        }
    }
}

bool exists(std::vector<mtt::Thing_ID>& list, mtt::Thing_ID value)
{
    return (std::find(list.begin(), list.end(), value) != list.end());
}

bool exists(mtt::Set<mtt::Thing_ID>& set, mtt::Thing_ID value)
{
    return (set.find(value) != set.end());
}
bool exists(mtt::Set_Stable<mtt::Thing_ID>& set, mtt::Thing_ID value)
{
    return (set.find(value) != set.end());
}

void insert(std::vector<mtt::Thing_ID>& list, mtt::Thing_ID value)
{
    list.emplace_back(value);
}
void insert(mtt::Set<mtt::Thing_ID>& set, mtt::Thing_ID value)
{
    set.emplace(value);
}
void insert(mtt::Set_Stable<mtt::Thing_ID>& set, mtt::Thing_ID value)
{
    set.emplace(value);
}

bool remove(mtt::Set<mtt::Thing_ID>& set, mtt::Thing_ID value)
{
    auto it_found = set.find(value);
    if (it_found != set.end()) {
        set.erase(it_found);
        return true;
    }
    
    return false;
}

bool remove(mtt::Set_Stable<mtt::Thing_ID>& set, mtt::Thing_ID value)
{
    auto it_found = set.find(value);
    if (it_found != set.end()) {
        set.erase(it_found);
        return true;
    }
    
    return false;
}

bool remove(std::vector<mtt::Thing_ID>& list, mtt::Thing_ID value)
{
    auto it = std::find(list.begin(), list.end(), value);
    if (it != list.end()) {
        list.erase(it);
        return true;
    }
    
    return false;
}


void connect_parent_to_child(mtt::World* world, mtt::Thing* parent, mtt::Thing* child)
{
    
    // update parent-child hierarchical connections
    
    bool parent_is_proxy = mtt::Thing_is_proxy(parent);
    bool child_is_proxy = mtt::Thing_is_proxy(child);
//    if (parent_is_proxy) {
//        parent = mtt::Thing_mapped_from_proxy(parent);
//    }
//    if (child_is_proxy) {
//        child = mtt::Thing_mapped_from_proxy(child);
//    }
    if (parent_is_proxy != child_is_proxy) {
        return;
    }
    
    if (mtt::get_parent(child) != nullptr) {
        mtt::disconnect_child_from_parent(world, child);
    }
    
    auto& children = parent->child_id_set;
    
    if (exists(children, child->id)) {
        return;
    }
    insert(children, child->id);
    
    child->parent_thing_id = parent->id;
    
    mtt::Representation* pa_rep = mtt::rep(parent);
    mtt::Representation* ch_rep = mtt::rep(child);
    {
        
        //        std:: cout << "parent translate: " << m::to_string(pa_rep->world_transform[3]) << " ";
        //        std:: cout << "child translate before: " << m::to_string(ch_rep->model_transform[3]) << std::endl;
        
        // child_local_transform = parent_world_transform^-1 * child_local_transform
        ch_rep->model_transform = m::inverse(pa_rep->hierarchy_model_transform) * ch_rep->model_transform;
        ch_rep->model_transform_inverse = m::inverse(ch_rep->model_transform);
        
        //        std:: cout << "child translate after: " << m::to_string(ch_rep->model_transform[3]) << std::endl;
        //        std:: cout << "child translate restored test: " << m::to_string((ch_rep->model_transform * pa_rep->world_transform)[3]) << std::endl;
        
        
        
    }
    
    //child->ecs_entity.add(world->ecs_world.lookup("ChildOfTransitive"), parent->ecs_entity);
}

bool exists_in_other_hierarchy(mtt::Thing* thing, mtt::Thing* other)
{
    if (thing == other) {
        return true;
    }
    // TODO path compression
    
    mtt::Thing* other_root = mtt::get_root(other);
    if (other_root == nullptr) {
        return true;
    }
    
    static auto q = std::deque<mtt::Thing_ID>();
    q.clear();
    q.emplace_back(other_root->id);
    
    while (!q.empty()) {
        mtt::Thing_ID other_id = q.front();
        q.pop_front();
        
        mtt::Thing* cur = nullptr;
        if (thing->world()->Thing_try_get(other_id, &cur)) {
            if (cur == thing) {
                return true;
            }
            
            for (auto it_children = cur->child_id_set.begin();
                 it_children != cur->child_id_set.end();
                 ++it_children) {
                
                q.emplace_back(*it_children);
            }
        }
    }
    return false;
}

bool is_ancestor_of(mtt::Thing* possible_parent, mtt::Thing* thing)
{
    if (possible_parent == thing) {
        return true;
    }
    
    while (!mtt::is_root(thing)) {
        thing->world()->Thing_get(thing->parent_thing_id, &thing);
        if (thing == possible_parent) {
            return true;
        }
    }
    
    return false;
}

void connect_parent_to_child(mtt::World* world, mtt::Thing_ID parent, mtt::Thing_ID child)
{
    mtt::Thing* parent_thing = nullptr;
    mtt::Thing* child_thing  = nullptr;
    
    if (world->Thing_try_get(parent, &parent_thing) &&
        world->Thing_try_get(child, &child_thing)) {
        
        connect_parent_to_child(world, parent_thing, child_thing);
    }
}

void disconnect_child_from_parent(mtt::World* world, mtt::Thing* child_thing)
{
//    if (mtt::Thing_is_proxy(child_thing)) {
//        child_thing = mtt::Thing_mapped_from_proxy(child_thing);
//    }
    
    mtt::Thing* parent_thing = nullptr;
    
    if (mtt::is_root(child_thing) ||
        (!world->Thing_try_get(child_thing->parent_thing_id, &parent_thing))) {
        return;
    }
    
    auto& children = parent_thing->child_id_set;
    
    remove(children, child_thing->id);
    
    child_thing->parent_thing_id = mtt::Thing_ID_INVALID;
    
    mtt::Representation* pa_rep = mtt::rep(parent_thing);
    mtt::Representation* ch_rep = mtt::rep(child_thing);
    {
        
        //        std:: cout << "parent translate: " << m::to_string(pa_rep->world_transform[3]) << " ";
        //        std:: cout << "child translate before: " << m::to_string(ch_rep->model_transform[3]) << std::endl;
        
        //        // child_local_transform = parent_world_transform^-1 * child_local_transform
        //        ch_rep->model_transform = m::inverse(pa_rep->world_transform) * ch_rep->model_transform;
        //        ch_rep->model_transform_inverse = m::inverse(ch_rep->model_transform);
        
        
        
        // child_local_transform = parent_world_transform * child_local_transform
        ch_rep->model_transform = ch_rep->hierarchy_model_transform;
        ch_rep->model_transform_inverse = m::inverse(ch_rep->model_transform);
        ch_rep->hierarchy_model_transform = ch_rep->model_transform;
        //ch_rep->world_transform_inverse = mat4(1.0f);
        //
        //        std:: cout << "child translate after: " << m::to_string(ch_rep->model_transform[3]) << std::endl;
    }
    
    
    child_thing->ecs_entity.remove(world->ecs_world.lookup("ChildOfTransitive"), parent_thing->ecs_entity);
}
void disconnect_child_from_parent(mtt::World* world, mtt::Thing_ID child)
{
    mtt::Thing* child_thing  = nullptr;
    
    if (!world->Thing_try_get(child, &child_thing)) {
        return;
    }
    
    disconnect_child_from_parent(world, child_thing);
}
void disconnect_parent_from_children(mtt::World* world, mtt::Thing* parent)
{
//    if (mtt::Thing_is_proxy(parent)) {
//        parent = mtt::Thing_mapped_from_proxy(parent);
//    }
    
    while (!parent->child_id_set.empty()) {
        usize size_before = parent->child_id_set.size();
        mtt::Thing_ID ch = parent->child_id_set.front();
        
        
        mtt::disconnect_child_from_parent(world, ch);
        
        if (size_before == parent->child_id_set.size()) {
            parent->child_id_set.erase(parent->child_id_set.begin());
        }
    }
}

void remove_thing_from_hierarchy(mtt::World* world, mtt::Thing* thing)
{
    // detach children from the to-remove node
    disconnect_parent_from_children(world, thing);
    // detach the to_remove node from its parent
    disconnect_child_from_parent(world, thing);
}

void add_to_group(mtt::World* world, mtt::Thing* group, mtt::Thing* child)
{
    ASSERT_MSG(group->archetype_id == ARCHETYPE_GROUP, "ERROR: attempted to connect children to a group with thing ID-[%llu], but group was not of archetype ARCHETYPE_GROUP!\n", group->id);
    
    connect_parent_to_child(world, group, child);
}

void remove_from_group(mtt::World* world, mtt::Thing* group, mtt::Thing* child)
{
    ASSERT_MSG(group->archetype_id == ARCHETYPE_GROUP, "ERROR: attempted to connect children to a group with thing ID-[%llu], but group was not of archetype ARCHETYPE_GROUP!\n", group->id);
    
    disconnect_child_from_parent(world, child);
}

bool is_root(mtt::Thing* thing)
{
    return thing->parent_thing_id == mtt::Thing_ID_INVALID;
}

mtt::Thing_ID get_parent_ID(mtt::Thing* thing)
{
    return thing->parent_thing_id;
}
mtt::Thing* get_parent(mtt::Thing* thing)
{
    mtt::Thing_ID parent_id = get_parent_ID(thing);
    if (parent_id == mtt::Thing_ID_INVALID) {
        return nullptr;
    }
    
    return mtt::Thing_try_get(mtt::world(thing), parent_id);
}

bool thing_group_is_active(mtt::Thing* const thing)
{
    mtt::Thing* curr_thing = thing;
    mtt::World* const world = curr_thing->world();
    
    if (!mtt::is_active_group(thing) || !mtt::is_active(thing)) {
        return false;
    }
    static mtt::Set_Stable<mtt::Thing_ID> seen = {};
    seen.clear();
    while (!mtt::is_root(curr_thing)) {
        world->Thing_get(curr_thing->parent_thing_id, &curr_thing);
        if (!mtt::is_active_group(curr_thing) || seen.contains(mtt::thing_id(curr_thing))) {
            return false;
        }
        if (curr_thing == thing) {
            return true;
        }
        seen.insert(mtt::thing_id(curr_thing));
    }
    
    return true;
}

bool thing_group_set_active_state(mtt::Thing* const thing, bool active_state)
{
    mtt::Thing* curr_thing = thing;
    mtt::World* const world = curr_thing->world();
    
    if (thing->archetype_id == ARCHETYPE_GROUP) {
        mtt::set_is_active_group(thing, active_state);
        return true;
    }
    
    while (!is_root(curr_thing)) {
        world->Thing_get(curr_thing->parent_thing_id, &curr_thing);
        if (thing->archetype_id == ARCHETYPE_GROUP) {
            mtt::set_is_active_group(thing, active_state);
            return true;
        }
    }
    
    return false;
}

bool thing_group_set_active_state(mtt::World* world, mtt::Thing_ID const thing_id, bool active_state)
{
    Thing* thing = nullptr;
    if (!world->Thing_try_get(thing_id, &thing)) {
        return false;
    }
    
    return thing_group_set_active_state(thing, active_state);
}

bool is_root(mtt::World* world, mtt::Thing_ID thing_id)
{
    mtt::Thing* thing = nullptr;
    if (world->Thing_try_get(thing_id, &thing)) {
        return mtt::is_root(thing);
    }
    
    return false;
}

mtt::Thing_Archetype* archetype_of(Thing* thing)
{
    Thing_Archetype* ptr;
    map_get(&(thing->world()->archetypes.instances), thing->archetype_id, &ptr);
    
    return ptr;
}

void add_selector(mtt::Thing_Archetype* arch, cstring name, const mtt::Procedure& proc)
{
    map_set(arch->selector_to_proc, mtt::string(name).id, proc);
}
bool add_selector(mtt::World* world, mtt::Thing_Archetype_ID id, cstring name, const mtt::Procedure& proc)
{
    Thing_Archetype* arch = nullptr;
    if (world->Thing_Archetype_try_get(id, &arch)) {
        add_selector(arch, name, proc);
        return true;
    }
    
    return false;
}

void add_own_selector(mtt::Thing* thing, cstring name, const mtt::Procedure& proc)
{
    thing->world()->things.own_selector_to_proc[thing->id][mtt::string(name).id] = proc;
}
void remove_own_selector(mtt::Thing* thing, cstring name)
{
    auto& entry = thing->world()->things.own_selector_to_proc[thing->id];
    auto find = entry.find(string_ref_get(name).id);
    if (find != entry.end()) {
        entry.erase(find);
    }
}


Procedure_Return_Type selector_invoke(mtt::Thing_Archetype* arch, mtt::Thing* thing, Message* message)
{
    mtt::Procedure* proc = nullptr;
    
    mtt::Map<MTT_String_Ref_ID, mtt::Procedure>* own_selectors = nullptr;
    if (!(
          (map_try_get(&thing->world()->things.own_selector_to_proc, thing->id, &own_selectors) && map_try_get(own_selectors, message->selector.id, &proc)) ||
          map_try_get(arch->selector_to_proc, message->selector.id, &proc))) {
              ASSERT_MSG(false, "selector [%s] invalid for Thing_ID [%llu]\n", string_get(message->selector), thing->id);
              return false;
          }
    
    //    struct Procedure_Input_Output {
    //        mtt::Thing* caller;
    //        void*       input;
    //        usize       input_count;
    //        uint64      input_flags;
    //        uint64      output_flags;
    //
    //        mtt::Any    output;
    //    };
    Procedure_Input_Output io = {};
    io.caller = thing;
    io.input = (void*)message;
    auto out_return = (*proc)(&io);
    
    message->output_value = io.output;
    message->flags = io.output_flags;
    
    return out_return;
}

Procedure_Return_Type selector_invoke(mtt::Thing* thing, Message* message)
{
    Thing_Archetype* arch = nullptr;
    if (thing->world()->Thing_Archetype_try_get(thing->archetype_id, &arch)) {
        return selector_invoke(arch, thing, message);
    }
    
    return Procedure_Return_Type(false);
}

//bool has_selector(mtt::Thing* thing, mtt::String selector, mtt::Procedure** proc)
//{
//    if (map_try_get(thing->world()->things.own_selector_to_proc, selector, proc)) {
//        return true;
//    }
//    Thing_Archetype* arch = nullptr;
//    thing->world()->Thing_Archetype_try_get(thing->archetype_id, &arch);
//
//    return map_try_get(arch->selector_to_proc, selector, proc);
//}


#undef WITH_THING

mtt::Thing* get_group(mtt::Thing* thing)
{
    while (!mtt::is_root(thing)) {
        if (thing->archetype_id == ARCHETYPE_GROUP) {
            return thing;
        }
        
        thing->world()->Thing_get(thing->parent_thing_id, &thing);
    }
    
    return (thing->archetype_id == ARCHETYPE_GROUP) ? thing : nullptr;
}

mtt::Thing* get_root_group(mtt::Thing* thing)
{
    mtt::Thing* out_candidate = nullptr;
    while (!mtt::is_root(thing)) {
        if (thing->archetype_id == ARCHETYPE_GROUP) {
            out_candidate = thing;
        }
        
        thing->world()->Thing_get(thing->parent_thing_id, &thing);
    }
    
    return (thing->archetype_id == ARCHETYPE_GROUP) ? thing : out_candidate;
}

mtt::Thing* get_root(mtt::Thing* thing)
{
    mtt::Thing* initial_thing = thing;
    while (!mtt::is_root(thing)) {
        thing->world()->Thing_get(thing->parent_thing_id, &thing);
        if (initial_thing == thing) {
            return nullptr;
        }
    }
    
    return thing;
}

usize get_lowest_child(mtt::Thing* thing, usize depth, usize max_depth, mtt::Thing** candidate)
{
    
    if (thing->child_id_set.size() == 0) {
        if (depth >= max_depth) {
            *candidate = thing;
        }
    } else {
        for (auto it = thing->child_id_set.begin(); it != thing->child_id_set.end(); ++it) {
            mtt::Thing* sub = nullptr;
            thing->world()->Thing_get(sub->id, &sub);
            usize sub_depth = get_lowest_child(sub, depth + 1, max_depth, candidate);
            if (sub_depth > max_depth) {
                max_depth = sub_depth;
            }
        }
    }
    
    return depth;
}


mtt::Thing* get_lowest_child(mtt::Thing* thing)
{
    Thing* out = thing;
    get_lowest_child(thing, 0, 0, &out);
    return out;
}



void Drawable_Instance_Info::init(sd::Drawable_Info* drawable, mem::Allocator allocator)
{
    this->source = drawable;
    this->allocator = allocator;
    
    Pool_Allocation_init(&this->list_pool_alloc, this->allocator, 1024, sizeof(MTT_List_Node), 16);
    Pool_Allocation_init(&this->drawable_pool_alloc, this->allocator, 1024, sizeof(sd::Drawable_Info), 16);
    MTT_List_init_with_allocator(&this->list, &this->list_pool_alloc.allocator);
    
    mtt::init(&this->array, this->allocator);
    
    
    //    return;
    //
    //    this->instances.push_back(mtt::Array<sd::Drawable_Info, 256>());
    //    Array_init(&this->instances.back());
    
}

void Drawable_Instance_Info::deinit(void)
{
    mtt::deinit(&this->array);
    Pool_Allocation_deinit(&this->drawable_pool_alloc);
    Pool_Allocation_deinit(&this->list_pool_alloc);
}


sd::Drawable_Info* Drawable_Instance_Info::create_instance()
{
    {
        sd::Drawable_Info* drawable = static_cast<sd::Drawable_Info*>(this->drawable_pool_alloc.allocator.allocate(&this->drawable_pool_alloc.allocator, sizeof(sd::Drawable_Info)));
        assert(drawable != nullptr);
        
        *drawable = *this->source;
        
        drawable->flags |= 1;
        
        auto* node = MTT_List_Node_make(&this->list, static_cast<void*>(drawable));
        
        
        drawable->is_enabled = true;
        set_transform(drawable, Mat4(1.0f));
        
        
        this->array.push_back(drawable);
        drawable->buffer_index = this->array.size() - 1;
        
        
        MTT_List_append_node(&this->list, node);
        
        return drawable;
    }
    //
    //
    //
    //    if (!free_id_list.empty()) {
    //        uint64 id = free_id_list.back();
    //        free_id_list.pop_back();
    //
    //        // higher bits represent chunk id, lower bits represent element index
    //        uint64 chunk = id >> 32;
    //        uint64 idx = id & 0x00000000ffFFffFF;
    //
    //        auto& drawable = this->instances[chunk][idx];
    //
    //        drawable = *this->source;
    //        drawable.is_enabled = true;
    //        drawable.buffer_index = id;
    //        drawable.transform = Mat4(1.0f);
    //
    //        return &drawable;
    //    }
    //
    //    auto* arr = &this->instances.back();
    //
    //    if (arr->count == arr->cap()) {
    //        this->instances.push_back(mtt::Array<sd::Drawable_Info, 256>());
    //        Array_init(&this->instances.back());
    //        arr = &this->instances.back();
    //    }
    //
    //
    //    append(arr, this->source);
    //    uint64 idx = arr->count - 1;
    //    sd::Drawable_Info* drawable = arr->last_ptr();
    //    *drawable = *this->source;
    //    drawable->is_enabled = true;
    //    drawable->transform = Mat4(1.0f);
    //
    //    uint64 chunk = this->instances.size() - 1;
    //    drawable->buffer_index = (chunk << 32) | idx;
    //
    //
    //    return drawable;
}

void Drawable_Instance_Info::destroy_instance(sd::Drawable_Info* drawable)
{
    drawable->is_enabled = false;
    drawable->flags = 0;
    auto* node = (MTT_List_Node*)((uintptr)drawable->buffer_index);
    
    MTT_List_remove_node(node);
    MTT_List_Node_destroy(&this->list, node);
    
    mtt::unordered_remove(&this->array, drawable->buffer_index);
    
    this->drawable_pool_alloc.allocator.do_deallocate(drawable, sizeof(sd::Drawable_Info));
    
    
    return;
    
    //    this->free_id_list.push_back(drawable->buffer_index);
}

void Thing_Archetype_Drawable_Instances::init(mem::Allocator allocator)
{
    this->allocator = allocator;
}


mtt::Thing* get_parent_with_archetype(mtt::Thing* thing, mtt::Thing_Archetype_ID arch)
{
    while (!mtt::is_root(thing)) {
        if (thing->archetype_id == arch) {
            return thing;
        }
        
        thing->world()->Thing_get(thing->parent_thing_id, &thing);
    }
    
    return (thing->archetype_id == arch) ? thing : nullptr;
}

mtt::Thing* get_first_child_with_archetype(mtt::Thing* thing, mtt::Thing_Archetype_ID arch)
{
    mtt::Thing* found = breadth_first_traverse_thing_hierarchy_with_stopping_condition(thing->world(), thing->id, [](mtt::World* world, mtt::Thing* thing, void* data) -> bool { return thing->archetype_id == (*((mtt::Thing_Archetype_ID*)data)); },(void*)&arch);
    
    return found;
}

void print_rules_with_all_results(mtt::World* world)
{
    
    
}

//static mtt::Map<mtt::String, mtt::Set<mtt::Thing_ID>> _eval_rules_results;
void eval_rules(mtt::World* world)
{
    
}
//{
//    return;
//
//    dt::DrawTalk* dt = dt::DrawTalk::ctx();
//    auto& rules = world->rules.map;
//    for (auto it = rules.begin(); it != rules.end(); ++it) {
//        dt::Rule_Context_old* rctx = &it->second;
//
//        for (usize t = 0; t < rctx->triggers.size(); t += 1) {
//            dt::Trigger_old* trigger = rctx->triggers[t];
//
//            trigger->it = ecs_rule_iter(trigger->ctx);
//        }
//
//        bool more_results = true;
//        usize result_count = 0;
//
//        while (more_results) {
//            for (usize t = 0; t < rctx->triggers.size(); t += 1) {
//                dt::Trigger_old* trigger = rctx->triggers[t];
//
//                if (!ecs_rule_next(&trigger->it) || ecs_rule_variable_count(trigger->ctx) == 0) {
//                    more_results = false;
//                    break;
//                }
//
//
//                for (usize s = 0; s < trigger->symbols.count; s += 1) {
//                    dt::Symbol* sym = &trigger->symbols[s];
//
//                    auto result_for_variable = ecs_rule_variable(&trigger->it, sym->rule_symbol_id);
//                    auto ent = flecs::entity(trigger->it.world, result_for_variable);
//                    if (ent.has(flecs::Prefab)) {
//                        int BLA_1_5 = 0;
//                        break;
//                    }
//
//                    //MTT_print("\t\t%s: [%s]\n", sym->key.c_str(), ent.path().c_str());
//                    int BLA = 2;
//
//                    sym->id = mtt::Thing_ID_INVALID;
//
//                    if (ent.has<mtt::Thing_Info>()) {
//                        mtt::Thing* thing = nullptr;
//                        world->Thing_get(ent.get<mtt::Thing_Info>()->thing_id, &thing);
//                        sym->id = thing->id;
//
//                        {
//                            uint64* response_symbol_id = nullptr;
//                            if (map_try_get(&rctx->response.name_to_symbol_idx, sym->key, &response_symbol_id)) {
//                                dt::Symbol& response_symbol = rctx->response.symbols[*response_symbol_id];
//                                response_symbol.id = thing->id;
//                            }
//                        }
//
//                    }
//                }
//            }
//
//            if (!more_results) {
//                continue;
//            }
//
//            //TODO: need to put all results in a single list per role
//            auto* response = &rctx->response;
//            MTT_print("Response action = %s\n", response->key.c_str());
//
//            MTT_Tree_Node* found = dt::search(MTT_Tree_root(&dt->sys_tree), "en").search(response->key);
//            if (found != NULL) {
//                MTT_print("Found action!\n");
//            }
//
//
//
//            for (auto role_it = response->role_to_symbol_idx.begin();
//                 role_it != response->role_to_symbol_idx.end();
//                 ++role_it) {
//
//
//
//                auto& syms = role_it->second;
//                const mtt::String& role_name = role_it->first;
//                auto& role_result_list = response->role_to_thing_id[role_name];
//                role_result_list.clear();
//
//                MTT_print("role: %s\n", role_name.c_str());
//
//
//                for (auto sym_idx_it = syms.begin(); sym_idx_it != syms.end(); ++sym_idx_it) {
//                    uint64 sym_idx = *sym_idx_it;
//
//                    dt::Symbol* sym = &response->symbols[sym_idx];
//
////                    ASSERT_MSG(sym->id != mtt::Thing_ID_INVALID, "Should not have an invalid id!\n");
//
//                    auto find_triggers = rctx->symbol_name_to_trigger.find(sym->key);
//                    if (find_triggers == rctx->symbol_name_to_trigger.end()) {
//                        continue;
//                    }
//
//                    auto& triggers = find_triggers->second;
//                    for (auto tr_it = triggers.begin(); tr_it != triggers.end(); ++tr_it) {
//                        dt::Trigger_old* trigger = (*tr_it);
//                        dt::Symbol* matched_sym = &(trigger->symbols[(trigger->name_to_symbol_idx[sym->key])]);
//
//                        ASSERT_MSG(matched_sym->id != mtt::Thing_ID_INVALID, "Should not have an invalid id");
//
//                    {
////                        dt::Symbol* matched_sym = sym;
//
//                        role_result_list.push_back(matched_sym->id);
//                        if (response->key.compare("catch_fire") == 0) {
//                            mtt::Thing* source = nullptr;
//                            world->Thing_try_get(matched_sym->id, &source);
//                            if (source->is_being_destroyed) {
//                                continue;
//                            }
//
//                            mtt::Rep* rep;
//                            mtt::rep(source, &rep);
//
//                            {
//                                auto& drawables = rep->render_data.drawable_info_list;
//                                for (usize d_idx = 0; d_idx < drawables.size(); d_idx += 1) {
//                                    drawables[d_idx]->color_factor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
//                                    drawables[d_idx]->color_addition = vec4(1.0f, 0.4f, 0.0f, 0.8f);
//                                }
//                            }
//
//                            mtt::Thing* timer_test = mtt::Thing_make(world, mtt::ARCHETYPE_TIMER);
//                            *mtt::access<float32>(timer_test, "initial_time") = 0.0f;
//                            *mtt::access<float32>(timer_test, "duration")     = 3.0f;
//                            *mtt::access<float32>(timer_test, "elapsed")      = 0.0f;
//                            *mtt::access<float32>(timer_test, "speed")        = 200.0f;
//
//                            mtt::Thing* destroyer_test = mtt::Thing_make(world, mtt::ARCHETYPE_DESTROYER);
//
//                            *mtt::access<float32>(destroyer_test, "time_delay") = 3.0f;
//
//                            {
//                                bool ok = mtt::add_connection(world, destroyer_test, "target", source, "destroy");
//                                ASSERT_MSG(ok, "connection should've worked!\n");
//                            }
//                            //        {
//                            //            bool ok = mtt::add_connection(&world->mtt_world, timer_test, "result", freehand_test, "color_factor");
//                            //            ASSERT_MSG(ok, "connection should've worked!\n");
//                            //        }
//                            {
//                                bool ok = mtt::add_connection(world, timer_test, "result", destroyer_test, "trigger");
//                                ASSERT_MSG(ok, "connection should've worked!\n");
//                            }
//
//
//
//                            world->to_destroy.emplace_back((Destroy_Command){
//                                matched_sym->id,
//                                true,
//                                .do_fade_animation = true,
//                                .time_remaining = 1.0f,
//                                .time_delay = 1.0f,
//                            });
//                            source->is_being_destroyed = true;
//
//
//                        } else if (response->key.compare("explode") == 0) {
//
//                            mtt::Thing* thing_to_explode = nullptr;
//                            world->Thing_try_get(matched_sym->id, &thing_to_explode);
//                            if (thing_to_explode->is_being_destroyed) {
//                                continue;
//                            }
//                            thing_to_explode->is_being_destroyed = true;
//
//                            vec3 position = *mtt::access<vec3>(thing_to_explode, "position");
//
//
//                            mtt::Thing* timer_test = mtt::Thing_make(world, mtt::ARCHETYPE_TIMER);
//                            *mtt::access<float32>(timer_test, "initial_time") = 0.0f;
//                            *mtt::access<float32>(timer_test, "duration")     = 200.0f;
//                            *mtt::access<float32>(timer_test, "elapsed")      = 0.0f;
//                            *mtt::access<float32>(timer_test, "speed")        = 200.0f;
//
//
//                            mtt::Rep* rep;
//                            mtt::Thing* freehand_test = mtt::Thing_make_with_unit_collider(world, mtt::ARCHETYPE_FREEHAND_SKETCH, vec2(1.0f), &rep, mtt::COLLIDER_TYPE_AABB, position);
//                            freehand_test->is_user_drawable = false;
//                            freehand_test->is_locked = true;
//                            rep->pose_scale = vec3(200.0f, 200.0f, 1.0f);
//                            freehand_test->is_being_destroyed = true;
//
//
//                            vis_word_derive_from(freehand_test, dt::noun_lookup("explosion"));
//
//                            sd::set_render_layer(world->renderer, LAYER_LABEL_DYNAMIC);
//
//                            sd::begin_polygon(world->renderer);
//                            {
//                                sd::set_color_rgba_v4(world->renderer, vec4(1.0f, 0.0f, 0.0f, 0.8f));
//                                //sd::rectangle(renderer, vec2(-0.5f), vec2(1.0f), 0.0f);
//
//                                sd::polygon_convex_regular(world->renderer, 0.5f, vec3(0.0f, 0.0f, 0.0f), 32);
//                            }
//                            auto* d = sd::end_polygon(world->renderer);
//                            rep->render_data.drawable_info_list.push_back(d);
//
//                            sd::set_color_rgba_v4(world->renderer, vec4(1.0f));
//
//                                mtt::Rep* rep2;
//                                mtt::Thing* freehand_test2 = mtt::Thing_make_with_unit_collider(world, mtt::ARCHETYPE_FREEHAND_SKETCH, vec2(1.0f), &rep2, mtt::COLLIDER_TYPE_AABB, position);
//                                freehand_test2->is_user_drawable = false;
//                                freehand_test2->is_locked = true;
//                                freehand_test2->is_being_destroyed = true;
//                                rep2->pose_scale = vec3(4000.0f, 10.0f, 1.0f);
//
//                                sd::begin_polygon(world->renderer);
//                                {
//                                    sd::set_color_rgba_v4(world->renderer, vec4(1.0f, 1.0f, 1.0f, 1.0f));
//                                    sd::rectangle(world->renderer, vec2(-0.5f), vec2(1.0f), 0.0f);
//                                }
//
//                                auto* d2 = sd::end_polygon(world->renderer);
//                                rep2->render_data.drawable_info_list.push_back(d2);
//
//
//
//
//                            mtt::Thing* destroyer_test = mtt::Thing_make(world, mtt::ARCHETYPE_DESTROYER);
//
//                            *mtt::access<float32>(destroyer_test, "time_delay") = 3.0f;
//                            *mtt::access<bool>(destroyer_test, "fade_effect") = true;
//
//                            {
//                                bool ok = mtt::add_connection(world, destroyer_test, "target", freehand_test, "destroy");
//                                ASSERT_MSG(ok, "connection should've worked!\n");
//                            }
//                            {
//                                bool ok = mtt::add_connection(world, destroyer_test, "target", freehand_test2, "destroy");
//                                ASSERT_MSG(ok, "connection should've worked!\n");
//                            }
//                            {
//                                bool ok = mtt::add_connection(world, timer_test, "result", freehand_test, "scale");
//                                ASSERT_MSG(ok, "connection should've worked!\n");
//                            }
//                            {
//                                bool ok = mtt::add_connection(world, timer_test, "result", freehand_test2, "scale");
//                                ASSERT_MSG(ok, "connection should've worked!\n");
//                            }
//                            //        {
//                            //            bool ok = mtt::add_connection(&world->mtt_world, timer_test, "result", freehand_test, "color_factor");
//                            //            ASSERT_MSG(ok, "connection should've worked!\n");
//                            //        }
//                            {
//                                bool ok = mtt::add_connection(world, timer_test, "result", destroyer_test, "trigger");
//                                ASSERT_MSG(ok, "connection should've worked!\n");
//                            }
//
//                            Destroy_Command destroy_cmd = {};
//                            destroy_cmd.affects_connected = true;
//                            destroy_cmd.do_fade_animation = true;
//                            destroy_cmd.thing_id = matched_sym->id;
//                            world->to_destroy.emplace_back(destroy_cmd);
//                        } else {
//                            int BPBP = 0;
//                        }
//
//
//                    }
//                    }
//
//
//                }
//            }
//        }
//
//    }
//
//
//
//    // TODO: ...
//}

mtt::World* mtt_world_global_ctx_;

void set_ctx(mtt::World* world)
{
    mtt_world_global_ctx_ = world;
}

mtt::World* ctx() { return mtt_world_global_ctx_; }

mtt::Runtime* mtt::Runtime::ctx(void)
{
    return &mtt::ctx()->runtime;
}



void Thing_set_position(mtt::Thing* thing, vec3 position)
{
    mtt::World* world = mtt::world(thing);
    mtt::Rep* rep;
    mtt::rep(thing, &rep);
    
    Mat4 pos_transform = m::translate(Mat4(1.0f), position);
    mtt::Thing* parent = nullptr;
    
    
    
    
    
    if (!mtt::is_root(thing) &&
        world->Thing_try_get(mtt::get_parent_ID(thing), &parent)) {
        
        mtt::Rep* pa_rep = nullptr;
        mtt::rep(parent, &pa_rep);
        rep->model_transform = m::inverse(pa_rep->hierarchy_model_transform) * pos_transform;
        rep->hierarchy_model_transform = pa_rep->hierarchy_model_transform * rep->model_transform;
    } else {
        rep->model_transform = pos_transform;
        rep->hierarchy_model_transform = rep->model_transform;
    }
    rep->model_transform_inverse = m::inverse(rep->model_transform);
    
    vec3 d_scale;
    quat d_orientation;
    vec3 d_translation;
    vec3 d_skew;
    vec4 d_perspective;
    {
        m::decompose(rep->hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
    }
    
    //rep->world_transform_inverse = rep->world_transform_inverse;
    
    auto* pos_ptr = mtt::access<vec3>(thing, "position");
    if (pos_ptr != nullptr) {
        *pos_ptr = position;
    }
    
    // update colliders
    for (usize i = 0; i < rep->colliders.size(); i += 1) {
        mtt::Collider* c = rep->colliders[i];
        ASSERT_MSG(c->type == mtt::COLLIDER_TYPE_AABB, "only support AABB colliders for now here %s\n", __PRETTY_FUNCTION__);
        mtt::Collider_remove(c->system, 0, c);
        
        c->center_anchor = position;
        c->transform = rep->pose_transform * m::toMat4(d_orientation);;
        mtt::push_AABBs(c->system, c, 1, 0);
    }
    
    for (usize i = 0; i < rep->render_data.drawable_info_list.size(); i += 1) {
        auto* drawable = rep->render_data.drawable_info_list[i];
        sd::set_transform(drawable, rep->hierarchy_model_transform * rep->pose_transform * m::translate(rep->offset));
    }
    
    rep->transform.translation = position;
    
}

mtt::Thing* Thing_set_position(mtt::World* world, mtt::Thing_ID id, vec3 position)
{
    mtt::Thing* thing = nullptr;
    if (!world->Thing_try_get(id, &thing)) {
        return nullptr;
    }
    
    Thing_set_position(thing, position);
    
    return thing;
}


void set_pose_transform(mtt::Thing* thing, const Mat4* mat)
{
    set_pose_transform(thing, *mat);
}

void set_pose_transform(mtt::Thing* thing, const Mat4& mat)
{
    mtt::Rep* rep;
    mtt::rep(thing, &rep);
    rep->pose_transform = mat;
    
    
    mtt::World* world = thing->world();
    
    vec3 d_scale;
    quat d_orientation;
    vec3 d_translation;
    vec3 d_skew;
    vec4 d_perspective;
    {
        m::decompose(rep->hierarchy_model_transform, d_scale, d_orientation, d_translation, d_skew, d_perspective);
    }
    
    vec3 p_scale;
    quat p_orientation;
    vec3 p_translation;
    vec3 p_skew;
    vec4 p_perspective;
    {
        m::decompose(rep->pose_transform, p_scale, p_orientation, p_translation, p_skew, p_perspective);
    }
    
    rep->pose_transform_values.translation = p_translation;
    rep->pose_transform_values.rotation = p_orientation;
    rep->pose_transform_values.scale    = p_scale;
    /*
     
     */
    for (usize i = 0; i < rep->colliders.size(); i += 1) {
        mtt::Collider* c = rep->colliders[i];
        mtt::Collider_remove(c->system, 0, c);
    }
    for (usize i = 0; i < rep->colliders.size(); i += 1) {
        mtt::Collider* c = rep->colliders[i];
        c->transform = rep->pose_transform * (
                                              ((rep->forward_dir.x < 0) ? m::inverse(m::toMat4(d_orientation)) : m::toMat4(d_orientation))
                                              );
        
        //ASSERT_MSG(c->type == mtt::COLLIDER_TYPE_AABB, "only support AABB colliders for now!\n");
        
        mtt::push_AABBs(c->system, c, 1, 0);
    }
    
    //rep->pose_transform_original        = rep->pose_transform;
//    rep->pose_transform_values_original = rep->pose_transform_values;
}

mtt::Thing* Thing_make_with_unit_collider(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, mtt::COLLIDER_TYPE collider_type, vec3 in_position, bool is_in_world, mtt::Collision_System* collision_system)
{
    mtt::Thing* thing = mtt::Thing_make_with_collider(world, rep_out, (mtt::ARCHETYPE)arch, collider_type, is_in_world, collision_system);
    
    mtt::Rep* rep = *rep_out;
    
    
    rep->pose_transform  = m::scale(Mat4(1.0f), vec3(scale_2d, 1.0f));
    
    rep->pose_transform_values.scale = vec3(scale_2d, 1.0f);
    
    for (usize i = 0; i < rep->colliders.size(); i += 1) {
        mtt::Collider* c = rep->colliders[i];
        c->aabb.half_extent = vec2(0.5f);
        c->aabb.tl = vec2(-0.5f, -0.5f);
        c->aabb.br = vec2(0.5f, 0.5f);
        c->center_anchor = in_position;
        c->transform = rep->pose_transform;
        mtt::push_AABBs(c->system, c, 1, 0);
    }
    
    auto* position = mtt::access<vec3>(thing, "position");
    if (position != nullptr) {
        *position = in_position;
    }
    rep->transform.translation = in_position;
    
    
    mtt::Thing_set_position(thing, in_position);
    
    
    return thing;
    
}

mtt::Thing* Thing_make_with_aabb_dimensions(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, vec3 in_position, vec2 dimensions, bool is_in_world, mtt::Collision_System* collision_system)
{
    mtt::Thing* thing = mtt::Thing_make_with_collider(world, rep_out, (mtt::ARCHETYPE)arch, mtt::COLLIDER_TYPE_AABB, is_in_world, collision_system);
    
    mtt::Rep* rep = *rep_out;
    
    
    rep->pose_transform  = m::scale(Mat4(1.0f), vec3(scale_2d, 1.0f));
    
    rep->pose_transform_values.scale = vec3(scale_2d, 1.0f);
    
    vec2 half_extent = dimensions / 2.0f;
    
    auto tl = vec2(in_position);
    auto br = vec2(in_position) + dimensions;
    auto center = vec3((tl + br) / 2.0f, in_position.z);
    
    for (usize i = 0; i < rep->colliders.size(); i += 1) {
        mtt::Collider* c = rep->colliders[i];
        c->aabb.half_extent = half_extent;
        c->aabb.tl = tl;
        c->aabb.br = br;
        
        c->center_anchor = center;
        c->transform = rep->pose_transform;
        mtt::push_AABBs(c->system, c, 1, 0);
    }
    
    auto* position = mtt::access<vec3>(thing, "position");
    if (position != nullptr) {
        *position = center;
    }
    rep->transform.translation = center;
    
    
    mtt::Thing_set_position(thing, center);
    
    
    return thing;
    
}


mtt::Thing* Thing_make_with_aabb_corners(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, mtt::Box box, float32 z_position, bool is_in_world, mtt::Collision_System* collision_system)
{
    mtt::Thing* thing = mtt::Thing_make_with_collider(world, rep_out, (mtt::ARCHETYPE)arch, mtt::COLLIDER_TYPE_AABB, is_in_world, collision_system);
    
    mtt::Rep* rep = *rep_out;
    
    
    rep->pose_transform  = m::scale(Mat4(1.0f), vec3(scale_2d, 1.0f));
    
    rep->pose_transform_values.scale = vec3(scale_2d, 1.0f);
    
    
    auto center      = (box.tl + box.br) / 2.0f;
    auto half_extent = (box.br - box.tl) / 2.0f;
    auto center_3d = vec3(center, z_position);
    
    for (usize i = 0; i < rep->colliders.size(); i += 1) {
        mtt::Collider* c = rep->colliders[i];
        c->aabb.half_extent = half_extent;
        c->aabb.tl = box.tl;
        c->aabb.br = box.br;
        
        c->center_anchor = center_3d;
        c->transform = rep->pose_transform;
        mtt::push_AABBs(c->system, c, 1, 0);
    }
    
    
    auto* position = mtt::access<vec3>(thing, "position");
    if (position != nullptr) {
        *position = center_3d;
    }
    rep->transform.translation = center_3d;
    
    
    mtt::Thing_set_position(thing, center_3d);
    
    
    return thing;
    
}

void Thing_set_aabb_corners(mtt::Thing* thing, mtt::Rep** rep_out, mtt::Box box, float32 z_position)
{
    ASSERT_MSG(*rep_out != nullptr, "representation should exist!\n");
    
    
    
//    Rep rep_new = Representation();
//    rep_new.colliders = (*rep_out)->colliders;
//    **rep_out = rep_new;
    
    Rep* rep = *rep_out;
    
    
//    rep->pose_transform  = m::scale(Mat4(1.0f), vec3(vec2(1.0f), 1.0f));
//
//    rep->pose_transform_values.scale = vec3(vec2(1.0f), 1.0f);
    
    
    auto center      = (box.tl + box.br) / 2.0f;
    auto half_extent = (box.br - box.tl) / 2.0f;
    auto center_3d = vec3(center, z_position);
    
    for (usize i = 0; i < rep->colliders.size(); i += 1) {
        mtt::Collider* c = rep->colliders[i];
        mtt::Collider_remove(c->system, 0, c);
        
        c->aabb.half_extent = half_extent;
        c->aabb.tl = box.tl;
        c->aabb.br = box.br;
        
        c->center_anchor = center_3d;
        c->transform = rep->pose_transform;
        mtt::push_AABBs(c->system, c, 1, 0);
    }
    
    
    auto* position = mtt::access<vec3>(thing, "position");
    if (position != nullptr) {
        *position = center_3d;
    }
    rep->transform.translation = center_3d;
    
    mtt::Thing_set_position(thing, center_3d);
}

//void Thing_set_aabb_corners(mtt::Thing* thing, mtt::Box box, float32 z_position)
//{
//    mtt::Rep* rep = mtt::rep(thing);
//
//    auto center      = (box.tl + box.br) / 2.0f;
//    auto half_extent = (box.br - box.tl) / 2.0f;
//    auto center_3d = vec3(center, z_position);
//
//    for (usize i = 0; i < rep->colliders.size(); i += 1) {
//        mtt::Collider* c = rep->colliders[i];
//        if (mtt::Collider_is_pushed(c)) {
//            mtt::Collider_remove(c->system, 0, c);
//        }
//
//        c->aabb.half_extent = half_extent;
//        c->aabb.tl = box.tl;
//        c->aabb.br = box.br;
//
//        c->center_anchor = center_3d;
//        c->transform = rep->pose_transform;
//        mtt::push_AABBs(c->system, c, 1, 0);
//    }
//
//
//    auto* position = mtt::access<vec3>(thing, "position");
//    if (position != nullptr) {
//        *position = center_3d;
//    }
//    rep->transform.position = center_3d;
//
//
//    mtt::Thing_set_position(thing, center_3d);
//}

void Thing_set_aabb_dimensions(mtt::Thing* thing, vec2 scale_2d, mtt::Rep** rep_out, vec3 in_position, vec2 dimensions)
{
    ASSERT_MSG(*rep_out != nullptr, "representation should exist!\n");
    
    
    
//    Rep rep_new = Representation();
//    rep_new.colliders = (*rep_out)->colliders;
//    **rep_out = rep_new;
    
    Rep* rep = *rep_out;
    
    
    rep->pose_transform  = m::scale(Mat4(1.0f), vec3(scale_2d, 1.0f));
    
    rep->pose_transform_values.scale = vec3(scale_2d, 1.0f);
    
    vec2 half_extent = dimensions / 2.0f;
    
    
    auto tl = vec2(in_position);
    auto br = vec2(in_position) + dimensions;
    auto center = vec3((tl + br) / 2.0f, in_position.z);
    
    for (usize i = 0; i < rep->colliders.size(); i += 1) {
        mtt::Collider* c = rep->colliders[i];
        mtt::Collider_remove(c->system, 0, c);
        
        c->aabb.half_extent = half_extent;
        c->aabb.tl = tl;
        c->aabb.br = br;
        
        c->center_anchor = center;
        c->transform = rep->pose_transform;
        mtt::push_AABBs(c->system, c, 1, 0);
    }
    
    
    auto* position = mtt::access<vec3>(thing, "position");
    if (position != nullptr) {
        *position = center;
    }
    rep->transform.translation = center;
    
    mtt::Thing_set_position(thing, center);
}



Thing_Iterator iterator(mtt::World* world)
{
    // TODO:
    Thing_Iterator it = {};
    it.things  = &world->things.things;
    it.idx     = 0;
    it.count_remaining = 0; // The number of resident Things
    
    it.it = world->things.instances.begin();
    it.world = world;
    it.value = nullptr;
    
    it.count_remaining = Thing_count(world);
    usize page_count = it.things->count;
    it.max_idx = ((THING_PAGE_SIZE * (page_count - 1)) + it.things->last_ptr()->count) - 1;
    
    return it;
}

bool next(Thing_Iterator* it)
{
    if (it->it == it->world->things.instances.end()) {
        it->value = nullptr;
        return false;
    } else {
        it->value = &it->it->second;
        ++it->it;
        it->count_remaining -= 1;
        return true;
    }
    
    usize idx = it->idx;
    
    mtt::Thing* thing = nullptr;
    
    const usize count_remaining = it->count_remaining;
    do {
        idx += 1;
        
        if (idx > count_remaining || idx > it->max_idx) {
            return false;
        }
        
        const usize offset   = idx & 0xFFF;
        const usize page_idx = idx >> 12;
        
        thing = &((*it->things)[page_idx][offset]);
        
    } while (!thing->is_resident);
    
    it->idx = idx;
    it->count_remaining -= 1;
    
    return true;
}

void Thing_get_properties_for_attribute(mtt::Thing* src, dt::Word_Dictionary_Entry* entry, my::Function<void(mtt::Thing*, dt::Word_Dictionary_Entry*, dt::Word_Dictionary_Entry*, const mtt::Any*)> proc)
{

    auto* val = src->ecs_entity.get<mtt::Any>(entry->typename_desc);
    if (val == nullptr) {
        const mtt::Any val = {};
        auto properties_for_src = entry->properties.find(src->id);
        if (properties_for_src != entry->properties.end()) {
            for (const auto& prop : properties_for_src->second) {
                proc(src, entry, prop, &val);
            }
        }
        
        return;
    }
    
    auto properties_for_src = entry->properties.find(src->id);
    if (properties_for_src != entry->properties.end()) {
        for (const auto& prop : properties_for_src->second) {
            proc(src, entry, prop, val);
        }
    }
}

std::vector<Attrib_Record> attribs = {};

mtt::String attrib_indent(uint32 lvl) {
    return mtt::String(lvl * 2u, ' ');
}


void attrib_visit_components(const flecs::entity& e, uint32 lvl) {
#ifdef MTT_FLECS_NEW
//    // Iterate all components of entity
//    e.each([&](flecs::id& id) {
//        // Skip IsA relations
//        if (id.has_relation(flecs::IsA)) {
//            return;
//        }
//        
//        std::cout << attrib_indent(lvl) << " - ";
//        
//        // Print role, if id has one
//        if (id.has_role()) {
//            std::cout << id.role_str() << " | ";
//        }
//        
//        // Print relation, if id has one
//        if (id.is_pair()) {
//            std::cout << "(" << id.relation().name() << id.object().name() << ")";
//        } else {
//            std::cout << id.object().name();
//        }
//        
//        std::cout << std::endl;
//    });
#else
    // Iterate all components of entity
    e.each([&](flecs::id& id) {
        // Skip IsA relations
        if (id.has_relation(flecs::IsA)) {
            return;
        }
        
        std::cout << attrib_indent(lvl) << " - ";
        
        // Print role, if id has one
        if (id.has_role()) {
            std::cout << id.role_str() << " | ";
        }
        
        // Print relation, if id has one
        if (id.is_pair()) {
            std::cout << "(" << id.relation().name() << id.object().name() << ")";
        } else {
            std::cout << id.object().name();
        }
        
        std::cout << std::endl;
    });
#endif
}

// Recursively visit IsA relationships

void attrib_visit_is_a(const flecs::entity& e, dt::Word_Dictionary_Entry* entry, uint32 lvl) {
    // Iterate all IsA relationships
    e.each(flecs::IsA, [&](const flecs::id& id) {
        if (id.entity().owns(ctx()->IS_ATTRIBUTE_TAG)) {
            
            //std::cout << attrib_indent(lvl) << "(IsA, " << object.name() << ")" << std::endl;
            
            attribs.push_back(
                              {
                                  .attrib = id.entity(),
                                  
                              });
        }
        attrib_visit_is_a(id.entity(), entry, lvl + 1);
        //attrib_visit_components(object, lvl + 1);
    });
}

//int main(int argc, char *argv[]) {
//    flecs::world ecs(argc, argv);
//
//    // SpaceShip state machine
//    auto ShipStateType = ecs.type("ShipState")
//        .add<ShipState::Idle>()
//        .add<ShipState::Flying>()
//        .add<ShipState::Fighting>();
//
//    // Base SpaceShip
//    auto SpaceShip = ecs.prefab("SpaceShip")
//        .set<MaxVelocity>({100})
//        .set_owned<Position>({0, 0})
//        .set_owned<Velocity>({0, 0})
//        .add<CanFly>()
//        .add_switch(ShipStateType)
//            .add_case<ShipState::Idle>();
//
//    // Frigate
//    auto Frigate = ecs.prefab("Frigate")
//        .add(IsA, SpaceShip)
//        .set<Attack>({100})
//        .set<Defense>({75})
//        .set<MaxVelocity>({150})
//        .add<CanFight>();
//
//    // Heavy Frigate
//    auto HeavyFrigate = ecs.prefab("HeavyFrigate")
//        .add(IsA, Frigate)
//        .set<Attack>({150});
//
//    // Frigate instance
//    auto Rocinante = ecs.entity("Roci")
//        .add(IsA, HeavyFrigate)
//        .set<Position>({10, 20})
//        .set<Velocity>({1, 1});
//
//
//    // Start printing contents of Rocinante
//    cout << endl;
//    cout << Rocinante.name() << ": " << endl;
//
//    // First, visit the IsA relationships to print the inheritance tree
//    visit_is_a(Rocinante);
//
//    // Print components of Rocinante itself
//    cout << endl;
//    cout << "Own components: " << endl;
//    visit_components(Rocinante);
//    cout << endl;
//


std::vector<Attrib_Record>& get_word_attributes(mtt::Thing* thing)
{
    attribs.clear();
    
    auto* word_dict = dt::word_dict();
    
    mtt::Set_Stable<dt::Word_Dictionary_Entry*>* word_entries = nullptr;
    mtt::map_try_get(&word_dict->thing_to_word, thing->id, &word_entries);
    if (word_entries == nullptr) {
        return attribs;
    }
    for (auto it = word_entries->begin(); it != word_entries->end(); ++it) {
        dt::Word_Dictionary_Entry* entry = *it;
        if (entry == nullptr) {
            MTT_error("%s", "ERROR: no word record\n");
        }
        

        mtt::attrib_visit_is_a(entry->typename_desc, entry);
    }
    
    return attribs;
}


bool is_user_movable(mtt::Thing* thing)
{
    return thing->is_user_movable && (mtt::is_root(thing) || !thing->lock_user_movement_if_not_root);
}

void frame_begin(World* world, bool do_clear)
{
    if (do_clear) {
        world->allocator_temporary_begin_frame();
    }
    world->instancing.clear();
}

void frame_end(World* world, bool do_clear)
{
    qt_cleanup(&world->collision_system.spatial_index);
    for (auto id : world->things.to_unmark_for_moving) {
        auto find_it = world->things.marked_for_moving.find(id);
        if (find_it != world->things.marked_for_moving.end()) {
            world->things.marked_for_moving.erase(find_it);
        }
    }
    world->things.to_unmark_for_moving.clear();
}

void set_movement_mode(mtt::Thing* thing, uint64 flag)
{
    thing->logic.option_flags &= ~(1UL);
    thing->logic.option_flags |= flag;
}

bool Thing_marked_for_moving(mtt::Thing* thing, UI_Key key)
{
    if (thing->archetype_id == mtt::ARCHETYPE_UI_ELEMENT) {
        return true;
    }
    auto& marked_for_moving = mtt::world(thing)->things.marked_for_moving;
    auto find_it = marked_for_moving.find(thing->id);
    if (find_it == marked_for_moving.end()) {
        marked_for_moving.insert({thing->id, key});
        return true;
    } else if (find_it->second == key) {
        return true;
    }
    
    return false;
}
bool Thing_unmark_for_moving(mtt::Thing* thing, UI_Key key)
{
    if (thing->archetype_id == mtt::ARCHETYPE_UI_ELEMENT) {
        return true;
    }
    auto& things = mtt::world(thing)->things;
    auto& marked_for_moving = things.marked_for_moving;
    auto find_it = marked_for_moving.find(thing->id);
    if (find_it == marked_for_moving.end()) {
        return false;
    } else if (find_it->second == key) {
        things.to_unmark_for_moving.insert(thing->id);
        
//        {
//            auto* world = mtt::world(thing);
//            for (auto id : world->things.to_unmark_for_moving) {
//                auto find_it = world->things.marked_for_moving.find(id);
//                if (find_it != world->things.marked_for_moving.end()) {
//                    world->things.marked_for_moving.erase(find_it);
//                }
//            }
//            world->things.to_unmark_for_moving.clear();
//        }
        return true;
    }
    
    return false;
}

Render_Data::Render_Data() :
is_shared(false),
_on_copy_inner(nullptr)
{}

void World::clear_all_of_type(mtt::ARCHETYPE arch)
{
    for (auto it = mtt::iterator(this); mtt::next(&it);) {
        if (it.value->archetype_id != arch || !it.value->is_user_destructible) {
            continue;
        }
        
        mtt::Thing_ID ID = it.value->id;
        
        mtt::Destroy_Command cmd = {};
        cmd.thing_id = ID;
        this->to_destroy.push_back(cmd);
    }
}



void World::clear_all_of_type_ignore_flags(mtt::ARCHETYPE arch)
{
    for (auto it = mtt::iterator(this); mtt::next(&it);) {
        if (it.value->archetype_id != arch) {
            continue;
        }
        
        mtt::Thing_ID ID = it.value->id;
        
        mtt::Destroy_Command cmd = {};
        cmd.thing_id = ID;
        this->to_destroy.push_back(cmd);
    }
}
mtt::Thing_ID get_thing_most_recently_selected_with_touch(mtt::World* world)
{
    return dt::DrawTalk::ctx()->scn_ctx.thing_most_recently_selected_with_touch;
}
mtt::Thing* get_thing_ptr_most_recently_selected_with_touch(mtt::World* world)
{
    return mtt::Thing_try_get(world, get_thing_most_recently_selected_with_touch(world));
}

void set_thing_most_recently_selected_with_touch(mtt::World* world, mtt::Thing_ID ID, int LINE, const char* const file)
{
    
//    static usize bla = 0;
//    MTT_print("{\n");
//    MTT_print(">>>>>>>>>>>>Changing most recently selected from ID=[%llu] to ID=[%llu] line=[%d] function=[%s]\n", dt::DrawTalk::ctx()->scn_ctx.thing_most_recently_selected_with_touch, ID, LINE, file);
    
    dt::DrawTalk::ctx()->scn_ctx.thing_most_recently_selected_with_touch = ID;
    
//    MTT_print(">>>>>>>>>>>>BLAAAA Changing most recently selected from ID=[%llu] to ID=[%llu] line=[%d] function=[%s]\n", bla, ID, LINE, file);
//    
//    
//    bla = ID;
//    
//    MTT_print("}\n");
    
}

void* selected_things(mtt::World* world)
{
    return &dt::DrawTalk::ctx()->scn_ctx.selected_things;
}

usize increment_selection_count(mtt::Thing* thing)
{
    thing->selection_count += 1;
    return thing->selection_count;
}

usize decrement_selection_count(mtt::Thing* thing)
{
    ASSERT_MSG(thing->selection_count > 0, "Selection count should always be nonzero when decrementing!\n");
    thing->selection_count -= 1;
    
    return thing->selection_count;
}

usize selection_count(mtt::Thing* thing)
{
    return thing->selection_count;
}

uint64 Watcher::next_id = 0;
Watcher Watcher::watch(mtt::Thing* thing, const mtt::String& tag, my::Function<void(mtt::Thing*, Field_Event)> proc, my::Function<void(mtt::Thing*)> on_watched_destroy_proc)
{
    Watcher watcher;
    watcher.id = thing->id;
    watcher.tag = tag;
    watcher.proc = proc;
    watcher.on_watched_destroy_proc = on_watched_destroy_proc;
    
    Watcher::next_id += 1;
    watcher.watcher_id = Watcher::next_id;
    
    auto field_handle = lookup<true>(thing, tag);
    auto* field = &thing->field_descriptor.fields[field_handle.index];
    if (field->watchers == nullptr) {
        field->watchers = mem::alloc_init<std::vector<Watcher>>(&thing->world()->allocator);
    }
    ((std::vector<Watcher>*)field->watchers)->push_back(watcher);
    
    return watcher;
}

void unwatch(mtt::World* world, mtt::Thing_ID id, const mtt::String& tag, uint64 watcher_id)
{
    mtt::Thing* thing = world->Thing_try_get(id);
    if (thing == nullptr) {
        return;
    }
    
    auto field_handle = lookup<true>(thing, tag);
    auto& watchers = *(std::vector<Watcher>*)thing->field_descriptor.fields[field_handle.index].watchers;
    for (usize i = 0; i < watchers.size(); i += 1) {
        if (watchers[i].watcher_id == watcher_id) {
            watchers.erase(watchers.begin() + i);
            return;
        }
    }
}



mtt::Thing* jump_make(mtt::World* world)
{
    mtt::Thing* jump = mtt::Thing_make(world, mtt::ARCHETYPE_JUMP);
    
    
    
    return jump;
}

float32 heading_direction_x(mtt::Rep* rep)
{
    return rep->init_forward_dir.x * rep->forward_dir.x;
}

float32 heading_direction_x(mtt::Thing* thing)
{
    return heading_direction_x(mtt::rep(thing));
}

void flip_init_direction(mtt::Thing* thing, mtt::Rep* parent_rep, mtt::Thing* parent, Mat4 xform, Mat4 pose_xform, mtt::Thing* root)
{
    mtt::Rep* rep = mtt::rep(thing);
    
    usize child_count = thing->child_id_set.size();
    
    rep->init_forward_dir.x *= -1.0f;
    set_pose_transform(thing, m::rotate(mat4(1.0f), MTT_PI_32, {0.0f, 1.0f, 0.0f}) * rep->pose_transform);
    
    for (usize i = 0; i < child_count; i += 1) {
        mtt::Thing* child = thing->world()->Thing_try_get(thing->child_id_set[i]);
        if (child == nullptr || child == root) {
            continue;
        }
        flip_init_direction(child, rep, thing, xform, pose_xform, root);
    }
    
    mtt::Thing_set_position(thing, rep->transform.translation + vec3(2.0f * (parent_rep->transform.translation.x - rep->transform.translation.x), 0.0f, 0.0f));
}

void flip_init_direction(mtt::Thing* thing)
{
    mtt::Rep* rep = mtt::rep(thing);
    rep->init_forward_dir.x *= -1.0f;
    
    usize child_count = thing->child_id_set.size();
    for (usize i = 0; i < child_count; i += 1) {
        mtt::Thing* child = thing->world()->Thing_try_get(thing->child_id_set[i]);
        if (child == nullptr) {
            continue;
        }
        flip_init_direction(child, rep, thing, rep->model_transform, rep->pose_transform, thing);
    }
}

void flip_left_right(mtt::Thing* thing, mtt::Rep* parent_rep, mtt::Thing* parent, Mat4 xform, Mat4 pose_xform, mtt::Thing* root);

void flip_left_right(mtt::Thing* thing)
{
    mtt::Rep* rep = mtt::rep(thing);
    
    rep->forward_dir.x *= -1.0f;
    set_pose_transform(thing, m::rotate(mat4(1.0f), MTT_PI_32, {0.0f, 1.0f, 0.0f}) * rep->pose_transform);
    
    
    usize child_count = thing->child_id_set.size();
    for (usize i = 0; i < child_count; i += 1) {
        mtt::Thing* child = thing->world()->Thing_try_get(thing->child_id_set[i]);
        if (child == nullptr) {
            continue;
        }
        flip_left_right(child, rep, thing, rep->model_transform, rep->pose_transform, thing);
    }
}

void flip_left_right(mtt::Thing* thing, mtt::Rep* parent_rep, mtt::Thing* parent, Mat4 xform, Mat4 pose_xform, mtt::Thing* root)
{
    mtt::Rep* rep = mtt::rep(thing);
    
    usize child_count = thing->child_id_set.size();
    
    rep->forward_dir.x *= -1.0f;
    set_pose_transform(thing, m::rotate(mat4(1.0f), MTT_PI_32, {0.0f, 1.0f, 0.0f}) * rep->pose_transform);
    
    for (usize i = 0; i < child_count; i += 1) {
        mtt::Thing* child = thing->world()->Thing_try_get(thing->child_id_set[i]);
        if (child == nullptr || child == root) {
            continue;
        }
        flip_left_right(child, rep, thing, xform, pose_xform, root);
    }
    
    mtt::Thing_set_position(thing, rep->transform.translation + vec3(2.0f * (parent_rep->transform.translation.x - rep->transform.translation.x), 0.0f, 0.0f));
    
    //mtt::disconnect_child_from_parent(thing->world, thing);
}

MTT_String_Pool* string_pool(void)
{
    return MTT_get_string_pool(0);
}

MTT_NODISCARD MTT_String_Ref string(cstring str)
{
    MTT_String_Ref ref = MTT_string_add(0, str);
    return ref;
}
MTT_NODISCARD MTT_String_Ref string(cstring str, MTT_String_Length length)
{
    return MTT_string_add_with_length(0, str, length);
}



bool string_free(MTT_String_Ref& str)
{
    return MTT_string_ref_release(&str);
}

MTT_NODISCARD cstring string_get(MTT_String_Ref& str)
{
    return MTT_string_ref_to_cstring_checked(str);
}

MTT_NODISCARD MTT_String_Ref string_ref_get(cstring str)
{
    return MTT_String_Ref_get(str, 0);
}



void set_is_actor(mtt::Thing_Archetype* t)
{
    t->is_actor = true;
}
void unset_is_actor(mtt::Thing_Archetype* t)
{
    t->is_actor = false;
}
bool is_actor(mtt::Thing_Archetype* t)
{
    return t->is_actor;
}
bool is_actor(mtt::Thing* thing)
{
    mtt::Thing_Archetype* arch;
    Thing_Archetype_get(mtt::world(thing), thing->archetype_id, &arch);
    return is_actor(arch);
}






mem::Allocator* field_allocator(mtt::World* world)
{
    return &world->buckets.allocator;
}

mem::Allocator* buckets_allocator(mtt::World* world)
{
    return &world->buckets.allocator;
}

mem::Allocator* message_allocator(mtt::World* world)
{
    return &world->message_allocation.allocator;
}






void message_free(Message* msg)
{
    if (msg->contents == nullptr || msg->length == 0) {
        return;
    }
    message_allocator(ctx())->do_deallocate(msg->contents, msg->length);
}

//struct Message_Queue {
//    mem::Arena arena;
//
//    void make(usize initial_count);
//
//    void poll_all(void(*handler)(Message* msg));
//
//    static mem::Pool_Allocation message_queue_entry_pool;
//    static mem::Allocator message_queue_backing_allocator;
//    static usize chunk_byte_count;
//    static usize alignment;
//
//    static void init(mem::Allocator* mem_alloc, usize chunk_byte_count, usize alignment);
//};





void Message_Queue::init(usize initial_count)
{
    mtt::init(&this->messages, *message_allocator(ctx()), 0, initial_count);
    this->head  = 0;
    this->tail  = 0;
    this->count = 0;
}
void Message_Queue::deinit(void)
{
    mtt::deinit(&this->messages);
}

void Message_Queue::enqueue(Message* message)
{
    if (this->is_full()) {
        const usize old_cap = this->messages.cap;
        const usize new_cap = this->messages.cap * 2;
        ASSERT_MSG(m::is_powerof2(new_cap), "invalid queue size %llu, must be power of 2");
        mtt::set_capacity(&this->messages, new_cap, this->head, this->count);
        
        this->head = 0;
        this->tail = old_cap;
    }
    this->messages.set_slot(this->tail, message);
    this->tail = ((this->tail + 1) & this->messages.cap - 1);
    this->count += 1;
}


void Message_Queue::dequeue(void(*handler)(Message* msg))
{
    Message* msg = &this->messages.get_slot(this->head);
    handler(msg);
    
    this->head = ((this->head + 1) & (this->messages.cap - 1));
    
    this->count -=1;
}

template <typename PROC>
void Message_Queue_dequeue_with_closure(Message_Queue* q, PROC&& proc)
{
#define this q
    Message* msg = &this->messages.get_slot(this->head);
    proc(msg);
    
    this->head = ((this->head + 1) & (this->messages.cap - 1));
    
    this->count -=1;
#undef this
}

void Message_Queue::dequeue_discard(void)
{
    Message* msg = &this->messages.get_slot(this->head);
    this->head = ((this->head + 1) & (this->messages.cap - 1));
    
    this->count -=1;
}

void Message_Queue::dequeue_all(void(*handler)(Message* msg))
{
    handler(this->messages.begin_ptr());
    this->clear();
}

void Message_Queue::clear(void)
{
    for (usize i = this->head, n = 0; n != this->count; i = (i + 1) & (this->messages.cap - 1), n+= 1) {
        message_free(this->messages.begin_ptr() + i);
    }
    mtt::clear(&this->messages);
    this->count = 0;
}

void Message_Queue_print(Message_Queue* q)
{
#define this q
    std::cout << "Q: cap = {" << this->messages.cap << " head= " << this->head << " tail= " << this->tail << "[" << std::endl;
    for (usize i = this->head, n = 0; n != this->count; i = (i + 1) & (this->messages.cap - 1), n+= 1) {
        Message* msg = (this->messages.begin_ptr() + i);
        std::cout << "    (Message) { label = " << msg->length <<  " idx : " << i << " }" << std::endl;
    }
    std::cout << "] }" << std::endl;
#undef this
}






void Particle_State_update(Particle_System_State* p)
{
    
}

bool arrow_edge_label_is_equal(mtt::Arrow_Link& a, mtt::Arrow_Link& b)
{
    return (arrow_get_label(a) == arrow_get_label(b));
}

bool arrow_is_equal(mtt::Arrow_Link& a, mtt::Arrow_Link& b)
{
    return (arrow_edge_label_is_equal(a, b) && (arrow_get_thing_id(a) == arrow_get_thing_id(b)));
}

void arrow_edge_add_non_recursive(mtt::World* world, mtt::Thing_ID src, mtt::Thing_ID dst, const mtt::String& label, ARROW_LINK_FLAGS flags)
{
    auto& arrows = *mtt::arrow_links(world);
    
    auto it_find = arrows.edges_forward.find(src);
    if (it_find == arrows.edges_forward.end()) {
        arrows.edges_forward.insert({src, (mtt::Arrow_Link_List){{dst, label, flags}}});
        arrows.edges_reverse[dst].push_back({src, label, flags});
    } else {
        mtt::Arrow_Link_List& val = it_find->second;
        for (auto it = val.begin(); it != val.end(); ++it) {
            if (arrow_get_thing_id(*it) == dst && arrow_get_label(*it) == label) {
                return;
            }
        }
        val.push_back({dst, label, flags});
        arrows.edges_reverse[dst].push_back({src, label, flags});
    }
}

void arrow_edge_add(mtt::World* world, mtt::Thing_ID src, mtt::Thing_ID dst, const mtt::String& label, ARROW_LINK_FLAGS flags)
{
    mtt::Thing* src_thing = nullptr;
    mtt::Thing* dst_thing = nullptr;
    if (!(world->Thing_try_get(src, &src_thing) && world->Thing_try_get(dst, &dst_thing))) {
        return;
    }
    
    if ((flags & ARROW_LINK_FLAGS_DIRECTED) != ARROW_LINK_FLAGS_DIRECTED) {
        arrow_edge_add_non_recursive(world, src, dst, label, flags);
        arrow_edge_add_non_recursive(world, dst, src, label, flags);
        return;
    }
    
    arrow_edge_add_non_recursive(world, src, dst, label, flags);
}

bool arrow_edge_add_or_if_exists_remove(mtt::World* world, mtt::Thing_ID src, mtt::Thing_ID dst, const mtt::String& label, ARROW_LINK_FLAGS flags)
{
    mtt::Thing* src_thing = nullptr;
    mtt::Thing* dst_thing = nullptr;
    if (!(world->Thing_try_get(src, &src_thing) && world->Thing_try_get(dst, &dst_thing))) {
        return false;
    }
    
    auto& arrows = *mtt::arrow_links(world);
    
    if ((flags & ARROW_LINK_FLAGS_DIRECTED) != ARROW_LINK_FLAGS_DIRECTED) {
        auto it_find_src = arrows.edges_forward.find(src);
        auto it_find_dst = arrows.edges_reverse.find(dst);
        if (it_find_src == arrows.edges_forward.end() || it_find_dst == arrows.edges_reverse.end()) {
            arrow_edge_add(world, src, dst, label, flags);
            arrow_edge_add(world, dst, src, label, flags);
        } else {
            mtt::Arrow_Link_List& val = it_find_src->second;
            for (auto it = val.begin(); it != val.end(); ++it) {
                if (arrow_get_thing_id(*it) == dst && arrow_get_label(*it) == label) {
                    arrow_edge_remove(world, src, dst, label);
                    arrow_edge_remove(world, dst, src, label);
                    return false;
                }
            }
            arrow_edge_add(world, src, dst, label, flags);
        }
        
        return true;
    }
    
    auto it_find = arrows.edges_forward.find(src);
    if (it_find == arrows.edges_forward.end()) {
        arrows.edges_forward.insert({src, (mtt::Arrow_Link_List){{dst, label, flags}}});
        arrows.edges_reverse[dst].push_back({src, label, flags});
    } else {
        mtt::Arrow_Link_List& val = it_find->second;
        for (auto it = val.begin(); it != val.end(); ++it) {
            if (arrow_get_thing_id(*it) == dst && arrow_get_label(*it) == label) {
                auto rev_it_find = arrows.edges_reverse.find(dst);
                if (rev_it_find != arrows.edges_reverse.end()) {
                    for (auto rev_it = rev_it_find->second.begin(); rev_it != rev_it_find->second.end(); ) {
                        if (arrow_get_thing_id(*rev_it) == src && arrow_get_label(*rev_it) == label) {
                            rev_it = rev_it_find->second.erase(rev_it);
                        } else {
                            ++rev_it;
                        }
                    }
                }
                val.erase(it);
                return false;
            }
        }
        val.push_back({dst, label, flags});
        arrows.edges_reverse[dst].push_back({src, label, flags});
    }
    
    return true;
}

void arrow_edge_remove(mtt::World* world, mtt::Thing_ID src, mtt::Thing_ID dst, const mtt::String& label)
{
    mtt::Thing* src_thing = nullptr;
    mtt::Thing* dst_thing = nullptr;
    if (!(world->Thing_try_get(src, &src_thing) && world->Thing_try_get(dst, &dst_thing))) {
        return;
    }
    
    auto& arrows = *mtt::arrow_links(world);
    
    auto it_find = arrows.edges_forward.find(src);
    if (it_find == arrows.edges_forward.end()) {
        return;
    } else {
        mtt::Arrow_Link_List& val = it_find->second;
        for (auto it = val.begin(); it != val.end(); ++it) {
            if (arrow_get_thing_id(*it) == dst && arrow_get_label(*it) == label) {
                auto rev_it_find = arrows.edges_reverse.find(dst);
                if (rev_it_find != arrows.edges_reverse.end()) {
                    for (auto rev_it = rev_it_find->second.begin(); rev_it != rev_it_find->second.end(); ++rev_it) {
                        if (arrow_get_thing_id(*rev_it) == src && arrow_get_label(*rev_it) == label) {
                            rev_it_find->second.erase(rev_it);
                            break;
                        }
                    }
                }
                val.erase(it);
                return;
            }
        }
    }
    
    return;
}

void arrow_edge_remove_all_with_source(mtt::Thing* src)
{
    auto& arrows = *mtt::arrow_links(src->world());
    
    auto it_find = arrows.edges_forward.find(src->id);
    if (it_find != arrows.edges_forward.end()) {
        
        // find where src is a dst
        auto rev_find = arrows.edges_reverse.find(src->id);
        if (rev_find != arrows.edges_reverse.end()) {
            for (auto it_src = rev_find->second.begin(); it_src != rev_find->second.end(); ++it_src) {
                auto _find = arrows.edges_forward.find(arrow_get_thing_id(*it_src));
                
                if (_find != arrows.edges_forward.end()) {
                    auto& _ = _find->second;
                    for (auto it_find_src = _.begin(); it_find_src != _.end();) {
                        if (arrow_get_thing_id(*it_find_src) == src->id) {
                            it_find_src = _.erase(it_find_src);
                        } else {
                            ++it_find_src;
                        }
                    }
                }
            }
        }
        
        arrows.edges_forward.erase(it_find);
    }
}

void arrows_reverse(mtt::Thing* root_thing)
{
    
    // TODO: ...
    return;
    auto& arrows = *mtt::arrow_links(root_thing->world());
    //arrows.edges_fwd.find(root_thing->id);
    //arrows.edges_reverse_lookup.find(root_thing->id);
    
    std::vector<Arrow_Link> fwd;
    std::vector<Arrow_Link> bwd;
    
    mtt::Set_Stable<mtt::Thing_ID> visited;
    
    std::queue<mtt::Thing_ID> q;
    visited.insert(root_thing->id);
    q.push(root_thing->id);
    
    do {
        mtt::Thing_ID now = q.front();
        q.pop();
        
        auto find = arrows.edges_reverse.find(now);
        if (find != arrows.edges_reverse.end()) {
            auto& list = find->second;
        }
    } while (!q.empty());
    
}

void arrows_copy_inner(mtt::World* world, mtt::Thing* thing)
{
    
}
void arrows_copy(mtt::Thing* thing)
{
//    auto& arrows = *mtt::arrow_links(mtt::world(thing));
//    
//    arrows.edges_forward

}


cstring text_thing_empty_string = "[..]";


void toggle_verbose_display(mtt::World* world)
{
    world->show_verbose = !world->show_verbose;
    mtt::arrow_links(world)->toggle_visibility();
}

void toggle_attachment_links_display(mtt::World* world)
{
    world->show_attachment_links = !world->show_attachment_links;
}

bool should_show_verbose_display(mtt::World* world)
{
    return world->show_verbose;
}

bool should_show_attachment_links(mtt::World* world)
{
    return world->show_attachment_links;
}

void toggle_debug_display(mtt::World* world)
{
    world->show_debug = !world->show_debug;
}

bool should_show_debug_display(mtt::World* world)
{
    return world->show_debug;
}



void run_deferred_per_frame(mtt::World* world, float32 fixed_dt, float32 time_prev, float32 time, MTT_Core* core, void* ctx)
{
    float32 realtime_dt = time - time_prev;
    for (isize i = world->deferred_per_frame.size() - 1; i >= 0; i -= 1)
    {
        if (!world->deferred_per_frame[i](world, fixed_dt, time_prev, time, realtime_dt, core, ctx)) {
            world->deferred_per_frame[i] = world->deferred_per_frame[world->deferred_per_frame.size() - 1];
        }
    }
}

void add_deferred_per_frame(mtt::World* world, bool (*op)(mtt::World* world, float32 fixed_dt, float32 time_prev, float32 time, float32 realtime_dt, MTT_Core* core, void* ctx))
{
    world->deferred_per_frame.push_back(op);
}

void set_priority_layer(mtt::World* world, Priority_Layer l)
{
    world->priority_layer = l;
}
Priority_Layer get_priority_layer(mtt::World* world)
{
    return world->priority_layer;
}

void push_priority_layer(mtt::World* world, Priority_Layer l)
{
    if (!world->priority_layer_stack.empty() && world->priority_layer_stack.back() == l) {
        return;
    }
    
    world->priority_layer_stack.push_back(l);
}
Priority_Layer pop_priority_layer(mtt::World* world)
{
    if (!world->priority_layer_stack.empty()) {
        Priority_Layer prev = world->priority_layer_stack.back();
        world->priority_layer_stack.pop_back();
        return prev;
    } else {
        return Priority_Layer_default();
    }
}

void Thing_set_priority_layer(mtt::Thing* thing, Priority_Layer l)
{
    auto* r = mtt::rep(thing);
    if (r != nullptr) {
        for (auto c : r->colliders) {
            c->priority_layer = l;
        }
    }
}


void set_graph(mtt::World* world, Eval_Connection_Graph* G)
{
    world->current_graph = G;
    ASSERT_MSG(curr_graph(world) != NULL, "HUH?");
}
void set_graph_to_root(mtt::World* world)
{
    world->current_graph = &world->root_graph;
}
Eval_Connection_Graph* curr_graph(mtt::World* world)
{
    return world->current_graph;
}
Eval_Connection_Graph* root_graph(mtt::World* world)
{
    return &world->root_graph;
}
bool current_is_root_graph(mtt::World* world)
{
    return curr_graph(world) == root_graph(world);
}
bool is_root_graph(mtt::World* world, Eval_Connection_Graph* G)
{
    return G == root_graph(world);
}

void save_current_graph(mtt::World* world)
{
    world->saved_graph = curr_graph(world);
}
void restore_current_graph(mtt::World* world)
{
    set_graph(world, world->saved_graph);
}


mtt::Map_Stable<Script_ID, Script> Script::scripts = {};

mtt::Map_Stable<Script_ID, Script_Shared_Args> Script::shared_args = {};

void push_script_task(Script_Tasks* tasks, Script_Instance* script)
{
    tasks->list.push_back(script);
}

void push_script_rule_task(Script_Tasks* tasks, Script_Instance* script, Script_Rules& rules)
{
    script->rules = rules;
    
    
    {
        auto* thing_dependency_map = &mtt::Runtime::ctx()->Thing_ID_to_Rule_Script;
        for (auto it = rules.triggers.dependent_thing_ids.begin(); it != rules.triggers.dependent_thing_ids.end(); ++it) {
            mtt::Thing_ID t_id = *it;
            auto f_it = thing_dependency_map->find(t_id);
            if (f_it != thing_dependency_map->end()) {
                mtt::Set<Script_ID>& s_ids = f_it->second;
                s_ids.insert((Script_ID)script->id);
             //   r_set.insert((Script_ID)script->id);
            } else {
                thing_dependency_map->insert({t_id, (mtt::Set<Script_ID>){(Script_ID)script->id}});
            }
        }
    }
    
    
    mtt::Runtime::ctx()->id_to_rule_script.insert({script->id, (mtt::Script_Instance_Ref)script});
    
    tasks->rules_list.push_back(script);
}

    
usize Script_context_push(Script_Instance* s, usize continuation_instruction_idx, usize last_index_scope)
{
    Script_Context next_ctx   = s->contexts[s->ctx_idx].ctx_stack.back();
    next_ctx.instruction_idx  = next_ctx.instruction_idx;
    next_ctx.first_idx        = next_ctx.instruction_idx;
    next_ctx.continuation_idx = continuation_instruction_idx;
    next_ctx.local_ops.clear();
    s->contexts[s->ctx_idx].ctx_stack.push_back(next_ctx);
    
    {
        usize begin_index = next_ctx.first_idx + 1;
        usize end_index = last_index_scope;
        ASSERT_MSG(begin_index <= end_index, "overlapping entries in group block is not allowed");
        init_script_instance_instructions_range(s, begin_index, end_index);
    }
#ifndef NDEBUG
//    MTT_print("PUSH CONTEXT: script=[%s] idx=[%llu] continue_idx=[%llu]\n", s->source_script->label.c_str(), next_ctx.instruction_idx, continuation_instruction_idx);
//    for (isize i = s->contexts[s->ctx_idx].ctx_stack.size() - 2; i >= 0; i -= 1) {
//        MTT_print("    CONTEXT: idx=[%llu] continue_idx=[%llu]\n", s->contexts[s->ctx_idx].ctx_stack[i].instruction_idx, s->contexts[s->ctx_idx].ctx_stack[i].continuation_idx);
//    }
#endif
    
    return next_ctx.instruction_idx;
}
    
usize Script_context_pop(Script_Instance* s)
{
    usize continuation_idx = s->contexts[s->ctx_idx].ctx_stack.back().continuation_idx;
    s->contexts[s->ctx_idx].ctx_stack.pop_back();
    
#ifndef NDEBUG
    ASSERT_MSG(s->contexts.size() > s->ctx_idx, "?");
    ASSERT_MSG(!s->contexts[s->ctx_idx].ctx_stack.empty(), "?");
#endif
    Script_Context& restored_ctx   = s->contexts[s->ctx_idx].ctx_stack.back();
    restored_ctx.instruction_idx      = continuation_idx;
#ifndef NDEBUG
//    MTT_print("POP CONTEXT: script=[%s] idx=[%llu] = continue_idx=[%llu]\n", s->source_script->label.c_str(), restored_ctx.instruction_idx, continuation_idx);
//
//    for (isize i = s->contexts[s->ctx_idx].ctx_stack.size() - 2; i >= 0; i -= 1) {
//        MTT_print("    CONTEXT: idx=[%llu] continue_idx=[%llu]\n", s->contexts[s->ctx_idx].ctx_stack[i].instruction_idx, s->contexts[s->ctx_idx].ctx_stack[i].continuation_idx);
//    }
#endif
    
    return continuation_idx;
}

static inline void* Script_Instance_caller(Script_Instance* s)
{
    return (void*)s->caller;
}

MTT_NODISCARD
Script_Property_List* Script_Lookup_get_var_with_key(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var)
{
    auto find_first = lookup.find(key_ctx);
    if (find_first == lookup.end()) {
        return nullptr;
    }
    auto find_second = find_first->second.find(key_var);
    if (find_second == find_first->second.end()) {
        return nullptr;
    }
    
    return &(find_second->second);
}

void Script_Lookup_set_var_group(Script_Lookup& lookup, const mtt::String& key_ctx, Script_Property_Lookup& var_group)
                     {
    lookup[key_ctx] = var_group;
}
MTT_NODISCARD
Script_Property_Lookup* Script_Lookup_get_var_group(Script_Lookup& lookup, const mtt::String& key_ctx)
                     {
    auto find_first = lookup.find(key_ctx);
    if (find_first == lookup.end()) {
        return nullptr;
    }
    return &(find_first->second);
}


void Script_Lookup_set_var_with_key(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var, const Script_Property_List& val)
                     {
    lookup[key_ctx][key_var] = val;
}
void Script_Lookup_set_key(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var)
                     {
    lookup[key_ctx][key_var] = {};
}
MTT_NODISCARD
Script_Property_List* Script_Lookup_get_var_with_key(Script_Instance* s, const mtt::String& key_ctx, const mtt::String& key_var)
{
    return Script_Lookup_get_var_with_key(s->lookup(), key_ctx, key_var);
}
void Script_Lookup_set_var_with_key(Script_Instance* s, const mtt::String& key_ctx, const mtt::String& key_var, const Script_Property_List& val)
{
    Script_Lookup_set_var_with_key(s->lookup(), key_ctx, key_var, val);
}

void Script_Lookup_mtt_print(Script_Instance* s)
{
    Script_Lookup_print(s->lookup());
}

Script_Context& Script_current_context_state(Script_Instance* s)
{
    return s->contexts[s->ctx_idx].ctx_stack.back();
}

void Script_set_current_context(Script_Instance* s, usize idx)
{
    s->ctx_idx = idx;
}
usize Script_get_current_context_idx(Script_Instance* s)
{
    return s->ctx_idx;
}

Script_Context& Script_current_context(Script_Contexts& s_contexts)
{
    ASSERT_MSG(!s_contexts.ctx_stack.empty(), "Should not be empty");
    return s_contexts.ctx_stack.back();
}


Script_Operation_Queue& Script_current_queue(Script_Instance* s)
{
    return Script_current_context_state(s).local_ops;
}

Script_Instance* Script_Operation_List_enqueue_script(Script_Operation_List& L, Script* callee_script, Script_Instance* s, void* caller_info)
{
    auto* s_i = Script_Instance_call_from_script(callee_script, s, caller_info);
    L.list.push_back({s_i});
    return s_i;
}

void Signal_Mailbox_push(Signal_Mailbox* mb, const mtt::String& name, mtt::Any& data)
{
    mb->signals[name].push_back(data);
}
void Signal_Mailbox_clear(Signal_Mailbox* mb)
{
    mb->signals.clear();
}


std::vector<mtt::Any>* Signal_Mailbox_access(Signal_Mailbox* mb, const mtt::String& name)
{
    std::vector<mtt::Any>* out = nullptr;
    mtt::map_try_get(&mb->signals, name, &out);
    return out;
    
}

MTT_NODISCARD
Query_Rule Query_Rule_make(mtt::World* world, const mtt::String& query_string)
{
    Query_Rule q = {};
    q.world_ = world;
    flecs::world& ecs_world = world->ecs_world;
    cstring rule_cstring = query_string.c_str();
    ecs_rule_t* rule = mtt_ecs_rule_new(ecs_world.c_ptr(), rule_cstring);
    
    if (rule == nullptr) {
        q.rule = nullptr;
        return q;
    }
    
    q.rule = rule;
    q.string_rep = query_string;
    
    
    return q;
}

bool Query_Rule_is_valid(Query_Rule* q)
{
    return (q->rule != nullptr);
}

Rule_Var_Handle Query_Rule_Var_for_name(Query_Rule* q, cstring name)
{
    return ecs_rule_find_var(q->rule, name);
}

void Query_Rule_Var_for_name(Query_Rule* q, cstring name, Query_Rule_Var_Info* v_handle)
{
    *v_handle = {};
    auto var_id = ecs_rule_find_var(q->rule, name);
    if (var_id == 0) {
        return;
    }
    
    v_handle->name = mtt::String(name);
    v_handle->var = var_id;
}

World* world(Query_Rule* q)
{
    return q->world_;
}

void Query_Rule_destroy(Query_Rule* query)
{
    if (query->rule != nullptr && !query->is_copy) {
        assert(query->ref_count > 0);
        query->ref_count -= 1;
        if (query->ref_count == 0) {
            ecs_rule_fini(query->rule);
            query->rule = nullptr;
        }
    }
}

const mtt::String& Query_string(Query_Rule* q)
{
    return q->string_rep;
}





void Any_print(mtt::Any& any)
{
    auto type = any.type;
    switch (type) {
            
        case MTT_NONE:
            MTT_print("%s\n", meta[type].name.c_str());
            break;
        case MTT_FLOAT:
            MTT_print("%s:%f\n", meta[type].name.c_str(), any.Float);
            break;
        case MTT_FLOAT64:
            MTT_print("%s:%f\n", meta[type].name.c_str(), any.Float64);
            break;
        case MTT_VECTOR2:
            MTT_print("%s:<%f, %f>\n", meta[type].name.c_str(), any.Vector2.x, any.Vector2.y);
            break;
        case MTT_VECTOR3:
            MTT_print("%s:<%f, %f, %f>\n", meta[type].name.c_str(), any.Vector2.x, any.Vector3.y, any.Vector3.z);
            break;
        case MTT_VECTOR4:
            MTT_print("%s:<%f, %f, %f, %f>\n", meta[type].name.c_str(), any.Vector4[0], any.Vector4[1], any.Vector4[2], any.Vector4[3]);
            break;
        case MTT_COLOR_RGBA:
            MTT_print("%s:<%f, %f, %f, %f>\n", meta[type].name.c_str(), any.Color_RGBA[0], any.Color_RGBA[1], any.Color_RGBA[2], any.Color_RGBA[3]);
            break;
        case MTT_INT32:
            MTT_print("%s:%d\n", meta[type].name.c_str(), any.Int32);
            break;
        case MTT_INT64:
            MTT_print("%s:%lld\n", meta[type].name.c_str(), any.Int64);
            break;
        case MTT_BOOLEAN:
            MTT_print("%s:%s\n", meta[type].name.c_str(), (any.Boolean != 0) ? "true" : "false");
            break;
        case MTT_CHAR:
            MTT_print("%s:%c\n", meta[type].name.c_str(), any.Char);
            break;
        case MTT_LIST:
            // TODO: ...
            break;
        case MTT_MAP:
            // TODO: ...
            break;
        case MTT_SET:
            // TODO: ...
            break;
        case MTT_POLYCURVE:
            // TODO: ...
            break;
        case MTT_TEXT:
            MTT_print("%s\n", MTT_string_ref_to_cstring_checked(any.String));
            break;
        case MTT_TAG:
            MTT_print("%s\n", MTT_string_ref_to_cstring_checked(any.String));
            break;
        case MTT_TAG_LIST:
            // TODO: ...
            break;
        case MTT_POINTER:
            // TODO: ...
            break;
        case MTT_ANY:
            // TODO: ...
            break;
        case MTT_PROCEDURE:
            // TODO: ...
            break;
        case MTT_THING:
            mtt::Thing_print(mtt::ctx(), any.thing_id);
            break;
        case MTT_THING_LIST:
            // TODO: ...
            break;
        case MTT_STRING:
            MTT_print("%s\n", MTT_string_ref_to_cstring_checked(any.String));
            break;
        default: {
            break;
        }
//        case MTT_UNKNOWN:
//            <#code#>
//            break;
//        case MTT_MATRIX4:
//            <#code#>
//            break;
//        case MTT_ARRAY_SLICE:
//            <#code#>
//            break;
//        case MTT_CUSTOM:
//            <#code#>
//            break;
//        case MTT_TYPE_COUNT:
//            <#code#>
//            break;
    }
}

void access_write_string(mtt::Thing* thing, const mtt::String& name, const mtt::String& set_to)
{
    auto* val = mtt::access<MTT_String_Ref>(thing, name);
    MTT_string_ref_release(val);
    *val = MTT_string_add(0, set_to.c_str());
}

cstring access_string(mtt::Thing* thing, const mtt::String& name)
{
    auto* val = mtt::access<MTT_String_Ref>(thing, name);
    return MTT_string_ref_to_cstring_checked(*val);
}


void Script_Instance::action(const mtt::String& a, mtt::Thing_ID src, mtt::Thing_ID dst)
{
    mtt::World* world = mtt::ctx();
    mtt::Thing* src_thing = mtt::Thing_try_get(world, src);
    mtt::Thing* dst_thing = mtt::Thing_try_get(world, dst);
    if (src_thing == nullptr || dst_thing == nullptr) {
        return;
    }
    
    auto* v = dt::verb_add(a);
    active_action_list.push_back((Active_Action) {
        .action = v,
        .src_thing    = src,
        .dst_thing    = dst
    });
    
    Thing_add_action(src_thing, v, dst_thing);
    
}
void Script_Instance::action(const mtt::String& a, mtt::Thing_ID src)
{
    mtt::World* world = mtt::ctx();
    mtt::Thing* src_thing = mtt::Thing_try_get(world, src);
    if (src_thing == nullptr) {
        return;
    }
    
    auto* v = dt::verb_add(a);
    active_action_list.push_back((Active_Action) {
        .action = v,
        .src_thing    = src,
        .dst_thing    = mtt::Thing_ID_INVALID,
    });
    
    
    Thing_add_action(src_thing, v);
}

void Script_Instance::remove_actions(void)
{
    mtt::World* world = mtt::ctx();
    for (isize i = 0; i < this->active_action_list.size(); i += 1) {
        auto& a = this->active_action_list[i];
        mtt::Thing* src = mtt::Thing_try_get(world, a.src_thing);
        if (src == nullptr) {
            continue;
        }
        
        if (a.dst_thing == mtt::Thing_ID_INVALID) {
            Thing_remove_action(src, a.action);
        } else {
            mtt::Thing* dst = mtt::Thing_try_get(world, a.dst_thing);
            Thing_remove_action(src, a.action, dst);
        }
    }
    this->active_action_list.clear();
}

void Script_Instance::remove_actions(mtt::Thing_ID t_id)
{
    mtt::World* world = mtt::ctx();
    mtt::Thing* src = mtt::Thing_try_get(world, t_id);
    if (src == nullptr) {
        return;
    }
    
    for (isize i = this->active_action_list.size(); i > 0; i -= 1) {
        const usize idx = i - 1;
        auto& a = this->active_action_list[idx];
        if (a.src_thing != t_id) {
            continue;
        }
        
        if (a.dst_thing == mtt::Thing_ID_INVALID) {
            Thing_remove_action(src, a.action);
        } else {
            mtt::Thing* dst = mtt::Thing_try_get(world, a.dst_thing);
            Thing_remove_action(src, a.action, dst);
        }
        this->active_action_list[idx] = this->active_action_list[this->active_action_list.size() - 1];
        this->active_action_list.pop_back();
    }
}

void clear_args_to_stop(Script_Instance* s_i)
{
    
    auto& map = Runtime::ctx()->Script_Instance_things_to_stop;
    auto f_it = map.find((Script_Instance_Ref)s_i);
    mtt::Dynamic_Array<mtt::Thing_ID>* container = nullptr;
    if (f_it != map.end()) {
        container = &(f_it->second);
        deinit(container);
        map.erase(f_it);
        return;
    }
}


void Script_Lookup_print(Script_Lookup& lookup)
{
#ifndef NDEBUG
    MTT_print("%s", "(Lookup) {\n");
    for (auto& [entry_key, scopes] : lookup) {
    MTT_print("  [%s] : {\n", entry_key.c_str());
        for (auto& [scope_key, prop_list] : scopes) {
    MTT_print("    [%s] : {\n", scope_key.c_str());
            for (auto& prop : prop_list) {
    //MTT_print("               label: [%s] : {\n", prop.label.c_str());
                MTT_print("%s", "      "); mtt::Any_print(prop.value); MTT_print("%s%s", "      ", prop.label.c_str());
            }
        }
    MTT_print("%s", "    }\n");
    }
    MTT_print("%s", "  }\n");
    MTT_print("%s", "}\n");
#endif
}


void* global_debug_args = nullptr;
void (*global_debug_proc)(void) = nullptr;

bool should_be_immobile(mtt::Thing* thing)
{
    return (dt::DrawTalk::ctx()->scn_ctx.thing_selected_with_pen == thing->id || mtt::selection_count(thing) != 0);
}
bool input_should_cancel_animation(mtt::Thing* thing)
{
    auto* u_input = &mtt_core_ctx()->input.users[0];
    return (mtt::selection_count(thing) > 1 || (mtt::selection_count(thing) == 1 &&  (key_modifier_flags_are_set(u_input, (UI_KEY_MODIFIER_FLAG)(UI_KEY_MODIFIER_FLAG_CONTROL | UI_KEY_MODIFIER_FLAG_COMMAND)))));
}

/// MARK: external world
///
void set_curr_external_world_id(mtt::World* world, usize id)
{
    world->curr_ext_world_id = id;
}
External_World* curr_external_world(mtt::World* world)
{
    return &world->ext_worlds[world->curr_ext_world_id];
}

External_Thing* External_Thing_map(mtt::External_World* world, mtt::Thing* thing)
{
    mtt::External_Thing* ext_thing = &(world->mappings.insert({thing->id, (External_Thing){}}).first->second);
    ext_thing->id = thing->id;
    
    return ext_thing;
}
void External_World_reset(mtt::External_World* world)
{
    External_World_mappings(world)->clear();
}


External_Mappings* External_World_mappings(mtt::External_World* ext_world)
{
    return &ext_world->mappings;
}

bool External_Thing_flags_are_set(mtt::External_Thing* thing, mtt::EXTERNAL_THING_FLAG flags)
{
    return (thing->flags & flags) == flags;
}


void Thing_set_is_proxy(mtt::Thing* proxy, mtt::Thing* of, usize scene_id)
{
    ASSERT_MSG(proxy->mapped_thing_id == mtt::Thing_ID_INVALID, "Should not already be a proxy");
    
    proxy->ecs_entity.add<Thing_Is_Proxy>();
    proxy->ecs_entity.add(flecs::Prefab);
    
    auto* world = mtt::world(of);
    world->things.thing_to_proxy_map[of->id].insert(proxy->id);
    proxy->mapped_thing_id = of->id;
    
    world->things.proxy_scenes[scene_id].thing_to_proxy_map[of->id].insert(proxy->id);
    world->things.proxy_id_to_scene_id.insert({proxy->id, scene_id});
}
void Thing_unset_is_proxy(mtt::Thing* thing)
{
    mtt::World* world = mtt::world(thing);
    
    Thing_Proxy_Storage* proxies = nullptr;
    mtt::map_try_get(world->things.thing_to_proxy_map, thing->mapped_thing_id, &proxies);
    ASSERT_MSG(proxies != nullptr, "if is a proxy, the list should already exist");
    auto find_it = proxies->find(thing->id);
    ASSERT_MSG(find_it != proxies->end(), "if is a proxy, should be found");
    proxies->erase(find_it);
    
    usize scene_id = world->things.proxy_id_to_scene_id[thing->id];
    world->things.proxy_scenes[scene_id].thing_to_proxy_map[thing->mapped_thing_id].erase(thing->id);
    world->things.proxy_id_to_scene_id.erase(thing->id);
    
    thing->mapped_thing_id = mtt::Thing_ID_INVALID;
    
    thing->ecs_entity.remove<Thing_Is_Proxy>();
    thing->ecs_entity.remove(flecs::Prefab);

}

bool Thing_is_proxy(mtt::Thing* thing)
{
    return thing->mapped_thing_id != mtt::Thing_ID_INVALID;
//    return thing->ecs_entity.has<Thing_Is_Proxy>() &&
//    thing->ecs_entity.has(flecs::Prefab);
}
bool Thing_is_proxy(mtt::World* world, mtt::Thing_ID id)
{
    mtt::Thing* thing = mtt::Thing_try_get(world, id);
    return (thing != nullptr && Thing_is_proxy(thing));
}

bool Thing_is_reserved(mtt::Thing* thing)
{
    return thing->is_reserved;
}
bool Thing_is_reserved(mtt::World* world, mtt::Thing_ID id)
{
    mtt::Thing* thing = mtt::Thing_try_get(world, id);
    return (thing != nullptr && Thing_is_reserved(thing));
}

mtt::Thing* Thing_mapped_from_proxy(mtt::Thing* thing)
{
    return mtt::Thing_try_get(mtt::world(thing), thing->mapped_thing_id);
}

mtt::Thing* Thing_mapped_from_proxy(mtt::World* world, mtt::Thing_ID thing_id)
{
    mtt::Thing* thing = mtt::Thing_try_get(world, thing_id);
    if (thing == nullptr) {
        return nullptr;
    }
    return mtt::Thing_try_get(world, thing->mapped_thing_id);
}



void destroy_all_proxies(mtt::World* world)
{
    static std::vector<mtt::Thing_ID> ids = {};
    ids.clear();
    auto& proxies = world->things.thing_to_proxy_map;
    for (auto [src_id, proxy_list] : proxies) {
        for (auto thing_proxy_id : proxy_list) {
            ids.push_back(thing_proxy_id);
        }
    }
    
    {
        const usize proxy_scene_count = Thing_Proxy_Scene_count(world);
        for (usize i = 0; i < proxy_scene_count; i += 1) {
            auto* scene = &world->things.proxy_scenes[i];
            scene->on_clear(scene);
        }
    }
    
    for (usize i = 0; i < ids.size(); i += 1) {
        mtt::Thing_destroy(world, ids[i]);
    }
    proxies.clear();
}

void destroy_all_proxies_for_scene_idx(mtt::World* world, usize idx)
{
    static std::vector<mtt::Thing_ID> ids = {};
    ids.clear();
    
    auto* scene = Thing_Proxy_Scene_for_idx(world, idx);
    if (scene == nullptr) {
        return;
    }
    auto& proxies = scene->thing_to_proxy_map;
    for (auto [src_id, proxy_list] : proxies) {
        for (auto thing_proxy_id : proxy_list) {
            ids.push_back(thing_proxy_id);
        }
    }
    
    {
        const usize proxy_scene_count = Thing_Proxy_Scene_count(world);
        for (usize i = 0; i < proxy_scene_count; i += 1) {
            auto* scene = &world->things.proxy_scenes[i];
            scene->on_clear(scene);
        }
    }
    
    for (usize i = 0; i < ids.size(); i += 1) {
        mtt::Thing_destroy(world, ids[i]);
    }
    proxies.clear();
}

static std::vector<mtt::Thing_ID> Thing_destroy_proxies_to_destroy = {};
void Thing_destroy_proxies(mtt::World* world, mtt::Thing_ID thing_id)
{
    Thing_destroy_proxies_to_destroy.clear();
    mtt::Thing* th = mtt::Thing_try_get(world, thing_id);
    if (!mtt::Thing_is_proxy(th)) {
        auto* proxies = mtt::Thing_proxies_try_get(th);
        if (proxies != nullptr) {
            Thing_destroy_proxies_to_destroy.clear();
            for (auto proxy_id : *proxies) {
                Thing_destroy_proxies_to_destroy.push_back(proxy_id);
            }
            for (usize i = 0; i < Thing_destroy_proxies_to_destroy.size(); i += 1) {
                mtt::Thing_destroy(world, Thing_destroy_proxies_to_destroy[i]);
            }
        }
    }
}


void set_logic_proc_for_archetype(mtt::World* world, mtt::Thing_Archetype_ID id, mtt::Logic_Procedure proc)
{
    for (auto it = mtt::iterator(world); mtt::next(&it);) {
        mtt::Thing* thing = it.value;
        
        if (thing->archetype_id == id) {
            thing->logic.proc = proc;
        }
    }
}

void Thing_Proxy_Scene_make_count(mtt::World* world, usize count)
{
    world->things.proxy_scenes.resize(count);
}
//usize Thing_Proxy_Scene_add_proxy(mtt::World* world, Thing_Proxy_Scene* scene, mtt:: id)
//{
//    ASSERT_MSG(scene->proxies.find(id) == scene->proxies.end(), "%s\n", "should not exist");
//    scene->proxies.insert(id);
//}
//usize Thing_Proxy_Scene_add_proxy(mtt::World* world, usize scene_id, Thing_Proxy_ID id)
//{
//    return Thing_Proxy_Scene_add_proxy(world, &world->things.proxy_scenes[scene_id], id);
//}
//usize Thing_Proxy_Scene_remove_proxy(mtt::World* world, Thing_Proxy_Scene* scene, Thing_Proxy_ID id)
//{
//    ASSERT_MSG(scene->proxies.find(id) != scene->proxies.end(), "%s\n", "should exist");
//    scene->proxies.erase(id);
//}
//usize Thing_Proxy_Scene_remove_proxy(mtt::World* world, usize scene_id, Thing_Proxy_ID id)
//{
//    return Thing_Proxy_Scene_remove_proxy(world, &world->things.proxy_scenes[scene_id], id);
//}
Thing_Proxy_Scene* Thing_Proxy_Scene_for_idx(mtt::World* world, usize idx)
{
    ASSERT_MSG(idx < world->things.proxy_scenes.size(), "%s\n", "out-of-bounds");
    return &world->things.proxy_scenes[idx];
}
Thing_Proxy_Scene* Thing_Proxy_Scene_for_Thing_Proxy_ID(mtt::World* world, Thing_Proxy_ID id)
{
    auto find_it = world->things.proxy_id_to_scene_id.find(id);
    if (find_it == world->things.proxy_id_to_scene_id.end()) {
        ASSERT_MSG(false, "%s\n", "null");
        return nullptr;
    }
    return Thing_Proxy_Scene_for_idx(world, find_it->second);
}
usize Thing_Proxy_Scene_count(mtt::World* world)
{
    return world->things.proxy_scenes.size();
}
Thing_Proxy_Scene* Thing_Proxy_Scene_for_Thing_Proxy(mtt::World* world, Thing* thing_proxy)
{
    ASSERT_MSG(thing_proxy != nullptr, "%s\n", "null");
    return Thing_Proxy_Scene_for_Thing_Proxy_ID(world, mtt::thing_id(thing_proxy));
}
void Thing_Proxy_Scene_prepare_for_render(Thing_Proxy_Scene* scene)
{
    ASSERT_MSG(scene != nullptr, "%s\n", "null");
    scene->proxy_aggregate.clear();
    if (scene->thing_to_proxy_map.size() > 0) {
        scene->proxy_aggregate.resize(scene->thing_to_proxy_map.size());
    }
}

void Thing_Proxy_Scene_remove_proxy(mtt::World* world, Thing* thing_proxy)
{
    auto find_it = world->things.proxy_id_to_scene_id.find(thing_proxy->id);
    ASSERT_MSG(find_it != world->things.proxy_id_to_scene_id.end(), "%s\n", "should be there");
    usize proxy_scene_id = find_it->second;
    auto* proxy_scene = Thing_Proxy_Scene_for_idx(world, proxy_scene_id);
    ASSERT_MSG(proxy_scene != nullptr, "%s\n", "should not be null");
    
    auto* thing_to_proxy_map = &proxy_scene->thing_to_proxy_map;
    {
        mtt::Thing* thing = thing_proxy;
        Thing_Proxy_Storage* proxies = nullptr;
        mtt::map_try_get(thing_to_proxy_map, thing_proxy->mapped_thing_id, &proxies);
        //ASSERT_MSG(proxies != nullptr, "if is a proxy, the list should already exist");
        if (proxies != nullptr) {
            
            auto find_it = proxies->find(thing->id);
            if (find_it == proxies->end()) {
                return;
            }
            //ASSERT_MSG(find_it != proxies->end(), "if is a proxy, should be found");
            proxies->erase(find_it);
            if (proxies->empty()) {
                thing_to_proxy_map->erase(thing->mapped_thing_id);
            }
        }
    }
}

mtt::Map<mtt::Thing*, mtt::Thing*> original_to_copy = {};

void make_clones_for_arrows(mtt::World* world, mtt::Thing* thing, mtt::Map<mtt::Thing*, mtt::Thing*>& original_to_copy, Arrow_Links* arrows, mtt::Set<mtt::Thing_ID>& visited)
{
    visited.insert(mtt::thing_id(thing));
    
    auto& fwd = arrows->edges_forward;
    auto& rev = arrows->edges_reverse;
    auto thing_ID = mtt::thing_id(thing);
    auto find_incoming = rev.find(thing_ID);
    auto find_outgoing = fwd.find(thing_ID);
    mtt::Thing* copy = nullptr;
    if (find_incoming != rev.end()) {
        for (const Arrow_Link& link : find_incoming->second) {
            auto* t = mtt::Thing_try_get(world, link.id);
            if (t == nullptr) {
                continue;
            }
            
            auto it_find = original_to_copy.find(t);
            if (it_find != original_to_copy.end()) {
                //copy = it_find->second;
            } else {
                copy = Thing_copy(world, link.id);
                original_to_copy.insert({t, copy});
            }
            if (!visited.contains(mtt::thing_id(t))) {
                make_clones_for_arrows(world, t, original_to_copy, arrows, visited);
            }
        }
    }
    if (find_outgoing != fwd.end()) {
        for (const Arrow_Link& link : find_outgoing->second) {
            auto* t = mtt::Thing_try_get(world, link.id);
            if (t == nullptr) {
                continue;
            }
            
            auto it_find = original_to_copy.find(t);
            if (it_find != original_to_copy.end()) {
                //copy = it_find->second;
            } else {
                copy = Thing_copy(world, link.id);
                original_to_copy.insert({t, copy});
            }
            if (!visited.contains(mtt::thing_id(t))) {
                make_clones_for_arrows(world, t, original_to_copy, arrows, visited);
            }
        }
    }
}

void copy_arrows(mtt::World* world, mtt::Thing* thing, mtt::Map<mtt::Thing*, mtt::Thing*>& original_to_copy, bool make_visible)
{
    auto* arrows = mtt::arrow_links(world);
    mtt::Set<mtt::Thing_ID> visited = {};
    make_clones_for_arrows(world, thing, original_to_copy, arrows, visited);
    
    auto& fwd = arrows->edges_forward;
    auto& rev = arrows->edges_reverse;
    
    struct Pair {
        mtt::Thing_ID src = mtt::Thing_ID_INVALID;
        mtt::Thing_ID dst = mtt::Thing_ID_INVALID;
    };
    dt::Dynamic_Array<Pair> arrow_edges_to_add = {};
    
    for (auto it = original_to_copy.begin(); it != original_to_copy.end(); ++it) {
        auto* t_original = it->first;
        auto* t_copy = it->second;
        auto thing_ID = mtt::thing_id(t_original);
        
        auto find_incoming = rev.find(thing_ID);
        auto find_outgoing = fwd.find(thing_ID);
        mtt::Thing* copy = nullptr;
        if (find_incoming != rev.end()) {
            for (const Arrow_Link& link : find_incoming->second) {
                auto* t_src_original = mtt::Thing_get(world, link.id);
                auto it_find = original_to_copy.find(t_src_original);
                
                auto* t_src_copy = it_find->second;
                
                arrow_edges_to_add.push_back({mtt::thing_id(t_src_copy), mtt::thing_id(t_copy)});
            }
        }
        if (find_outgoing != fwd.end()) {
            for (const Arrow_Link& link : find_outgoing->second) {
                auto* t_dst_original = mtt::Thing_get(world, link.id);
                auto it_find = original_to_copy.find(t_dst_original);
                
                auto* t_dst_copy = it_find->second;
                
                arrow_edges_to_add.push_back({mtt::thing_id(t_copy), mtt::thing_id(t_dst_copy)});
            }
        }
    }
    
    auto flags_to_add = (ARROW_LINK_FLAGS)((make_visible) ? ARROW_LINK_FLAGS_VISIBLE : ~ARROW_LINK_FLAGS_VISIBLE);
    for (usize i = 0; i < arrow_edges_to_add.size(); i += 1) {
        arrow_edge_add(world, arrow_edges_to_add[i].src, arrow_edges_to_add[i].dst, "", flags_to_add);
    }
}

Thing* Thing_save_as_preset_recursive(mtt::World* world, mtt::Thing* thing);
Thing* Thing_save_as_preset_recursive(mtt::World* world, mtt::Thing* thing)
{
    Thing* copy = nullptr;
    
    auto it_find = original_to_copy.find(thing);
    if (it_find != original_to_copy.end()) {
        copy = it_find->second;
    } else {
        copy = Thing_copy(thing);
        original_to_copy.insert({thing, copy});
    }
    
    copy->ecs_entity.add(flecs::Prefab);
    
    mtt::unset_should_render(copy);
    copy->is_user_destructible = false;
    copy->is_user_drawable = false;
    copy->is_user_destructible = false;
    copy->is_locked = true;
    copy->is_reserved = true;
    copy->is_visible = thing->is_visible;
    world->thing_saved_presents_rendering_states[mtt::thing_id(copy)] = mtt::is_rendering_something(thing);
    unset_is_active(copy);
    
    
    
    auto saved_child_set = thing->child_id_set;
    for (auto c_it = saved_child_set.begin(); c_it != saved_child_set.end(); ++c_it) {
        Thing* child = thing->world()->Thing_try_get(*c_it);
        if (child == nullptr) {
            continue;
        }
        
        mtt::Thing* sub_copy = Thing_save_as_preset_recursive(world, child);
        
        mtt::connect_parent_to_child(world, copy, sub_copy);
        
//        copy->child_id_set.push_back(sub_copy->id);
//        sub_copy->parent_thing_id = copy->id;
        sub_copy->ecs_entity.add(thing->world()->ecs_world.lookup("ChildOfTransitive"), copy->ecs_entity);
    }
    
    return copy;
}

bool Thing_save_as_preset(mtt::Thing* thing)
{
    if (!mtt::is_actor(thing)) {
        return false;
    }
    
    mtt::World* world = mtt::world(thing);
    
    
    auto& named_things = dt::DrawTalk::ctx()->lang_ctx.dictionary.thing_to_word;
    auto find_it = named_things.find(mtt::thing_id(thing));
    if (find_it == named_things.end()) {
        return false;
    }
    
    auto& words = find_it->second;
//    if (words.size() <= 1) {
//        return false;
//    }
    
    
    bool has_something = false;
    for (auto w = words.begin(); w != words.end(); ++w) {
        dt::Word_Dictionary_Entry* entry = (*w);
        if (entry->name == "thing") {
            continue;
        }
        
        has_something = true;
    }
    
    if (!has_something) {
        return false;
    }
    
    original_to_copy.clear();
    
    mtt::Thing* copy = mtt::Thing_save_as_preset_recursive(world, thing);
    mtt::Thing_ID copy_id = mtt::thing_id(copy);
    copy_arrows(world, thing, original_to_copy, false);
    
    vec3* pos_ptr = mtt::access<vec3>(copy, "position");
    vec3 pos = (pos_ptr != nullptr) ? vec3(0.0f, 0.0f, pos_ptr->z) : vec3(0.0f, 0.0f, 0.0f);
    mtt::Thing_set_position(copy, pos);
    
    auto& presets = world->thing_saved_presets;
    for (auto w = words.begin(); w != words.end(); ++w) {
        dt::Word_Dictionary_Entry* entry = (*w);
        if (entry->name == "thing") {
            continue;
        }
        
        presets[entry->name].insert(copy_id);
    }
    
    return true;
}

mtt::Thing* Thing_make_from_preset_recursive(mtt::World* world, mtt::Thing* thing);
mtt::Thing* Thing_make_from_preset_recursive(mtt::World* world, mtt::Thing* thing)
{
    Thing* copy = nullptr;
    
    auto it_find = original_to_copy.find(thing);
    if (it_find != original_to_copy.end()) {
        copy = it_find->second;
    } else {
        copy = Thing_copy(thing);
        original_to_copy.insert({thing, copy});
    }
    copy->ecs_entity.remove(flecs::Prefab);
    
    
    copy->is_user_destructible = true;
    copy->is_user_drawable = true;
    copy->is_user_destructible = true;
    copy->is_locked = false;
    copy->is_reserved = false;
    set_is_active(copy);
    auto it_find_rendering_state = world->thing_saved_presents_rendering_states.find(mtt::thing_id(thing));
    assert(it_find_rendering_state != world->thing_saved_presents_rendering_states.end());
    (it_find_rendering_state->second) ? mtt::set_should_render(copy) : mtt::unset_should_render(copy);
    
    auto saved_child_set = thing->child_id_set;
    for (auto c_it = saved_child_set.begin(); c_it != saved_child_set.end(); ++c_it) {
        Thing* child = thing->world()->Thing_try_get(*c_it);
        if (child == nullptr) {
            continue;
        }
        
        mtt::Thing* sub_copy = Thing_make_from_preset_recursive(world, child);
        
        mtt::connect_parent_to_child(world, copy, sub_copy);
        
//        copy->child_id_set.push_back(sub_copy->id);
//        sub_copy->parent_thing_id = copy->id;
        sub_copy->ecs_entity.add(thing->world()->ecs_world.lookup("ChildOfTransitive"), copy->ecs_entity);
    }
    
    return copy;
}

mtt::Thing* Thing_make_from_preset(mtt::World* world, const mtt::String& key)
{
    auto& presets = world->thing_saved_presets;
    auto find_it = presets.find(key);
    if (find_it == presets.end()) {
        return nullptr;
    }
    
    auto& choices = find_it->second;
    if (choices.empty()) {
        return nullptr;
    }
    
    auto idx = MTT_Random_range(0, choices.size());
    auto it = choices.begin();
    for (isize i = 0; i < idx; i += 1) {
        ++it;
    }
    mtt::Thing_ID choice_id = *it;
    mtt::Thing* choice_thing = mtt::Thing_try_get(world, choice_id);
    if (choice_thing == nullptr) {
        return nullptr;
    }
    
    original_to_copy.clear();
    
    auto* t = Thing_make_from_preset_recursive(world, choice_thing);
    copy_arrows(world, t, original_to_copy, true);
    return t;
}

mtt::Thing* Thing_make_from_preset(mtt::World* world, const mtt::String& key, mtt::Array_Slice<mtt::String> modifier_list)
{
    auto& presets = world->thing_saved_presets;
    auto find_it = presets.find(key);
    if (find_it == presets.end()) {
        return nullptr;
    }
    
    auto& choices = find_it->second;
    if (choices.empty()) {
        return nullptr;
    }
    
    
    bool thing_found = false;
    mtt::Thing* choice_thing = nullptr;
    for (auto it = choices.begin(); it != choices.end(); ++it) {
        choice_thing = mtt::Thing_try_get(world, *it);
        if (choice_thing == nullptr) {
            
            continue;
        }
        
        auto* attribs = dt::Thing_get_own_attributes(choice_thing);
        if (attribs->empty()) {
            continue;
        }
        
        usize m_idx = 0;
        for (; m_idx < modifier_list.size(); m_idx += 1) {
            bool found = false;
            for (auto w = attribs->begin(); w != attribs->end(); ++w) {
                auto ent_name = (*w)->typename_desc.name();
                cstring name = ent_name.c_str();
                if (name == modifier_list[m_idx]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                break;
            }
        }
        if (modifier_list.size() == m_idx) {
            thing_found = true;
            break;
        }
        
    }
    if (thing_found && choice_thing != nullptr) {
        original_to_copy.clear();
        auto* t = Thing_make_from_preset_recursive(world, choice_thing);
        copy_arrows(world, t, original_to_copy, true);
        return t;
    } else {
        return nullptr;
    }
}

bool Thing_find_preset(mtt::World* world, const mtt::String& key, mtt::Set<mtt::Thing_ID>** out)
{
    auto& presets = world->thing_saved_presets;
    auto find_it = presets.find(key);
    if (find_it == presets.end()) {
        return false;
    }
    
    auto* choices = &find_it->second;
    if (choices->empty()) {
        return false;
    }
    
    *out = choices;
    return true;
}
bool Thing_find_preset(mtt::World* world, const mtt::String& key, mtt::Array_Slice<mtt::String> modifier_list, mtt::Set<mtt::Thing_ID>* in_out)
{
    auto& presets = world->thing_saved_presets;
    auto find_it = presets.find(key);
    if (find_it == presets.end()) {
        return false;
    }
    
    auto& choices = find_it->second;
    if (choices.empty()) {
        return false;
    }
    
    
    bool thing_found = false;
    mtt::Thing* choice_thing = nullptr;
    auto& res = *in_out;
    for (auto it = choices.begin(); it != choices.end(); ++it) {
        choice_thing = mtt::Thing_try_get(world, *it);
        if (choice_thing == nullptr) {
            continue;
        }
        
        auto* attribs = dt::Thing_get_own_attributes(choice_thing);
        if (attribs->empty()) {
            continue;
        }
        
        usize m_idx = 0;
        for (; m_idx < modifier_list.size(); m_idx += 1) {
            bool found = false;
            for (auto w = attribs->begin(); w != attribs->end(); ++w) {
                auto ent_name = (*w)->typename_desc.name();
                cstring name = ent_name.c_str();
                if (name == modifier_list[m_idx]) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                break;
            }
        }
        if (modifier_list.size() == m_idx) {
            res.insert(*it);
        }
        
    }
    if (!res.empty()) {
        return true;
    } else {
        return false;
    }
}


void Thing_defer_enable_to_next_frame(mtt::World* world,mtt::Thing* thing)
{
    auto* thing_rep = mtt::rep(thing);
    for (auto* d_info : thing_rep->render_data.drawable_info_list) {
        d_info->is_enabled = 0;
    }
    
    mtt::unset_is_active(thing);
    
    
    mtt::send_system_message_deferred(&world->message_passer, mtt::MTT_NONE, mtt::thing_id(thing), nullptr, mtt::Procedure_make([](void* state, mtt::Procedure_Input_Output* io) {
        mtt::Thing* thing = io->caller;
        if (thing == nullptr) {
            return mtt::Procedure_Return_Type();
        }
        
        mtt::Rep* rep = mtt::rep(thing);
        for (auto* d_info : rep->render_data.drawable_info_list) {
            d_info->is_enabled = 1;
        }
        
        mtt::set_is_active(thing);
        
        return mtt::Procedure_Return_Type();
    }, world));
}

mtt::Call_Descriptor* Call_Descriptor_from_Thing(mtt::World* world, mtt::Thing_ID thing_id)
{
    mtt::Thing* thing = mtt::Thing_try_get(world, thing_id);
    ASSERT_MSG(thing != nullptr, "%s\n", "should exist");
    return Call_Descriptor_from_Thing(thing);
}
mtt::Call_Descriptor* Call_Descriptor_from_Thing(mtt::Thing* thing)
{
    mtt::Call_Descriptor* call_desc = mtt::access<Call_Descriptor>(thing, "state");
    ASSERT_MSG(call_desc != nullptr, "%s\n", "should exist");
    return call_desc;
}

mtt::Call_Descriptor* Call_Descriptor_from_Script_Instance(Script_Instance* s)
{
    if (mtt::Thing_ID_is_valid(s->caller)) {
        mtt::World* world = mtt::ctx();
        return Call_Descriptor_from_Thing(world, s->caller);
    }
    
    return nullptr;
}

void Script_Property_print(Script_Property* s_prop)
{
    mtt::Any value = {};
    mtt::String label = {};
    mtt::String selector_name = {};
    mtt::String selector_value = {};
    mtt::String scope = mtt::DEFAULT_LOOKUP_SCOPE;
    mtt::String display_annotation = {};
    
    MTT_println_s("(Script_Property) {");
    MTT_print("    %s\n    %s\n    %s\n    %s\n    %s\n", label.c_str(), selector_name.c_str(), selector_value.c_str(), scope.c_str(), display_annotation.c_str());
    MTT_println_s("}");
}

void Call_Descriptor_stop_for_source(Call_Descriptor* call, mtt::Thing_ID thing_to_stop, uint64 stop_time)
{
    //call->sources_to_stop.insert(thing_to_stop);
    Runtime::ctx()->sources_to_stop_for_call[call->call_instruction].insert({thing_to_stop, stop_time});
}



void Thing_overwrite_root_representation(Thing* dst, Thing* src)
{
    mtt::Thing_ID dst_id = mtt::thing_id(dst);
    
    mtt::World* world = mtt::world(src);
    
    mtt::Thing_destroy_proxies(world, dst_id);
    
    vec3* dst_pos_ptr = mtt::access<vec3>(dst, "position");
    vec3 dst_pos = (dst_pos_ptr != nullptr) ? *dst_pos_ptr : vec3(0.0f, 0.0f, 0.0f);
    
    vec3* src_pos_ptr = mtt::access<vec3>(src, "position");
    vec3 src_pos = (dst_pos_ptr != nullptr) ? *src_pos_ptr : vec3(0.0f, 0.0f, 0.0f);
    
    mtt::Thing_set_position(src, dst_pos);
    
    auto& src_rep = *mtt::rep(src);
    auto& copy_rep = *mtt::rep(dst);
    
    mtt::Thing_ID dst_parent = mtt::get_parent_ID(dst);
    if (dst_parent != mtt::Thing_ID_INVALID) {
        mtt::disconnect_child_from_parent(world, dst);
    }
    auto saved_children = dst->child_id_set;
    usize child_count = dst->child_id_set.size();
    for (auto it = saved_children.begin(); it != saved_children.end(); ++it) {
        mtt::disconnect_child_from_parent(world, *it);
    }
    
    copy_properties(dst, src);
    
    {
        copy_rep.model_transform         = src_rep.model_transform;
        copy_rep.model_transform_inverse = src_rep.model_transform_inverse;
        
        
        copy_rep.hierarchy_model_transform         = src_rep.hierarchy_model_transform;
        //copy_rep.world_transform_inverse = src_rep.world_transform_inverse;
        
        copy_rep.pose_transform          = src_rep.pose_transform;
        copy_rep.offset                  = src_rep.offset;
        
        
        
        copy_rep.points                  = src_rep.points;
        copy_rep.points_alt              = src_rep.points_alt;
        copy_rep.radii                   = src_rep.radii;
        copy_rep.colors                  = src_rep.colors;
        copy_rep.representation_types    = src_rep.representation_types;
        
        copy_rep.velocity            = src_rep.velocity;
        copy_rep.acceleration        = src_rep.acceleration;
        copy_rep.saved_velocity      = src_rep.saved_velocity;
        copy_rep.saved_acceleration  = src_rep.saved_acceleration;
        
        copy_rep.init_forward_dir = src_rep.init_forward_dir;
        copy_rep.forward_dir   = src_rep.forward_dir;
        copy_rep.center_offset = src_rep.center_offset;
        
        copy_rep.offset_align = src_rep.offset_align;
        
        copy_rep.transform = src_rep.transform;
        copy_rep.pose_transform_values = src_rep.pose_transform_values;
        //copy_rep.local_transform_values = src_rep.local_transform_values;
        
        //copy_rep.pose_transform_original = src_rep.pose_transform_original;
//        copy_rep.pose_transform_values_original = src_rep.pose_transform_values_original;
        
        for (auto c_it = copy_rep.colliders.begin(); c_it != copy_rep.colliders.end(); ++c_it) {
            Collider_remove((*c_it)->system, 0, *c_it);
            Collider_destroy((*c_it)->system, *c_it);
        }
        copy_rep.colliders.clear();
        
        sd::Renderer* renderer = world->renderer;
        for (usize i = 0; i < copy_rep.render_data.drawable_info_list.size(); i += 1) {
            auto* dr_info = copy_rep.render_data.drawable_info_list[i];
            dr_info->is_enabled = false;
            sd::Drawable_Info_release(renderer, dr_info);
        }
        copy_rep.render_data.drawable_info_list.clear();
        
        if (src->archetype_id == mtt::ARCHETYPE_NUMBER) {
            auto* old_value = mtt::access<float32>(src, "value");
            auto* value = mtt::access<float32>(dst, "value");
            *value = *old_value;
        }
        
        for (auto c_it = src_rep.colliders.begin(); c_it != src_rep.colliders.end(); ++c_it) {
            Collider* c_src = *c_it;
            Collider* c_cpy = Collider_copy(c_src, (void*)dst->id);
            
            copy_rep.colliders.push_back(c_cpy);
            
            Collider_push(c_cpy);
        }
        
        copy_rep.render_data.is_shared = src_rep.render_data.is_shared;
        //copy_rep.render_data.layer = src_rep.render_data.layer;
        
        copy_rep.shader_id = src_rep.shader_id;
        
        //copy_rep.render_data.drawable_info_list.resize(src_rep.render_data.drawable_info_list.size());
        src_rep.render_data.on_copy(static_cast<void*>(dst), copy_rep.render_data, (void*)-1);
        
        copy_rep.render_data.set_on_copy(src_rep.render_data.get_on_copy());
    }
    
    auto& thing_to_word = dt::DrawTalk::ctx()->lang_ctx.dictionary.thing_to_word;
    {
        auto words = thing_to_word.find(dst_id);
        if (words != thing_to_word.end()) {
            //MTT_print("WORDS SIZE: %zu\n", words->second.size());
            auto saved_words = words->second;
            //words->second.clear();
            for (auto w_it = saved_words.begin(); w_it != saved_words.end(); ++w_it) {
                dt::vis_word_underive_from(dst, *w_it);
            }
        }
    }
    {
        auto words = thing_to_word.find(mtt::thing_id(src));
        if (words != thing_to_word.end()) {
            //MTT_print("WORDS SIZE: %zu\n", words->second.size());
            for (auto w_it = words->second.begin(); w_it != words->second.end(); ++w_it) {
                dt::vis_word_derive_from(dst, *w_it);
            }
        }
    }
    
    dt::Thing_remove_all_own_attributes(dst);
    dt::Thing_copy_own_attributes(src, dst);
    
    
    dst->mapped_thing_id = src->mapped_thing_id;
    
    mtt::Thing_set_position(src, src_pos);
    
    if (dst_parent != mtt::Thing_ID_INVALID) {
        mtt::connect_parent_to_child(world, dst_parent, dst_id);
    }
    
    for (auto it = saved_children.begin(); it != saved_children.end(); ++it) {
        mtt::connect_parent_to_child(world, dst_id, *it);
    }
}

void Thing_set_activate_colliders(mtt::Thing* thing)
{
    mtt::Rep* rep = mtt::rep(thing);
    for (usize i = 0; i < rep->colliders.size(); i += 1) {
        //rep->colliders[i]->priority_layer |= (1 << 31);
        rep->colliders[i]->is_inactive = false;
    }
}


void Thing_deactivate_colliders(mtt::Thing* thing)
{
    mtt::Rep* rep = mtt::rep(thing);
    for (usize i = 0; i < rep->colliders.size(); i += 1) {
        //rep->colliders[i]->priority_layer &= ~(1 << 31);
        rep->colliders[i]->is_inactive = true;
    }
}


void thing_refer_to(mtt::Thing* thing, mtt::Thing* tgt)
{
    thing_refer_to(thing, mtt::id(tgt));
}
void thing_refer_to(mtt::Thing* thing, mtt::Thing_ID tgt)
{
    mtt::World* world = mtt::world(thing);
    auto& refs = world->thing_refs;
    
    if (Thing_is_proxy(thing)) {
        thing = Thing_mapped_from_proxy(thing);
    }
    
    mtt::Thing_ID thing_id = mtt::id(thing);
    auto find_it = refs.find(thing_id);
    if (find_it == refs.end()) {
        refs.insert({thing_id, (mtt::Set<mtt::Thing_ID>){tgt}});
    } else {
        find_it->second.insert(tgt);
    }
}
void thing_remove_ref_to(mtt::Thing* thing, mtt::Thing_ID tgt)
{
    if (Thing_is_proxy(thing)) {
        thing = Thing_mapped_from_proxy(thing);
    }
    
    mtt::World* world = mtt::world(thing);
    auto& refs = world->thing_refs;
    
    auto find_it = refs.find(mtt::id(thing));
    if (find_it == refs.end()) {
        return;
    }
    
    find_it->second.erase(tgt);
}
void thing_remove_referrents(mtt::Thing* thing)
{
    mtt::World* world = mtt::world(thing);
    auto& refs = world->thing_refs;
    
    if (Thing_is_proxy(thing)) {
        thing = Thing_mapped_from_proxy(thing);
    }
    
    refs.erase(mtt::id(thing));
}
mtt::Set<mtt::Thing_ID>* thing_referrents(mtt::Thing* thing)
{
    mtt::World* world = mtt::world(thing);
    auto& refs = world->thing_refs;
    
    auto find_it = refs.find(mtt::id(thing));
    if (find_it == refs.end()) {
        return nullptr;
    }
    
    return &find_it->second;
}

mtt::Set<mtt::Thing_ID>* thing_referrents(mtt::World* world, mtt::Thing_ID thing_id)
{
    mtt::Thing* thing = mtt::Thing_try_get(world, thing_id);
    if (thing == nullptr) {
        return nullptr;
    }
    
    return thing_referrents(thing);
}


Script_Shared_Args* Script_shared_args(Script* s)
{
    auto find_it = Script::shared_args.find(s->id);
    if (find_it == Script::shared_args.end()) {
        return nullptr;
    }
    
    return &find_it->second;
}

Script_Shared_Args* Script_shared_args_update(Script* s)
{
    auto find_it = Script::shared_args.find(s->id);
    if (find_it == Script::shared_args.end()) {
        return &Script::shared_args.insert({s->id, (Script_Shared_Args){}}).first->second;
    }
    
    return &find_it->second;
}

void Script_shared_eval_pre(void)
{
    for (auto [k, v] : Script::shared_args) {
        auto* script = &Script::scripts.find(k)->second;
        Script_Shared_Args* args = &v;
        if (v.procedure_pre_script_eval != nullptr) {
            v.procedure_pre_script_eval(script, (void*)args);
        }
    }
}

void Script_shared_eval_post(void)
{
    for (auto [k, v] : Script::shared_args) {
        auto* script = &Script::scripts.find(k)->second;
        Script_Shared_Args* args = &v;
        if (v.procedure_post_script_eval != nullptr) {
            v.procedure_post_script_eval(script, (void*)args);
        }
    }
}

vec3 translation(Transform* xf)
{
    return xf->translation;
}
quat rotation(Transform* xf)
{
    return xf->rotation;
}
float32 rotation_z(Transform* xf)
{
    return glm::eulerAngles(xf->rotation).z;
}
vec3 rotation_euler(Transform* xf)
{
    return glm::eulerAngles(xf->rotation);
}
vec3 scale(Transform* xf)
{
    return xf->scale;
}

void translate(Transform* xf, vec3 translation)
{
    xf->translation += translation;
}
void translate_xy(Transform* xf, float32 x, float32 y)
{
    xf->translation += vec3(x, y, 0);
}
void rotate(Transform* xf, float32 angle_rad, vec3 axis)
{
    xf->rotation = glm::rotate(xf->rotation, (float32)angle_rad, axis);
}
void rotate_z(Transform* xf, float32 angle_rad)
{
    xf->rotation = glm::rotate(xf->rotation, angle_rad, vec3(0, 0, 1));
}
void scale(Transform* xf, vec3 scale)
{
    xf->scale *= scale;
}
void scale_uniform(Transform* xf, float32 scale)
{
    xf->scale *= scale;
}

void calc_trs_mat4(Transform* xf)
{
    xf->matrix = m::scale(m::mat4_identity(), xf->scale) * toMat4(xf->rotation) * m::translate(m::mat4_identity(), xf->translation);
    xf->matrix_inverse = m::inverse(xf->matrix);
}

mat4 trs_mat4(Transform* xf)
{
    return xf->matrix;
}
mat4 trs_mat4_inverse(Transform* xf)
{
    return xf->matrix_inverse;
}

void Transform_init(Transform* xf)
{
    xf->translation = m::vec3_zero();
    xf->rotation = m::quat_identity();
    xf->scale = m::vec3_one();
    xf->matrix = m::mat4_identity();
    xf->matrix_inverse = m::mat4_identity();
    
#ifndef NDEBUG
    xf->began = false;
#endif
    
}
void transform_edit_begin(Transform* xf)
{
#ifndef NDEBUG
    ASSERT_MSG(!xf->began, "%s\n", "cannot begin edit for transform already being edited");
    xf->began = true;
#endif
}
void transform_edit_end(Transform* xf)
{
#ifndef NDEBUG
    ASSERT_MSG(xf->began, "%s\n", "cannot end edit for transform not being edited");
    xf->began = false;
#endif
}

Transform& Transform::do_translate(vec3 translation)
{
    mtt::translate(this, translation);
    return *this;
}
Transform& Transform::do_translate_xy(float32 x, float32 y)
{
    mtt::translate_xy(this, x, y);
    return *this;
}
Transform& Transform::do_rotate(float32 angle_rad, vec3 axis)
{
    mtt::rotate(this, angle_rad, axis);
    return *this;
}
Transform& Transform::do_rotate_z(float32 angle_rad)
{
    mtt::rotate_z(this, angle_rad);
    return *this;
}
Transform& Transform::do_scale(vec3 scale)
{
    mtt::scale(this, scale);
    return *this;
}
Transform& Transform::do_scale_uniform(float32 scale)
{
    mtt::scale_uniform(this, scale);
    return *this;
}


}
