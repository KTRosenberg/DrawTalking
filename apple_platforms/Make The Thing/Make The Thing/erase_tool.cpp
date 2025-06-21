//
//  erase_tool.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 1/17/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#include "erase_tool.hpp"

namespace mtt {



Intersection_Result intersection_test(Input_Record* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags, void* info)
{
#if 0
#define __DEBUG_SD(statement) statement
#else
#define __DEBUG_SD(statement)
#endif
    
    Intersection_Result res;
    res.something_erased = false;
    res.will_be_deleted = false;
    
    
    mtt::Representation& rep = *mtt::rep(thing);
    Collider* collider = rep.colliders.back();
    
    Erase_Tool_Args* args = (Erase_Tool_Args*)info;
    sd::Renderer* renderer = args->renderer;
    
    
    
    mat4 pose_inv = m::inverse(rep.pose_transform);
    mat4 model_inv = m::inverse(rep.model_transform);
    vec3 scale;
    quat orientation;
    vec3 translation;
    vec3 skew;
    vec4 perspective;
    
    m::decompose(rep.model_transform, scale, orientation, translation, skew, perspective);
    
//    vec3 world_point_inv = pose_inv * model_inv * vec4(world_pos, 1.0f);
//    world_point_inv += translation;
    
    vec3 world_point = world_pos;//rep.hierarchy_model_transform * rep.pose_transform * m::translate(rep.offset) * vec4(world_pos, 1.0f);
    world_point = xform(world_point, rep) - rep.offset;
    
    vec3 pose_scale = vec3(vec2(rep.pose_transform_values.scale), 1.0f);
    Circle comp_point;
    comp_point.center = world_point * pose_scale;
    comp_point.radius = 16.0f;
    
    __DEBUG_SD(sd::save(renderer));
    
    __DEBUG_SD(sd::set_render_layer(renderer, LAYER_LABEL_CANVAS_PER_FRAME));
    
    __DEBUG_SD(sd::set_color_rgba_v4(renderer, vec4(1.0f, 0.0f, 0.0f, 1.0f)));
    __DEBUG_SD(sd::begin_polygon(renderer));
    __DEBUG_SD(sd::circle(renderer, comp_point.radius, world_point * pose_scale));
    
    //m::translate(rep.offset);
    auto& curves = rep.points;
    //MTT_print("BEFORE\n");
    for (usize i = 0;;) {
    START_ITER:;
        if (i >= curves.size()) {
            break;
        }
        
        if (rep.representation_types[i] == REPRESENTATION_TYPE_IMAGE) {
            i += 1;
            continue;
        }
        
        auto& curve = curves[i];
        if (curve.size() == 0) MTT_UNLIKELY {
            i += 1;
            continue;
        }
        
        auto remove_curve = [&](usize i) -> bool {
            curves.erase(curves.begin() + i);
            sd::Drawable_Info_release(renderer, rep.render_data.drawable_info_list[i]);
            rep.render_data.drawable_info_list.erase(rep.render_data.drawable_info_list.begin() + i);
            rep.colors.erase(rep.colors.begin() + i);
            rep.radii.erase(rep.radii.begin() + i);
            if (rep.render_data.drawable_info_list.empty()) {
                //mtt::world(thing)->to_destroy_end.push_back(thing->id);
                if (mtt::Thing_is_proxy(thing)) {
                    mtt::Thing_destroy(mtt::world(thing), thing->mapped_thing_id);
                }
                mtt::Thing_destroy(thing);
                thing->is_visible = false;
                res.will_be_deleted = true;
            } else {
                if (!mtt::Thing_is_proxy(thing)) {
                    mtt::Thing_Proxy_Storage* proxies = mtt::Thing_proxies_try_get(thing);
                    if (proxies != nullptr) {
                        for (auto proxy_id : *proxies) {
                            mtt::Thing* proxy;
                            if (!mtt::Thing_try_get(mtt::world(thing), proxy_id, &proxy)) {
                                continue;
                            }
                            auto& proxy_rep = *mtt::rep(proxy);
                            
                            // old
                            if constexpr ((false)) {
                                proxy_rep.points.erase(proxy_rep.points.begin() + i);
                                sd::Drawable_Info_release(renderer, proxy_rep.render_data.drawable_info_list[i]);
                                proxy_rep.render_data.drawable_info_list.erase(proxy_rep.render_data.drawable_info_list.begin() + i);
                                proxy_rep.colors.erase(proxy_rep.colors.begin() + i);
                                proxy_rep.radii.erase(proxy_rep.radii.begin() + i);
                            } else {
                                proxy_rep.render_data.drawable_info_list.erase(proxy_rep.render_data.drawable_info_list.begin() + i);
                            }
                        }
                    }
                } else {
                    // TODO: erase the source via proxy?
                    
                }
            }
            return true;
        };
        
        if (curve.size() == 1) MTT_UNLIKELY {
            Point p;
            p.coord = curve[0];
            bool intersected = Circle_Point_intersection(&comp_point, &p);
            if (intersected) {
                if (remove_curve(i)) {
                    res.something_erased = true;

                    goto START_ITER;
                }
            }
        } else {
            vec3 prev_point = curve[0] * pose_scale;
            for (usize j = 0; j < curve.size() - 1; j += 1) {
//                Point p;
//                p.coord = curve[j];
                
                Line_Segment segment;
                vec3 next_point = curve[j + 1] * pose_scale;;
                segment.a = prev_point;
                segment.b = next_point;
                prev_point = next_point;
                
                
                bool intersected = Line_Segment_Circle_intersection(&segment, &comp_point);
                
                if (intersected) {
                    __DEBUG_SD(sd::set_color_rgba_v4(renderer, vec4(0.0f, 1.0f, 0.0f, 1.0f)));
                    __DEBUG_SD(sd::circle(renderer, 2.0f, vec3(segment.a, 0.0f)));
                    __DEBUG_SD(sd::circle(renderer, 2.0f, vec3(segment.b, 0.0f)));
                    if (remove_curve(i)) {
                        res.something_erased = true;

                        goto START_ITER;
                    }
                } else {
                    __DEBUG_SD(sd::set_color_rgba_v4(renderer, vec4(0.0f, 0.0f, 1.0f, 1.0f)));
                    __DEBUG_SD(sd::circle(renderer, 2.0f, vec3(segment.a, 0.0f)));
                    __DEBUG_SD(sd::circle(renderer, 2.0f, vec3(segment.b, 0.0f)));
                }
            }
        }
        
        i += 1;
    }
    //MTT_print("AFTER\n");
    
//    sd::set_render_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
//    sd::rewind_layer(renderer, LAYER_LABEL_DRAWING_BUFFER);
//    sd::begin_polygon(renderer);
//    {
//        sd::circle(renderer, 5, world_point);
//    }
//    sd::end_polygon(renderer);
    __DEBUG_SD(sd::end_polygon(renderer)->set_transform(m::translate(Mat4(1.0f), vec3(150.0f, 150.0f, 0.0f))));
    __DEBUG_SD(sd::restore(renderer));
    
    return res;
#undef __DEBUG_SD
}

Intersection_Result erase_tool_began(Input_Record* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags, void* info)
{
    return intersection_test(in, thing, world_pos, canvas_pos, event, touch, flags, info);
}
Intersection_Result erase_tool_moved(Input_Record* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags, void* info)
{
    return intersection_test(in, thing, world_pos, canvas_pos, event, touch, flags, info);
}
Intersection_Result erase_tool_cancelled(Input_Record* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags, void* info)
{
    return intersection_test(in, thing, world_pos, canvas_pos, event, touch, flags, info);
}

Intersection_Result erase_tool_ended(Input_Record* in, Thing* thing, vec3 world_pos, vec2 canvas_pos, UI_Event* event, UI_Touch* touch, uint64 flags, void* info)
{
    auto result = intersection_test(in, thing, world_pos, canvas_pos, event, touch, flags, info);
//    if (result.will_be_deleted) {
//        return result;
//    }
    
    mtt::Representation& rep = *mtt::rep(thing);
    
    Erase_Tool_Args* args = (Erase_Tool_Args*)info;
    sd::Renderer* renderer = args->renderer;
    
    
    std::vector<std::vector<vec3>>& curves = rep.points;
    
    Collider* collider = rep.colliders.back();

    
    vec3 scale;
    quat orientation;
    vec3 translation;
    vec3 skew;
    vec4 perspective;
    
    m::decompose(rep.model_transform, scale, orientation, translation, skew, perspective);
    
    mat4 pose_inv = m::inverse(rep.pose_transform);
    mat4 model_inv = m::inverse(rep.model_transform);

#if defined(__ARM_NEON__)
    float32x4_t vres = vdupq_n_f32(POSITIVE_INFINITY);
    uint32x4_t vsignbit { 0, 0, 0x80000000, 0x80000000 };
    
    for (auto& curve : curves) {
        usize count = curve.size();
        
        const float32* curve_ptr = m::value_ptr(curve[0]);
        while (count >= 4) {
            float32x4x4_t vpt = vld1q_f32_x4(curve_ptr);
            {
                float32x4_t vpt2 = vcombine_f32(vget_low_f32(vpt.val[0]), vget_low_f32(vpt.val[0]));
                vpt2 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(vpt2 ), vsignbit));
                vres = vminq_f32(vres, vpt2);
            }
            {
                float32x4_t vpt2 = vcombine_f32(vget_low_f32(vpt.val[1]), vget_low_f32(vpt.val[1]));
                vpt2 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(vpt2 ), vsignbit));
                vres = vminq_f32(vres, vpt2);
            }
            {
                float32x4_t vpt2 = vcombine_f32(vget_low_f32(vpt.val[2]), vget_low_f32(vpt.val[2]));
                vpt2 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(vpt2 ), vsignbit));
                vres = vminq_f32(vres, vpt2);
            }
            {
                float32x4_t vpt2 = vcombine_f32(vget_low_f32(vpt.val[3]), vget_low_f32(vpt.val[3]));
                vpt2 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(vpt2 ), vsignbit));
                vres = vminq_f32(vres, vpt2);
            }
            
            count -= 4;
            curve_ptr += 4 * 4;
        }
                
        while (count != 0) {
            float32x2_t vpt = vld1_f32(curve_ptr);
            float32x4_t vpt2 = vcombine_f32(vpt, vpt);
            vpt2 = vreinterpretq_f32_u32(veorq_u32(vreinterpretq_u32_f32(vpt2), vsignbit));
            vres = vminq_f32(vres, vpt2);
            curve_ptr += 4;
            count -= 1;
        }
    }
    
    vec2 tl = vec2 { vres[0], vres[1] };
    vec2 br = vec2 { -vres[2], -vres[3] };
    
#else
    vec2 tl = {POSITIVE_INFINITY, POSITIVE_INFINITY};
    vec2 br = {NEGATIVE_INFINITY, NEGATIVE_INFINITY};
    
    constexpr const auto curve_increment = (sizeof(vec3) / sizeof(float32));
    
    for (const auto& curve : curves) {
        const float32* curve_ptr = m::value_ptr(curve[0]);
        usize count = curve.size();
        while (count != 0) {
            const auto point = vec3(curve_ptr[0], curve_ptr[1], curve_ptr[2]);
            tl = m::min<vec2>(tl, point);
            br = m::max<vec2>(br, point);
            
            curve_ptr += curve_increment;
            count -= 1;
        }
    }
        
#endif
    
    Collider_remove(collider->system, 0, collider);
    
    {
        auto final_transform = rep.hierarchy_model_transform * m::translate(rep.offset);
        collider->aabb.tl = final_transform * vec4(tl, 0.0f, 1.0f);
        collider->aabb.br = final_transform * vec4(br, 0.0f, 1.0f);
        collider->aabb.half_extent = (br - tl) / 2.0f;
    }
    
    auto adjust_orientation = (rep.forward_dir.x < 0) ? m::inverse(m::toMat4(orientation)) : m::toMat4(orientation);
    
    collider->transform = rep.pose_transform * adjust_orientation * m::scale(Mat4(1.0f), scale);
    
    Box box = calc_transformed_aabb(collider);
    vec3 new_center = vec3((box.tl + box.br) / 2.0, 0.0);

    vec2 old_center = collider->center_anchor;
    collider->center_anchor = new_center;
    
    push_AABBs(collider->system, collider, 1, 0);
    
    vec3 OFF = pose_inv * m::inverse(m::toMat4(orientation)) * vec4((new_center - vec3(old_center, 0.0f)), 1.0f);
    
    rep.model_transform = m::translate(Mat4(1.0f), new_center) * m::toMat4(orientation);
    rep.model_transform_inverse = m::inverse(rep.model_transform);
    rep.hierarchy_model_transform = rep.model_transform;
    
    rep.offset -= OFF;
    
    auto new_xform = rep.model_transform * rep.pose_transform * m::translate(rep.offset);

    for (auto& dr : rep.render_data.drawable_info_list) {
        dr->set_transform(new_xform);
    }
    
    {
        auto old_saved_children = mtt::world(thing)->saved_children;
        if (!mtt::Thing_is_proxy(thing)) {
            mtt::Thing_Proxy_Storage* proxies = mtt::Thing_proxies_try_get(thing);
            if (proxies != nullptr) {
                usize c_idx = 0;
                const usize collider_count = rep.colliders.size();
                for (; c_idx < collider_count; c_idx += 1) {
                    if (collider == rep.colliders[c_idx]) {
                        break;
                    }
                }
                for (auto proxy_id : *proxies) {
                    mtt::Thing* proxy;
                    if (!mtt::Thing_try_get(mtt::world(thing), proxy_id, &proxy)) {
                        continue;
                    }
                    
                    {
                        proxy->world()->saved_children = proxy->child_id_set;
                        mtt::disconnect_parent_from_children(proxy->world(), proxy);
                        
                        auto& proxy_rep = *mtt::rep(proxy);
                        
                        auto* proxy_collider = proxy_rep.colliders[c_idx];
                        auto* proxy_collider_user_data = proxy_collider->user_data;
                        mtt::Collider_copy_into(proxy_collider, collider, proxy_collider_user_data);
                        
                        
                        /*
                        //Collider* c_src = rep.colliders.back();
                        {
                            for (usize i = 0; i < proxy_rep.colliders.size(); i += 1) {
                                mtt::Collider* collider = proxy_rep.colliders[i];
                                
                                mtt::Collider_remove(collider->system, collider->layer, collider);
                                
                                Collider_destroy(collider->system, collider);
                            }
                            proxy_rep.colliders.clear();
                        }
                        
                        for (auto c_it = rep.colliders.begin(); c_it != rep.colliders.end(); ++c_it) {
                            Collider* c_src = *c_it;
                            Collider* c_cpy = Collider_copy(c_src, (void*)proxy->id);
                            
                            proxy_rep.colliders.push_back(c_cpy);
                            
                            Collider_push(c_cpy);
                        }
                        */

                        proxy_rep.offset = rep.offset;
                        
                        if (proxy->saved_parent_thing_id != mtt::Thing_ID_INVALID) {
                            mtt::Thing* parent = nullptr;
                            if (proxy->world()->Thing_try_get(proxy->saved_parent_thing_id, &parent)) {
                                mtt::connect_parent_to_child(proxy->world(), parent, proxy);
                            }
                        }
                        for (auto it = proxy->world()->saved_children.begin(); it != proxy->world()->saved_children.end(); ++it) {
                            mtt::Thing* child = nullptr;
                            if (proxy->world()->Thing_try_get(*it, &child)) {
                                mtt::connect_parent_to_child(proxy->world(), proxy, child);
                            }
                        }
                        proxy->world()->saved_children.clear();
                        
                    }
                }
            }
        } else {
        }
        mtt::world(thing)->saved_children = old_saved_children;
    }
    
    return result;
}

}
