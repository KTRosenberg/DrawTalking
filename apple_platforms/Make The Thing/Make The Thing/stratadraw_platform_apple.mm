//
//  stratadraw_platform_apple.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/24/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "stratadraw_platform_apple.hpp"
#import "sd_metal_renderer.h"

#import <MetalPerformanceShaders/MetalPerformanceShaders.h>


namespace sd {

extern_link_begin()


extern_link_end()

static inline void save_path_renderer(Render_Layer* l)
{
    l->saved_path_renderer = l->path_renderer;
}
static inline void restore_path_renderer(Render_Layer* l)
{
    l->path_renderer = l->saved_path_renderer;
}

void set_transform(Drawable_Info* d_info, const SD_mat4& v)
{
    d_info->set_transform(v);
}
SD_mat4 get_transform(Drawable_Info* d_info)
{
    return d_info->get_transform();
}
SD_mat4* get_ptr_transform(Drawable_Info* d_info)
{
    return d_info->get_ptr_transform();
}
void set_ptr_transform(Drawable_Info* d_info, const SD_mat4* v)
{
    d_info->set_ptr_transform(v);
}

#ifdef __clang__

void set_transform_intrin(Drawable_Info* d_info, const simd_float4x4& v)
{
    memcpy(d_info->get_ptr_transform(), &v, sizeof(simd_float4x4));
}
simd_float4x4 get_transform_intrin(Drawable_Info* d_info)
{
    simd_float4x4 out_var;
    memcpy(&out_var, d_info->get_ptr_transform(), sizeof(simd_float4x4));
    return out_var;
}
void set_ptr_transform_intrin(Drawable_Info* d_info, const simd_float4x4* v)
{
    memcpy(d_info->get_ptr_transform(), v, sizeof(simd_float4x4));
}

#endif

inline static Drawable_Info* Drawable_Info_get_active(sd::Renderer* r, Render_Layer* l)
{
    return &l->drawable_info_list[l->drawable_info_list_count - 1];
}

inline static Drawable_Info* Drawable_Info_reserve_next(sd::Renderer* r, Render_Layer* l);
Drawable_Info* Drawable_Info_reserve_next(sd::Renderer* r, Render_Layer* l)
{
    l->drawable_info_list_count += 1;
    Drawable_Info* rc = Drawable_Info_get_active(r, l);
    rc->buffer_index = l->drawable_info_list_count - 1;
    
    return rc;
}




void reallocate_render_layer_with_allocator(Renderer* renderer, Render_Layer* l)
{
    //MTT_print("REALLOCATING\n");
    mem::Allocator* allocator = l->allocator;
    {
        usize new_count = l->polygon_color_vertices_count_max * 2;
        while (new_count < l->polygon_color_vertices_count * 2) {
            new_count *= 2;
        }
        
        VERTEX_Polygon_Color* saved_vertices = l->polygon_color_vertices;
        l->polygon_color_vertices = (VERTEX_Polygon_Color*)allocator->allocate(allocator, new_count * sizeof(VERTEX_Polygon_Color));
        
        memcpy(l->polygon_color_vertices, saved_vertices, l->polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color));
        allocator->deallocate(allocator, saved_vertices, 1);
        
        l->polygon_color_vertices_count_max   = new_count;
        l->polygon_color_vertices_count_bytes = new_count * sizeof(VERTEX_Polygon_Color);
    }
    {
        usize new_count = l->polygon_color_indices_count_max * 2;
        while (new_count < l->polygon_color_indices_count * 2) {
            new_count *= 2;
        }
        
        Index_Type* saved_indices = l->polygon_color_indices;
        l->polygon_color_indices = (uint32*)allocator->allocate(allocator, sizeof(sd::Index_Type) * new_count);
        
        memcpy(l->polygon_color_indices, saved_indices, l->polygon_color_indices_count_max * sizeof(sd::Index_Type));
        allocator->deallocate(allocator, saved_indices, 1);
        
        l->polygon_color_indices_count_max   = new_count;
        l->polygon_color_indices_count_bytes = new_count * sizeof(sd::Index_Type);
    }
    
    //MTT_print("DONE\n");
}


void reallocate_render_layer_vertices_with_allocator(Renderer* renderer, Render_Layer* l)
{
    //MTT_print("REALLOCATING Vertices layer %llu %d\n", l->layer_index, __LINE__);
    mem::Allocator* allocator = l->allocator;
    {
        usize new_count = l->polygon_color_vertices_count_max * 2;
        while (new_count < l->polygon_color_vertices_count * 2) {
            new_count *= 2;
        }
        
        VERTEX_Polygon_Color* saved_vertices = l->polygon_color_vertices;
        l->polygon_color_vertices = (VERTEX_Polygon_Color*)allocator->allocate(allocator, new_count * sizeof(VERTEX_Polygon_Color));
        
        memcpy(l->polygon_color_vertices, saved_vertices, l->polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color));
        allocator->deallocate(allocator, saved_vertices, 1);
        
        l->polygon_color_vertices_count_max   = new_count;
        l->polygon_color_vertices_count_bytes = new_count * sizeof(VERTEX_Polygon_Color);
        
        //MTT_print("DONE new count = %llu\n", new_count * sizeof(VERTEX_Polygon_Color));
        
    }
    
}

void reallocate_render_layer_indices_with_allocator(Renderer* renderer, Render_Layer* l)
{
    //MTT_print("REALLOCATING Indices layer %llu %d\n", l->layer_index, __LINE__);
    mem::Allocator* allocator = l->allocator;
    {
        usize new_count = l->polygon_color_indices_count_max * 2;
        while (new_count < l->polygon_color_indices_count * 2) {
            new_count *= 2;
        }
        
        uint32* saved_indices = l->polygon_color_indices;
        l->polygon_color_indices = (uint32*)allocator->allocate(allocator, sizeof(sd::Index_Type) * new_count);
        
        memcpy(l->polygon_color_indices, saved_indices, l->polygon_color_indices_count_max * sizeof(sd::Index_Type));
        allocator->deallocate(allocator, saved_indices, 1);
        
        l->polygon_color_indices_count_max   = new_count;
        l->polygon_color_indices_count_bytes = new_count * sizeof(sd::Index_Type);
        
        //MTT_print("DONE new count = %llu\n", new_count * sizeof(sd::Index_Type));
        
    }
}


#define USE_SD_BOUNDS_CHECK (true)

#if (USE_SD_BOUNDS_CHECK)

inline static bool bounds_check(sd::Renderer* renderer)
{
    usize old_size = (renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color));
    
    if (renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count * sizeof(VERTEX_Polygon_Color) >
        renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color) * 0.75f) {
        
        //        MTT_print("Too many vertices, out-of-safe-bounds byte count=[%llu] safe-max byte count=[%llu] * 0.75 = [%llu]\n\n",
        //        renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count * sizeof(VERTEX_Polygon_Color),
        //        renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color),
        //        (usize)(renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color) * 0.75f));
        
        if (renderer->render_layers[renderer->active_render_layer_id].allocator != nullptr) {
            reallocate_render_layer_vertices_with_allocator(renderer, &renderer->render_layers[renderer->active_render_layer_id]);
        } else {
            //MTT_print("ACTIVE_LAYER=[%llu]\n", renderer->active_render_layer_id);
            usize new_size = renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color) * 2;
            while (new_size < (usize)(renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count * sizeof(VERTEX_Polygon_Color) * 0.75f * 2)) {
                new_size *= 2;
            }
            if (!Render_Layer_reallocate_vertex_bytes(renderer, new_size, renderer->active_render_layer_id)) {
                return false;
            }
        }
    }
    if (renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count * sizeof(sd::Index_Type) >
        renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count_max * sizeof(sd::Index_Type) * 0.75f) {
        //        MTT_print("Too many indices, out-of-safe-bounds byte count=[%llu] safe-max byte count=[%llu] * 0.75 = [%llu]\n\n",
        //        renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count * sizeof(sd::Index_Type),
        //        renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count_max * sizeof(sd::Index_Type),
        //        (usize)(renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count_max * sizeof(sd::Index_Type) * 0.75f));
        
        if (renderer->render_layers[renderer->active_render_layer_id].allocator != nullptr) {
            reallocate_render_layer_indices_with_allocator(renderer, &renderer->render_layers[renderer->active_render_layer_id]);
        } else {
            //MTT_print("ACTIVE_LAYER=[%llu]\n", renderer->active_render_layer_id);
            usize new_size = renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count_max * sizeof(sd::Index_Type) * 2;
            while (new_size < renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count * sizeof(sd::Index_Type) * 0.75f * 2) {
                new_size *= 2;
            }
            if (!Render_Layer_reallocate_index_bytes(renderer, new_size, renderer->active_render_layer_id)) {
                return false;
            }
        }
    }
    return true;
}

#else

inline constexpr bool bounds_check(sd::Renderer* renderer) { return true; }

#endif


void set_render_layer(Renderer* renderer, Render_Layer_ID render_layer)
{
    renderer->active_render_layer_id = render_layer;
}

Render_Layer_ID get_render_layer(Renderer* renderer)
{
    return renderer->active_render_layer_id;
}


void begin_path(Renderer* renderer)
{
    renderer->shape_mode = SHAPE_MODE_PATH;
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    Drawable_Info* rc = Drawable_Info_reserve_next(renderer, l);
    
    rc->layer_id = renderer->active_render_layer_id;
    
    rc->type = SHAPE_MODE_PATH;
    
    rc->init_render_data();
    
    // first time
    if (l->drawable_info_list_count == 1) {
        rc->v_buffer_offset = 0;
        rc->v_buffer_count  = 0;
        rc->i_buffer_offset = 0;
        rc->i_buffer_count  = 0;
        
        rc->first_index = 0;
        
        rc->is_enabled = 1;
    } else {
        Drawable_Info* rc_prev = &l->drawable_info_list[l->drawable_info_list_count - 2];
        rc->v_buffer_offset = rc_prev->v_buffer_offset + rc_prev->v_buffer_count;
        rc->v_buffer_count = 0;
        rc->i_buffer_offset = rc_prev->i_buffer_offset + rc_prev->i_buffer_count;
        rc->i_buffer_count = 0;
        
        rc->first_index = l->polygon_color_next_index;
        
        rc->is_enabled = 1;
    }
    
    rc->next_index = 0;
    
    l->path_renderer.strokeColor.r = (renderer->color.r);
    l->path_renderer.strokeColor.g = (renderer->color.g);
    l->path_renderer.strokeColor.b = (renderer->color.b);
    l->path_renderer.strokeColor.a = (renderer->color.a);
    l->path_renderer.beginLine();
    l->path_renderer.cap  = sd::LineRenderer::CAP_ROUND;
    l->path_renderer.join = sd::LineRenderer::JOIN_ROUND;
    
    rc->i_buffer_count_used = rc->i_buffer_count;
    
    l->polygon_color_next_index = 0;
    rc->base_vertex = l->polygon_color_vertices_count;
}

Drawable_Info* end_path(Renderer* renderer)
{
    // Toby TODO: -  use the path buffer instead
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    l->path_renderer.endLine(false);
    
    Drawable_Info* rc = Drawable_Info_get_active(renderer, l);
    
    rc->v_buffer_count  = l->polygon_color_vertices_count - rc->v_buffer_offset;
    rc->i_buffer_count  = (sd::Index_Type)l->polygon_color_indices_count - rc->i_buffer_offset;
    rc->next_index      = l->polygon_color_next_index;
    
    // remove empty shapes
    if (rc->v_buffer_count == 0 || rc->i_buffer_count == 0) {
        l->drawable_info_list_count -= 1;
    }
    
    renderer->shape_mode = SHAPE_MODE_NONE;
    
    rc->is_enabled = true;
    
    rc->i_buffer_count_used = rc->i_buffer_count;
    
    l->polygon_color_next_index = 0;
    
    return rc;
}

Drawable_Info* end_path_closed(Renderer* renderer)
{
    // Toby TODO: -  use the path buffer instead
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    l->path_renderer.endLine(true);
    
    Drawable_Info* rc = Drawable_Info_get_active(renderer, l);
    
    rc->v_buffer_count  = l->polygon_color_vertices_count - rc->v_buffer_offset;
    rc->i_buffer_count  = (sd::Index_Type)l->polygon_color_indices_count - rc->i_buffer_offset;
    rc->next_index      = l->polygon_color_next_index;
    
    // remove empty shapes
    if (rc->v_buffer_count == 0 || rc->i_buffer_count == 0) {
        l->drawable_info_list_count -= 1;
    }
    
    renderer->shape_mode = SHAPE_MODE_NONE;
    
    rc->is_enabled = true;
    
    rc->i_buffer_count_used = rc->i_buffer_count;
    
    return rc;
}

void break_path(Renderer* renderer)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
#if SAVED_LAYER
    if (renderer->shape_mode == SHAPE_MODE_PATH &&
        l->path_renderer.lineVertexCount > 1) {
        l->path_renderer.endLine(false);
        l->path_renderer.beginLine();
    }
#else
    //if (renderer->shape_mode == SHAPE_MODE_PATH)
    {
        l->path_renderer.endLine(false);
        l->path_renderer.beginLine();
    }
#endif
    
    auto* rc = Drawable_Info_get_active(renderer, l);
    rc->i_buffer_count_used = rc->i_buffer_count;
}

void continue_path(Renderer* renderer)
{
    renderer->shape_mode = SHAPE_MODE_PATH;
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    
    
    if (l->drawable_info_list_count == 0) {
        begin_path(renderer);
        
        return;
    } else {
        
        auto* rc = Drawable_Info_get_active(renderer, l);
        //rc->i_buffer_count_used = rc->i_buffer_count;
        
        *rc = l->saved_drawable_info;
        l->saved_drawable_info = {};
        //        l->saved_drawable_info.is_enabled = false;
        
        restore_path_renderer(l);
        
#if SAVED_LAYER
        memcpy(l, &renderer->saved_layer, sizeof(Render_Layer));
#endif
        
        
    }
    
    
    
    
}



Drawable_Info* pause_path(Renderer* renderer)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    //    if (!bounds_check(renderer)) {
    //        abort();
    //    }
    
#if SAVED_LAYER
    memcpy(&renderer->saved_layer, l, sizeof(Render_Layer));
#endif
    
    Drawable_Info* rc = Drawable_Info_get_active(renderer, l);
    
    
    
    l->saved_drawable_info = *rc;
    save_path_renderer(l);
    
    l->path_renderer.endLine(false);
    //    l->path_renderer.circle(l->path_renderer.user_data, l->path_renderer.lx, l->path_renderer.ly, l->path_renderer.r, 14, l->path_renderer.strokeColor);
    //
    rc->v_buffer_count  = l->polygon_color_vertices_count - rc->v_buffer_offset;
    rc->i_buffer_count  = (sd::Index_Type)l->polygon_color_indices_count - rc->i_buffer_offset;
    rc->next_index      = l->polygon_color_next_index;
    
    // remove empty shapes
    //    if (rc->v_buffer_count == 0 || rc->i_buffer_count == 0) {
    //        l->drawable_info_list_count -= 1;
    //    }
    
    renderer->shape_mode = SHAPE_MODE_NONE;
    
    rc->is_enabled = true;
    
    rc->i_buffer_count_used = rc->i_buffer_count;
    
    return rc;
}

void begin_path_no_new_drawable(Renderer* renderer)
{
    renderer->shape_mode = SHAPE_MODE_PATH;
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    Drawable_Info* rc = Drawable_Info_get_active(renderer, l);
    
    rc->layer_id = renderer->active_render_layer_id;
    
    rc->type = SHAPE_MODE_PATH;
    
    
    l->path_renderer.strokeColor.r = (renderer->color.r);
    l->path_renderer.strokeColor.g = (renderer->color.g);
    l->path_renderer.strokeColor.b = (renderer->color.b);
    l->path_renderer.strokeColor.a = (renderer->color.a);
    l->path_renderer.beginLine();
    l->path_renderer.cap  = sd::LineRenderer::CAP_ROUND;
    l->path_renderer.join = sd::LineRenderer::JOIN_ROUND;
    
    rc->i_buffer_count_used = rc->i_buffer_count;
}

void end_path_no_new_drawable(Renderer* renderer)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    l->path_renderer.endLine(false);
    
    renderer->shape_mode = SHAPE_MODE_NONE;
    
}


void begin_polygon(Renderer* renderer)
{
    renderer->shape_mode = SHAPE_MODE_POLYGON;
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    
    Drawable_Info* rc = Drawable_Info_reserve_next(renderer, l);
    
    rc->layer_id = renderer->active_render_layer_id;
    
    rc->type = SHAPE_MODE_POLYGON;
    
    rc->init_render_data();
    
    rc->v_buffer_offset = 0;
    rc->v_buffer_count  = 0;
    rc->i_buffer_offset = 0;
    rc->i_buffer_count  = 0;
    
    // first time
    if (l->drawable_info_list_count == 1) {
        rc->v_buffer_offset = 0;
        rc->v_buffer_count  = 0;
        rc->i_buffer_offset = 0;
        rc->i_buffer_count  = 0;
        
        rc->first_index = 0;
        
        rc->is_enabled = 1;
    } else {
        Drawable_Info* rc_prev = &l->drawable_info_list[l->drawable_info_list_count - 2];
        rc->v_buffer_offset = rc_prev->v_buffer_offset + rc_prev->v_buffer_count;
        rc->v_buffer_count = 0;
        rc->i_buffer_offset = rc_prev->i_buffer_offset + rc_prev->i_buffer_count;
        rc->i_buffer_count = 0;
        
        rc->first_index = l->polygon_color_next_index;
        
        rc->is_enabled = 1;
    }
    rc->next_index = 0;
    
    
    rc->i_buffer_count_used = rc->i_buffer_count;
    
    
    l->polygon_color_next_index = 0;
    rc->base_vertex = l->polygon_color_vertices_count;
}

Drawable_Info* end_polygon(Renderer* renderer)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    ASSERT_MSG(l->drawable_info_list_count < SD_MAX_DRAWABLE_INFO_PER_LAYER, "some corrupted offsets\n");
    Drawable_Info* rc = Drawable_Info_get_active(renderer, l);
    
    
    rc->v_buffer_count  = l->polygon_color_vertices_count - rc->v_buffer_offset;
    rc->i_buffer_count  = l->polygon_color_indices_count - rc->i_buffer_offset;
    rc->next_index      = l->polygon_color_next_index;
    
    // remove empty shapes
    if (rc->v_buffer_count == 0 && rc->i_buffer_count == 0) {
        l->drawable_info_list_count -= 1;
    }
    
    renderer->shape_mode = SHAPE_MODE_NONE;
    
    rc->is_enabled = true;
    
    rc->i_buffer_count_used = rc->i_buffer_count;
    
    l->polygon_color_next_index = 0;
    
    return rc;
}

void begin_polygon_no_new_drawable(Renderer* renderer)
{
    renderer->shape_mode = SHAPE_MODE_POLYGON;
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    Drawable_Info* rc = Drawable_Info_get_active(renderer, l);
    
    rc->type = SHAPE_MODE_POLYGON;
}

void end_polygon_no_new_drawable(Renderer* renderer)
{
    renderer->shape_mode = SHAPE_MODE_NONE;
}

void begin_drawable(Renderer* renderer)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    
    Drawable_Info* rc = Drawable_Info_reserve_next(renderer, l);
    
    rc->layer_id = renderer->active_render_layer_id;
    
    rc->init_render_data();
    
    rc->v_buffer_offset = 0;
    rc->v_buffer_count  = 0;
    rc->i_buffer_offset = 0;
    rc->i_buffer_count  = 0;
    
    // first time
    if (l->drawable_info_list_count == 1) {
        rc->v_buffer_offset = 0;
        rc->v_buffer_count  = 0;
        rc->i_buffer_offset = 0;
        rc->i_buffer_count  = 0;
        
        rc->first_index = 0;
        
        rc->is_enabled = 1;
    } else {
        Drawable_Info* rc_prev = &l->drawable_info_list[l->drawable_info_list_count - 2];
        rc->v_buffer_offset = rc_prev->v_buffer_offset + rc_prev->v_buffer_count;
        rc->v_buffer_count = 0;
        rc->i_buffer_offset = rc_prev->i_buffer_offset + rc_prev->i_buffer_count;
        rc->i_buffer_count = 0;
        
        rc->first_index = l->polygon_color_next_index;
        
        rc->is_enabled = 1;
    }
    rc->next_index = 0;
    
    
    rc->i_buffer_count_used = rc->i_buffer_count;
    
    
    l->polygon_color_next_index = 0;
    rc->base_vertex = l->polygon_color_vertices_count;
}
Drawable_Info* end_drawable(Renderer* renderer)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    ASSERT_MSG(l->drawable_info_list_count < SD_MAX_DRAWABLE_INFO_PER_LAYER, "some corrupted offsets\n");
    Drawable_Info* rc = Drawable_Info_get_active(renderer, l);
    
    
    rc->v_buffer_count  = l->polygon_color_vertices_count - rc->v_buffer_offset;
    rc->i_buffer_count  = l->polygon_color_indices_count - rc->i_buffer_offset;
    rc->next_index      = l->polygon_color_next_index;
    
    // remove empty shapes
    if (rc->v_buffer_count == 0 && rc->i_buffer_count == 0) {
        l->drawable_info_list_count -= 1;
    }
    
    renderer->shape_mode = SHAPE_MODE_NONE;
    
    rc->is_enabled = true;
    
    rc->i_buffer_count_used = rc->i_buffer_count;
    
    l->polygon_color_next_index = 0;
    
    return rc;
}


void set_color_rgba(Renderer* renderer, float32 r, float32 g, float32 b, float32 a)
{
    renderer->prev_color = renderer->color;
    renderer->color[0] = r;
    renderer->color[1] = g;
    renderer->color[2] = b;
    renderer->color[3] = a;
    
    Render_Layer* const l = &renderer->render_layers[renderer->active_render_layer_id];
    
    //    if (renderer->shape_mode == SHAPE_MODE_PATH &&
    //        l->path_renderer.lineVertexCount > 1) {
    //
    //        l->path_renderer.endLine(false);
    //        l->path_renderer.beginLine();
    //    }
    
    l->path_renderer.strokeColor.r = r;
    l->path_renderer.strokeColor.g = g;
    l->path_renderer.strokeColor.b = b;
    l->path_renderer.strokeColor.a = a;
    
}

void set_color_rgba_v4(Renderer* renderer, SD_vec4 color)
{
    return set_color_rgba(renderer, color.r, color.g, color.b, color.a);
}

vec4 get_color_rgba(Renderer* renderer)
{
    return renderer->color;
}

void set_color_hsva(Renderer* renderer, float32 h, float32 s, float32 v, float32 a)
{
    h /= 359.0f;
    return set_color_rgba(renderer, h, s, v, a);
}
void set_color_hsva_v4(Renderer* renderer, SD_vec4 hsva)
{
    hsva[0] /= 359.0f;
    return set_color_rgba_v4(renderer, hsva);
}

vec4 get_background_color(Renderer* renderer)
{
    return renderer->background_color;
}
void set_background_color_rgba(Renderer* renderer, float32 r, float32 g, float32 b, float32 a)
{
    renderer->background_color[0] = r;
    renderer->background_color[1] = g;
    renderer->background_color[2] = b;
    renderer->background_color[3] = a;
}

void set_background_color_rgba_v4(Renderer* renderer, SD_vec4 color)
{
    renderer->background_color = color;
}

void rewind_layer(Renderer* renderer, Render_Layer_ID render_layer)
{
    Render_Layer* const l = &renderer->render_layers[render_layer];
    l->polygon_color_vertices_count  = 0;
    l->polygon_color_indices_count   = 0;
    l->polygon_color_next_index      = 0;
    //    l->path_color_vertices_count     = 0;
    //    l->path_color_indices_count      = 0;
    //    l->path_color_next_index         = 0;
    l->drawable_info_list_count = 0;
}

void rewind_staging_layer(Renderer* renderer, Render_Layer* render_layer)
{
    rewind_layer(renderer, render_layer->layer_index);
    *render_layer = renderer->render_layers[renderer->active_render_layer_id];
}

void seek_layer_to_(Renderer* renderer, Drawable_Info* rc)
{
    
    Render_Layer* l = &renderer->render_layers[rc->layer_id];
    
    l->polygon_color_next_index      = (sd::Index_Type)rc->next_index;
    l->drawable_info_list_count      = rc->buffer_index + 1;
    l->polygon_color_vertices_count  = rc->v_buffer_offset + rc->v_buffer_count;
    l->polygon_color_indices_count   = rc->i_buffer_offset + rc->i_buffer_count;
    
    
    if (rc->buffer_index == SD_MAX_RENDER_LAYERS - 1) {
        return;
    }
    
    (rc + 1)->is_enabled = 0;
    
}




void vertex(Renderer* renderer, float64 x, float64 y, float64 z)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    // create new vertex
    VERTEX_Polygon_Color vertex = {
        simd_make_float4((float32)x, (float32)y, (float32)z, 1.0f),
        simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
        simd_make_float2(0.0f, 0.0f)
    };
    
    VERTEX_Polygon_Color* v_source      = &vertex;
    VERTEX_Polygon_Color* v_destination = v_buf;
    
    
    // copy into buffers, update counts/offsets
    *v_destination = *v_source;
    *i_buf = l->polygon_color_next_index;
    
    
    l->polygon_color_next_index += 1;
    l->polygon_color_vertices_count += 1;
    l->polygon_color_indices_count += 1;
    
}

void vertex_v2(Renderer* renderer, SD_vec2 position)
{
    return vertex(renderer, position.x, position.y, renderer->active_depth);
}

void vertex_v3(Renderer* renderer, SD_vec3 position)
{
    return vertex(renderer, position.x, position.y, position.z);
}

void vertex(Renderer* renderer, float64 x, float64 y, float64 z, SD_vec2 uv)
{
    vertex(renderer, x, y, z, uv, renderer->color);
}

void vertex(Renderer* renderer, float64 x, float64 y, float64 z, SD_vec2 uv, SD_vec4 color)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    // create new vertex
    VERTEX_Polygon_Color vertex = {
        simd_make_float4((float32)x, (float32)y, (float32)z, 1.0f),
        simd_make_float4(color.r, color.g, color.b, color.a),
        simd_make_float2(uv.x, uv.y)
    };
    
    VERTEX_Polygon_Color* v_source      = &vertex;
    VERTEX_Polygon_Color* v_destination = v_buf;
    
    
    // copy into buffers, update counts/offsets
    *v_destination = *v_source;
    *i_buf = l->polygon_color_next_index;
    
    
    l->polygon_color_next_index += 1;
    l->polygon_color_vertices_count += 1;
    l->polygon_color_indices_count += 1;
    
}

mtt::Array_Slice<sd::Index_Type> index_list(Renderer* renderer, sd::Drawable_Info* info, usize size)
{
    if (!bounds_check(renderer)) {
        abort();
    }
    
    Render_Layer* l = &renderer->render_layers[info->layer_id];
    
    Index_Type* i_buf = &l->polygon_color_indices[info->i_buffer_offset];
    
    l->polygon_color_indices_count = (l->polygon_color_indices_count - info->i_buffer_count) + size;
    
    info->i_buffer_count = size;
    info->i_buffer_count_used = info->i_buffer_count;
    
    memset(i_buf, 0, size * sizeof(Index_Type));
    
    return mtt::Array_Slice<sd::Index_Type>::from_buffer(i_buf, size);
}

void set_triangle_indices(sd::Renderer* renderer, mtt::Array_Slice<sd::Index_Type>& index_list, usize idx, Index_Type a, Index_Type b, Index_Type c)
{
    sd::Index_Type* triangle = index_list.data + (idx * 3);
    triangle[0] = a;
    triangle[1] = b;
    triangle[2] = c;
}

void triangle_strip_to_indexed_triangles_in_place(Renderer* renderer, sd::Drawable_Info* info)
{
    usize v_buffer_count = info->v_buffer_count;
    if (v_buffer_count < 3 || !bounds_check(renderer)) {
        return;
    }
    
    Render_Layer* l = &renderer->render_layers[info->layer_id];
    
    Index_Type* i_buf = &l->polygon_color_indices[info->i_buffer_offset];
    
    const usize size = ((v_buffer_count - 2) * 3);
    
    l->polygon_color_indices_count = (l->polygon_color_indices_count - info->i_buffer_count) + size;
    
    info->i_buffer_count = size;
    info->i_buffer_count_used = info->i_buffer_count;
    
    
    for (Index_Type i = 0; i < v_buffer_count - 2; i += 1, i_buf += 3) {
        // preserve winding order
        if (i & 1) {
            i_buf[0] = i + 2;
            i_buf[1] = i + 1;
            i_buf[2] = i;
        } else {
            i_buf[0] = i;
            i_buf[1] = i + 1;
            i_buf[2] = i + 2;
        }
    }
}

void vertex_v2(Renderer* renderer, SD_vec2 position, SD_vec2 uv)
{
    return vertex(renderer, position.x, position.y, renderer->active_depth, uv);
}
void vertex_v3(Renderer* renderer, SD_vec3 position, SD_vec2 uv)
{
    return vertex(renderer, position.x, position.y, position.z, uv);
}

void path_vertex(Renderer* renderer, float64 x, float64 y, float64 z);
void path_vertex(Renderer* renderer, float64 x, float64 y, float64 z)
{
    // Toby TODO: - support z coordinate
    Render_Layer* const l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    l->path_renderer.z = z;
    l->path_renderer.lineVertex(x, y);
}
void path_vertex_v2(Renderer* renderer, SD_vec2 position)
{
    return path_vertex(renderer, position.x, position.y, renderer->active_depth);
}
void path_vertex_v3(Renderer* renderer, SD_vec3 position)
{
    return path_vertex(renderer, position.x, position.y, position.z);
}
void path_list(Renderer* renderer, SD_vec3* list, usize count)
{
    for (usize i = 0; i < count; i += 1) {
        path_vertex_v3(renderer, list[i]);
    }
}
void path_list(Renderer* renderer, SD_vec3* list, usize count, float64* radii)
{
    for (usize i = 0; i < count; i += 1) {
        set_path_radius(renderer, radii[i]);
        path_vertex_v3(renderer, list[i]);
    }
}

void path_list_with_offset(Renderer* renderer, SD_vec3* list, usize count, SD_vec3 offset)
{
    for (usize i = 0; i < count; i += 1) {
        path_vertex_v3(renderer, list[i] + offset);
    }
}

void path_list_with_offset(Renderer* renderer, SD_vec3* list, usize count, SD_vec3 offset, float64* radii)
{
    for (usize i = 0; i < count; i += 1) {
        set_path_radius(renderer, radii[i]);
        path_vertex_v3(renderer, list[i] + offset);
    }
}

void triangle(Renderer* renderer, SD_vec3 a, SD_vec3 b, SD_vec3 c);
void triangle_color(Renderer* renderer, SD_vec3 a, SD_vec3 b, SD_vec3 c, SD_vec4 color);
void triangle(Renderer* renderer, SD_vec3 a, SD_vec3 b, SD_vec3 c)
{
    return triangle_color(renderer, a, b, c, renderer->color);
}

void triangle_color(Renderer* renderer, SD_vec3 a, SD_vec3 b, SD_vec3 c, SD_vec4 color)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    // create new vertex
    VERTEX_Polygon_Color triangle_vertices[] = {
        {
            simd_make_float4((float32)a.x, (float32)a.y, (float32)a.z, 1.0f),
            simd_make_float4(color.r, color.g, color.b, color.a),
            simd_make_float2(0.0f, 0.0f)
        },
        {
            simd_make_float4((float32)b.x, (float32)b.y, (float32)b.z, 1.0f),
            simd_make_float4(color.r, color.g, color.b, color.a),
            simd_make_float2(0.0f, 1.0f)
        },
        {
            simd_make_float4((float32)c.x, (float32)c.y, (float32)c.z, 1.0f),
            simd_make_float4(color.r, color.g, color.b, color.a),
            simd_make_float2(1.0f, 1.0f)
        }
    };
    
    VERTEX_Polygon_Color* v_source      = triangle_vertices;
    VERTEX_Polygon_Color* v_destination = v_buf;
    
    // copy into buffers, update counts/offsets
    
    for (uint32 i = 0; i < 3; i += 1) {
        *v_destination = *v_source;
        v_source += 1;
        v_destination += 1;
    }
    for (uint32 i = 0; i < 3; i += 1) {
        *i_buf = l->polygon_color_next_index;
        i_buf += 1;
        l->polygon_color_next_index += 1;
    }
    
    l->polygon_color_vertices_count += 3;
    l->polygon_color_indices_count += 3;
}

void triangle_per_vertex_color(Renderer* renderer, vec3 a, vec3 b, vec3 c, vec4 color_a, vec4 color_b, vec4 color_c)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    // create new vertex
    VERTEX_Polygon_Color triangle_vertices[] = {
        {
            simd_make_float4((float32)a.x, (float32)a.y, (float32)a.z, 1.0f),
            simd_make_float4(color_a.r, color_a.g, color_a.b, color_a.a),
            simd_make_float2(0.0f, 0.0f)
        },
        {
            simd_make_float4((float32)b.x, (float32)b.y, (float32)b.z, 1.0f),
            simd_make_float4(color_b.r, color_b.g, color_b.b, color_b.a),
            simd_make_float2(0.0f, 1.0f)
        },
        {
            simd_make_float4((float32)c.x, (float32)c.y, (float32)c.z, 1.0f),
            simd_make_float4(color_c.r, color_c.g, color_c.b, color_c.a),
            simd_make_float2(1.0f, 1.0f)
        }
    };
    
    VERTEX_Polygon_Color* v_source      = triangle_vertices;
    VERTEX_Polygon_Color* v_destination = v_buf;
    
    // copy into buffers, update counts/offsets
    
    for (uint32 i = 0; i < 3; i += 1) {
        *v_destination = *v_source;
        v_source += 1;
        v_destination += 1;
    }
    for (uint32 i = 0; i < 3; i += 1) {
        *i_buf = l->polygon_color_next_index;
        i_buf += 1;
        l->polygon_color_next_index += 1;
    }
    
    l->polygon_color_vertices_count += 3;
    l->polygon_color_indices_count += 3;
}

const float64 sqrt3_over_2 = sqrt(3) / 2;
void triangle_equilateral(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side)
{
    
    float32 height = height_sign * side * sqrt3_over_2;
    float32 side_half = height_sign * side * 0.5;
    vec3 vh = origin + vec3(0, height, 0);
    vec3 vl = origin - vec3(side_half, 0, 0);
    vec3 vr = origin + vec3(side_half, 0, 0);
    
    triangle(renderer, vh, vr, vl);
}
void triangle_equilateral_color(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side, vec4 color)
{
    float32 height = height_sign * side * sqrt3_over_2;
    float32 side_half = height_sign * side * 0.5;
    vec3 vh = origin + vec3(0, height, 0);
    vec3 vl = origin - vec3(side_half, 0, 0);
    vec3 vr = origin + vec3(side_half, 0, 0);
    
    triangle_color(renderer, vh, vr, vl, color);
}
void triangle_equilateral_per_vertex_color(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side, vec4 color_height, vec4 color_left, vec4 color_right)
{
    float32 height = height_sign * side * sqrt3_over_2;
    float32 side_half = height_sign * side * 0.5;
    vec3 vh = origin + vec3(0, height, 0);
    vec3 vl = origin - vec3(side_half, 0, 0);
    vec3 vr = origin + vec3(side_half, 0, 0);
    
    const vec4 colors[3] = {color_left, color_right, color_left};
    const usize32 color_idx = (height_sign + 1) >> 1;
    
    triangle_per_vertex_color(renderer, vh, vr, vl, color_height, colors[color_idx + 1], colors[color_idx]);
}

void path_radius(Renderer* renderer, float32 pixel_radius)
{
    renderer->render_layers[renderer->active_render_layer_id].path_renderer.r = pixel_radius;
}

void set_path_radius(Renderer* renderer, float32 pixel_radius)
{
    path_radius(renderer, pixel_radius);
}

float32 get_path_radius(Renderer* renderer)
{
    return renderer->render_layers[renderer->active_render_layer_id].path_renderer.r;
}


void path_arrow_head(Renderer* renderer, SD_vec3 src_point, SD_vec3 dst_point, float32 radius)
{
    float32 angle = m::atan2<float32>(dst_point.x - src_point.x, dst_point.y - src_point.y);
    float32 x = radius * m::cos(angle);
    float32 y = radius * m::sin(angle);
    
    path_vertex_v3(renderer, {dst_point.x - x - y, dst_point.y + y - x, dst_point.z});
    path_vertex_v3(renderer, {dst_point.x, dst_point.y, dst_point.z});
    path_vertex_v3(renderer, {dst_point.x + x - y, dst_point.y - y - x, dst_point.z});
    
    break_path(renderer);
}

void path_cross(Renderer* renderer, const SD_vec3 center, SD_vec3 offset)
{
    const SD_vec3 p3D = center;
    const SD_vec3 segments[2][2] = {
        {
            p3D + (offset * SD_vec3(-1, -1, 1)),
            p3D + (offset * SD_vec3( 1,  1, 1)),
        },
        {
            p3D + (offset * SD_vec3(-1,  1, 1)),
            p3D + (offset * SD_vec3( 1, -1, 1)),
        }
    };
    
    for (usize i = 0; i < 2; i += 1) {
        for (usize j = 0; j < 2; j += 1) {
            sd::path_vertex_v3(renderer, segments[i][j]);
        }
        sd::break_path(renderer);
        
        
    }
}

float
glm_bezier(float s, float p0, float c0, float c1, float p1) {
    float x, xx, ss, xs3, a;
    
    x   = 1.0f - s;
    xx  = x * x;
    ss  = s * s;
    xs3 = (s - ss) * 3.0f;
    a   = p0 * xx + c0 * xs3;
    
    return a + s * (c1 * xs3 + p1 * ss - a);
}

void path_bezier(Renderer* renderer, SD_vec3 src_point, SD_vec3 p_mid_anchor1, SD_vec3 p_mid_anchor2, SD_vec3 dst_point, usize segment_count)
{
    float32 dist = m::dist(src_point, dst_point);
    
    SD_vec3 last_point = src_point;
    const float32 mul = 1.0f / (dist * (1.0f / segment_count));
    for (usize i = 0; i < dist * (1.0f / segment_count); i += 1) {
        auto t = i * mul;
        
        last_point = SD_vec3(
                             glm_bezier(t, src_point.x, p_mid_anchor1.x, p_mid_anchor2.x, dst_point.x),
                             glm_bezier(t, src_point.y, p_mid_anchor1.y, p_mid_anchor2.y, dst_point.y),
                             src_point.z);
        
        path_vertex_v3(renderer, last_point);
    }
    
    path_vertex_v3(renderer, dst_point);
    
    break_path(renderer);
}

void path_bezier_arrow_head_with_tail(Renderer* renderer, SD_vec3 src_point, SD_vec3 p_mid_anchor1, SD_vec3 p_mid_anchor2, SD_vec3 dst_point, float32 radius, usize segment_count)
{
    float32 dist = m::dist(src_point, dst_point);
    
    SD_vec3 last_point = src_point;
    const float32 mul = 1.0f / (dist * (1.0f / segment_count));
    for (usize i = 0; i < dist * (1.0f / segment_count); i += 1) {
        auto t = i * mul;
        
        last_point = SD_vec3(
                             glm_bezier(t, src_point.x, p_mid_anchor1.x, p_mid_anchor2.x, dst_point.x),
                             glm_bezier(t, src_point.y, p_mid_anchor1.y, p_mid_anchor2.y, dst_point.y),
                             src_point.z);
        
        path_vertex_v3(renderer, last_point);
    }
    
    path_vertex_v3(renderer, dst_point);
    
    break_path(renderer);
    
    path_arrow_head(renderer, last_point, dst_point, radius);
}

void path_arrow_head_with_tail(Renderer* renderer, SD_vec3 src_point, SD_vec3 dst_point, float32 radius)
{
    path_vertex_v3(renderer, src_point);
    path_vertex_v3(renderer, dst_point);
    break_path(renderer);
    path_arrow_head(renderer, src_point, dst_point, radius);
}

void path_arrow_head_bidirectional(Renderer* renderer, SD_vec3 src_point, SD_vec3 dst_point, float32 radius)
{
    path_vertex_v3(renderer, src_point);
    path_vertex_v3(renderer, dst_point);
    break_path(renderer);
    path_arrow_head(renderer, src_point, dst_point, radius);
    break_path(renderer);
    path_arrow_head(renderer, dst_point, src_point, radius);
}

void path_quad(Renderer* renderer, SD_vec2 tl, SD_vec2 bl, SD_vec2 br, SD_vec2 tr, float32 depth)
{
    Render_Layer* const l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    l->path_renderer.z = depth;
    l->path_renderer.lineVertex((float32)tl.x, (float32)tl.y);
    l->path_renderer.lineVertex((float32)bl.x, (float32)bl.y);
    l->path_renderer.lineVertex((float32)br.x, (float32)br.y);
    l->path_renderer.lineVertex((float32)tr.x, (float32)tr.y);
    l->path_renderer.lineVertex((float32)tl.x, (float32)tl.y);
}

void quad(Renderer* renderer, SD_vec2 tl, SD_vec2 bl, SD_vec2 br, SD_vec2 tr, float32 depth)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    // create new vertex
    VERTEX_Polygon_Color triangle_vertices[] = {
        {
            simd_make_float4((float32)tl.x, (float32)tl.y, depth, 1.0f),
            simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
            simd_make_float2(0.0f, 0.0f)
        },
        {
            simd_make_float4((float32)bl.x, (float32)bl.y, depth, 1.0f),
            simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
            simd_make_float2(0.0f, 1.0f)
        },
        {
            simd_make_float4((float32)br.x, (float32)br.y, depth, 1.0f),
            simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
            simd_make_float2(1.0f, 1.0f)
        },
        {
            simd_make_float4((float32)tr.x, (float32)tr.y, depth, 1.0f),
            simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
            simd_make_float2(1.0f, 0.0f)
        }
    };
    
    VERTEX_Polygon_Color* v_source      = triangle_vertices;
    VERTEX_Polygon_Color* v_destination = v_buf;
    
    // copy into buffers, update counts/offsets
    
    for (uint32 i = 0; i < 4; i += 1) {
        *v_destination = *v_source;
        v_source += 1;
        v_destination += 1;
    }
    
    *i_buf = l->polygon_color_next_index;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 1;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 2;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 2;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 3;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index;
    
    l->polygon_color_vertices_count += 4;
    l->polygon_color_indices_count  += 6;
    l->polygon_color_next_index     += 4;
}
void quad_color(Renderer* renderer, SD_vec2 tl, SD_vec2 bl, SD_vec2 br, SD_vec2 tr, float32 depth, SD_vec4 ctl, SD_vec4 cbl, SD_vec4 cbr, SD_vec4 ctr)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    // create new vertex
    VERTEX_Polygon_Color triangle_vertices[] = {
        {
            simd_make_float4((float32)tl.x, (float32)tl.y, depth, 1.0f),
            simd_make_float4(ctl.r, ctl.g, ctl.b, ctl.a),
            simd_make_float2(0.0f, 0.0f)
        },
        {
            simd_make_float4((float32)bl.x, (float32)bl.y, depth, 1.0f),
            simd_make_float4(cbl.r, cbl.g, cbl.b, cbl.a),
            simd_make_float2(0.0f, 1.0f)
        },
        {
            simd_make_float4((float32)br.x, (float32)br.y, depth, 1.0f),
            simd_make_float4(cbr.r, cbr.g, cbr.b, cbr.a),
            simd_make_float2(1.0f, 1.0f)
        },
        {
            simd_make_float4((float32)tr.x, (float32)tr.y, depth, 1.0f),
            simd_make_float4(ctr.r, ctr.g, ctr.b, ctr.a),
            simd_make_float2(1.0f, 0.0f)
        }
    };
    
    VERTEX_Polygon_Color* v_source      = triangle_vertices;
    VERTEX_Polygon_Color* v_destination = v_buf;
    
    // copy into buffers, update counts/offsets
    
    for (uint32 i = 0; i < 4; i += 1) {
        *v_destination = *v_source;
        v_source += 1;
        v_destination += 1;
    }
    
    *i_buf = l->polygon_color_next_index;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 1;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 2;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 2;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 3;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index;
    
    l->polygon_color_vertices_count += 4;
    l->polygon_color_indices_count  += 6;
    l->polygon_color_next_index     += 4;
}

void quad_color_uv(Renderer* renderer, SD_vec2 tl, SD_vec2 bl, SD_vec2 br, SD_vec2 tr, float32 depth, SD_vec4 ctl, SD_vec4 cbl, SD_vec4 cbr, SD_vec4 ctr, SD_vec2 uv_tl, SD_vec2 uv_bl, SD_vec2 uv_br, SD_vec2 uv_tr)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    // create new vertex
    VERTEX_Polygon_Color triangle_vertices[] = {
        {
            simd_make_float4((float32)tl.x, (float32)tl.y, depth, 1.0f),
            simd_make_float4(ctl.r, ctl.g, ctl.b, ctl.a),
            simd_make_float2(uv_tl.x, uv_tl.y)
        },
        {
            simd_make_float4((float32)bl.x, (float32)bl.y, depth, 1.0f),
            simd_make_float4(cbl.r, cbl.g, cbl.b, cbl.a),
            simd_make_float2(uv_bl.x, uv_bl.y)
        },
        {
            simd_make_float4((float32)br.x, (float32)br.y, depth, 1.0f),
            simd_make_float4(cbr.r, cbr.g, cbr.b, cbr.a),
            simd_make_float2(uv_br.x, uv_br.y)
        },
        {
            simd_make_float4((float32)tr.x, (float32)tr.y, depth, 1.0f),
            simd_make_float4(ctr.r, ctr.g, ctr.b, ctr.a),
            simd_make_float2(uv_tr.x, uv_tr.y)
        }
    };
    
    VERTEX_Polygon_Color* v_source      = triangle_vertices;
    VERTEX_Polygon_Color* v_destination = v_buf;
    
    // copy into buffers, update counts/offsets
    
    for (uint32 i = 0; i < 4; i += 1) {
        *v_destination = *v_source;
        v_source += 1;
        v_destination += 1;
    }
    
    *i_buf = l->polygon_color_next_index;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 1;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 2;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 2;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 3;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index;
    
    l->polygon_color_vertices_count += 4;
    l->polygon_color_indices_count  += 6;
    l->polygon_color_next_index     += 4;
}

void rectangle_dashed(Renderer* renderer, SD_vec2 tl, SD_vec2 dimensions, float32 depth, usize segments)
{
    ASSERT_MSG(false, "TODO %s\n", __PRETTY_FUNCTION__);
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    SD_vec2 br = tl + dimensions;
    SD_vec2 tr = tl + SD_vec2(dimensions.x, 0.0f);
    SD_vec2 bl = tl + SD_vec2(0.0f, dimensions.y);
    
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    // create new vertex
    VERTEX_Polygon_Color triangle_vertices[] = {
        {
            simd_make_float4((float32)tl.x, (float32)tl.y, depth, 1.0f),
            simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
            simd_make_float2(0.0f, 0.0f)
        },
        {
            simd_make_float4((float32)bl.x, (float32)bl.y, depth, 1.0f),
            simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
            simd_make_float2(0.0f, 1.0f)
        },
        {
            simd_make_float4((float32)br.x, (float32)br.y, depth, 1.0f),
            simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
            simd_make_float2(1.0f, 1.0f)
        },
        {
            simd_make_float4((float32)tr.x, (float32)tr.y, depth, 1.0f),
            simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
            simd_make_float2(1.0f, 0.0f)
        }
    };
    
    VERTEX_Polygon_Color* v_source      = triangle_vertices;
    VERTEX_Polygon_Color* v_destination = v_buf;
    
    // copy into buffers, update counts/offsets
    
    for (uint32 i = 0; i < 4; i += 1) {
        *v_destination = *v_source;
        v_source += 1;
        v_destination += 1;
    }
    
    *i_buf = l->polygon_color_next_index;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 1;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 2;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 2;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index + 3;
    i_buf += 1;
    *i_buf = l->polygon_color_next_index;
    
    l->polygon_color_vertices_count += 4;
    l->polygon_color_indices_count  += 6;
    l->polygon_color_next_index     += 4;
}
void rectangle(Renderer* renderer, SD_vec2 tl, SD_vec2 dimensions, float32 depth)
{
    return quad(renderer,
                tl,
                {tl.x, tl.y + dimensions.y},
                {tl.x + dimensions.x, tl.y + dimensions.y},
                {tl.x + dimensions.x, tl.y},
                depth
                );
}

void rectangle_w_corners(Renderer* renderer, SD_vec2 tl, SD_vec2 br, float32 depth)
{
    return rectangle(renderer, tl, br - tl, depth);
}

// based on RayLib implementation
void rectangle_rounded(Renderer* renderer, SD_vec2 tl, SD_vec2 dimensions, float32 depth, float32 roundness, int32 segments)
{
    // TODO, draws a normal un-rounded rectangle for now
    //return rectangle_(renderer, tl, dimensions, depth);
    
    // credit goes to RayLib:
    
    struct Rectangle {
        float32 x;
        float32 y;
        float32 width;
        float32 height;
    } rec = {tl.x, tl.y, dimensions.x, dimensions.y};
    
    //Not a rounded rectangle
    if ((roundness <= 0.0f) || (rec.width < 1) || (rec.height < 1 )) {
        return rectangle(renderer, tl, dimensions, depth);
    }
    
    if (roundness >= 1.0f) roundness = 1.0f;
    
    // Calculate corner radius
    float radius = (rec.width > rec.height)? (rec.height*roundness)/2 : (rec.width*roundness)/2;
    if (radius <= 0.0f) return;
    
    // Calculate number of segments to use for the corners
    if (segments < 4)
    {
#define SMOOTH_CIRCLE_ERROR_RATE (0.5f)
        // Calculate the maximum angle between segments based on the error rate (usually 0.5f)
        float32 th = acosf(2*powf(1 - SMOOTH_CIRCLE_ERROR_RATE/radius, 2) - 1);
        segments = (int)(ceilf(2*M_PI/th)/4.0f);
        if (segments <= 0) segments = 4;
    }
    
    float stepLength = 90.0f/(float)segments;
    //
    //     /*  Quick sketch to make sense of all of this (there are 9 parts to draw, also mark the 12 points we'll use below)
    //     *  Not my best attempt at ASCII art, just preted it's rounded rectangle :)
    //     *     P0                    P1
    //     *       ____________________
    //     *     /|                    |\
    //     *    /1|          2         |3\
    //     *P7 /__|____________________|__\ P2
    //     *  |   |P8                P9|   |
    //     *  | 8 |          9         | 4 |
    //     *  | __|____________________|__ |
    //     *P6 \  |P11              P10|  / P3
    //     *    \7|          6         |5/
    //     *     \|____________________|/
    //     *     P5                    P4
    //     */
    //
    const Vector2 point[12] = { // coordinates of the 12 points that define the rounded rect (the idea here is to make things easier)
        {(float)rec.x + radius, rec.y}, {(float)(rec.x + rec.width) - radius, rec.y}, { rec.x + rec.width, (float)rec.y + radius }, // PO, P1, P2
        {rec.x + rec.width, (float)(rec.y + rec.height) - radius}, {(float)(rec.x + rec.width) - radius, rec.y + rec.height}, // P3, P4
        {(float)rec.x + radius, rec.y + rec.height}, { rec.x, (float)(rec.y + rec.height) - radius}, {rec.x, (float)rec.y + radius}, // P5, P6, P7
        {(float)rec.x + radius, (float)rec.y + radius}, {(float)(rec.x + rec.width) - radius, (float)rec.y + radius}, // P8, P9
        {(float)(rec.x + rec.width) - radius, (float)(rec.y + rec.height) - radius}, {(float)rec.x + radius, (float)(rec.y + rec.height) - radius} // P10, P11
    };
    
    const Vector2 centers[4] = { point[8], point[9], point[10], point[11] };
    const float angles[4] = { 180.0f, 90.0f, 0.0f, 270.0f };
    
    //#if defined(SUPPORT_QUADS_DRAW_MODE)
    //#else
    //    if (rlCheckBufferLimit(12*segments + 5*6)) rlglDraw(); // 4 corners with 3 vertices per segment + 5 rectangles with 6 vertices each
    //
    //    rlBegin(RL_TRIANGLES);
    // Draw all of the 4 corners: [1] Upper Left Corner, [3] Upper Right Corner, [5] Lower Right Corner, [7] Lower Left Corner
    auto const DEG2RAD = (MTT_PI_32/180.0f);
    for (int k = 0; k < 4; ++k)
    {
        float angle = angles[k];
        const Vector2 center = centers[k];
        for (int i = 0; i < segments; i++)
        {
            sd::vertex_v3(renderer, SD_vec3(center, depth));
            sd::vertex_v3(renderer, SD_vec3(center.x + sinf(DEG2RAD*angle)*radius, center.y + cosf(DEG2RAD*angle)*radius, depth));
            sd::vertex_v3(renderer, SD_vec3(center.x + sinf(DEG2RAD*(angle + stepLength))*radius, center.y + cosf(DEG2RAD*(angle + stepLength))*radius, depth));
            
            angle += stepLength;
        }
    }
    //
    //    // [2] Upper Rectangle
    //    rlColor4ub(color.r, color.g, color.b, color.a);
    //    rlVertex2f(point[0].x, point[0].y);
    //    rlVertex2f(point[8].x, point[8].y);
    //    rlVertex2f(point[9].x, point[9].y);
    //    rlVertex2f(point[1].x, point[1].y);
    //    rlVertex2f(point[0].x, point[0].y);
    //    rlVertex2f(point[9].x, point[9].y);
    sd::triangle(renderer, SD_vec3(point[0], depth), SD_vec3(point[8], depth), SD_vec3(point[9], depth));
    sd::triangle(renderer, SD_vec3(point[1], depth), SD_vec3(point[0], depth), SD_vec3(point[9], depth));
    
    
    //    // [4] Right Rectangle
    //    rlColor4ub(color.r, color.g, color.b, color.a);
    //    rlVertex2f(point[9].x, point[9].y);
    //    rlVertex2f(point[10].x, point[10].y);
    //    rlVertex2f(point[3].x, point[3].y);
    //    rlVertex2f(point[2].x, point[2].y);
    //    rlVertex2f(point[9].x, point[9].y);
    //    rlVertex2f(point[3].x, point[3].y);
    sd::triangle(renderer, SD_vec3(point[9], depth), SD_vec3(point[10], depth), SD_vec3(point[3], depth));
    sd::triangle(renderer, SD_vec3(point[2], depth), SD_vec3(point[9], depth), SD_vec3(point[3], depth));
    //
    //    // [6] Bottom Rectangle
    //    rlColor4ub(color.r, color.g, color.b, color.a);
    //    rlVertex2f(point[11].x, point[11].y);
    //    rlVertex2f(point[5].x, point[5].y);
    //    rlVertex2f(point[4].x, point[4].y);
    //    rlVertex2f(point[10].x, point[10].y);
    //    rlVertex2f(point[11].x, point[11].y);
    //    rlVertex2f(point[4].x, point[4].y);
    
    sd::triangle(renderer, SD_vec3(point[11], depth), SD_vec3(point[5], depth), SD_vec3(point[4], depth));
    sd::triangle(renderer, SD_vec3(point[10], depth), SD_vec3(point[11], depth), SD_vec3(point[4], depth));
    
    //
    //    // [8] Left Rectangle
    //    rlColor4ub(color.r, color.g, color.b, color.a);
    //    rlVertex2f(point[7].x, point[7].y);
    //    rlVertex2f(point[6].x, point[6].y);
    //    rlVertex2f(point[11].x, point[11].y);
    //    rlVertex2f(point[8].x, point[8].y);
    //    rlVertex2f(point[7].x, point[7].y);
    //    rlVertex2f(point[11].x, point[11].y);
    
    sd::triangle(renderer, SD_vec3(point[7], depth), SD_vec3(point[6], depth), SD_vec3(point[11], depth));
    sd::triangle(renderer, SD_vec3(point[8], depth), SD_vec3(point[7], depth), SD_vec3(point[11], depth));
    
    //
    //    // [9] Middle Rectangle
    //    rlColor4ub(color.r, color.g, color.b, color.a);
    //    rlVertex2f(point[8].x, point[8].y);
    //    rlVertex2f(point[11].x, point[11].y);
    //    rlVertex2f(point[10].x, point[10].y);
    //    rlVertex2f(point[9].x, point[9].y);
    //    rlVertex2f(point[8].x, point[8].y);
    //    rlVertex2f(point[10].x, point[10].y);
    //    rlEnd();
    
    sd::triangle(renderer, SD_vec3(point[8], depth), SD_vec3(point[11], depth), SD_vec3(point[10], depth));
    sd::triangle(renderer, SD_vec3(point[9], depth), SD_vec3(point[8], depth), SD_vec3(point[10], depth));
    //#endif
    //}
}


void polygon_convex_regular(Renderer* renderer, float32 radius, SD_vec3 center, usize count_sides)
{
    usize count_tris = count_sides - 2;
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    float32 delta_angle = -MTT_TAU / count_sides;
    float32 c = m::cos(delta_angle);
    float32 s = m::sin(delta_angle);
    m::complex_float32 point{radius, 0.0f};
    m::complex_float32 delta{c, s};
    for (usize p = 0; p < count_sides; p += 1) {
        float32 x = point.real();
        float32 y = point.imag();
        point *= delta;
        
        v_buf->position = simd_make_float4(
                                           x + center.x,
                                           y + center.y,
                                           center.z,
                                           1.0
                                           );
        v_buf->color     = simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a);
        v_buf->tex_coord = simd_make_float2((x/radius + 1)*0.5, (y/radius + 1)*0.5);
        
        v_buf += 1;
        
    }
    
    l->polygon_color_vertices_count += count_sides;
    
    for (uint32 p = 0; p < count_tris; p += 1) {
        i_buf[0] = l->polygon_color_next_index + 0;
        i_buf[1] = l->polygon_color_next_index + p + 1;
        i_buf[2] = l->polygon_color_next_index + p + 2;
        i_buf += 3;
    }
    
    l->polygon_color_next_index += count_sides;
    l->polygon_color_indices_count += 3 * count_tris;
}

void polygon_arc_fraction(Renderer* renderer, float32 radius, SD_vec3 center, usize count_sides, float32 arc_fraction)
{
    usize count_tris = count_sides * arc_fraction;
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    v_buf->position = simd_make_float4(
                                       center.x,
                                       center.y,
                                       center.z,
                                       1.0
                                       );
    v_buf->color     = simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a);
    v_buf->tex_coord = simd_make_float2(0.5, 0.5);
    
    v_buf += 1;
    
    float32 delta_angle = -MTT_TAU / count_sides;
    float32 c = m::cos(delta_angle);
    float32 s = m::sin(delta_angle);
    m::complex_float32 point{radius, 0.0f};
    m::complex_float32 delta{c, s};
    for (usize p = 0; p < count_tris + 2; p += 1) {
        float32 x = point.real();
        float32 y = point.imag();
        point *= delta;
        
        v_buf->position = simd_make_float4(
                                           x + center.x,
                                           y + center.y,
                                           center.z,
                                           1.0
                                           );
        v_buf->color     = simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a);
        v_buf->tex_coord = simd_make_float2((x/radius + 1)*0.5, (y/radius + 1)*0.5);
        
        v_buf += 1;
        
    }
    
    l->polygon_color_vertices_count += count_tris + 1 + 1;
    
    for (uint32 p = 0; p < count_tris; p += 1) {
        i_buf[0] = l->polygon_color_next_index + 0;
        i_buf[1] = l->polygon_color_next_index + p + 1;
        i_buf[2] = l->polygon_color_next_index + p + 2;
        i_buf += 3;
    }
    
    l->polygon_color_next_index += count_tris + 1 + 1;
    l->polygon_color_indices_count += 3 * count_tris;
}

void polygon_convex_regular_v2(Renderer* renderer, float32 radius, SD_vec2 center, usize count_sides)
{
    usize count_tris = count_sides - 2;
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    float32 delta_angle = -MTT_TAU / count_sides;
    float32 c = m::cos(delta_angle);
    float32 s = m::sin(delta_angle);
    m::complex_float32 point{radius, 0.0f};
    m::complex_float32 delta{c, s};
    for (usize p = 0; p < count_sides; p += 1) {
        float32 x = point.real();
        float32 y = point.imag();
        point *= delta;
        
        v_buf->position = simd_make_float4(
                                           x + center.x,
                                           y + center.y,
                                           renderer->active_depth,
                                           1.0
                                           );
        v_buf->color     = simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a);
        v_buf->tex_coord = simd_make_float2((x/radius + 1)*0.5, (y/radius + 1)*0.5);
        
        v_buf += 1;
    }
    
    l->polygon_color_vertices_count += count_sides;
    
    for (uint32 p = 0; p < count_tris; p += 1) {
        i_buf[0] = l->polygon_color_next_index + 0;
        i_buf[1] = l->polygon_color_next_index + p + 1;
        i_buf[2] = l->polygon_color_next_index + p + 2;
        i_buf += 3;
    }
    
    l->polygon_color_next_index += count_sides;
    l->polygon_color_indices_count += 3 * count_tris;
}

void circle(Renderer* renderer, float32 radius, SD_vec3 center)
{
    polygon_convex_regular(renderer, radius, center, 24);
}
void circle(Renderer* renderer, float32 radius, SD_vec3 center, float32 poly_mode_sides)
{
    polygon_convex_regular(renderer, radius, center, poly_mode_sides);
}

void triangle_for_line_renderer(void* user_data, sd::FLOAT x1, sd::FLOAT y1, sd::FLOAT x2, sd::FLOAT y2, sd::FLOAT x3, sd::FLOAT y3, sd::Color c1, sd::Color c2, sd::Color c3);
void triangle_for_line_renderer(void* user_data, sd::FLOAT x1, sd::FLOAT y1, sd::FLOAT x2, sd::FLOAT y2, sd::FLOAT x3, sd::FLOAT y3, sd::Color c1, sd::Color c2, sd::Color c3)
{
    const float z_val = ((sd::Renderer*)user_data)->active_depth;
    
    sd::Renderer* renderer = (sd::Renderer*)user_data;
    
    sd::vertex(renderer,  x1, y1,z_val, SD_vec2(0.0f, 0.0f),  {(float32)c1.r, (float32)c1.g, (float32)c1.b, (float32)c1.a});
    
    sd::vertex(renderer,  x2, y2,z_val, SD_vec2(0.0f, 0.0f),  {(float32)c2.r, (float32)c2.g, (float32)c2.b, (float32)c2.a});
    
    sd::vertex(renderer,  x3, y3,z_val, SD_vec2(0.0f, 0.0f),  {(float32)c3.r, (float32)c3.g, (float32)c3.b, (float32)c3.a});
    //    triangle_color((sd::Renderer*)user_data,
    //                    {x1,y1,z_val}, {x2,y2,z_val}, {x3,y3,z_val},
    //                    {
    //        (float32)fill.r,
    //        (float32)fill.g,
    //        (float32)fill.b,
    //        (float32)fill.a
    //    });
}

void rectangle_for_line_renderer(void* user_data, sd::FLOAT x1, sd::FLOAT y1, sd::FLOAT x2, sd::FLOAT y2, sd::FLOAT x3, sd::FLOAT y3, sd::FLOAT x4, sd::FLOAT y4, sd::FLOAT x5, sd::FLOAT y5, sd::FLOAT x6, sd::FLOAT y6, sd::Color fill);
void rectangle_for_line_renderer(void* user_data, sd::FLOAT x1, sd::FLOAT y1, sd::FLOAT x2, sd::FLOAT y2, sd::FLOAT x3, sd::FLOAT y3, sd::FLOAT x4, sd::FLOAT y4, sd::FLOAT x5, sd::FLOAT y5, sd::FLOAT x6, sd::FLOAT y6, sd::Color fill)
{
    const float z_val = ((sd::Renderer*)user_data)->active_depth;
    
    /*
     triangle_color_((sd::Renderer*)user_data,
     {x1,y1,z_val}, {x2,y2,z_val}, {x3,y3,z_val},
     {
     (float32)fill.r,
     (float32)fill.g,
     (float32)fill.b,
     (float32)fill.a
     });
     */
    //    triangle_color_((sd::Renderer*)user_data,
    //                    {x3,y3,z_val}, {x1,y1,z_val}, {x2,y2,z_val},
    //                    {
    //        (float32)fill.r,
    //        (float32)fill.g,
    //        (float32)fill.b,
    //        (float32)fill.a
    //    });
    //
    //    triangle_color_((sd::Renderer*)user_data,
    //                    {x1,y1,z_val}, {x4,y4,z_val}, {x2,y2,z_val},
    //                    {
    //        (float32)fill.r,
    //        (float32)fill.g,
    //        (float32)fill.b,
    //        (float32)fill.a
    //    });
    return quad((sd::Renderer*)user_data,
                {x3, y3},
                {x1, y1},
                {x2, y2},
                {x6, y6},
                z_val
                );
}

void circle_for_line_renderer(void* user_data, sd::FLOAT x1, sd::FLOAT y1, sd::FLOAT r, usize count_sides, sd::Color fill);
void circle_for_line_renderer(void* user_data, sd::FLOAT x1, sd::FLOAT y1, sd::FLOAT r, usize count_sides, sd::Color fill)
{
    sd::Renderer* renderer = (sd::Renderer*)user_data;
    const float z_val = renderer->active_depth;
    
    vec4 existing_color = renderer->color;
    renderer->color = {fill.r, fill.g, fill.b, fill.a};
    polygon_convex_regular(renderer,
                           r,
                           {x1,y1, z_val},
                           count_sides);
    renderer->color = existing_color;
}

void set_depth(Renderer* renderer, float64 depth)
{
    renderer->active_depth = depth;
}

void triangulate(Renderer* renderer, sd::Point2D_List* solution)
{
    std::vector<sd::N> const & indices = mapbox::earcut<sd::N>(*solution);
    
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    
    if (!bounds_check(renderer)) {
        abort();
    }
    // offsets into buffers
    const usize v_count = l->polygon_color_vertices_count;
    const usize i_count = l->polygon_color_indices_count;
    
    // get current buffer entries
    VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
    Index_Type*           i_buf = &l->polygon_color_indices[i_count];
    
    VERTEX_Polygon_Color* v_destination = v_buf;
    
    usize vertex_count = 0;
    
    // earcut standalone
    for (usize i = 0; i < solution->size(); i += 1) {
        std::vector<Point>* const polycurve = &((*solution)[i]);
        for (usize j = 0; j < polycurve->size(); j += 1) {
            sd::Point* pt = &(*polycurve)[j];
            
            *v_destination = {
                simd_make_float4((float32)((*pt)[0]), (float32)((*pt)[1]), -1.0f, 1.0f),
                simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
                simd_make_float2(0.0f, 0.0f)
            };
            v_destination += 1;
            l->polygon_color_vertices_count += 1;
            vertex_count += 1;
        }
    }
    
    
    // copy into buffers, update counts/offsets
    const uint32 base_idx = l->polygon_color_next_index;
    l->polygon_color_next_index += vertex_count;
    l->polygon_color_indices_count += indices.size();
    for (usize i = 0; i < indices.size(); i += 1) {
        *i_buf = base_idx + indices[i];
        
        i_buf += 1;
    }
}

void contour(Renderer* renderer, std::vector<Boolean_Contour_Element> const & contour)
{
    Clipper2Lib::PathsD solution;
    Clipper2Lib::PathsD solution_open;
    
    Clipper2Lib::ClipperD clipper;
    
    if (contour.empty()) {
        return;
    }
    // all but last
    usize contour_size = contour.size();
    for (usize i = 0; i < contour_size - 1; i += 1) {
        Clipper2Lib::PathsD path_section(contour[i].paths.size());
        
        {
            for (usize path_idx = 0; path_idx < contour[i].paths.size(); path_idx += 1) {
                const std::vector<vec2>* const path = &contour[i].paths[path_idx];
                
                for (usize pt_idx = 0; pt_idx < path->size(); pt_idx += 1) {
                    const SD_vec2* const pt = &(*path)[pt_idx];
                    Clipper2Lib::PointD pt_d;
                    pt_d.x = pt->x;
                    pt_d.y = pt->y;
                    path_section[path_idx].push_back(pt_d);
                }
            }
            
            switch (contour[i].clip_polygon_type) {
                case CLIP_POLYGON_TYPE_SUBJECT: {
                    (contour[i].is_closed) ? clipper.AddSubject(path_section) : clipper.AddOpenSubject(path_section);
                    break;
                }
                case CLIP_POLYGON_TYPE_CLIP: {
                    clipper.AddClip(path_section);
                    break;
                }
                default: { break; }
            }
            //clipper.AddPaths(path_section, (Clipper2Lib::PathType)contour[i].clip_polygon_type, !contour[i].is_closed);
        }
        
        switch (contour[i].clip_operation_type) {
            case sd::CLIP_OPERATION_TYPE_NONE: {
                break;
            }
            default: {
                clipper.Execute((Clipper2Lib::ClipType)contour[i].clip_operation_type, Clipper2Lib::FillRule::NonZero, solution, solution_open);
                clipper.Clear();
                (contour[i].is_closed) ? clipper.AddSubject(solution) : clipper.AddOpenSubject(solution_open);
                //clipper.AddPaths(solution, (Clipper2Lib::PolyType)sd::CLIP_POLYGON_TYPE_SUBJECT, !contour[i].is_closed);
                
                break;
            }
        }
    }
    // last, must execute
    {
        const usize i = contour.size() - 1;
        Clipper2Lib::PathsD path_section(contour[i].paths.size());
        
        {
            for (usize path_idx = 0; path_idx < contour[i].paths.size(); path_idx += 1) {
                const std::vector<vec2>* const path = &contour[i].paths[path_idx];
                
                for (usize pt_idx = 0; pt_idx < path->size(); pt_idx += 1) {
                    const SD_vec2* const pt = &(*path)[pt_idx];
                    Clipper2Lib::PointD pt_d;
                    pt_d.x = pt->x;
                    pt_d.y = pt->y;
                    path_section[path_idx].push_back(pt_d);
                }
            }
            
            switch (contour[i].clip_polygon_type) {
                case CLIP_POLYGON_TYPE_SUBJECT: {
                    (contour[i].is_closed) ? clipper.AddSubject(path_section) : clipper.AddOpenSubject(path_section);
                    break;
                }
                case CLIP_POLYGON_TYPE_CLIP: {
                    clipper.AddClip(path_section);
                    break;
                }
                default: { break; }
            }
            
            clipper.Execute((Clipper2Lib::ClipType)contour[i].clip_operation_type, Clipper2Lib::FillRule::NonZero, solution, solution_open);
            
            //clipper.AddPaths(path_section, (ClipperLib::PolyType)contour[i].clip_polygon_type, contour[i].is_closed);
            //            clipper.Execute((ClipperLib::ClipType)contour[i].clip_operation_type, solution, ClipperLib::pftNonZero, ClipperLib::pftNonZero);
        }
    }
    
    if (solution.size() < 1 && solution_open.size() < 1) {
        return;
    }
    
    Clipper2Lib::PathsD* solutions[2] = { &solution, &solution_open };
    
    for (usize s = 0; s < 2; s += 1) {
        auto& solution = *solutions[s];
        std::vector<sd::N> const & indices = mapbox::earcut<sd::N>(solution);
        
        Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
        
        if (!bounds_check(renderer)) {
            abort();
        }
        // offsets into buffers
        const usize v_count = l->polygon_color_vertices_count;
        const usize i_count = l->polygon_color_indices_count;
        
        // get current buffer entries
        VERTEX_Polygon_Color* v_buf = &l->polygon_color_vertices[v_count];
        Index_Type*           i_buf = &l->polygon_color_indices[i_count];
        
        VERTEX_Polygon_Color* v_destination = v_buf;
        
        usize vertex_count = 0;
        
        // earcut standalone
        //    for (usize i = 0; i < contour.size(); i += 1) {
        //        std::vector<Point>* const polycurve = &contour[i];
        //        for (usize j = 0; j < polycurve->size(); j += 1) {
        //            sd::Point* pt = &(*polycurve)[j];
        //
        //            *v_destination = {
        //                simd_make_float4((float32)((*pt)[0]), (float32)((*pt)[1]), -1.0f, 1.0f),
        //                simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
        //                simd_make_float2(0.0f, 0.0f)
        //            };
        //            v_destination += 1;
        //            l->polygon_color_vertices_count += 1;
        //            vertex_count += 1;
        //        }
        //    }
        
        // earcut with clipper
        for (usize i = 0; i < solution.size(); i += 1) {
            Clipper2Lib::PathD* path = &solution[i];
            for (usize j = 0; j < path->size(); j += 1) {
                Clipper2Lib::PointD* pt = &(*path)[j];
                
                *v_destination = {
                    simd_make_float4((float32)(pt->x), (float32)(pt->y), renderer->active_depth, 1.0f),
                    simd_make_float4(renderer->color.r, renderer->color.g, renderer->color.b, renderer->color.a),
                    simd_make_float2(0.0f, 0.0f)
                };
                v_destination += 1;
                l->polygon_color_vertices_count += 1;
                vertex_count += 1;
            }
        }
        
        // copy into buffers, update counts/offsets
        const Index_Type base_idx = l->polygon_color_next_index;
        l->polygon_color_next_index += vertex_count;
        l->polygon_color_indices_count += indices.size();
        for (usize i = 0; i < indices.size(); i += 1) {
            *i_buf = base_idx + indices[i];
            
            i_buf += 1;
        }
    }
}

void Renderer_init(Renderer* renderer, mem::Allocator* allocator)
{
    renderer->cmd_allocator_backing = *allocator;
    mem::Arena_init(&renderer->cmd_mem, renderer->cmd_allocator_backing, sizeof(Command) * 2048 * 16, alignof(Command));
    renderer->cmd_allocator = mem::Arena_Allocator(&renderer->cmd_mem);
    
    renderer->projection = SD_mat4_identity();
    renderer->view       = SD_mat4_identity();
    renderer->active_render_layer_id = 0;
    renderer->saved_render_layer_id  = 0;
    renderer->render_layer_max_count = SD_MAX_RENDER_LAYERS;
    renderer->render_layer_count = 0;
    
    renderer->render_layers = (sd::Render_Layer*)calloc(sizeof(sd::Render_Layer), renderer->render_layer_max_count + 1);
    
    renderer->cmd_lists = (sd::Command_List*)calloc(sizeof(sd::Command_List), 512);
    
    renderer->shape_mode = SHAPE_MODE_NONE;
    
    renderer->active_depth = 0.0;
    
    renderer->cmd_list_count = 0;
    
    renderer->color            = {1.0f, 1.0f, 1.0f, 1.0f};
    renderer->background_color = {0.0f, 0.0f, 0.0f, 1.0f};
    
    
    
}

void set_view(Renderer* renderer, SD_mat4 transform)
{
    renderer->view = transform;
}
SD_mat4 get_view(Renderer* renderer)
{
    return renderer->view;
}

usize Render_Layer_sizeof_(void)
{
    return sizeof(Render_Layer);
}

usize Render_Layer_sizeof_polygon_vertex_(void)
{
    return sizeof(VERTEX_Polygon_Color);
}

usize Render_Layer_sizeof_path_vertex_(void)
{
    return sizeof(VERTEX_Path_Color);
}

usize Render_Layer_sizeof_index_(void)
{
    return sizeof(uint32);
}

usize Renderer_sizeof_(void)
{
    return sizeof(Renderer);
}

void Render_Layer_copy_(Renderer* renderer, usize to, Render_Layer* from)
{
    
}

//void Render_Layer_copy_at_(Renderer* renderer, Drawable_Info* to_command, Drawable_Info* from_command)
//{
//    Render_Layer* lto   = &renderer->render_layers[to_command->layer_id];
//    Render_Layer* lfrom = &renderer->render_layers[from_command->layer_id];
//
//    if (to_command->buffer_index == lto->drawable_info_list_count - 1) {
//        // vertices
//        VERTEX_Polygon_Color* to_vbegin   = &lto->polygon_color_vertices[to_command->v_buffer_offset];
//
//        VERTEX_Polygon_Color* from_vbegin = &lfrom->polygon_color_vertices[from_command->v_buffer_offset];
//        usize from_vertex_count           =  lfrom->polygon_color_vertices_count - from_command->v_buffer_offset;
//
//        memcpy(to_vbegin, from_vbegin, from_vertex_count);
//
//        // indices
//        Index_Type* to_ibegin = &lto->polygon_color_indices[to_command->i_buffer_offset];
//
//        Index_Type* from_ibegin    = &lfrom->polygon_color_indices[from_command->i_buffer_offset];
//        usize from_index_count = lfrom->polygon_color_vertices_count - from_command->i_buffer_offset;
//
//        memcpy(to_ibegin, from_ibegin, from_index_count);
//
//        for (usize i = 0; i < from_index_count; i += 1) {
//            to_ibegin[i] += to_command->first_index;
//        }
//
//
//        if (to_command->buffer_index == 0) {
//            memcpy(to_command,
//                   from_command,
//                   sizeof(Drawable_Info) * (lfrom->drawable_info_list_count - from_command->buffer_index));
//
//                lto->drawable_info_list_count += lfrom->drawable_info_list_count - from_command->buffer_index;
//
//        } else {
//            usize first_index = to_command->first_index;
//            usize v_buffer_offset = to_command->v_buffer_offset;
//            usize i_buffer_offset = to_command->i_buffer_count;
//
//            memcpy(to_command,
//                   from_command,
//                   sizeof(Drawable_Info) * (lfrom->drawable_info_list_count - from_command->buffer_index));
//
//            for (usize i = 0; i < lfrom->drawable_info_list_count; i += 1) {
//                to_command->first_index     += first_index;
//                to_command->next_index      += first_index;
//                to_command->v_buffer_offset += v_buffer_offset;
//                to_command->i_buffer_offset += i_buffer_offset;
//            }
//
//            lto->drawable_info_list_count += lfrom->drawable_info_list_count - from_command->buffer_index;
//        }
//
//    } else {
//        // shift
//        {
//            Drawable_Info*       to_cmd_shift = to_command + 1;
//            VERTEX_Polygon_Color* to_v_shift = &lto->polygon_color_vertices[to_cmd_shift->v_buffer_offset];
//            Index_Type*               to_i_shift = &lto->polygon_color_indices[to_cmd_shift->i_buffer_offset];
//
//
//            usize from_vertex_count = lfrom->polygon_color_vertices_count - from_command->v_buffer_offset;
//            usize from_index_count  = lfrom->polygon_color_indices_count - from_command->i_buffer_offset;
//
//            usize to_vertex_count = lto->polygon_color_vertices_count - to_command->v_buffer_offset - to_command->v_buffer_count;
//            usize to_index_count  = lto->polygon_color_indices_count - to_command->i_buffer_offset - to_command->i_buffer_offset;
//        }
//        // vertices
//        VERTEX_Polygon_Color* to_vbegin   = &lto->polygon_color_vertices[to_command->v_buffer_offset];
//
//        VERTEX_Polygon_Color* from_vbegin = &lfrom->polygon_color_vertices[from_command->v_buffer_offset];
//        usize from_vertex_count           =  lfrom->polygon_color_vertices_count - from_command->v_buffer_offset;
//
//        memcpy(to_vbegin, from_vbegin, from_vertex_count);
//
//        // indices
//        uint32* to_ibegin = &lto->polygon_color_indices[to_command->i_buffer_offset];
//
//        uint32* from_ibegin    = &lfrom->polygon_color_indices[from_command->i_buffer_offset];
//        usize from_index_count = lfrom->polygon_color_indices_count - from_command->i_buffer_offset;
//
//        memcpy(to_ibegin, from_ibegin, from_index_count);
//
//        for (usize i = 0; i < from_index_count; i += 1) {
//            to_ibegin[i] += to_command->first_index;
//
//        }
//    }
//
//
//}

/////
///
//[[deprecated]]
//void Render_Layer_insert_copy_at_(Renderer* renderer, Drawable_Info* to_command, Drawable_Info* from_command)
//{
//    Render_Layer* lto   = &renderer->render_layers[to_command->layer_id];
//    Render_Layer* lfrom = &renderer->render_layers[from_command->layer_id];
//
//    if ((false) && to_command->buffer_index == lto->drawable_info_list_count - 1) {
//        // vertices
//        VERTEX_Polygon_Color* to_vbegin   = &lto->polygon_color_vertices[to_command->v_buffer_offset];
//
//        VERTEX_Polygon_Color* from_vbegin = &lfrom->polygon_color_vertices[from_command->v_buffer_offset];
//        usize from_vertex_count           =  lfrom->polygon_color_vertices_count - from_command->v_buffer_offset;
//
//        memcpy(to_vbegin, from_vbegin, from_vertex_count);
//
//        // indices
//        uint32* to_ibegin = &lto->polygon_color_indices[to_command->i_buffer_offset];
//
//        uint32* from_ibegin    = &lfrom->polygon_color_indices[from_command->i_buffer_offset];
//        usize from_index_count = lfrom->polygon_color_vertices_count - from_command->i_buffer_offset;
//
//        memcpy(to_ibegin, from_ibegin, from_index_count);
//
//        for (usize i = 0; i < from_index_count; i += 1) {
//            to_ibegin[i] += to_command->first_index;
//        }
//
//
//        if (to_command->buffer_index == 0) {
//            memcpy(to_command,
//                   from_command,
//                   sizeof(Drawable_Info) * (lfrom->drawable_info_list_count - from_command->buffer_index));
//
//                lto->drawable_info_list_count += lfrom->drawable_info_list_count - from_command->buffer_index;
//
//        } else {
//            usize first_index = to_command->first_index;
//            usize v_buffer_offset = to_command->v_buffer_offset;
//            usize i_buffer_offset = to_command->i_buffer_count;
//
//            memcpy(to_command,
//                   from_command,
//                   sizeof(Drawable_Info) * (lfrom->drawable_info_list_count - from_command->buffer_index));
//
//            for (usize i = 0; i < lfrom->drawable_info_list_count; i += 1) {
//                to_command->first_index     += first_index;
//                to_command->next_index      += first_index;
//                to_command->v_buffer_offset += v_buffer_offset;
//                to_command->i_buffer_offset += i_buffer_offset;
//            }
//
//            lto->drawable_info_list_count += lfrom->drawable_info_list_count - from_command->buffer_index;
//        }
//
//    } else {
//        // shift
//        {
//            Drawable_Info*       to_cmd_shift = to_command + 1;
//            VERTEX_Polygon_Color* to_v_shift = &lto->polygon_color_vertices[to_cmd_shift->v_buffer_offset];
//            uint32*               to_i_shift = &lto->polygon_color_indices[to_cmd_shift->i_buffer_offset];
//
//
//            usize from_vertex_count = lfrom->polygon_color_vertices_count - from_command->v_buffer_offset;
//            usize from_index_count  = lfrom->polygon_color_indices_count  - from_command->i_buffer_offset;
//
//            usize vertex_count_to_shift = lto->polygon_color_vertices_count - to_cmd_shift->v_buffer_offset;
//            usize index_count_to_shift  = lto->polygon_color_indices_count  - to_cmd_shift->i_buffer_offset;
//
//            isize how_many_vertices_to_shift = lfrom->polygon_color_vertices_count - from_command->v_buffer_offset - (to_command->v_buffer_offset) - to_command->v_buffer_count;
//            isize how_many_indices_to_shift  = lfrom->polygon_color_indices_count - from_command->i_buffer_offset - (to_command->i_buffer_offset) - to_command->i_buffer_count;
//
//            if (how_many_vertices_to_shift <= 0) {
//                if (how_many_indices_to_shift <= 0) {
//
//                    MTT_print("%lld %lld insertion requires no shifting, size difference <= 0\n", how_many_vertices_to_shift, how_many_indices_to_shift);
//
//
//                    // data shrank
//
//                    // offset indices for copying into the buffer
//                    for (usize i = from_command->i_buffer_offset; i < lfrom->polygon_color_indices_count; i += 1) {
//                        lfrom->polygon_color_indices[i] += to_command->i_buffer_offset;
//                    }
//
//                    for (usize i = from_command->buffer_index; i < lfrom->drawable_info_list_count; i += 1) {
//                        lfrom->drawable_info_list[i].buffer_index   += to_command->buffer_index;
//                        lfrom->drawable_info_list[i].i_buffer_offset += to_command->i_buffer_offset;
//                        lfrom->drawable_info_list[i].v_buffer_offset += to_command->v_buffer_offset;
//                        lfrom->drawable_info_list[i].first_index     += to_command->first_index;
//                        lfrom->drawable_info_list[i].next_index      += to_command->first_index;
//                        lfrom->drawable_info_list[i].layer_id      = to_command->layer_id;
//                    }
//
//                    memcpy(
//                           &lto->polygon_color_vertices[to_command->v_buffer_offset],
//                           &lfrom->polygon_color_vertices[from_command->v_buffer_offset],
//                           lfrom->polygon_color_vertices_count - from_command->v_buffer_offset
//                    );
//
//                    lto->drawable_info_list_count += lfrom->drawable_info_list_count - from_command->buffer_index;
//
//                } else {
//                    MTT_print("%lld fewer vertices, %lld more indices", how_many_vertices_to_shift, how_many_indices_to_shift);
//                }
////            for (usize i = 0; i < index_count_to_shift; i += 1) {
////                to_i_shift[i] += vertex_count_to_shift;
////            }
//            } else {
//                if (how_many_indices_to_shift <= 0) {
//                    MTT_print("%lld more vertices, %lld fewer indices", how_many_vertices_to_shift, how_many_indices_to_shift);
//                } else {
//                    MTT_print("%lld more vertices, %lld more indices", how_many_vertices_to_shift, how_many_indices_to_shift);
//                }
//            }
//
//        }
//        return;
//        // vertices
//        VERTEX_Polygon_Color* to_vbegin   = &lto->polygon_color_vertices[to_command->v_buffer_offset];
//
//        VERTEX_Polygon_Color* from_vbegin = &lfrom->polygon_color_vertices[from_command->v_buffer_offset];
//        usize from_vertex_count           =  lfrom->polygon_color_vertices_count - from_command->v_buffer_offset;
//
//        memcpy(to_vbegin, from_vbegin, from_vertex_count);
//
//        // indices
//        uint32* to_ibegin = &lto->polygon_color_indices[to_command->i_buffer_offset];
//
//        uint32* from_ibegin    = &lfrom->polygon_color_indices[from_command->i_buffer_offset];
//        usize from_index_count = lfrom->polygon_color_indices_count - from_command->i_buffer_offset;
//
//        memcpy(to_ibegin, from_ibegin, from_index_count);
//
//        for (usize i = 0; i < from_index_count; i += 1) {
//            to_ibegin[i] += to_command->first_index;
//
//        }
//    }
//
//
//}


#define SD_ALIGN
// user staging buffer, must be copied into an internal buffer using Render_Layer_copy()
void make_staging_layer_(Renderer* renderer,
                         Render_Layer** location,
                         void* vertex_memory,
                         usize vertex_byte_count,
                         void* index_memory,
                         usize index_byte_count) {
    
    // TODO: eventually
    
    Render_Layer* l = *location;
    
    
    l->polygon_color_vertices = (VERTEX_Polygon_Color*)vertex_memory;
    l->polygon_color_vertices_count_max = vertex_byte_count / sizeof(VERTEX_Polygon_Color);
    l->polygon_color_vertices_count = 0;
    l->polygon_color_vertices_count_bytes = vertex_byte_count;
    
    l->polygon_color_indices = (uint32*)index_memory;
    l->polygon_color_indices_count_max = index_byte_count / sizeof(uint32);
    l->polygon_color_indices_count     = 0;
    l->polygon_color_indices_count_bytes = index_byte_count;
    
    l->path_renderer.user_data = (void*)renderer;
    l->path_renderer.triangle = triangle_for_line_renderer;
    l->path_renderer.rectangle = rectangle_for_line_renderer;
    l->path_renderer.circle = circle_for_line_renderer;
    // Toby TODO: - provide external way to set line radius in pixels
    l->path_renderer.r = 2;
    
    l->drawable_info_list_count = 0;
    
    
    l->polygon_color_next_index = 0;
    
    l->layer_index = SD_MAX_RENDER_LAYERS;
}

void make_staging_layer_with_allocator_(
                                        Renderer* renderer,
                                        Render_Layer** location,
                                        usize vertex_instance_count,
                                        usize index_instance_count,
                                        mem::Allocator* allocator
                                        ) {
    Render_Layer* l = (Render_Layer*)allocator->allocate(allocator, sizeof(Render_Layer));
    
    l->polygon_color_vertices = (VERTEX_Polygon_Color*)allocator->allocate(allocator, sizeof(VERTEX_Polygon_Color) * vertex_instance_count);
    
    l->polygon_color_vertices_count_max = vertex_instance_count;
    l->polygon_color_vertices_count = 0;
    l->polygon_color_vertices_count_bytes = vertex_instance_count * sizeof(VERTEX_Polygon_Color);
    
    l->polygon_color_indices = (uint32*)allocator->allocate(allocator, sizeof(uint32) * index_instance_count);
    l->polygon_color_indices_count_max = index_instance_count;
    l->polygon_color_indices_count     = 0;
    l->polygon_color_indices_count_bytes = index_instance_count * sizeof(uint32);
    
    l->path_renderer.user_data = (void*)renderer;
    l->path_renderer.triangle = triangle_for_line_renderer;
    l->path_renderer.rectangle = rectangle_for_line_renderer;
    l->path_renderer.circle = circle_for_line_renderer;
    // Toby TODO: - provide external way to set line radius in pixels
    l->path_renderer.r = 2;
    
    l->drawable_info_list_count = 0;
    
    
    l->polygon_color_next_index = 0;
    
    l->layer_index = SD_MAX_RENDER_LAYERS;
    
    l->allocator = allocator;
    
    (*location) = l;
}

void deallocate_staging_layer_with_allocator_(
                                              Renderer* renderer,
                                              Render_Layer** location
                                              )
{
    Render_Layer* l = *location;
    if (l->allocator != nullptr) {
        l->allocator->deallocate(l->allocator, l->polygon_color_vertices, 1);
        l->allocator->deallocate(l->allocator, l->polygon_color_indices, 1);
        l->allocator = nullptr;
    }
    
    l->polygon_color_vertices = nullptr;
    l->polygon_color_indices = nullptr;
    l->drawable_info_list_count = 0;
    l->polygon_color_vertices_count = 0;
    l->polygon_color_indices_count = 0;
    
    (*location) = nullptr;
}

void resize_staging_layer_(Renderer* renderer,
                           Render_Layer** location,
                           void* vertex_memory,
                           usize vertex_byte_count,
                           void* index_memory,
                           usize index_byte_count) {
    
    // TODO: eventually
    
    Render_Layer* l = *location;
    
    VERTEX_Polygon_Color* old_vertices = l->polygon_color_vertices;
    memcpy(l->polygon_color_vertices, old_vertices, l->polygon_color_vertices_count_bytes);
    l->polygon_color_vertices = (VERTEX_Polygon_Color*)vertex_memory;
    l->polygon_color_vertices_count_max = vertex_byte_count / sizeof(VERTEX_Polygon_Color);
    l->polygon_color_vertices_count_bytes = vertex_byte_count;
    
    uint32* old_indices = l->polygon_color_indices;
    memcpy(l->polygon_color_indices, old_indices, l->polygon_color_indices_count_bytes);
    l->polygon_color_indices = (uint32*)index_memory;
    l->polygon_color_indices_count_max = index_byte_count / sizeof(uint32);
    l->polygon_color_indices_count_bytes = index_byte_count;
}

void set_staging_layer_(Renderer* renderer, Render_Layer* stage)
{
    renderer->render_layers[SD_MAX_RENDER_LAYERS] = *stage;
    renderer->active_render_layer_id = SD_MAX_RENDER_LAYERS;
}

void save_staging_layer(Renderer* renderer, Render_Layer* stage)
{
    *stage = renderer->render_layers[SD_MAX_RENDER_LAYERS];
}

void append_padding(Renderer* renderer, usize vertex_count)
{
    Render_Layer* l = &renderer->render_layers[renderer->active_render_layer_id];
    auto* d_info = Drawable_Info_get_active(renderer, l);
    d_info->i_buffer_offset += vertex_count;
    d_info->v_buffer_offset += vertex_count;
}

void Render_Command_copy_(Renderer* renderer, Drawable_Info* to, Drawable_Info* from)
{
    
}

[[deprecated]]
Drawable_Info_Range append_staging_layer_(Renderer* renderer, Render_Layer_ID to_layer, Render_Layer* from_stage)
{
    *from_stage = renderer->render_layers[SD_MAX_RENDER_LAYERS];
    Render_Layer* l = &renderer->render_layers[to_layer];
    
    Drawable_Info* last = nullptr;
    if (l->drawable_info_list_count == 0) {
        last = (&l->drawable_info_list[0] - 1);
    } else {
        last = &l->drawable_info_list[l->drawable_info_list_count - 1];
    }
    
    if (l->polygon_color_vertices_count + from_stage->polygon_color_vertices_count >= l->polygon_color_vertices_count_max) {
        MTT_error("Too many vertices, out-of-bounds count=[%llu] max=[%llu]\n", (l->polygon_color_vertices_count + from_stage->polygon_color_vertices_count), l->polygon_color_vertices_count_max);
        
        const usize doubled = l->polygon_color_vertices_count * 2;
        const usize added   = l->polygon_color_vertices_count + from_stage->polygon_color_vertices_count;
        
        usize new_count = (doubled > added) ? doubled : added;
        
        if (!Render_Layer_reallocate_vertex_bytes(renderer, new_count, to_layer)) {
            return {0};
        }
    } else if (l->polygon_color_indices_count + from_stage->polygon_color_indices_count >= l->polygon_color_indices_count_max) {
        MTT_error("Too many indices, out-of-bounds count=[%llu] max=[%llu]\n", (l->polygon_color_indices_count + from_stage->polygon_color_indices_count), l->polygon_color_indices_count_max);
        
        const usize doubled = l->polygon_color_indices_count * 2;
        const usize added   = l->polygon_color_indices_count + from_stage->polygon_color_indices_count;
        
        usize new_count = (doubled > added) ? doubled : added;
        
        if (!Render_Layer_reallocate_index_bytes(renderer, new_count, to_layer)) {
            return {0};
        }
    }
    
    VERTEX_Polygon_Color* to_v_buf = &l->polygon_color_vertices[l->polygon_color_vertices_count];
    uint32*               to_i_buf = &l->polygon_color_indices[l->polygon_color_indices_count];
    
    usize last_command_i_buffer_offset = last->i_buffer_offset;
    usize last_command_v_buffer_offset = last->v_buffer_offset;
    
    
    VERTEX_Polygon_Color* from_v_buf = from_stage->polygon_color_vertices;
    uint32*               from_i_buf = from_stage->polygon_color_indices;
    
    memcpy(to_v_buf, from_v_buf, from_stage->polygon_color_vertices_count * sizeof(VERTEX_Polygon_Color));
    memcpy(to_i_buf, from_i_buf, from_stage->polygon_color_indices_count * sizeof(uint32));
    for (usize i = 0; i < from_stage->polygon_color_indices_count; i += 1) {
        to_i_buf[i] += l->polygon_color_next_index;
    }
    
    memcpy(last + 1, from_stage->drawable_info_list, from_stage->drawable_info_list_count * sizeof(Drawable_Info));
    
    for (usize i = 0; i < from_stage->drawable_info_list_count; i += 1) {
        Drawable_Info* cmd = &l->drawable_info_list[i + l->drawable_info_list_count];
        
        cmd->buffer_index = i + l->drawable_info_list_count;
        cmd->v_buffer_offset += last_command_v_buffer_offset + last->v_buffer_count;
        cmd->i_buffer_offset += last_command_i_buffer_offset + last->i_buffer_count;
        cmd->layer_id = l->layer_index;
        cmd->first_index += l->polygon_color_next_index;
        cmd->next_index += l->polygon_color_next_index;
    }
    
    l->drawable_info_list_count += from_stage->drawable_info_list_count;
    l->polygon_color_vertices_count  += from_stage->polygon_color_vertices_count;
    l->polygon_color_indices_count   += from_stage->polygon_color_indices_count;
    l->polygon_color_next_index      += from_stage->polygon_color_next_index;
    
    return {
        from_stage->drawable_info_list_count,
        last + 1,
        last + from_stage->drawable_info_list_count - 1
    };
}

//Drawable_Info_Range append_layer_(Renderer* renderer, Render_Layer_ID to_layer, Render_Layer* from_layer)
//{
//    // TODO: test
//
//    Render_Layer* l = &renderer->render_layers[to_layer];
//
//    VERTEX_Polygon_Color* to_v_buf = &l->polygon_color_vertices[l->polygon_color_vertices_count];
//    uint32*               to_i_buf = &l->polygon_color_indices[l->polygon_color_indices_count];
//
//    Drawable_Info* last           = &l->drawable_info_list[l->drawable_info_list_count - 1];
//
//    usize last_command_i_buffer_offset = last->i_buffer_offset;
//    usize last_command_v_buffer_offset = last->v_buffer_offset;
//
//
//    VERTEX_Polygon_Color* from_v_buf = from_layer->polygon_color_vertices;
//    uint32*               from_i_buf = from_layer->polygon_color_indices;
//
//    memcpy(to_v_buf, from_v_buf, from_layer->polygon_color_vertices_count * sizeof(VERTEX_Polygon_Color));
//    memcpy(to_i_buf, from_i_buf, from_layer->polygon_color_indices_count * sizeof(uint32));
//    for (usize i = 0; i < from_layer->polygon_color_indices_count; i += 1) {
//        to_i_buf[i] += l->polygon_color_next_index;
//    }
//
//    memcpy(last + 1, from_layer->drawable_info_list, from_layer->drawable_info_list_count * sizeof(Drawable_Info));
//
//    for (usize i = 0; i < from_layer->drawable_info_list_count; i += 1) {
//        Drawable_Info* cmd = &l->drawable_info_list[i + l->drawable_info_list_count];
//
//        cmd->buffer_index = i + l->drawable_info_list_count;
//        cmd->v_buffer_offset += last_command_v_buffer_offset + last->v_buffer_count;
//        cmd->i_buffer_offset += last_command_i_buffer_offset + last->i_buffer_count;
//        cmd->layer_id = l->layer_index;
//        cmd->first_index += l->polygon_color_next_index;
//        cmd->next_index += l->polygon_color_next_index;
//    }
//
//    l->drawable_info_list_count += from_layer->drawable_info_list_count;
//    l->polygon_color_vertices_count  += from_layer->polygon_color_vertices_count;
//    l->polygon_color_indices_count   += from_layer->polygon_color_indices_count;
//    l->polygon_color_next_index      += from_layer->polygon_color_next_index;
//
//    return {
//        from_layer->drawable_info_list_count,
//        last + 1,
//        last + from_layer->drawable_info_list_count - 1
//    };
//}


void* Buffer_contents(sd::Buffer* b)
{
    return ((__bridge id<MTLBuffer>)b->buffer).contents;
}
usize Buffer_byte_size(sd::Buffer* b)
{
    return [(__bridge id<MTLBuffer>)(b->buffer) length];
}

bool Buffer_is_valid(sd::Buffer* b)
{
    return ((__bridge id<MTLBuffer>)b->buffer) == nullptr;
}

bool Buffer_make(Renderer* renderer, Buffer* buffer, usize byte_count)
{
    const usize page_size = get_page_size();
    const usize alignment = page_size;
    usize alloc_size = (((byte_count) + alignment - 1) / alignment) * alignment;
    if (alloc_size >= renderer->max_buffer_size) {
        alloc_size = (m::max(alignment, renderer->max_buffer_size - alignment) + (alignment - 1)) * alignment;
    }
    
    void* mem = mmap(
                     0,
                     alloc_size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON,
                     -1,
                     0
                     );
    if (mem == MAP_FAILED) {
        MTT_error("%s %u memory allocation failed", __func__, __LINE__);
        return false;
    }
    
    buffer->memory = mem;
    buffer->alloc_bytes = alloc_size;
    buffer->size_bytes = byte_count;
    
    [((__bridge SD_Renderer_Metal_Backend*)renderer->backend) allocate_buffer_resource:buffer deallocator:[](void* memory, usize byte_count) {
        munmap(memory, byte_count);
    }];
    
    
    return true;
}

void Buffer_destroy(Renderer* renderer, Buffer* buffer)
{
    if (buffer->buffer == nil) {
        return;
    }
    @autoreleasepool {
        CFBridgingRelease(buffer->buffer);
        buffer->buffer = nil;
        buffer->memory = nullptr;
        buffer->alloc_bytes = 0;
    }
}

#define SD_RENDERER_DEFAULT_BLOCK_COUNT (1000)
Render_Layer_ID Render_Layer_make(Renderer* renderer, usize block_count)
{
    if (renderer->render_layer_count == renderer->render_layer_max_count) {
        MTT_error("%s", "ERROR: max render layers reached");
        return -1;
    }
    
    if (block_count == 0) {
        block_count = SD_RENDERER_DEFAULT_BLOCK_COUNT;
    }
    
    const usize page_size = get_page_size();
    const usize alignment = page_size;
    usize alloc_size = (((block_count * page_size) + alignment - 1) / alignment) * alignment;
    if (alloc_size >= renderer->max_buffer_size) {
        alloc_size = (m::max(alignment, renderer->max_buffer_size - alignment) + (alignment - 1)) * alignment;
    }
#ifdef SD_DEBUG_ALLOC
#ifndef NDEBUG
    MTT_print("allocating %s %d\n", __func__, __LINE__);
    MTT_print("align %lu\n", sizeof(VERTEX_Polygon_Color));
#endif
#endif
    
    // TODO(Toby): support only one layer for now
    Render_Layer* l = &renderer->render_layers[renderer->render_layer_count];
    // MARK: - allocate polygon vertices
    {
        void* mem = mmap(
                         0,
                         alloc_size,
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANON,
                         -1,
                         0
                         );
        if (mem == MAP_FAILED) {
            MTT_error("%s %u memory allocation failed", __func__, __LINE__);
            return -1;
        }
        l->polygon_color_vertices = (VERTEX_Polygon_Color*)mem;
        l->polygon_color_vertices_count_max = alloc_size / sizeof(VERTEX_Polygon_Color);
        l->polygon_color_vertices_count = 0;
        l->polygon_color_vertices_count_bytes = alloc_size;
        
    }
    // MARK: - allocate polygon indices
    {
        void* mem = mmap(
                         0,
                         alloc_size,
                         PROT_READ | PROT_WRITE,
                         MAP_PRIVATE | MAP_ANON,
                         -1,
                         0
                         );
        if (mem == MAP_FAILED) {
            MTT_error("%s %u memory allocation failed", __func__, __LINE__);
            return -1;
        }
        l->polygon_color_indices = (sd::Index_Type*)mem;
        l->polygon_color_indices_count_max = alloc_size / sizeof(sd::Index_Type);
        l->polygon_color_indices_count     = 0;
        l->polygon_color_indices_count_bytes = alloc_size;
    }
    
    [((__bridge SD_Renderer_Metal_Backend*)renderer->backend) allocate_resources_for_layer:l with_label:[NSString stringWithFormat:@"%llu", renderer->render_layer_count] deallocator:[](void* memory, usize byte_count) {
        munmap(memory, byte_count);
    }];
    
    
    renderer->render_layers[renderer->render_layer_count].path_renderer.user_data = (void*)renderer;
    renderer->render_layers[renderer->render_layer_count].path_renderer.triangle = triangle_for_line_renderer;
    renderer->render_layers[renderer->render_layer_count].path_renderer.rectangle = rectangle_for_line_renderer;
    renderer->render_layers[renderer->render_layer_count].path_renderer.circle = circle_for_line_renderer;
    renderer->render_layers[renderer->render_layer_count].path_renderer.r = 2;
    
    //    renderer->render_layers[renderer->render_layer_count].render_commands_polygon_count = 0;
    //    renderer->render_layers[renderer->render_layer_count].render_commands_path_count    = 0;
    
    l->layer_index = renderer->render_layer_count;
    renderer->render_layer_count += 1;
    
    l->polygon_color_next_index = 0;
    
    l->allocator = nullptr;
    
    l->saved_v_buffer_count = 0;
    l->saved_i_buffer_count = 0;
    l->saved_next_index = 0;
    l->saved_i_buffer_count_used = 0;
    
    {
        l->free_list = mtt::Dynamic_Array<Drawable_Info*>::make(*mem::get_main_allocator());
        l->free_list_info_SD_HACK = mtt::Dynamic_Array<Drawable_Info*>::make(*mem::get_main_allocator());
    }
    
    return renderer->render_layer_count - 1;
}

//bool Render_Layer_reallocate_(Renderer* renderer, usize block_count, Render_Layer_ID rlid)
//{
//    if (block_count == 0) {
//        block_count = SD_RENDERER_DEFAULT_BLOCK_COUNT;
//    }
//    
//    const usize page_size = get_page_size();
//    const usize alignment = page_size;
//    const usize alloc_size = (((block_count * page_size) + alignment - 1) / alignment) * alignment;
//    
//    MTT_print("allocating %s %d\n", __func__, __LINE__);
//    MTT_print("alloc_size %llu align %lu\n", alloc_size, sizeof(VERTEX_Polygon_Color));
//    
//    // TODO(Toby): support only one layer for now
//    Render_Layer* l = &renderer->render_layers[rlid];
//
//    // MARK: - allocate polygon vertices
//    
//        void* vertex_mem = mmap(
//            0,
//            alloc_size,
//            PROT_READ | PROT_WRITE,
//            MAP_PRIVATE | MAP_ANON,
//            -1,
//            0
//        );
//        if (vertex_mem == MAP_FAILED) {
//            MTT_error("%s %u memory allocation failed", __func__, __LINE__);
//            return false;
//        }
//
//        
//    
//    // MARK: - allocate polygon indices
//    
//        void* index_mem = mmap(
//            0,
//            alloc_size,
//            PROT_READ | PROT_WRITE,
//            MAP_PRIVATE | MAP_ANON,
//            -1,
//            0
//        );
//        if (index_mem == MAP_FAILED) {
//            MTT_error("%s %u memory allocation failed", __func__, __LINE__);
//            return false;
//        }
//
//    memcpy(vertex_mem, l->polygon_color_vertices, l->polygon_color_vertices_count_bytes);
//    //munmap(l->polygon_color_vertices, l->polygon_color_vertices_count_bytes);
//    l->polygon_color_vertices = (VERTEX_Polygon_Color*)vertex_mem;
//    
//    l->polygon_color_vertices_count_max = alloc_size / sizeof(VERTEX_Polygon_Color);
//    l->polygon_color_vertices_count_bytes = alloc_size;
//    
//    memcpy(index_mem, l->polygon_color_indices, l->polygon_color_indices_count_bytes);
//    //munmap(l->polygon_color_indices, l->polygon_color_indices_count_bytes);
//    l->polygon_color_indices = (sd::Index_Type*)index_mem;
//    
//    
//    l->polygon_color_indices_count_max = alloc_size / sizeof(sd::Index_Type);
//    l->polygon_color_indices_count_bytes = alloc_size;
//    
//    [((__bridge SD_Renderer_Metal_Backend*)renderer->backend) reallocate_resources_for_layer:l with_label:[NSString stringWithFormat:@"%llu", rlid] and_index: rlid deallocator:[](void* memory, usize byte_count) {
//        munmap(memory, byte_count);
//    }];
//    
//    return true;
//}

bool Render_Layer_reallocate_bytes(Renderer* renderer, usize bytes, Render_Layer_ID rlid)
{
    // TODO
    ASSERT_MSG(false, "NOT IMPLEMENTED %s\n", __PRETTY_FUNCTION__);
    return false;
}

bool Render_Layer_reallocate_vertex_bytes(Renderer* renderer, usize bytes, Render_Layer_ID rlid)
{
    const usize page_size = get_page_size();
    const usize alignment = page_size;
    const usize alloc_size = (((bytes) + alignment - 1) / alignment) * alignment;
#ifdef SD_DEBUG_ALLOC
#ifndef NDEBUG
    MTT_print("allocating %s %d\n", __func__, __LINE__);
    MTT_print("alloc_size %llu align %lu\n", alloc_size, sizeof(VERTEX_Polygon_Color));
#endif
#endif
    
    // TODO(Toby): support only one layer for now
    Render_Layer* l = &renderer->render_layers[rlid];
    
    // MARK: - allocate polygon vertices
    
    void* vertex_mem = mmap(
                            0,
                            alloc_size,
                            PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANON,
                            -1,
                            0
                            );
    if (vertex_mem == MAP_FAILED) {
        MTT_error("%s %u memory allocation failed", __func__, __LINE__);
        return false;
    }
    
    memcpy(vertex_mem, l->polygon_color_vertices, l->polygon_color_vertices_count_bytes);
    //munmap(l->polygon_color_vertices, l->polygon_color_vertices_count_bytes);
    l->polygon_color_vertices = (VERTEX_Polygon_Color*)vertex_mem;
    
    l->polygon_color_vertices_count_max = alloc_size / sizeof(VERTEX_Polygon_Color);
    l->polygon_color_vertices_count_bytes = alloc_size;
    
    [((__bridge SD_Renderer_Metal_Backend*)renderer->backend) reallocate_vertex_buffer_for_layer:l with_label:[NSString stringWithFormat:@"%llu", rlid] and_index: rlid deallocator:[](void* memory, usize byte_count) {
        munmap(memory, byte_count);
    }];
    return true;
}

bool Render_Layer_reallocate_index_bytes(Renderer* renderer, usize bytes, Render_Layer_ID rlid)
{
    
    const usize page_size = get_page_size();
    const usize alignment = page_size;
    const usize alloc_size = (((bytes) + alignment - 1) / alignment) * alignment;
#ifdef SD_DEBUG_ALLOC
#ifndef NDEBUG
    MTT_print("allocating %s %d\n", __func__, __LINE__);
    MTT_print("alloc_size %llu align %lu\n", alloc_size, sizeof(sd::Index_Type));
#endif
#endif
    
    // TODO(Toby): support only one layer for now
    Render_Layer* l = &renderer->render_layers[rlid];
    
    // MARK: - allocate polygon indices
    
    void* index_mem = mmap(
                           0,
                           alloc_size,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANON,
                           -1,
                           0
                           );
    if (index_mem == MAP_FAILED) {
        MTT_error("%s %u memory allocation failed", __func__, __LINE__);
        return false;
    }
    
    memcpy(index_mem, l->polygon_color_indices, l->polygon_color_indices_count_bytes);
    //munmap(l->polygon_color_indices, l->polygon_color_indices_count_bytes);
    l->polygon_color_indices = (sd::Index_Type*)index_mem;
    
    
    l->polygon_color_indices_count_max = alloc_size / sizeof(sd::Index_Type);
    l->polygon_color_indices_count_bytes = alloc_size;
    
    [((__bridge SD_Renderer_Metal_Backend*)renderer->backend) reallocate_index_buffer_for_layer:l with_label:[NSString stringWithFormat:@"%llu", rlid] and_index: rlid deallocator:[](void* memory, usize byte_count) {
        munmap(memory, byte_count);
    }];
    
    return true;
}

bool Render_Layer_reserve_render_data_space(sd::Renderer* renderer, usize requested_vertex_count, usize requested_index_count)
{
    //    usize old_size = (renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color));
    
    if ((renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count + requested_vertex_count) * sizeof(VERTEX_Polygon_Color) >
        renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color) * 0.75f) {
        
        //        MTT_print("Too many vertices, out-of-safe-bounds byte count=[%llu] safe-max byte count=[%llu] * 0.75 = [%llu]\n\n",
        //        renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count * sizeof(VERTEX_Polygon_Color),
        //        renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color),
        //        (usize)(renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color) * 0.75f));
        
        if (renderer->render_layers[renderer->active_render_layer_id].allocator != nullptr) {
            reallocate_render_layer_vertices_with_allocator(renderer, &renderer->render_layers[renderer->active_render_layer_id]);
        } else {
            //MTT_print("ACTIVE_LAYER=[%llu]\n", renderer->active_render_layer_id);
            usize new_size = renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count_max * sizeof(VERTEX_Polygon_Color) * 2;
            while (new_size < (usize)((renderer->render_layers[renderer->active_render_layer_id].polygon_color_vertices_count + requested_vertex_count) * sizeof(VERTEX_Polygon_Color) * 0.75f * 2)) {
                new_size *= 2;
            }
            if (!Render_Layer_reallocate_vertex_bytes(renderer, new_size, renderer->active_render_layer_id)) {
                return false;
            }
        }
    }
    if ((renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count * sizeof(sd::Index_Type) + requested_index_count) >
        renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count_max * sizeof(sd::Index_Type) * 0.75f) {
        //        MTT_print("Too many indices, out-of-safe-bounds byte count=[%llu] safe-max byte count=[%llu] * 0.75 = [%llu]\n\n",
        //        renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count * sizeof(sd::Index_Type),
        //        renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count_max * sizeof(sd::Index_Type),
        //        (usize)(renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count_max * sizeof(sd::Index_Type) * 0.75f));
        
        if (renderer->render_layers[renderer->active_render_layer_id].allocator != nullptr) {
            reallocate_render_layer_indices_with_allocator(renderer, &renderer->render_layers[renderer->active_render_layer_id]);
        } else {
            //MTT_print("ACTIVE_LAYER=[%llu]\n", renderer->active_render_layer_id);
            usize new_size = renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count_max * sizeof(sd::Index_Type) * 2;
            while (new_size < (renderer->render_layers[renderer->active_render_layer_id].polygon_color_indices_count + requested_index_count) * sizeof(sd::Index_Type) * 0.75f * 2) {
                new_size *= 2;
            }
            if (!Render_Layer_reallocate_index_bytes(renderer, new_size, renderer->active_render_layer_id)) {
                return false;
            }
        }
    }
    return true;
}

#undef SD_RENDERER_DEFAULT_BLOCK_COUNT



Renderer Renderer_make(void)
{
    MTT_print("%s\n", "stratadraw IOS");
    sd::Renderer r;
    
    
    
    return r;
}
void Renderer_deinit(Renderer* renderer)
{
    for (usize i = 0; i < renderer->render_layer_count; i += 1) {
        Render_Layer* l = &renderer->render_layers[i];
        //        munmap(l->polygon_color_vertices, l->polygon_color_vertices_count_bytes);
        //        munmap(l->polygon_color_indices, l->polygon_color_indices_count_bytes);
        
        //munmap(l->path_color_vertices, l->path_color_vertices_count_bytes);
        //munmap(l->path_color_indices, l->path_color_indices_count_bytes);
    }
    
    free(renderer->render_layers);
    
    mem::Arena_deinit(&renderer->cmd_mem);
    
    MTT_print("%s\n", "Statodraw IOS freed resources");
}

inline Command* Command_make(Renderer* renderer)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({});
    return &cmd_list.commands.back();
}

void push_projection_view_transform(Renderer* renderer, SD_mat4 projection_transform, SD_mat4 view_transform)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type = COMMAND_TYPE_PROJECTION_VIEW_TRANSFORM;
    cmd->projection_view_transform.view = view_transform;
    cmd->projection_view_transform.projection = projection_transform;
}
void pop_view_transform(Renderer* renderer)
{
    
}
void push_draw_command_with_layer(Renderer* renderer, Render_Layer_ID render_layer)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type             = COMMAND_TYPE_DRAW;
    cmd->draw.layer_id    = render_layer;
    cmd->draw.buffer_layer_id = render_layer;
    cmd->draw.layer_range = {0, renderer->render_layers[render_layer].drawable_info_list_count};
}

// TODO: This is a hack that really suggests I need a new kind of loop
[[deprecated]]
void push_draw_command_with_info_layer_and_buffer_layer(Renderer* renderer, Render_Layer_ID render_layer_w_drawable_info, Render_Layer_ID render_layer_w_buffer)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type             = COMMAND_TYPE_DRAW;
    cmd->draw.layer_id    = render_layer_w_drawable_info;
    cmd->draw.buffer_layer_id = render_layer_w_buffer;
    cmd->draw.layer_range = {0, renderer->render_layers[render_layer_w_drawable_info].drawable_info_list_count};
}

void push_draw_command_with_staging_layer(Renderer* renderer, Render_Layer* staging_layer)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    
    // TODO
    
}

void push_draw_command_with_layer_range(Renderer* renderer, Render_Layer_ID render_layer, Render_Layer_Range layer_range)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type             = COMMAND_TYPE_DRAW;
    cmd->draw.layer_id    = render_layer;
    cmd->draw.buffer_layer_id = render_layer;
    cmd->draw.layer_range = layer_range;
}

void push_draw_command_with_drawable_info(Renderer* renderer, Drawable_Info* drawable_info)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type             = COMMAND_TYPE_DRAW;
    cmd->draw.layer_id    = drawable_info->layer_id;
    cmd->draw.buffer_layer_id = drawable_info->layer_id;
    cmd->draw.layer_range = {drawable_info->buffer_index, 1};
}

void push_scissor_rect(Renderer* renderer, Scissor_Rect scissor_rect)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type              = COMMAND_TYPE_SCISSOR_RECTANGLE;
    cmd->scissor_rectangle = scissor_rect;
}

void push_viewport(Renderer* renderer, Viewport viewport)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type              = COMMAND_TYPE_VIEWPORT;
    cmd->viewport = viewport;
}

void push_instanced_draw_command_with_layer(Renderer* renderer, Render_Layer_ID render_layer, Drawable_Instance_Data* instance_data)
{
    // TODO
    ASSERT_MSG(false, "Not implemented!");
}


//void push_instanced_draw_command_with_drawable(Renderer* renderer, Drawable_Info* drawable, Drawable_Info* list, usize count)
//{
//    Command_List& cmd_list = *renderer->active_cmd_list;
//
//    auto* cmd = &cmd_list.commands[cmd_list.count];
//    cmd->type             = COMMAND_TYPE_DRAW_INSTANCED;
//    cmd->draw_instanced.layer_id    = drawable->layer_id;
//    cmd->draw_instanced.layer_range = {0, count};
//    cmd->draw_instanced.instance_list    = list;
//    cmd->draw_instanced.instance_drawable = drawable;
//
//    cmd_list.count += 1;
//}

//void push_instanced_draw_command_with_drawable(Renderer* renderer, Drawable_Info* drawable, Drawable_Info* alt)
//{
//    Command_List& cmd_list = *renderer->active_cmd_list;
//    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
//    cmd->type             = COMMAND_TYPE_DRAW_INSTANCED;
//    cmd->draw_instanced.layer_id    = drawable->layer_id;
//    cmd->draw_instanced.buffer_layer_id = drawable->layer_id;
//    //cmd->draw_instanced.layer_range = {0, count};
//    cmd->draw_instanced.array    = {};
//    cmd->draw_instanced.array.count = 1;
//    
//    
//    
//    cmd->draw_instanced.instance_drawable = drawable;
//    cmd->draw_instanced.array.data = &cmd->single_info;
//    *(cmd->draw_instanced.array.data) = *alt;
//    cmd->flags = COMMAND_FLAGS_INIT;
//}

void push_instanced_draw_command_with_drawable_list(Renderer* renderer, Drawable_Info* drawable, MTT_List* list)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type             = COMMAND_TYPE_DRAW_INSTANCED;
    cmd->draw_instanced.layer_id    = drawable->layer_id;
    cmd->draw_instanced.buffer_layer_id = drawable->layer_id;
    //cmd->draw_instanced.layer_range = {0, count};
    cmd->draw_instanced.list    = list;
    cmd->draw_instanced.instance_drawable = drawable;
    cmd->flags = COMMAND_FLAGS_INIT;
}


void push_instanced_draw_command_with_drawable_list(Renderer* renderer, Drawable_Info* drawable, mtt::Array_Slice<sd::Drawable_Info> list)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type             = COMMAND_TYPE_DRAW_INSTANCED;
    cmd->draw_instanced.layer_id    = drawable->layer_id;
    cmd->draw_instanced.buffer_layer_id = drawable->layer_id;
    //cmd->draw_instanced.layer_range = {0, count};
    cmd->draw_instanced.array    = list;
    cmd->draw_instanced.instance_drawable = drawable;
    cmd->flags = COMMAND_FLAGS_INIT;
}

void push_instanced_draw_command_with_staging_layer(Renderer* renderer, Render_Layer* staging_layer, Drawable_Instance_Data* instance_data)
{
    // TODO
}


void push_pipeline(Renderer* renderer, uint64 pipeline_id)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type        = COMMAND_TYPE_PIPELINE;
    cmd->pipeline_id = pipeline_id;
}

void push_color_pipeline(Renderer* renderer)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type        = COMMAND_TYPE_COLOR_PIPELINE;
    cmd->pipeline_id = sd::PIPELINE_MODE_COLOR;
}

void push_color_hsv_pipeline(Renderer* renderer)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type        = COMMAND_TYPE_COLOR_HSV_PIPELINE;
    cmd->pipeline_id = sd::PIPELINE_MODE_COLOR_HSV;
}

void push_texture_pipeline(Renderer* renderer)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type        = COMMAND_TYPE_TEXTURE_PIPELINE;
    cmd->pipeline_id = sd::PIPELINE_MODE_TEXTURE;
}

void push_text_pipeline(Renderer* renderer)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type        = COMMAND_TYPE_TEXT_PIPELINE;
    cmd->pipeline_id = sd::PIPELINE_MODE_TEXT;
}

void set_depth_write_mode(Renderer* renderer, sd::DEPTH_WRITE_MODE is_enabled)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type       = COMMAND_TYPE_SET_DEPTH_MODE;
    cmd->depth_mode = is_enabled;
}

void set_stencil_reference_value(Renderer* renderer, uint32 reference_value)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type       = COMMAND_TYPE_SET_STENCIL_REFERENCE_VALUE;
    cmd->stencil_reference_value = reference_value;
}

void set_render_pipeline(Renderer* renderer, uint64 ID)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type = COMMAND_TYPE_SET_RENDER_PASS;
    cmd->ID = 0;
}

void render_to_texture_sync_begin(Renderer* renderer, sd::Rectangle bounds)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type = COMMAND_TYPE_RENDER_TO_TEXTURE_BEGIN;
    cmd->viewport = Rectangle_to_Viewport(bounds);
}
void render_to_texture_end(Renderer* renderer, void (*callback)(bool status, void* texture_data))
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type = COMMAND_TYPE_RENDER_TO_TEXTURE_END;
    cmd->callback = callback;
}



void render_pass_begin(Renderer* renderer, const Render_Pass_Descriptor& desc)
{
    auto* cmd = Command_make(renderer);
    cmd->type = COMMAND_TYPE_RENDER_PASS_BEGIN;
    cmd->render_pass.id = desc.id;
    cmd->render_pass.color_targets[0] = desc.color_targets[0];
    cmd->render_pass.viewport = desc.viewport;
    //memcpy(cmd->render_pass.color_targets, desc.color_targets, sizeof(cmd->render_pass.color_targets));
}
void render_pass_begin(Renderer* renderer)
{
    auto* cmd = Command_make(renderer);
    cmd->type = COMMAND_TYPE_RENDER_PASS_BEGIN;
    cmd->render_pass.id = 0;
}
void render_pass_end(Renderer* renderer)
{
    auto* cmd = Command_make(renderer);
    cmd->type = COMMAND_TYPE_RENDER_PASS_END;
}

void compute_pass_begin(Renderer* renderer, const Compute_Pass_Descriptor& desc)
{
    auto* cmd = Command_make(renderer);
    cmd->type = COMMAND_TYPE_COMPUTE_PASS_BEGIN;
    cmd->compute_pass.descriptor = desc;
}
void compute_pass_end(Renderer* renderer)
{
    auto* cmd = Command_make(renderer);
    cmd->type = COMMAND_TYPE_COMPUTE_PASS_END;
    cmd->compute_pass.end_pass.state = nullptr;
    cmd->compute_pass.end_pass.callback = nullptr;
    cmd->compute_pass.end_pass.should_wait = true;
}

void compute_pass_end(Renderer* r, const Compute_Pass_End_Descriptor& desc)
{
    auto* cmd = Command_make(r);
    cmd->type = COMMAND_TYPE_COMPUTE_PASS_END;
    cmd->compute_pass.end_pass.state = desc.state;
    cmd->compute_pass.end_pass.callback = desc.callback;
    cmd->compute_pass.end_pass.should_wait = desc.should_wait;
}

// Encodes a compute command using a grid aligned to threadgroup boundaries.
void dispatch_threadgroups(sd::Renderer* r, vec3 threadgroups_per_grid, vec3 threads_per_threadgroup)
{
    auto* cmd = Command_make(r);
    cmd->type = COMMAND_TYPE_COMPUTE_DISPATCH_UNIFORM_GRID;
    cmd->compute_pass.dispatch.uniform_grid_args.threadgroups_per_grid = threadgroups_per_grid;
    cmd->compute_pass.dispatch.uniform_grid_args.threads_per_threadgroup = threads_per_threadgroup;
    ASSERT_MSG(false, "Not fully implemented in backend!\n");
}
// Encodes a compute command using an arbitrarily sized grid.
void dispatch_threads(sd::Renderer* r, vec3 threads_per_grid, vec3 threads_per_threadgroup)
{
    auto* cmd = Command_make(r);
    cmd->type = COMMAND_TYPE_COMPUTE_DISPATCH_NON_UNIFORM_GRID;
    cmd->compute_pass.dispatch.nonuniform_grid_args.threads_per_grid = threads_per_grid;
    cmd->compute_pass.dispatch.nonuniform_grid_args.threads_per_threadgroup = threads_per_threadgroup;
}

bool compute_shader_info(sd::Renderer* r, sd::Shader_ID shader_id, Compute_Shader_Info* info)
{
    @autoreleasepool {
        auto* r_backend = ((__bridge SD_Renderer_Metal_Backend*)r->backend);
        id<MTLComputePipelineState> pipeline = [r_backend compute_shader_lookup_with_id:shader_id];
        if (pipeline == nil) {
            return false;
        }
        
        info->thread_execution_width = pipeline.threadExecutionWidth;
        info->max_total_threads_per_threadgroup = pipeline.maxTotalThreadsPerThreadgroup;
        
        return true;
    }
}

void compute_shader_set(sd::Renderer* r, sd::Shader_ID shader_id)
{
    auto* cmd = Command_make(r);
    cmd->type = COMMAND_TYPE_COMPUTE_SHADER_SET;
    cmd->compute_pass.shader_set.shader_id = shader_id;
}


void vertex_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset)
{
    auto* cmd = Command_make(r);
    cmd->type = COMMAND_TYPE_VERTEX_BUFFER_SET;
    cmd->render_pass_buffer_set.buffer = *buffer;
    cmd->render_pass_buffer_set.slot = slot;
    cmd->render_pass_buffer_set.byte_offset = byte_offset;
}
void vertex_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot)
{
    vertex_buffer_set(r, buffer, slot, 0);
}
void fragment_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset)
{
    auto* cmd = Command_make(r);
    cmd->type = COMMAND_TYPE_FRAGMENT_BUFFER_SET;
    cmd->render_pass_buffer_set.buffer = *buffer;
    cmd->render_pass_buffer_set.slot = slot;
    cmd->render_pass_buffer_set.byte_offset = byte_offset;
}
void fragment_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot)
{
    fragment_buffer_set(r, buffer, slot);
}

void compute_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset)
{
    auto* cmd = Command_make(r);
    cmd->type = COMMAND_TYPE_COMPUTE_BUFFER_SET;
    cmd->compute_pass.buffer_set.buffer = *buffer;
    cmd->compute_pass.buffer_set.slot = slot;
    cmd->compute_pass.buffer_set.byte_offset = byte_offset;
}
void compute_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot)
{
    compute_buffer_set(r, buffer, slot, 0);
}

void compute_texture_set(sd::Renderer* r, sd::Texture_Handle texture_handle, uint64 slot);

void push_custom_command(Renderer* renderer, Custom_Command custom)
{
    Command_List& cmd_list = *renderer->active_cmd_list;
    cmd_list.commands.push_back({}); auto* cmd = &cmd_list.commands.back();
    cmd->type           = COMMAND_TYPE_CUSTOM;
    cmd->custom_command = custom;
}

void Command_List_begin(Renderer* renderer, Command_List* cmd_list)
{
    // TODO: support multiple command lists at the same time
    mtt::Array_init(&cmd_list->commands, renderer->cmd_allocator, 0, 1024);
    //mtt::init()
    //cmd_list->cmds
    renderer->active_cmd_list = cmd_list;
}

void Command_List_end(Renderer* renderer, Command_List* cmd_list)
{
    renderer->active_cmd_list = nullptr;
}

void Command_List_submit(Renderer* renderer, Command_List* cmd_list)
{
    renderer->cmd_lists[renderer->cmd_list_count] = *cmd_list;
    
    renderer->cmd_list_count += 1;
}


//void print_commands(Renderer* renderer)
//{
//    for (usize i = 0; i < renderer->cmd_list_count; i += 1) {
//        MTT_print("{\n");
//
//        Command_List& cmd_list = renderer->cmd_lists[i];
//        for (usize j = 0; j < cmd_list.count; j += 1) {
//            MTT_print("\t{\n");
//            switch (cmd_list.commands[j].type) {
//            case COMMAND_TYPE_DRAW: {
//                MTT_print("\t\tCOMMAND DRAW L=[%llu], range={first_index=%llu, count=%llu}\n", cmd_list.commands[j].draw.layer_id, cmd_list.commands[j].draw.layer_range.first_index, cmd_list.commands[j].draw.layer_range.count);
//                break;
//            }
//            case COMMAND_TYPE_PROJECTION_VIEW_TRANSFORM: {
//                MTT_print("\t\tCOMMAND VIEW_TRANSFORM \n\t\t");
//                std::cout << "view " << m::to_string(cmd_list.commands[j].projection_view_transform.view) <<
//                "projection " << m::to_string(cmd_list.commands[j].projection_view_transform.projection) << std::endl;
//                break;
//            }
//            default: { break; }
//            }
//            MTT_print("\t}\n");
//        }
//
//        MTT_print("}\n");
//    }
//}

void clear_commands(Renderer* renderer)
{
    mem::Arena_rewind(&renderer->cmd_mem);
    renderer->cmd_list_count = 0;
}

void disable_command(Renderer* renderer, Drawable_Info* rc)
{
    rc->is_enabled = 0;
}

void enable_command(Renderer* renderer, Drawable_Info* rc)
{
    rc->is_enabled = 1;
}

sd::Texture_ID texture_id_from_name(sd::Renderer* renderer, const mtt::String& name)
{
    sd::Images* imgs = renderer->images;
    sd::Texture_ID* ID = nullptr;
    if (mtt::map_try_get(imgs->name_to_id, name, &ID)) {
        return *ID;
    }
    return Texture_ID_INVALID;
}


void setup(void)
{
    sd::Renderer_sizeof      = Renderer_sizeof_;
    sd::Render_Layer_sizeof  = Render_Layer_sizeof_;
    
    sd::Render_Layer_sizeof_polygon_vertex = Render_Layer_sizeof_polygon_vertex_;
    sd::Render_Layer_sizeof_path_vertex    = Render_Layer_sizeof_path_vertex_;
    sd::Render_Layer_sizeof_index          = Render_Layer_sizeof_index_;
    sd::Render_Layer_copy                  = Render_Layer_copy_;
    sd::make_staging_layer                 = make_staging_layer_;
    sd::make_staging_layer_with_allocator  = make_staging_layer_with_allocator_;
    sd::deallocate_staging_layer_with_allocator = deallocate_staging_layer_with_allocator_;
    sd::append_staging_layer               = append_staging_layer_;
    //sd::append_layer                       = append_layer_;
    sd::set_staging_layer                  = set_staging_layer_;
    //sd::Render_Layer_reallocate            = Render_Layer_reallocate_;
    
    sd::seek_layer_to = seek_layer_to_;
    
    sd::resize_staging_layer = resize_staging_layer_;
}


Render_Layer_ID layer_id(Render_Layer* layer)
{
    if (layer == nullptr) {
        return Render_Layer_ID_INVALID;
    }
    
    return layer->layer_index;
}

void save(Renderer* renderer)
{
    renderer->saved_state.color        = renderer->color;
    renderer->saved_state.prev_color = renderer->prev_color;
    renderer->saved_state.bg_color     = renderer->background_color;
    renderer->saved_state.layer_id     = renderer->active_render_layer_id;
    renderer->saved_state.active_depth = renderer->active_depth;
}

void restore(Renderer* renderer)
{
    renderer->color                  = renderer->saved_state.color;
    renderer->prev_color             = renderer->saved_state.prev_color;
    renderer->background_color       = renderer->saved_state.bg_color;
    renderer->active_render_layer_id = renderer->saved_state.layer_id;
    renderer->active_depth           = renderer->saved_state.active_depth;
}

Drawable_Info* Drawable_Info_copy_into_existing(Renderer* renderer, Render_Layer* to_l, Render_Layer* from_l, Drawable_Info* info_to, Drawable_Info* info)
{
    set_render_layer(renderer, to_l->layer_index);
    Render_Layer_ID active_id = renderer->active_render_layer_id;
    renderer->active_render_layer_id = active_id;
    
    // target buffer offsets
    VERTEX_Polygon_Color*     to_v_buf = &to_l->polygon_color_vertices[info_to->v_buffer_offset];
    Index_Type*               to_i_buf = &to_l->polygon_color_indices[info_to->i_buffer_offset];
    
    // source buffer offsets
    VERTEX_Polygon_Color*     from_v_buf = &from_l->polygon_color_vertices[info->v_buffer_offset];
    Index_Type*               from_i_buf = &from_l->polygon_color_indices[info->i_buffer_offset];
    
//    usize last_command_v_buffer_count  = 0;
//    usize last_command_i_buffer_count  = 0;
//    usize last_command_v_buffer_offset = 0;
//    usize last_command_i_buffer_offset = 0;
//    usize last_buffer_index = 0;
//
//    if (info_to->buffer_index != 0) {
//        Drawable_Info* p_info = &to_l->drawable_info_list[info_to->buffer_index - 1];
//
//        last_command_v_buffer_count  = p_info->v_buffer_count;
//        last_command_i_buffer_count  = p_info->i_buffer_count;
//        last_command_v_buffer_offset = p_info->v_buffer_offset;
//        last_command_i_buffer_offset = p_info->i_buffer_offset;
//        last_buffer_index = p_info->buffer_index;
//    }
    
    memset(to_v_buf, 0, info->v_buffer_max_count * sizeof(VERTEX_Polygon_Color));
    memcpy(to_v_buf, from_v_buf, info->v_buffer_count * sizeof(VERTEX_Polygon_Color));
    memset(to_i_buf, 0, info->i_buffer_max_count * sizeof(Index_Type));
    memcpy(to_i_buf, from_i_buf, info->i_buffer_count * sizeof(Index_Type));
    
    // adjusting indices to be correct in the new buffer
//    usize index_diff = 0;
    
//    auto* prev = (info_to - 1);
//    if (info->first_index > prev->next_index) {
//        index_diff = (info->first_index - prev->next_index);
//        for (usize i = 0; i < info->i_buffer_count; i += 1) {
//            to_i_buf[i] -= index_diff;
//        }
//    } else if (info->first_index < prev->next_index) {
//        index_diff = (prev->next_index - info->first_index);
//        for (usize i = 0; i < info->i_buffer_count; i += 1) {
//            to_i_buf[i] += index_diff;
//        }
//    }
    
    
    Drawable_Info* next_info = info_to;
    next_info->layer_id = to_l->layer_index;
    
    next_info->v_buffer_count = next_info->v_buffer_max_count;
    next_info->i_buffer_count = next_info->i_buffer_max_count;
    //next_info->check_to_draw = true;
    next_info->i_buffer_count_used = info->i_buffer_count_used;
    next_info->flags = info->flags;
    next_info->type = info->type;
    
    next_info->set_transform(info);
    next_info->set_texture_ID(info);
    next_info->set_texture_sampler_ID(info);
    next_info->set_texture_animation_speed(info);
    next_info->set_color_addition(info);
    next_info->set_color_factor(info);
    
    next_info->is_enabled = info->is_enabled;
    

    return next_info;
}

Drawable_Info* Drawable_Info_copy(Renderer* renderer, Render_Layer_ID ID, Drawable_Info* info)
{
    Render_Layer* to_l = &renderer->render_layers[ID];
    Render_Layer* from_l = &renderer->render_layers[info->layer_id];
   
    Render_Layer_ID active_id = renderer->active_render_layer_id;
    
    constexpr const bool do_reclaim_memory = true;
    if constexpr ((do_reclaim_memory)) {
        auto& free_l = to_l->free_list;
        if (!free_l.empty()) {
            Drawable_Info* info_to = nullptr;
            usize idx = 0;
            
            const usize req_v = info->v_buffer_count;
            const usize req_i = info->i_buffer_count;
            
            const usize sz = free_l.size();
            for (usize i = 0; i < sz; i += 1) {
                Drawable_Info* comp = free_l[i];
                if (comp->v_buffer_max_count < req_v ||
                    comp->i_buffer_max_count < req_i) {
                    continue;
                }
                
                
                
//#if          SD_END_LAYER_DESTROY_OPT
//                if (comp->buffer_index == to_l->drawable_info_list_count - 1) {
//                    seek_layer_to(renderer, comp - 1);
//                    continue;
//                }
//#endif
                
                // NOTE:
                // This would be "best fit, but for now, use "first fit"
//                if (info_to != nullptr &&
//                    (info_to->v_buffer_max_count > comp->v_buffer_max_count ||
//                    info_to->i_buffer_max_count >
//                    comp->i_buffer_max_count)
//                    ) {
//                    info_to = comp;
//                    idx = i;
//                } else {
//                    info_to = comp;
//                    idx = i;
//                }
                {
                    info_to = comp;
                    idx = i;
                    break;
                }
            }
            
            if (info_to != nullptr) {
                
                
                auto* out_info = Drawable_Info_copy_into_existing(renderer, to_l, from_l, info_to, info);
                unordered_remove(&free_l, idx);
                return out_info;
            }
        }
    }
    
    set_render_layer(renderer, to_l->layer_index);
    if (!bounds_check(renderer)) {
        abort();
    }
    renderer->active_render_layer_id = active_id;
    
    to_l->drawable_info_list_count += 1;
    
    auto base_vertex = to_l->polygon_color_vertices_count;
    // target buffer offsets
    VERTEX_Polygon_Color*     to_v_buf = &to_l->polygon_color_vertices[to_l->polygon_color_vertices_count];
    Index_Type*               to_i_buf = &to_l->polygon_color_indices[to_l->polygon_color_indices_count];

    // source buffer offsets
    VERTEX_Polygon_Color*     from_v_buf = &from_l->polygon_color_vertices[info->v_buffer_offset];
    Index_Type*               from_i_buf = &from_l->polygon_color_indices[info->i_buffer_offset];
    
    
    usize last_command_v_buffer_count  = 0;
    usize last_command_i_buffer_count  = 0;
    usize last_command_v_buffer_offset = 0;
    usize last_command_i_buffer_offset = 0;
    usize last_buffer_index = 0;
    
    if (to_l->drawable_info_list_count != 1) {
        Drawable_Info* info = &to_l->drawable_info_list[to_l->drawable_info_list_count - 2];
        
        last_command_v_buffer_count  = info->v_buffer_count;
        last_command_i_buffer_count  = info->i_buffer_count;
        last_command_v_buffer_offset = info->v_buffer_offset;
        last_command_i_buffer_offset = info->i_buffer_offset;
        last_buffer_index = info->buffer_index;
    }
    
    memcpy(to_v_buf, from_v_buf, info->v_buffer_count * sizeof(VERTEX_Polygon_Color));
    memcpy(to_i_buf, from_i_buf, info->i_buffer_count * sizeof(Index_Type));
    
    // adjusting indices to be correct in the new buffer
//    usize index_diff = 0;
//    if (info->first_index > to_l->polygon_color_next_index) {
//        index_diff = (info->first_index - to_l->polygon_color_next_index);
//        for (usize i = 0; i < info->i_buffer_count; i += 1) {
//            to_i_buf[i] -= index_diff;
//        }
//    } else if (info->first_index < to_l->polygon_color_next_index) {
//        index_diff = (to_l->polygon_color_next_index - info->first_index);
//        for (usize i = 0; i < info->i_buffer_count; i += 1) {
//            to_i_buf[i] += index_diff;
//        }
//    }
    
    
    Drawable_Info* next_info = &to_l->drawable_info_list[to_l->drawable_info_list_count - 1];
    
    *next_info = *info;
    next_info->buffer_index = last_buffer_index + 1;
    next_info->layer_id = ID;
    next_info->v_buffer_offset = last_command_v_buffer_offset + last_command_v_buffer_count;
    next_info->i_buffer_offset = last_command_i_buffer_offset + last_command_i_buffer_count;
    next_info->first_index = to_l->polygon_color_next_index;
    next_info->next_index = to_l->polygon_color_next_index + (info->next_index - info->first_index);
    
    to_l->polygon_color_vertices_count += info->v_buffer_count;
    to_l->polygon_color_indices_count  += info->i_buffer_count;
    
    to_l->polygon_color_next_index = (sd::Index_Type)next_info->next_index;
    
    next_info->i_buffer_count_used = info->i_buffer_count_used;
    
    next_info->base_vertex = base_vertex;
    next_info->reference_count = 0;
    
    return next_info;
}



sd::Images* images(Renderer* renderer)
{
    return renderer->images;
}

bool is_see_through_background(sd::Renderer* r)
{
#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
        SD_Renderer_Metal_Backend* backend = (__bridge SD_Renderer_Metal_Backend*)r->backend;
        return backend->override_bg_color_with_clear;
#else
        return false;
#endif
}
void set_see_through_background(sd::Renderer* r, bool state)
{
#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
        SD_Renderer_Metal_Backend* backend = (__bridge SD_Renderer_Metal_Backend*)r->backend;
        backend->override_bg_color_with_clear = state;
        if (state) {
            [[[NSApplication sharedApplication] mainWindow] setBackgroundColor:[NSColor clearColor]];
        } else {
            [[[NSApplication sharedApplication] mainWindow] setBackgroundColor:[NSColor whiteColor]];
        }
#else
        return;
#endif
}




vec4 view_offset_for_safe_area(sd::Renderer* renderer)
{
    return view_offset_for_safe_area_platform(backing_view);
}


void Drawable_Info_release(Renderer* r, Drawable_Info* info)
{
    if (!Drawable_Info_is_tracked(r, info)) {
        return;
    }
    
    info->reference_count -= 1;
    

    if (info->reference_count != 0) {
        return;
    }
    
//#if SD_END_LAYER_DESTROY_OPT
//    {
//        auto* l = &r->render_layers[info->layer_id];
//        if (info->buffer_index == l->drawable_info_list_count - 1)
//             {
//            l->drawable_info_list_count -= 1;
//            return;
//        }
//    }
//#endif
    
    info->is_enabled = false;
    info->set_texture_ID(sd::Texture_ID_INVALID);
    
    auto& free_list = r->render_layers[info->layer_id].free_list;
    free_list.push_back(info);
    
    info->v_buffer_max_count = m::max(info->v_buffer_count, info->v_buffer_max_count);
    info->i_buffer_max_count = m::max(info->i_buffer_count, info->i_buffer_max_count);
}

void Drawable_Info_print(Drawable_Info* rc)
{
    MTT_print("{\n\tv_buffer_offset=[%llu],\n"
              "\tv_buffer_count=[%llu],\n"
              "\tv_buffer_byte_count=[%llu],\n"
              "\ti_buffer_offset=[%llu],\n"
              "\ti_buffer_count=[%llu],\n"
              "\ti_buffer_byte_count=[%llu],\n"
              "\tfirst_index=[%llu],\n"
              "\tnext_index=[%llu],\n"
              "\tcommand_index=[%llu]\n"
              "}\n",
              rc->v_buffer_offset,
              rc->v_buffer_count,
              rc->i_buffer_count * Render_Layer_sizeof_polygon_vertex(),
              rc->i_buffer_offset,
              rc->i_buffer_count,
              rc->i_buffer_count * sizeof(sd::Index_Type),
              rc->first_index,
              rc->next_index,
              rc->buffer_index
              );
}

// TODO: replace with proper way to proxy same memory
[[deprecated]]
Drawable_Info* Drawable_Info_make_proxy(sd::Renderer* renderer, Drawable_Info* d_info, Render_Layer_ID layer_dst)
{
    
    Render_Layer* l = &renderer->render_layers[d_info->layer_id];

    auto* proxy_drawable = sd::Drawable_Info_reserve_next(renderer, l);
    
    *proxy_drawable = *d_info;
    
    return proxy_drawable;
}

MTT_NODISCARD sd::Shader_ID rasterization_shader_make_with_builtin_vertex_function(sd::Renderer* renderer, sd::Shader_ID vertex_shader_function_id, cstring fragment_function_src, cstring fragment_function_name)
{
    @autoreleasepool {
        auto* r = ((__bridge SD_Renderer_Metal_Backend*)renderer->backend);
        
        NSString* src = [NSString stringWithCString:fragment_function_src encoding:NSUTF8StringEncoding];
        NSString* name = [NSString stringWithCString:fragment_function_src encoding:NSUTF8StringEncoding];
        
        return [r rasterization_shader_make_with_builtin_vertex_function:vertex_shader_function_id fragment_function_src:src fragment_function_name:name];
    }
}
sd::Shader_ID rasterization_shader_replace_with_builtin_vertex_function(sd::Renderer* renderer, sd::Shader_ID vertex_shader_function_id, cstring fragment_function_src, cstring fragment_function_name, Shader_ID shader_id)
{
    @autoreleasepool {
        auto* r = ((__bridge SD_Renderer_Metal_Backend*)renderer->backend);
        
        NSString* src = [NSString stringWithCString:fragment_function_src encoding:NSUTF8StringEncoding];
        NSString* name = [NSString stringWithCString:fragment_function_name encoding:NSUTF8StringEncoding];
        
        return [r rasterization_shader_replace_with_builtin_vertex_function:vertex_shader_function_id fragment_function_src:src fragment_function_name:name with_shader_id:shader_id];
    }
}
void rasterization_shader_destroy(sd::Renderer* renderer, Shader_ID shader_id)
{
    @autoreleasepool {
        auto* r = ((__bridge SD_Renderer_Metal_Backend*)renderer->backend);
        [r rasterization_shader_destroy:shader_id];
    }
}



MTT_NODISCARD Shader_ID compute_shader_make(sd::Renderer* renderer, cstring cstr, cstring main_name)
{
    @autoreleasepool {
        auto* r = ((__bridge SD_Renderer_Metal_Backend*)renderer->backend);
        
        NSString* src = [NSString stringWithCString:cstr encoding:NSUTF8StringEncoding];
        NSString* name = [NSString stringWithCString:main_name encoding:NSUTF8StringEncoding];
        
        return [r compute_shader_make:src with_function_name:name];
    }
}
MTT_NODISCARD Shader_ID compute_shader_make_from_builtin(sd::Renderer* renderer, cstring main_name)
{
    @autoreleasepool {
        auto* r = ((__bridge SD_Renderer_Metal_Backend*)renderer->backend);
        
        NSString* name = [NSString stringWithCString:main_name encoding:NSUTF8StringEncoding];
        
        return [r compute_shader_make_from_builtin:name];
    }
}

void compute_shader_destroy(sd::Renderer* renderer, Shader_ID shader_id)
{
    @autoreleasepool {
        auto* r = ((__bridge SD_Renderer_Metal_Backend*)renderer->backend);
        [r compute_shader_destroy:shader_id];
    }
}
sd::Shader_ID compute_shader_replace(sd::Renderer* renderer, cstring src_str, cstring main_name, sd::Shader_ID shader_id)
{
    @autoreleasepool {
        auto* r = ((__bridge SD_Renderer_Metal_Backend*)renderer->backend);
        
        NSString* src = [NSString stringWithCString:src_str encoding:NSUTF8StringEncoding];
        NSString* name = [NSString stringWithCString:main_name encoding:NSUTF8StringEncoding];
        
        return [r compute_shader_replace:src with_function_name:name with_shader_id:shader_id];
    }
}

SD_vec2 depth_range_default(sd::Renderer* renderer)
{
    return renderer->depths_default;
}


Blur Blur_make(sd::Renderer* renderer, float32 radius)
{
    auto* r = ((__bridge SD_Renderer_Metal_Backend*)renderer->backend);
    @autoreleasepool {
        id<MTLDevice> dev = [r active_device];
        
        MPSImageGaussianBlur *blur = [[MPSImageGaussianBlur alloc] initWithDevice: dev sigma:radius];
        
        Blur blur_out = {
            .handle = CFBridgingRetain(blur),
            .radius = radius
        };
        
        return blur_out;
    }
}
void Blur_destroy(sd::Renderer* renderer, Blur* blur)
{
    @autoreleasepool {
        CFBridgingRelease(blur->handle);
        blur->handle = nil;
    }
}

void compute_set_blur(sd::Renderer* renderer, Blur* blur, Texture_Handle* texture_handle, bool should_wait)
{
    auto* cmd = Command_make(renderer);
    cmd->type = COMMAND_TYPE_COMPUTE_SET_BLUR;
    cmd->compute_pass.blur_set.blur = *blur;
    cmd->compute_pass.blur_set.texture_handle = *texture_handle;
    cmd->compute_pass.blur_set.should_wait = should_wait;
}

void compute_set_blur(sd::Renderer* renderer, float32 radius, Texture_Handle* texture_handle, bool should_wait)
{
    auto* cmd = Command_make(renderer);
    cmd->type = COMMAND_TYPE_COMPUTE_SET_BLUR;
    cmd->compute_pass.blur_set.blur.handle = nullptr;
    cmd->compute_pass.blur_set.blur.radius = radius;
    cmd->compute_pass.blur_set.texture_handle = *texture_handle;
    cmd->compute_pass.blur_set.should_wait = should_wait;
}

void compute_set_sobel(sd::Renderer* renderer, Texture_Handle* texture_handle_dst, Texture_Handle* texture_handle_src)
{
    auto* cmd = Command_make(renderer);
    cmd->type = COMMAND_TYPE_COMPUTE_SET_SOBEL;
    cmd->compute_pass.sobel_set.texture_handle_src = *texture_handle_src;
    cmd->compute_pass.sobel_set.texture_handle_dst = *texture_handle_dst;
}

bool Render_Target_make_msaa(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out)
{
#ifndef NDEBUG
    if ((flags & MTLTextureUsageRenderTarget) == 0) {
        ASSERT_MSG(false, "Must be a render target");
    }
#endif
    sd::Images* imgs = sd::images(renderer);
    MTT_UNUSED(imgs);
    
    sd::Texture_Handle tex_initial;
    sd::Texture_Handle tex_resolved;
    {
        sd::Image* img = sd::Image_create_empty_w_size(sd::images(renderer), name + "__initial", size, {.usage_flags = flags, .for_msaa = true}, &tex_initial);
        if (img == nullptr) {
            return false;
        }
    }
    {
        sd::Image* img = sd::Image_create_empty_w_size(sd::images(renderer), name + "__resolved", size, {.usage_flags = flags}, &tex_resolved);
        if (img == nullptr) {
            return false;
        }
    }
    
    sd::Texture_Handle depth_stencil_initial;
    sd::Texture_Handle depth_stencil_resolved;
    {
        sd::Image* img = sd::Image_create_empty_w_size(sd::images(renderer), name + "__depth_initial", size, {.usage_flags = flags, .for_msaa = true, .for_depth = true}, &depth_stencil_initial);
        if (img == nullptr) {
            return false;
        }
    }
    {
        sd::Image* img = sd::Image_create_empty_w_size(sd::images(renderer), name + "__depth_resolved", size, {.usage_flags = flags, .for_depth = true}, &depth_stencil_resolved);
        if (img == nullptr) {
            return false;
        }
    }
    
    Render_Target rt = Render_Target_init_msaa(&tex_resolved, &tex_initial, &depth_stencil_resolved, &depth_stencil_initial);
    rt.size = size;
    rt.flags = flags;
    *rt_out = rt;
    
    return true;
}
bool Render_Target_make_no_depth_stencil_msaa(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out)
{
#ifndef NDEBUG
    if ((flags & MTLTextureUsageRenderTarget) == 0) {
        ASSERT_MSG(false, "Must be a render target");
    }
#endif
    sd::Images* imgs = sd::images(renderer);
    MTT_UNUSED(imgs);
    
    sd::Texture_Handle tex_initial;
    sd::Texture_Handle tex_resolved;
    {
        sd::Image* img = sd::Image_create_empty_w_size(sd::images(renderer), name + "__initial", size, {.usage_flags = flags, .for_msaa = true}, &tex_initial);
        if (img == nullptr) {
            return false;
        }
    }
    {
        sd::Image* img = sd::Image_create_empty_w_size(sd::images(renderer), name + "__resolved", size, {.usage_flags = flags}, &tex_resolved);
        if (img == nullptr) {
            return false;
        }
    }
    
    Render_Target rt = Render_Target_init_msaa_no_depth_stencil(&tex_resolved, &tex_initial);
    rt.size = size;
    rt.flags = flags;
    *rt_out = rt;
    return true;
}

bool Render_Target_make(sd::Renderer* renderer, const mtt::String& name, uvec2 size,  uint64 flags, Render_Target* rt_out)
{
    ASSERT_MSG(false, "not implemented %s\n", __PRETTY_FUNCTION__);
    return false;
}
bool Render_Target_make_no_depth_stencil(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out)
{
#ifndef NDEBUG
    if ((flags & MTLTextureUsageRenderTarget) == 0) {
        ASSERT_MSG(false, "Must be a render target");
    }
#endif
    sd::Images* imgs = sd::images(renderer);
    MTT_UNUSED(imgs);
    sd::Texture_Handle tex;
    sd::Image* img = sd::Image_create_empty_w_size(sd::images(renderer), name, size, {.usage_flags = flags}, &tex);
    if (img == nullptr) {
        return false;
    }
    Render_Target rt = Render_Target_init_no_depth_stencil(&tex);
    rt.size = size;
    rt.flags = flags;
    *rt_out = rt;
    return true;
}

usize max_texture_dimension_2(sd::Renderer* renderer)
{
    return [((__bridge SD_Renderer_Metal_Backend*)renderer->backend) max_texture_dimension_2];
}


}
#ifdef SD_BOUNDS_CHECK
#undef SD_BOUNDS_CHECK
#endif
