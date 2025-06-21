//
//  dynamic_api.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/1/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef dynamic_api_hpp
#define dynamic_api_hpp

#include "stratadraw.h"
#include "image.hpp"

namespace dt {
void* selections(void);
}
namespace sd {

struct Renderer_API {
    void (*Drawable_Info_release)(Renderer* r, Drawable_Info* info);
    
    void (*begin_path)(Renderer* renderer);
    Drawable_Info* (*end_path)(Renderer* renderer);
    Drawable_Info* (*end_path_closed)(Renderer* renderer);
    void (*break_path)(Renderer* renderer);

    void (*begin_path_no_new_drawable)(Renderer* renderer);
    void (*end_path_no_new_drawable)(Renderer* renderer);

    void (*contour)(Renderer* renderer, std::vector<Boolean_Contour_Element> const & contour);

    void (*begin_polygon)(Renderer* renderer);
    Drawable_Info* (*end_polygon)(Renderer* renderer);

    void (*begin_polygon_no_new_drawable)(Renderer* renderer);
    void (*end_polygon_no_new_drawable)(Renderer* renderer);

    void (*begin_drawable)(Renderer* renderer);
    Drawable_Info* (*end_drawable)(Renderer* renderer);

    void (*set_color_rgba)(Renderer* renderer, float32, float32, float32, float32);
    void (*set_color_rgba_v4)(Renderer* renderer, vec4);
    
    vec4 (*get_color_rgba)(Renderer* renderer);

    void (*set_color_hsva)(Renderer* renderer, float32, float32, float32, float32);
    void (*set_color_hsva_v4)(Renderer* renderer, vec4);


    vec4 (*get_background_color)(Renderer* renderer);
    void (*set_background_color_rgba)(Renderer* renderer, float32, float32, float32, float32);

    void (*set_background_color_rgba_v4)(Renderer* renderer, vec4);

    void (*set_render_layer)(Renderer* renderer, Render_Layer_ID render_layer);
    Render_Layer_ID (*get_render_layer)(Renderer* renderer);

    void (*rewind_layer)(Renderer* renderer, Render_Layer_ID render_layer);
    void (*rewind_staging_layer)(Renderer* renderer, Render_Layer*);
    
    void (*vertex)(Renderer* renderer, float64 x, float64 y, float64 z);
    void (*vertex_v2)(Renderer* renderer, vec2 position);
    void (*vertex_v3)(Renderer* renderer, vec3 position);
    void (*vertex_color)(Renderer* renderer, float64 x, float64 y, float64 z, SD_vec2 uv, SD_vec4 color);
    
    void (*vertex_w_uv)(Renderer* renderer, float64 x, float64 y, float64 z, SD_vec2 uv);
    void (*vertex_v2_w_uv)(Renderer* renderer, vec2 position, SD_vec2 uv);
    void (*vertex_v3_w_uv)(Renderer* renderer, vec3 position, SD_vec2 uv);

    void (*path_vertex)(Renderer* renderer, float64 x, float64 y, float64 z);
    void (*path_vertex_v2)(Renderer* renderer, vec2 position);
    void (*path_vertex_v3)(Renderer* renderer, vec3 position);
    void (*path_list)(Renderer* renderer, vec3*, usize count);
    void (*path_list_with_offset)(Renderer* renderer, vec3* list, usize count, vec3 offset);
    void (*path_list_with_radii)(Renderer* renderer, vec3* list, usize count, float64* radii);
    void (*path_list_with_offset_with_radii)(Renderer* renderer, vec3* list, usize count, vec3 offset, float64* radii);
    void (*path_radius)(Renderer* renderer, float32 pixel_radius);
    void (*set_path_radius)(Renderer* renderer, float32 pixel_radius);
    float32 (*get_path_radius)(Renderer* renderer);
    void (*path_arrow_head)(Renderer* renderer, vec3 src_point, vec3 dst_point, float32 radius);
    void (*path_arrow_head_with_tail)(Renderer* renderer, vec3 src_point, vec3 dst_point, float32 radius);

    void (*quad_color)(Renderer* renderer, vec2 tl, vec2 bl, vec2 br, vec2 tr, float32 depth, vec4 ctl, vec4 cbl, vec4 cbr, vec4 ctr);
    void (*quad_color_uv)(Renderer* renderer, SD_vec2 tl, SD_vec2 bl, SD_vec2 br, SD_vec2 tr, float32 depth, SD_vec4 ctl, SD_vec4 cbl, SD_vec4 cbr, SD_vec4 ctr, SD_vec2 uv_tl, SD_vec2 uv_bl, SD_vec2 uv_br, SD_vec2 uv_tr);

    void (*triangle)(Renderer* renderer, vec3 a, vec3 b, vec3 c);
    void (*triangle_color)(Renderer* renderer, vec3 a, vec3 b, vec3 c, vec4 color);
    void (*triangle_per_vertex_color)(Renderer* renderer, vec3 a, vec3 b, vec3 c, vec4 color_a, vec4 color_b, vec4 color_c);
    
    void (*triangle_equilateral)(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side);
    void (*triangle_equilateral_color)(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side, vec4 color);
    void (*triangle_equilateral_per_vertex_color)(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side, vec4 color_height, vec4 color_left, vec4 color_right);
    
    void (*disable_command)(Renderer*, Drawable_Info*);
    void (*enable_command)(Renderer*, Drawable_Info*);
    void (*polygon_convex_regular)(Renderer*, float32, vec3, usize);
    void (*polygon_convex_regular_v2)(Renderer*, float32, vec2, usize);
    void (*polygon_arc_fraction)(Renderer* renderer, float32 radius, vec3 center, usize count_sides, float32 arc_fraction);
    void (*circle)(Renderer*, float32, vec3);
    void (*circle_with_sides)(Renderer*, float32, vec3, float32);
    void (*quad)(Renderer* renderer, vec2 tl, vec2 bl, vec2 br, vec2 tr, float32 depth);
    void (*path_quad)(Renderer*, vec2, vec2, vec2, vec2, float32);
    void (*rectangle)(Renderer* renderer, vec2 tl, vec2 dimensions, float32 depth);
    void (*rectangle_w_corners)(Renderer* renderer, vec2 tl, vec2 br, float32 depth);

    void (*rectangle_rounded)(Renderer* renderer, vec2 tl, vec2 dimensions, float32 depth, float32 roundness, int32 segments);

    void (*rectangle_dashed)(Renderer* renderer, vec2 tl, vec2 dimensions, float32 depth, usize segments);



    void (*set_depth)(Renderer*, float64);

    void (*Renderer_init)(Renderer* renderer, mem::Allocator* allocator);




    Render_Layer_ID (*Render_Layer_make)(Renderer* renderer, usize block_count);
    //extern bool (*Render_Layer_reallocate)(Renderer* renderer, usize block_count, Render_Layer_ID rlid);
    bool (*Render_Layer_reallocate_bytes)(Renderer* renderer, usize block_count, Render_Layer_ID rlid);
    bool (*Render_Layer_reallocate_vertex_bytes)(Renderer* renderer, usize block_count, Render_Layer_ID rlid);
    bool (*Render_Layer_reallocate_index_bytes)(Renderer* renderer, usize block_count, Render_Layer_ID rlid);
    void (*path_bezier_arrow_head_with_tail)(Renderer* renderer, vec3 src_point, vec3 p_mid_anchor1, vec3 p_mid_anchor2, vec3 dst_point, float32 radius, usize segment_count);


    void (*path_cross)(Renderer* renderer, const vec3 center, vec3 offset);

    void (*continue_path)(Renderer* renderer);




    void (*triangulate)(Renderer* renderer, sd::Point2D_List* solution);



    vec4 (*view_offset_for_safe_area)(sd::Renderer* renderer);


    void (*set_view)(Renderer*, mat4);
    Mat4 (*get_view)(Renderer*);

    void (*push_projection_view_transform)(Renderer*, mat4, mat4);
    void (*pop_view_transform)(Renderer*);

    void (*push_draw_command_with_layer)(Renderer*, Render_Layer_ID);
    void (*push_draw_command_with_staging_layer)(Renderer*, Render_Layer*);

    void (*push_draw_command_with_info_layer_and_buffer_layer)(Renderer* renderer, Render_Layer_ID render_layer_w_drawable_info, Render_Layer_ID render_layer_w_buffer);

    void (*push_instanced_draw_command_with_layer)(Renderer* renderer, Render_Layer_ID render_layer, Drawable_Instance_Data* instance_data);
//    void (*push_instanced_draw_command_with_drawable)(Renderer* renderer, Drawable_Info* drawable, Drawable_Info* list, usize count);

    void (*push_instanced_draw_command_with_drawable_list)(Renderer* renderer, Drawable_Info* drawable, MTT_List* list);

    void (*push_instanced_draw_command_with_array_slice)(Renderer* renderer, Drawable_Info* drawable, mtt::Array_Slice<sd::Drawable_Info> list);

//    void (*push_instanced_draw_command_lists_with_drawable)(Renderer* renderer, Drawable_Info* drawable, Drawable_Info_List* lists, usize count);


    void (*push_draw_command_with_layer_range)(Renderer* renderer, Render_Layer_ID render_layer, Render_Layer_Range layer_range);

    void (*push_instanced_draw_command_with_staging_layer)(Renderer* renderer, Render_Layer* staging_layer, Drawable_Instance_Data* instance_data);

    void (*push_draw_command_with_drawable_info)(Renderer* renderer, Drawable_Info* drawable_info);

    void (*push_pipeline)(Renderer*, uint64);
    void (*push_color_pipeline)(Renderer*);
    void (*push_color_hsv_pipeline)(Renderer* renderer);
    void (*push_texture_pipeline)(Renderer*);
    void (*push_text_pipeline)(Renderer*);

    void (*push_scissor_rect)(Renderer*, Scissor_Rect);
    void (*push_viewport)(Renderer* renderer, Viewport viewport);

    void (*push_custom_command)(Renderer*, Custom_Command custom);

    void (*set_render_pipeline)(Renderer* renderer, uint64 ID);
    void (*render_to_texture_sync_begin)(Renderer* renderer, sd::Rectangle bounds);
    void (*render_to_texture_end)(Renderer* renderer, void (*callback)(bool, void*));

    void (*set_depth_write_mode)(Renderer*, sd::DEPTH_WRITE_MODE);

    void (*Command_List_begin)(Renderer*, Command_List*);
    void (*Command_List_end)(Renderer*, Command_List*);

    void (*Command_List_submit)(Renderer*, Command_List*);

    void (*clear_commands)(Renderer*);

    //void print_commands(Renderer*);

    Drawable_Info* (*pause_path)(Renderer* renderer);


    sd::Texture_ID (*texture_id_from_name)(sd::Renderer* renderer, const mtt::String& name);

    bool (*texture_is_valid)(sd::Renderer* renderer, sd::Texture_ID texture_id);




    Render_Layer_ID (*layer_id)(Render_Layer* layer);

    void (*save)(Renderer* renderer);
    void (*restore)(Renderer* renderer);

    Drawable_Info* (*Drawable_Info_copy)(Renderer* renderer, Render_Layer_ID ID, Drawable_Info* info);


    Rectangle (*get_display_bounds)(void);
    Rectangle (*get_native_display_bounds)(void);
    float64 (*get_native_display_scale)(void);


    sd::Images* (*images)(Renderer* renderer);

    bool (*is_see_through_background)(sd::Renderer* r);
    void (*set_see_through_background)(sd::Renderer* r, bool state);

    void (*Drawable_Info_print)(Drawable_Info* rc);
    
    void (*set_transform)(Drawable_Info* d_info, const SD_mat4& v);
    SD_mat4 (*get_transform)(Drawable_Info* d_info);
    SD_mat4* (*get_ptr_transform)(Drawable_Info* d_info);
    void (*set_ptr_transform)(Drawable_Info* d_info, const mat4* v);
    
    // images api
    void (*Images_init)(Renderer* renderer, Images* images, usize max_resident, const Images_Init_Args& args);
    void (*images_pick_async)(Renderer* renderer, Image_Loader* loader, Image_Pick_Descriptor* desc, bool (*callback)(Image_Load_Result));
    bool (*images_load_async)(Renderer* renderer, Image_Load_Descriptor* desc, bool (*callback)(const Image_Load_Result& result, bool status));
    bool (*images_load_remote_async)(sd::Renderer* renderer, Image_Load_Descriptor* desc, bool (*callback)(const Image_Load_Result& result, bool status));
    bool (*Images_lookup)(sd::Images* images, cstring name, Image** out_img);
    sd::Image* (*Image_create_empty)(sd::Images* imgs, const mtt::String& name, const sd::Image_Create_Args& args);
    sd::Image* (*Image_create_empty_w_size)(sd::Images* imgs, const mtt::String& name, uvec2 size, const sd::Image_Create_Args& args, Texture_Handle* out_handle);
    void (*Image_create_from_baked)(Images* imgs, cstring name, void* texture, void (*callback)(Image* image));
    void (*Image_save)(Images* images, Image* img, const Image_Save_Descriptor& args);
    void (*Images_for_each)(sd::Images* images, void (callback)(sd::Image* img));
    
    
#ifdef __clang__
    void (*set_transform_intrin)(Drawable_Info* d_info, const simd_float4x4& v);
    simd_float4x4 (*get_transform_intrin)(Drawable_Info* d_info);
    void (*set_ptr_transform_intrin)(Drawable_Info* d_info, const simd_float4x4* v);
#endif
    
    
    mem::Allocator* (*get_main_allocator)(void);
    mem::Allocator (*Heap_Allocator)(void);
    
    void* (*Arena_init)(mem::Arena* arena, mem::Allocator backing, usize chunk_byte_count, usize alignment);
    mem::Allocator (*Arena_Allocator)(mem::Arena* arena);
    void* (*Arena_allocate_proc)(void* state, usize size);
    void  (*Arena_deallocate_all_proc)(void* state, usize count);

    void (*Arena_deinit)(mem::Arena* arena);

    void (*Arena_rewind)(mem::Arena* arena);
    
    mem::Allocator (*Memory_Pool_Fixed_Allocator)(mem::Memory_Pool_Fixed* pool);
    
    void* (*Memory_Pool_Fixed_init)(mem::Memory_Pool_Fixed* pool, mem::Allocator backing, usize num_blocks, usize block_size, usize block_align);
    
    void* (*Memory_Pool_Fixed_allocate_proc)(void* state, usize size);
    void  (*Memory_Pool_Fixed_deallocate_proc)(void* state, void* memory, usize count);

    void (*Memory_Pool_Fixed_deinit)(mem::Memory_Pool_Fixed* pool);
    
    void (*Pool_Allocation_init)(mem::Pool_Allocation* pool_allocation, mem::Allocator backing, usize initial_count, usize element_size, usize alignment);
    
    void (*Pool_Allocation_deinit)(mem::Pool_Allocation* pool_allocation);

    void (*Fixed_Buckets_init)(mem::Fixed_Buckets* alloc, mem::Allocator backing, usize initial_counts);
    void* (*Fixed_Buckets_Allocator_allocate_proc)(void* state, usize size);
    void  (*Fixed_Buckets_Allocator_deallocate_proc)(void* state, void* memory, usize size);
    void (*Fixed_Buckets_Allocator_clear)(mem::Fixed_Buckets* buckets);
    
    void (*Buckets_Allocation_init)(mem::Buckets_Allocation* buckets_allocation, mem::Allocator backing, usize initial_counts);
    void (*Buckets_Allocation_deinit)(mem::Buckets_Allocation* buckets_allocation);
    
    usize32 (*BLOCK_SIZE_LOOKUP)(usize size);
    
    void (*render_pass_begin)(Renderer* r, const Render_Pass_Descriptor& conf);
    void (*render_pass_begin_default)(Renderer* r);
    void (*render_pass_end)(Renderer* r);
    void (*compute_pass_begin)(Renderer* r, const Compute_Pass_Descriptor& conf);
    void (*compute_pass_end)(Renderer* r);
    void (*compute_pass_end_w_descriptor)(Renderer* r, const Compute_Pass_End_Descriptor& desc);
    void (*dispatch_threadgroups)(sd::Renderer* r, vec3 threadgroups_per_grid, vec3 threads_per_threadgroup);
    void (*dispatch_threads)(sd::Renderer* r, vec3 threads_per_grid, vec3 threads_per_threadgroup);
    
    void* (*Buffer_contents)(sd::Buffer* b);

    usize (*Buffer_byte_size)(sd::Buffer* b);

    bool (*Buffer_is_valid)(sd::Buffer* b);

    bool (*Buffer_make)(Renderer* renderer, Buffer* buffer, usize byte_count);
    
    void (*Buffer_destroy)(Renderer* renderer, Buffer* buffer);
    
    void (*vertex_buffer_set_w_offset)(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset);
    void (*vertex_buffer_set)(sd::Renderer* r, Buffer* buffer, uint64 slot);
    void (*fragment_buffer_set_w_offset)(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset);
    void (*fragment_buffer_set)(sd::Renderer* r, Buffer* buffer, uint64 slot);

    sd::Shader_ID (*rasterization_shader_make_with_builtin_vertex_function)(sd::Renderer* renderer, sd::Shader_ID vertex_shader_function_id, cstring fragment_function_src, cstring fragment_function_name);
    sd::Shader_ID (*rasterization_shader_replace_with_builtin_vertex_function)(sd::Renderer* renderer, sd::Shader_ID vertex_shader_function_id, cstring fragment_function_src, cstring fragment_function_name, Shader_ID shader_id);
    void (*rasterization_shader_destroy)(sd::Renderer* renderer, Shader_ID shader_id);
    
    void (*compute_buffer_set_w_offset)(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset);
    void (*compute_buffer_set)(sd::Renderer* r, Buffer* buffer, uint64 slot);
    
    
    bool (*compute_shader_info)(sd::Renderer* r, sd::Shader_ID shader_id, Compute_Shader_Info* info);
    
    void (*compute_shader_set)(sd::Renderer* r, sd::Shader_ID shader_id);
    
    sd::Shader_ID (*compute_shader_make)(sd::Renderer* renderer, cstring cstr, cstring main_name);

    void (*compute_shader_destroy)(sd::Renderer* renderer, Shader_ID shader_id);
    sd::Shader_ID (*compute_shader_replace)(sd::Renderer* renderer, cstring src_str, cstring main_name, sd::Shader_ID shader_id);
    
    SD_vec2 (*depth_range_default)(sd::Renderer* renderer);
    
    Blur (*Blur_make)(sd::Renderer* renderer, float32 radius);
    void (*Blur_destroy)(sd::Renderer* renderer, Blur* blur);
    void (*compute_set_blur)(sd::Renderer* renderer, Blur* blur, Texture_Handle* texture_handle, bool should_wait);
    void (*compute_set_blur_once)(sd::Renderer* renderer, float32 radius, Texture_Handle* texture_handle, bool should_wait);
    void (*compute_set_sobel)(sd::Renderer* renderer, Texture_Handle* texture_handle_dst, Texture_Handle* texture_handle_src);
    
    bool (*Render_Target_make_msaa)(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out);
    bool (*Render_Target_make_no_depth_stencil_msaa)(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out);
    bool (*Render_Target_make)(sd::Renderer* renderer, const mtt::String& name, uvec2 size,  uint64 flags, Render_Target* rt_out);
    bool (*Render_Target_make_no_depth_stencil)(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out);
    
    void (*HSVtoRGB)(int H, double S, double V, float64 output[3]);
    
    void (*set_stencil_reference_value)(Renderer* renderer, uint32 reference_value);
    
    mtt::Array_Slice<sd::Index_Type> (*index_list)(Renderer* renderer, sd::Drawable_Info* info, usize size);
    void (*triangle_strip_to_indexed_triangles_in_place)(Renderer* renderer, sd::Drawable_Info* info);
    void (*set_triangle_indices)(sd::Renderer* renderer, mtt::Array_Slice<sd::Index_Type>& index_list, usize idx, Index_Type a, Index_Type b, Index_Type c);
    
    usize (*max_texture_dimension_2)(sd::Renderer* renderer);
    
    void (*Image_Atlas_init)(Renderer* renderer, Image_Atlas* atlas, usize max_image_dimension_size, uvec2 dimensions);
    
    bool (*Image_Atlas_image_reserve_rectangle)(Image_Atlas* atlas, vec2 required_space, sd::Image_Atlas::Region* reserved_space);
    
    void (*Image_Atlas_clear)(Image_Atlas* atlas);
    
    usize (*Image_Atlas_image_count)(Image_Atlas* atlas);


};
void Renderer_API_set(Renderer_API* api_in);

#ifndef MTT_DYNAMIC_LIBRARY_CLIENT

static inline void Renderer_API_init(Renderer_API* api)
{
    api->Drawable_Info_release = sd::Drawable_Info_release;
    api->begin_path = sd::begin_path;
    api->end_path = sd::end_path;
    api->end_path_closed = sd::end_path_closed;
    
    api->break_path = sd::break_path;
    
    api->begin_path_no_new_drawable = sd::begin_path_no_new_drawable;
    api->end_path_no_new_drawable = sd::end_path_no_new_drawable;
    
    api->contour = sd::contour;
    
    api->begin_polygon = sd::begin_polygon;
    api->end_polygon = sd::end_polygon;
    
    api->begin_polygon_no_new_drawable = sd::begin_polygon_no_new_drawable;
    api->end_polygon_no_new_drawable = sd::end_polygon_no_new_drawable;
    
    api->begin_drawable = sd::begin_drawable;
    api->end_drawable = sd::end_drawable;
    
    api->set_color_rgba = sd::set_color_rgba;
    api->set_color_rgba_v4 = sd::set_color_rgba_v4;
    
    api->get_color_rgba = sd::get_color_rgba;
    api->set_color_hsva = sd::set_color_hsva;
    api->set_color_hsva_v4 = sd::set_color_hsva_v4;
    
    api->get_background_color = sd::get_background_color;
    api->set_background_color_rgba = sd::set_background_color_rgba;
    api->set_background_color_rgba_v4 = sd::set_background_color_rgba_v4;
    
    api->set_render_layer = sd::set_render_layer;
    api->get_render_layer = sd::get_render_layer;
    
    api->rewind_layer = sd::rewind_layer;
    api->rewind_staging_layer = sd::rewind_staging_layer;
    
    
    api->vertex = sd::vertex;
    api->vertex_v2 = sd::vertex_v2;
    api->vertex_v3 = sd::vertex_v3;
    
    api->vertex_color = sd::vertex;
    
    api->vertex_w_uv = sd::vertex;
    api->vertex_v2_w_uv = sd::vertex_v2;
    api->vertex_v3_w_uv = sd::vertex_v3;

    api->path_vertex = sd::path_vertex;
    api->path_vertex_v2 = sd::path_vertex_v2;
    api->path_vertex_v3 = sd::path_vertex_v3;
    api->path_list = sd::path_list;
    api->path_list_with_offset = sd::path_list_with_offset;
    api->path_list_with_radii = sd::path_list;
    api->path_list_with_offset_with_radii = sd::path_list_with_offset;
    api->path_radius = sd::path_radius;
    api->set_path_radius = sd::set_path_radius;
    api->get_path_radius = sd::get_path_radius;
    api->path_arrow_head = sd::path_arrow_head;
    api->path_arrow_head_with_tail = sd::path_arrow_head_with_tail;

    api->quad_color = sd::quad_color;
    api->quad_color_uv = sd::quad_color_uv;

    api->triangle = sd::triangle;
    api->triangle_color = sd::triangle_color;
    api->triangle_per_vertex_color = sd::triangle_per_vertex_color;
    
    api->triangle_equilateral = sd::triangle_equilateral;
    api->triangle_equilateral_color = sd::triangle_equilateral_color;
    api->triangle_equilateral_per_vertex_color = sd::triangle_equilateral_per_vertex_color;
    
    api->disable_command = sd::disable_command;
    api->enable_command = sd::enable_command;
    api->polygon_convex_regular = sd::polygon_convex_regular;
    api->polygon_convex_regular_v2 = sd::polygon_convex_regular_v2;
    api->polygon_arc_fraction = sd::polygon_arc_fraction;
    api->circle = sd::circle;
    api->circle_with_sides = sd::circle;
    api->quad = sd::quad;
    api->path_quad = sd::path_quad;
    api->rectangle = sd::rectangle;
    api->rectangle_w_corners = sd::rectangle_w_corners;

    api->rectangle_rounded = sd::rectangle_rounded;

    api->rectangle_dashed = sd::rectangle_dashed;


    api->set_depth = sd::set_depth;

    api->Renderer_init = sd::Renderer_init;


    api->Render_Layer_make = sd::Render_Layer_make;
    
    api->Render_Layer_reallocate_bytes = sd::Render_Layer_reallocate_bytes;
    api->Render_Layer_reallocate_vertex_bytes = sd::Render_Layer_reallocate_vertex_bytes;
    api->Render_Layer_reallocate_index_bytes = sd::Render_Layer_reallocate_index_bytes;
    api->path_bezier_arrow_head_with_tail = sd::path_bezier_arrow_head_with_tail;


    api->path_cross = sd::path_cross;

    api->continue_path = sd::continue_path;



    api->triangulate = sd::triangulate;


    api->view_offset_for_safe_area = sd::view_offset_for_safe_area;


    api->set_view = sd::set_view;
    api->get_view = sd::get_view;

    api->push_projection_view_transform = sd::push_projection_view_transform;
    api->pop_view_transform = sd::pop_view_transform;

    api->push_draw_command_with_layer = sd::push_draw_command_with_layer;
    api->push_draw_command_with_staging_layer = sd::push_draw_command_with_staging_layer;

    api->push_draw_command_with_info_layer_and_buffer_layer = sd::push_draw_command_with_info_layer_and_buffer_layer;
    api->push_instanced_draw_command_with_layer = sd::push_instanced_draw_command_with_layer;
//    api->push_instanced_draw_command_with_drawable = sd::push_instanced_draw_command_with_drawable;

    api->push_instanced_draw_command_with_drawable_list = sd::push_instanced_draw_command_with_drawable_list;

    api->push_instanced_draw_command_with_array_slice = sd::push_instanced_draw_command_with_drawable_list;

//    api->push_instanced_draw_command_lists_with_drawable = sd::push_instanced_draw_command_lists_with_drawable;


    api->push_draw_command_with_layer_range = sd::push_draw_command_with_layer_range;

    api->push_instanced_draw_command_with_staging_layer = sd::push_instanced_draw_command_with_staging_layer;

    api->push_draw_command_with_drawable_info = sd::push_draw_command_with_drawable_info;

    api->push_pipeline = sd::push_pipeline;
    api->push_color_pipeline = sd::push_color_pipeline;
    api->push_color_hsv_pipeline = sd::push_color_hsv_pipeline;
    api->push_texture_pipeline = sd::push_texture_pipeline;
    api->push_text_pipeline = sd::push_text_pipeline;

    api->push_scissor_rect = sd::push_scissor_rect;
    api->push_viewport = sd::push_viewport;

    api->push_custom_command = sd::push_custom_command;

    api->set_render_pipeline = sd::set_render_pipeline;
    api->render_to_texture_sync_begin = sd::render_to_texture_sync_begin;
    api->render_to_texture_end = sd::render_to_texture_end;

    api->set_depth_write_mode = sd::set_depth_write_mode;

    api->Command_List_begin = sd::Command_List_begin;
    api->Command_List_end = sd::Command_List_end;

    api->Command_List_submit = sd::Command_List_submit;

    api->clear_commands = sd::clear_commands;


    api->pause_path = sd::pause_path;

    api->texture_id_from_name = sd::texture_id_from_name;

    api->texture_is_valid = sd::texture_is_valid;


    api->layer_id = sd::layer_id;

    api->save = sd::save;
    api->restore = sd::restore;

    api->Drawable_Info_copy = sd::Drawable_Info_copy;
    
    api->get_transform = sd::get_transform;
    api->get_ptr_transform = sd::get_ptr_transform;
    api->set_transform = sd::set_transform;
    api->set_ptr_transform = sd::set_ptr_transform;


    api->get_display_bounds = sd::get_display_bounds;
    api->get_native_display_bounds = sd::get_native_display_bounds;
    api->get_native_display_scale = sd::get_native_display_scale;


    api->images = sd::images;

    api->is_see_through_background = sd::is_see_through_background;
    api->set_see_through_background = sd::set_see_through_background;

    api->Drawable_Info_print = sd::Drawable_Info_print;
    
    // images
    
    api->Images_init = sd::Images_init;
    api->images_pick_async = sd::images_pick_async;
    api->images_load_async = sd::images_load_async;
    api->images_load_remote_async = sd::images_load_remote_async;
    api->Images_lookup = sd::Images_lookup;
    api->Image_create_empty = sd::Image_create_empty;
    api->Image_create_empty_w_size = sd::Image_create_empty_w_size;
    api->Image_create_from_baked = sd::Image_create_from_baked;
    api->Image_save = sd::Image_save;
    api->Images_for_each = sd::Images_for_each;

#ifdef __clang__
    api->set_transform_intrin = sd::set_transform_intrin;
    api->get_transform_intrin = sd::get_transform_intrin;
    api->set_ptr_transform_intrin = sd::set_ptr_transform_intrin;
#endif
    
    api->render_pass_begin = sd::render_pass_begin;
    api->render_pass_begin_default = render_pass_begin;
    api->render_pass_end = render_pass_end;
    api->compute_pass_begin = sd::compute_pass_begin;
    api->compute_pass_end = sd::compute_pass_end;
    api->compute_pass_end_w_descriptor = sd::compute_pass_end;
    api->dispatch_threadgroups = sd::dispatch_threadgroups;
    api->dispatch_threads = sd::dispatch_threads;
    
    api->Buffer_contents = sd::Buffer_contents;

    api->Buffer_byte_size = sd::Buffer_byte_size;

    api->Buffer_is_valid = sd::Buffer_is_valid;

    api->Buffer_make = sd::Buffer_make;
    
    api->Buffer_destroy = sd::Buffer_destroy;
    
    api->vertex_buffer_set_w_offset = sd::vertex_buffer_set;
    api->vertex_buffer_set = sd::vertex_buffer_set;
    api->fragment_buffer_set_w_offset = sd::fragment_buffer_set;
    api->fragment_buffer_set = sd::fragment_buffer_set;
    
    api->rasterization_shader_make_with_builtin_vertex_function = sd::rasterization_shader_make_with_builtin_vertex_function;
    api->rasterization_shader_replace_with_builtin_vertex_function = sd::rasterization_shader_replace_with_builtin_vertex_function;
    api->rasterization_shader_destroy = sd::rasterization_shader_destroy;
    
    api->compute_buffer_set_w_offset = sd::compute_buffer_set;
    api->compute_buffer_set = sd::compute_buffer_set;
    
    api->compute_shader_info = sd::compute_shader_info;
    
    api->compute_shader_set = sd::compute_shader_set;
    
    api->compute_shader_make = sd::compute_shader_make;

    api->compute_shader_destroy = sd::compute_shader_destroy;
    api->compute_shader_replace = sd::compute_shader_replace;
    
    api->depth_range_default = sd::depth_range_default;
    
    api->Blur_make = sd::Blur_make;
    api->Blur_destroy = sd::Blur_destroy;
    api->compute_set_blur = sd::compute_set_blur;
    api->compute_set_blur_once = sd::compute_set_blur;
    
    api->compute_set_sobel = sd::compute_set_sobel;
    
    api->Render_Target_make_msaa = sd::Render_Target_make_msaa;
    api->Render_Target_make_no_depth_stencil_msaa = sd::Render_Target_make_no_depth_stencil_msaa;
    api->Render_Target_make = sd::Render_Target_make;
    api->Render_Target_make_no_depth_stencil = sd::Render_Target_make_no_depth_stencil;
    
    api->HSVtoRGB = HSVtoRGB;
    
    api->set_stencil_reference_value = sd::set_stencil_reference_value;
    
    api->index_list = sd::index_list;
    api->triangle_strip_to_indexed_triangles_in_place = sd::triangle_strip_to_indexed_triangles_in_place;
    api->set_triangle_indices = sd::set_triangle_indices;
    
    api->max_texture_dimension_2 = sd::max_texture_dimension_2;
    
    api->Image_Atlas_init = sd::Image_Atlas_init;
    
    api->Image_Atlas_image_reserve_rectangle = sd::Image_Atlas_image_reserve_rectangle;
    
    api->Image_Atlas_clear = sd::Image_Atlas_clear;
    
    api->Image_Atlas_image_count = sd::Image_Atlas_image_count;

}


#endif

}



namespace mtt {

struct MTT_API {
    mtt::World* (*MTT_ctx)(void);
    
    mtt::Thing_ID (*Thing_make)(mtt::World* world, mtt::Thing_Archetype_ID arch_id, mtt::Thing** out, const mtt::Thing_Make_Args& args);
    mtt::Thing* (*Thing_try_get)(mtt::World* world, mtt::Thing_ID id);
    void (*Thing_destroy)(mtt::Thing* thing);
    
    mtt::Thing* (*Thing_make_with_collider)(mtt::World* world, mtt::Rep** rep, mtt::ARCHETYPE arch, mtt::COLLIDER_TYPE collider_type, bool is_in_world, mtt::Collision_System* collision_system);
    
    mtt::Thing* (*Thing_make_with_aabb_corners)(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, mtt::Box box, float32 z_position, bool is_in_world, mtt::Collision_System* collision_system);
    
    mtt::Thing* (*Thing_make_with_aabb_dimensions)(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, vec3 in_position, vec2 dimensions, bool is_in_world, mtt::Collision_System* collision_system);
    
    mtt::Thing* (*Thing_make_with_unit_collider)(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, mtt::COLLIDER_TYPE collider_type, vec3 in_position, bool is_in_world, mtt::Collision_System* collision_system);
    
    mtt::Thing* (*Thing_copy)(mtt::Thing* src);
    
    mtt::Thing_ID (*get_thing_most_recently_selected_with_touch)(mtt::World* world);
    
    void (*Thing_set_position_w_ptr)(mtt::Thing* thing, vec3 position);
    mtt::Thing* (*Thing_set_position_w_id)(mtt::World* world, mtt::Thing_ID id, vec3 position);
    
    void (*set_pose_transform)(mtt::Thing* thing, const Mat4* mat);
    void (*set_pose_transform_w_ref)(mtt::Thing* thing, const Mat4& mat);
    
    void (*push_AABB)(Collision_System* sys, Collider* c);
    void (*push_AABBs)(Collision_System* sys, Collider* boxes, usize count, usize layer);
    void (*Collider_remove)(Collision_System* sys, usize layer, Collider* c);
    Collider* (*Collider_make_aabb)(Collision_System* sys);
    
    mem::Allocator* (*get_main_allocator)(void);
    mem::Allocator (*Heap_Allocator)(void);
    
    void* (*Arena_init)(mem::Arena* arena, mem::Allocator backing, usize chunk_byte_count, usize alignment);
    mem::Allocator (*Arena_Allocator)(mem::Arena* arena);
    void* (*Arena_allocate_proc)(void* state, usize size);
    void  (*Arena_deallocate_all_proc)(void* state, usize count);

    void (*Arena_deinit)(mem::Arena* arena);

    void (*Arena_rewind)(mem::Arena* arena);
    
    mem::Allocator (*Memory_Pool_Fixed_Allocator)(mem::Memory_Pool_Fixed* pool);
    
    void* (*Memory_Pool_Fixed_init)(mem::Memory_Pool_Fixed* pool, mem::Allocator backing, usize num_blocks, usize block_size, usize block_align);
    
    void* (*Memory_Pool_Fixed_allocate_proc)(void* state, usize size);
    void  (*Memory_Pool_Fixed_deallocate_proc)(void* state, void* memory, usize count);

    void (*Memory_Pool_Fixed_deinit)(mem::Memory_Pool_Fixed* pool);
    
    void (*Pool_Allocation_init)(mem::Pool_Allocation* pool_allocation, mem::Allocator backing, usize initial_count, usize element_size, usize alignment);
    
    void (*Pool_Allocation_deinit)(mem::Pool_Allocation* pool_allocation);

    void (*Fixed_Buckets_init)(mem::Fixed_Buckets* alloc, mem::Allocator backing, usize initial_counts);
    void* (*Fixed_Buckets_Allocator_allocate_proc)(void* state, usize size);
    void  (*Fixed_Buckets_Allocator_deallocate_proc)(void* state, void* memory, usize size);
    void (*Fixed_Buckets_Allocator_clear)(mem::Fixed_Buckets* buckets);
    
    void (*Buckets_Allocation_init)(mem::Buckets_Allocation* buckets_allocation, mem::Allocator backing, usize initial_counts);
    void (*Buckets_Allocation_deinit)(mem::Buckets_Allocation* buckets_allocation);
    
    usize32 (*BLOCK_SIZE_LOOKUP)(usize size);
    
    MTT_String_Ref (*string)(cstring str);
    MTT_String_Ref (*string_w_len)(cstring str, MTT_String_Length length);
    bool (*string_free)(MTT_String_Ref& str);
    cstring (*string_get)(MTT_String_Ref& str);
    MTT_String_Ref (*string_ref_get)(cstring str);
    
    void* (*selected_things)(mtt::World* world);
    
    void* (*selections)(void);
    
    void (*Particle_State_update)(Particle_System_State*);
    
    void (*init_flag)(THING_FLAG* src);
    void (*set_flag)(THING_FLAG* src, THING_FLAG flag);
    void (*unset_flag)(THING_FLAG* src, THING_FLAG flag);
    bool (*flag_is_set)(THING_FLAG* src, THING_FLAG flag);
    void (*set_flag_w_state)(THING_FLAG* src, THING_FLAG flag, bool state);
    
    void (*access_write_string)(mtt::Thing* thing, const mtt::String& name, const mtt::String& set_to);
    cstring (*access_string)(mtt::Thing* thing, const mtt::String& name);
    
    void (*set_logic_proc_for_archetype)(mtt::World* world, mtt::Thing_Archetype_ID id, mtt::Logic_Procedure proc);
    
    void* (*access)(Thing* thing, const String& tag, MTT_TYPE type);
    
    void (*connect_parent_to_child_w_ptrs)(mtt::World* world, mtt::Thing* parent, mtt::Thing* child);
    void (*connect_parent_to_child_w_ids)(mtt::World* world, mtt::Thing_ID parent, mtt::Thing_ID child);

    void (*disconnect_child_from_parent_w_ptr)(mtt::World* world, mtt::Thing* child_thing);
    void (*disconnect_child_from_parent_w_id)(mtt::World* world, mtt::Thing_ID child);
    void (*disconnect_parent_from_children)(mtt::World* world, mtt::Thing* thing);

    void (*remove_thing_from_hierarchy)(mtt::World* world, mtt::Thing* thing);
    
    void (*flip_left_right)(mtt::Thing* thing);
    
    mtt::Thing_ID (*get_parent_ID)(mtt::Thing* thing);
    mtt::Thing* (*get_parent)(mtt::Thing* thing);
    mtt::Thing* (*get_parent_with_archetype)(mtt::Thing* thing, mtt::Thing_Archetype_ID arch);
    void (*set_option)(Thing* thing, uint64 option, uint64 flags);
    
    bool (*is_root_w_ptr)(mtt::Thing* thing);
    bool (*is_root_w_id)(mtt::World* world, mtt::Thing_ID thing_id);
    bool (*exists_in_other_hierarchy)(mtt::Thing* thing, mtt::Thing* other);
    
    bool (*add_connection_w_ids)(World* world, Thing_ID src, const String& src_tag, Thing_ID dst, const String& dst_tag, Thing** src_thing_out, Thing** dst_thing_out);

    bool (*add_connection_w_ptrs)(World* world, Thing* src, const String& src_tag, Thing* dst, const String& dst_tag);
    
    bool (*remove_all_connections)(World* world, Thing_ID thing_id);
    
    void (*add_execution_connection)(mtt::Thing* thing_src, mtt::Thing* thing_dst);
    void (*remove_execution_connection)(mtt::Thing* thing_src, mtt::Thing* thing_dst);
    void (*remove_incoming_execution_connections)(mtt::Thing* thing);
    void (*remove_outgoing_execution_connections)(mtt::Thing* thing);
    void (*remove_all_execution_connections)(mtt::Thing* thing);
    void (*delete_execution_connections)(mtt::Thing* thing);


    void (*Connection_print)(Connection* connection, World* world);
    
    void (*add_selector_w_ptr)(mtt::Thing_Archetype* arch, cstring name, const mtt::Procedure& proc);
    bool (*add_selector_w_id)(mtt::World* world, mtt::Thing_Archetype_ID id, cstring name, const mtt::Procedure& proc);
    
    void (*add_own_selector)(mtt::Thing* thing, cstring name, const mtt::Procedure& proc);
    void (*remove_own_selector)(mtt::Thing* thing, cstring name);
    Procedure_Return_Type (*selector_invoke_w_arch)(mtt::Thing_Archetype* arch, mtt::Thing* thing, Message* message);
    Procedure_Return_Type (*selector_invoke)(mtt::Thing* thing, Message* message);
    
    bool (*thing_group_is_active)(mtt::Thing* const thing);
    bool (*thing_group_set_active_state_w_ptr)(mtt::Thing* const thing, bool active_state);
    bool (*thing_group_set_active_state_w_id)(mtt::World* world, mtt::Thing_ID const thing_id, bool active_state);
    
    bool (*input_should_cancel_animation)(mtt::Thing* thing);
    
    bool (*is_ancestor_of)(mtt::Thing* possible_parent, mtt::Thing* thing);
    
    bool (*should_be_immobile)(mtt::Thing* thing);
    
    Result<Evaluation_Output_Entry_Port&> (*get_in_port)(Thing* thing, Port_Input_List* input_list, const String& tag);
    Result<Evaluation_Output_Entry_Port&> (*get_out_port)(Thing* thing, const String& tag);
    
    MTT_NODISCARD
    Script_Property_List* (*Script_Lookup_get_var_with_key)(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var);
    void (*Script_Lookup_set_var_group)(Script_Lookup& lookup, const mtt::String& key_ctx, Script_Property_Lookup& var_group);
    MTT_NODISCARD
    Script_Property_Lookup* (*Script_Lookup_get_var_group)(Script_Lookup& lookup, const mtt::String& key_ctx);
    void (*Script_Lookup_set_var_with_key)(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var, const Script_Property_List& val);
    void (*Script_Lookup_set_key)(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var);
    MTT_NODISCARD
    Script_Property_List* (*Script_Lookup_get_var_with_key_w_script_instance)(Script_Instance* s, const mtt::String& key_ctx, const mtt::String& key_var);
    void (*Script_Lookup_set_var_with_key_w_script_instance)(Script_Instance* s, const mtt::String& key_ctx, const mtt::String& key_var, const Script_Property_List& val);
    
    
    dt::DrawTalk* (*DrawTalk_ctx)(void);
    
    uint64 (*MTT_Random)(void);
    sint64 (*MTT_Random_lower_bounded)(sint64 lower_bound_exclusive);
    sint64 (*MTT_Random_range)(sint64 lower_bound_inclusive, sint64 upper_bound_exclusive);
    float64 (*MTT_Random_Float64_value)(void);
    float64 (*MTT_Random_Float64_value_scaled)(float64 scale);
    void (*MTT_Random_Float64_value_within_circle)(float64 extent, float64* radius_out, float64* angle_radians_out);
    
    void (*MTT_set_active_logger)(struct MTT_Logger* logger);
    struct MTT_Logger* (*MTT_active_logger)(void);
    void (*MTT_set_default_active_logger)(void);
    void (*MTT_Logger_init)(struct MTT_Logger* logger_out, cstring subsystem, cstring category);
    void (*MTT_Logger_destroy)(struct MTT_Logger* logger);
    
    void (*debug_mode_log_enable)(bool state);
    bool (*debug_mode_log_is_enabled)(void);
};

void MTT_API_set(MTT_API* api_in);

#ifndef MTT_DYNAMIC_LIBRARY_CLIENT

static inline void MTT_API_init(MTT_API* api)
{
    api->MTT_ctx = mtt::ctx;
    api->Thing_make = mtt::Thing_make;
    api->Thing_try_get = mtt::Thing_try_get;
    api->Thing_destroy = mtt::Thing_destroy;
    
    api->Thing_make_with_collider = mtt::Thing_make_with_collider;
    api->Thing_make_with_aabb_corners = mtt::Thing_make_with_aabb_corners;
    api->Thing_make_with_aabb_dimensions = mtt::Thing_make_with_aabb_dimensions;
    api->Thing_make_with_unit_collider = mtt::Thing_make_with_unit_collider;
    api->Thing_copy = mtt::Thing_copy;
    api->get_thing_most_recently_selected_with_touch = mtt::get_thing_most_recently_selected_with_touch;
    
    api->Thing_set_position_w_ptr = mtt::Thing_set_position;
    api->Thing_set_position_w_id = mtt::Thing_set_position;
    api->set_pose_transform = mtt::set_pose_transform;
    api->set_pose_transform_w_ref = mtt::set_pose_transform;
    
    
    api->push_AABB = mtt::push_AABB;
    api->push_AABBs = mtt::push_AABBs;
    api->Collider_remove = mtt::Collider_remove;
    api->Collider_make_aabb = mtt::Collider_make_aabb;
    
    
    
    api->get_main_allocator = mem::get_main_allocator;
    api->Heap_Allocator = mem::Heap_Allocator;
    
    api->Arena_init = mem::Arena_init;
    api->Arena_Allocator = mem::Arena_Allocator;
    api->Arena_allocate_proc = mem::Arena_allocate_proc;
    api->Arena_deallocate_all_proc = mem::Arena_deallocate_all_proc;

    api->Arena_deinit = mem::Arena_deinit;

    api->Arena_rewind = mem::Arena_rewind;
    
    api->Memory_Pool_Fixed_Allocator = mem::Memory_Pool_Fixed_Allocator;
    
    api->Memory_Pool_Fixed_init = mem::Memory_Pool_Fixed_init;
    
    api->Memory_Pool_Fixed_allocate_proc = mem::Memory_Pool_Fixed_allocate_proc;
    api->Memory_Pool_Fixed_deallocate_proc = mem::Memory_Pool_Fixed_deallocate_proc;

    api->Memory_Pool_Fixed_deinit = mem::Memory_Pool_Fixed_deinit;
    
    api->Pool_Allocation_init = mem::Pool_Allocation_init;
    
    api->Pool_Allocation_deinit = mem::Pool_Allocation_deinit;

    api->Fixed_Buckets_init = mem::Fixed_Buckets_init;
    api->Fixed_Buckets_Allocator_allocate_proc = mem::Fixed_Buckets_Allocator_allocate_proc;
    api->Fixed_Buckets_Allocator_deallocate_proc = mem::Fixed_Buckets_Allocator_deallocate_proc;
    api->Fixed_Buckets_Allocator_clear = mem::Fixed_Buckets_Allocator_clear;
    
    api->Buckets_Allocation_init = mem::Buckets_Allocation_init;
    api->Buckets_Allocation_deinit = mem::Buckets_Allocation_deinit;
    
    api->BLOCK_SIZE_LOOKUP = mem::BLOCK_SIZE_LOOKUP;
    
    api->string = mtt::string;
    api->string_w_len = mtt::string;
    api->string_free = mtt::string_free;
    api->string_get = mtt::string_get;
    api->string_ref_get = mtt::string_ref_get;
    
    api->selected_things = mtt::selected_things;
    api->selections = dt::selections;
    
    api->Particle_State_update = mtt::Particle_State_update;
    
    api->init_flag = mtt::init_flag;
    api->set_flag = mtt::set_flag;
    api->unset_flag = mtt::unset_flag;
    api->flag_is_set = mtt::flag_is_set;
    api->set_flag_w_state = mtt::set_flag;
    
    api->access_write_string = mtt::access_write_string;
    api->access_string = mtt::access_string;
    
    api->set_logic_proc_for_archetype = mtt::set_logic_proc_for_archetype;
    
    api->access = mtt::access;
    
    api->connect_parent_to_child_w_ptrs = mtt::connect_parent_to_child;
    api->connect_parent_to_child_w_ids = mtt::connect_parent_to_child;

    api->disconnect_child_from_parent_w_id = mtt::disconnect_child_from_parent;
    api->disconnect_child_from_parent_w_ptr = mtt::disconnect_child_from_parent;
    api->disconnect_parent_from_children = mtt::disconnect_parent_from_children;

    api->remove_thing_from_hierarchy = mtt::remove_thing_from_hierarchy;

    api->flip_left_right = mtt::flip_left_right;
    
    api->get_parent_ID = mtt::get_parent_ID;
    api->get_parent = mtt::get_parent;
    
    api->get_parent_with_archetype = mtt::get_parent_with_archetype;
    
    api->set_option = mtt::set_option;
    
    api->is_root_w_ptr = mtt::is_root;
    api->is_root_w_id = mtt::is_root;
    
    
    api->exists_in_other_hierarchy = mtt::exists_in_other_hierarchy;
    
    api->add_connection_w_ptrs = mtt::add_connection;
    api->add_connection_w_ids = mtt::add_connection;
    
    api->remove_all_connections = mtt::remove_all_connections;
    
    api->Connection_print = mtt::Connection_print;
    
    api->add_execution_connection = mtt::add_execution_connection;
    api->remove_execution_connection = mtt::remove_execution_connection;
    api->remove_incoming_execution_connections = mtt::remove_incoming_execution_connections;
    api->remove_outgoing_execution_connections = mtt::remove_outgoing_execution_connections;
    api->remove_all_execution_connections = mtt::remove_all_execution_connections;
    api->delete_execution_connections = mtt::delete_execution_connections;
    
    api->add_selector_w_ptr = mtt::add_selector;
    api->add_selector_w_id = mtt::add_selector;
    api->add_own_selector = mtt::add_own_selector;
    api->remove_own_selector = mtt::remove_own_selector;
    api->selector_invoke_w_arch = mtt::selector_invoke;
    api->selector_invoke = mtt::selector_invoke;
    
    
    api->thing_group_is_active = mtt::thing_group_is_active;
    api->thing_group_set_active_state_w_ptr = mtt::thing_group_set_active_state;
    api->thing_group_set_active_state_w_id = mtt::thing_group_set_active_state;
    
    api->input_should_cancel_animation = mtt::input_should_cancel_animation;
    
    api->is_ancestor_of = mtt::is_ancestor_of;
    
    api->should_be_immobile = mtt::should_be_immobile;
    
    
    api->get_in_port = mtt::get_in_port;
    api->get_out_port = mtt::get_out_port;
    
    api->Script_Lookup_get_var_with_key = mtt::Script_Lookup_get_var_with_key;
    api->Script_Lookup_set_var_group = mtt::Script_Lookup_set_var_group;
    api->Script_Lookup_get_var_group = mtt::Script_Lookup_get_var_group;
    api->Script_Lookup_set_var_with_key = mtt::Script_Lookup_set_var_with_key;
    api->Script_Lookup_set_key = mtt::Script_Lookup_set_key;
    api->Script_Lookup_get_var_with_key_w_script_instance = mtt::Script_Lookup_get_var_with_key;
    api->Script_Lookup_set_var_with_key_w_script_instance = mtt::Script_Lookup_set_var_with_key;
    
    api->DrawTalk_ctx = dt::ctx;
    
    api->MTT_Random = MTT_Random;
    api->MTT_Random_lower_bounded = MTT_Random_lower_bounded;
    api->MTT_Random_range = MTT_Random_range;
    api->MTT_Random_Float64_value = MTT_Random_Float64_value;
    api->MTT_Random_Float64_value_scaled = MTT_Random_Float64_value_scaled;
    api->MTT_Random_Float64_value_within_circle = MTT_Random_Float64_value_within_circle;
    
    api->MTT_set_active_logger = MTT_set_active_logger;
    api->MTT_active_logger = MTT_active_logger;
    api->MTT_set_default_active_logger = MTT_set_default_active_logger;
    api->MTT_Logger_init = MTT_Logger_init;
    api->MTT_Logger_destroy = MTT_Logger_destroy;
    api->debug_mode_log_enable = debug_mode_log_enable;
    api->debug_mode_log_is_enabled = debug_mode_log_is_enabled;
}

#endif




}

#endif /* dynamic_api_hpp */
