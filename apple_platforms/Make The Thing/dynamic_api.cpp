//
//  dynamic_stratadraw_api.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/1/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#include "dynamic_api.hpp"

#ifdef MTT_DYNAMIC_LIBRARY_CLIENT


namespace sd {

Renderer_API api;

void Drawable_Info_release(Renderer* r, Drawable_Info* info)
{
    api.Drawable_Info_release(r, info);
}

void begin_path(Renderer* renderer)
{
    api.begin_path(renderer);
}
Drawable_Info* end_path(Renderer* renderer)
{
    return api.end_path(renderer);
}

Drawable_Info* end_path_closed(Renderer* renderer)
{
    return api.end_path_closed(renderer);
}

void break_path(Renderer* renderer)
{
    api.break_path(renderer);
}

void begin_path_no_new_drawable(Renderer* renderer)
{
    api.begin_path_no_new_drawable(renderer);
}
void end_path_no_new_drawable(Renderer* renderer)
{
    api.end_path_no_new_drawable(renderer);
}

void contour(Renderer* renderer, std::vector<Boolean_Contour_Element> const & contour)
{
    api.contour(renderer, contour);
}

void begin_polygon(Renderer* renderer)
{
    api.begin_polygon(renderer);
}

Drawable_Info* end_polygon(Renderer* renderer)
{
    return api.end_polygon(renderer);
}

void begin_polygon_no_new_drawable(Renderer* renderer)
{
    api.begin_polygon_no_new_drawable(renderer);
}

void end_polygon_no_new_drawable(Renderer* renderer)
{
    api.end_polygon_no_new_drawable(renderer);
}

void begin_drawable(Renderer* renderer)
{
    api.begin_drawable(renderer);
}

Drawable_Info* end_drawable(Renderer* renderer)
{
    return api.end_drawable(renderer);
}

void set_color_rgba(Renderer* renderer, float32 r, float32 g, float32 b, float32 a)
{
    api.set_color_rgba(renderer, r, g, b, a);
}

void set_color_rgba_v4(Renderer* renderer, vec4 c)
{
    api.set_color_rgba_v4(renderer, c);
}

vec4 get_color_rgba(Renderer* renderer)
{
    return api.get_color_rgba(renderer);
}

void set_color_hsva(Renderer* renderer, float32 h, float32 s, float32 v, float32 a)
{
    return api.set_color_hsva(renderer, h, s, v, a);
}

void set_color_hsva_v4(Renderer* renderer, vec4 c)
{
    api.set_color_hsva_v4(renderer, c);
}


vec4 get_background_color(Renderer* renderer)
{
    return api.get_background_color(renderer);
}
void set_background_color_rgba(Renderer* renderer, float32 r, float32 g, float32 b, float32 a)
{
    api.set_background_color_rgba(renderer, r, g, b, a);
}

void set_background_color_rgba_v4(Renderer* renderer, vec4 c)
{
    api.set_background_color_rgba_v4(renderer, c);
}

void set_render_layer(Renderer* renderer, Render_Layer_ID render_layer)
{
    api.set_render_layer(renderer, render_layer);
}
Render_Layer_ID get_render_layer(Renderer* renderer)
{
    return api.get_render_layer(renderer);
}

void rewind_layer(Renderer* renderer, Render_Layer_ID render_layer)
{
    api.rewind_layer(renderer, render_layer);
}
void rewind_staging_layer(Renderer* renderer, Render_Layer* render_layer)
{
    api.rewind_staging_layer(renderer, render_layer);
}


void vertex(Renderer* renderer, float64 x, float64 y, float64 z)
{
    api.vertex(renderer, x, y, z);
}
void vertex_v2(Renderer* renderer, vec2 position)
{
    api.vertex_v2(renderer, position);
}
void vertex_v3(Renderer* renderer, vec3 position)
{
    api.vertex_v3(renderer, position);
}

void vertex(Renderer* renderer, float64 x, float64 y, float64 z, SD_vec2 uv, SD_vec4 color)
{
    api.vertex_color(renderer, x, y, z, uv, color);
}

void vertex(Renderer* renderer, float64 x, float64 y, float64 z, SD_vec2 uv)
{
    api.vertex_w_uv(renderer, x, y, z, uv);
}
void vertex_v2(Renderer* renderer, vec2 position, SD_vec2 uv)
{
    api.vertex_v2_w_uv(renderer, position, uv);
}
void vertex_v3(Renderer* renderer, vec3 position, SD_vec2 uv)
{
    api.vertex_v3_w_uv(renderer, position, uv);
}


void path_vertex(Renderer* renderer, float64 x, float64 y, float64 z)
{
    api.path_vertex(renderer, x, y, z);
}
void path_vertex_v2(Renderer* renderer, vec2 position)
{
    api.path_vertex_v2(renderer, position);
}
void path_vertex_v3(Renderer* renderer, vec3 position)
{
    api.path_vertex_v3(renderer, position);
}
void path_list(Renderer* renderer, vec3* list, usize count)
{
    api.path_list(renderer, list, count);
}
void path_list_with_offset(Renderer* renderer, vec3* list, usize count, vec3 offset)
{
    api.path_list_with_offset(renderer, list, count, offset);
}
void path_list(Renderer* renderer, vec3* list, usize count, float64* radii)
{
    api.path_list_with_radii(renderer, list, count, radii);
}
void path_list_with_offset(Renderer* renderer, vec3* list, usize count, vec3 offset, float64* radii)
{
    api.path_list_with_offset_with_radii(renderer, list, count, offset, radii);
}
void path_radius(Renderer* renderer, float32 pixel_radius)
{
    api.path_radius(renderer, pixel_radius);
}
void set_path_radius(Renderer* renderer, float32 pixel_radius)
{
    api.set_path_radius(renderer, pixel_radius);
}
float32 get_path_radius(Renderer* renderer)
{
    return api.get_path_radius(renderer);
}
void path_arrow_head(Renderer* renderer, vec3 src_point, vec3 dst_point, float32 radius)
{
    api.path_arrow_head(renderer, src_point, dst_point, radius);
}
void path_arrow_head_with_tail(Renderer* renderer, vec3 src_point, vec3 dst_point, float32 radius)
{
    api.path_arrow_head_with_tail(renderer, src_point, dst_point, radius);
}

void quad_color(Renderer* renderer, vec2 tl, vec2 bl, vec2 br, vec2 tr, float32 depth, vec4 ctl, vec4 cbl, vec4 cbr, vec4 ctr)
{
    api.quad_color(renderer, tl, bl, br, tr, depth, ctl, cbl, cbr, ctr);
}

void quad_color_uv(Renderer* renderer, SD_vec2 tl, SD_vec2 bl, SD_vec2 br, SD_vec2 tr, float32 depth, SD_vec4 ctl, SD_vec4 cbl, SD_vec4 cbr, SD_vec4 ctr, SD_vec2 uv_tl, SD_vec2 uv_bl, SD_vec2 uv_br, SD_vec2 uv_tr)
{
    api.quad_color_uv(renderer, tl, bl, br, tr, depth, ctl, cbl, cbr, ctr, uv_tl, uv_bl, uv_br, uv_tr);
}

void triangle(Renderer* renderer, vec3 a, vec3 b, vec3 c)
{
    api.triangle(renderer, a, b, c);
}
void triangle_color(Renderer* renderer, vec3 a, vec3 b, vec3 c, vec4 color)
{
    api.triangle_color(renderer, a, b, c, color);
}
void triangle_per_vertex_color(Renderer* renderer, vec3 a, vec3 b, vec3 c, vec4 color_a, vec4 color_b, vec4 color_c)
{
    api.triangle_per_vertex_color(renderer, a, b, c, color_a, color_b, color_c);
}

void triangle_equilateral(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side)
{
    api.triangle_equilateral(renderer, origin, height_sign, side);
}
void triangle_equilateral_color(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side, vec4 color)
{
    api.triangle_equilateral_color(renderer, origin, height_sign, side, color);
}
void triangle_equilateral_per_vertex_color(Renderer* renderer, vec3 origin, sint32 height_sign, float32 side, vec4 color_height, vec4 color_left, vec4 color_right)
{
    api.triangle_equilateral_per_vertex_color(renderer, origin, height_sign, side, color_height, color_left, color_right);
}

void disable_command(Renderer* r, Drawable_Info* info)
{
    api.disable_command(r, info);
}
void enable_command(Renderer* r, Drawable_Info* info)
{
    api.enable_command(r, info);
}
void polygon_convex_regular(Renderer* r, float32 radius, vec3 center, usize sides)
{
    api.polygon_convex_regular(r, radius, center, sides);
}
void polygon_convex_regular_v2(Renderer* r, float32 radius, vec2 center, usize sides)
{
    api.polygon_convex_regular_v2(r, radius, center, sides);
}
void polygon_arc_fraction(Renderer* renderer, float32 radius, vec3 center, usize count_sides, float32 arc_fraction)
{
    api.polygon_arc_fraction(renderer, radius, center, count_sides, arc_fraction);
}
void circle(Renderer* r, float32 radius, vec3 center)
{
    api.circle(r, radius, center);
}
void circle(Renderer* r, float32 radius, vec3 center, float32 sides)
{
    api.circle_with_sides(r, radius, center, sides);
}
void quad(Renderer* renderer, vec2 tl, vec2 bl, vec2 br, vec2 tr, float32 depth)
{
    api.quad(renderer, tl, bl, br, tr, depth);
}
void path_quad(Renderer* r, vec2 tl, vec2 bl, vec2 br, vec2 tr, float32 depth)
{
    api.path_quad(r, tl, bl, br, tr, depth);
}
void rectangle(Renderer* renderer, vec2 tl, vec2 dimensions, float32 depth)
{
    api.rectangle(renderer, tl, dimensions, depth);
}
void rectangle_w_corners(Renderer* renderer, vec2 tl, vec2 br, float32 depth)
{
    api.rectangle_w_corners(renderer, tl, br, depth);
}

void rectangle_rounded(Renderer* renderer, vec2 tl, vec2 dimensions, float32 depth, float32 roundness, int32 segments)
{
    api.rectangle_rounded(renderer, tl, dimensions, depth, roundness, segments);
}

void rectangle_dashed(Renderer* renderer, vec2 tl, vec2 dimensions, float32 depth, usize segments)
{
    api.rectangle_dashed(renderer, tl, dimensions, depth, segments);
}

void set_depth(Renderer* r, float64 depth)
{
    api.set_depth(r, depth);
}

void Renderer_init(Renderer* renderer, mem::Allocator* allocator)
{
    api.Renderer_init(renderer, allocator);
}


Render_Layer_ID Render_Layer_make(Renderer* renderer, usize block_count)
{
    return api.Render_Layer_make(renderer, block_count);
}

bool Render_Layer_reallocate_bytes(Renderer* renderer, usize block_count, Render_Layer_ID rlid)
{
    return api.Render_Layer_reallocate_bytes(renderer, block_count, rlid);
}
bool Render_Layer_reallocate_vertex_bytes(Renderer* renderer, usize block_count, Render_Layer_ID rlid)
{
    return api.Render_Layer_reallocate_vertex_bytes(renderer, block_count, rlid);
}
bool Render_Layer_reallocate_index_bytes(Renderer* renderer, usize block_count, Render_Layer_ID rlid)
{
    return api.Render_Layer_reallocate_index_bytes(renderer, block_count, rlid);
}
void path_bezier_arrow_head_with_tail(Renderer* renderer, vec3 src_point, vec3 p_mid_anchor1, vec3 p_mid_anchor2, vec3 dst_point, float32 radius, usize segment_count)
{
    api.path_bezier_arrow_head_with_tail(renderer, src_point, p_mid_anchor1, p_mid_anchor2, dst_point, radius, segment_count);
}


void path_cross(Renderer* renderer, const vec3 center, vec3 offset)
{
    api.path_cross(renderer, center, offset);
}

void continue_path(Renderer* renderer)
{
    api.continue_path(renderer);
}

void triangulate(Renderer* renderer, sd::Point2D_List* solution)
{
    api.triangulate(renderer, solution);
}

vec4 view_offset_for_safe_area(sd::Renderer* renderer)
{
    return api.view_offset_for_safe_area(renderer);
}


void set_view(Renderer* r, mat4 v)
{
    api.set_view(r, v);
}
Mat4 get_view(Renderer* r)
{
    return api.get_view(r);
}

void push_projection_view_transform(Renderer* r, mat4 a, mat4 b)
{
    api.push_projection_view_transform(r, a, b);
}
void pop_view_transform(Renderer* r)
{
    api.pop_view_transform(r);
}

void push_draw_command_with_layer(Renderer* r, Render_Layer_ID l)
{
    api.push_draw_command_with_layer(r, l);
}
void push_draw_command_with_staging_layer(Renderer* r, Render_Layer* l)
{
    api.push_draw_command_with_staging_layer(r, l);
}

void push_draw_command_with_info_layer_and_buffer_layer(Renderer* renderer, Render_Layer_ID render_layer_w_drawable_info, Render_Layer_ID render_layer_w_buffer)
{
    api.push_draw_command_with_info_layer_and_buffer_layer(renderer, render_layer_w_drawable_info, render_layer_w_buffer);
}

void push_instanced_draw_command_with_layer(Renderer* renderer, Render_Layer_ID render_layer, Drawable_Instance_Data* instance_data)
{
    api.push_instanced_draw_command_with_layer(renderer, render_layer, instance_data);
}
//void push_instanced_draw_command_with_drawable(Renderer* renderer, Drawable_Info* drawable, Drawable_Info* list, usize count)
//{
//    api.push_instanced_draw_command_with_drawable(renderer, drawable, list, count);
//}

void push_instanced_draw_command_with_drawable_list(Renderer* renderer, Drawable_Info* drawable, MTT_List* list)
{
    api.push_instanced_draw_command_with_drawable_list(renderer, drawable, list);
}

void push_instanced_draw_command_with_drawable_list(Renderer* renderer, Drawable_Info* drawable, mtt::Array_Slice<sd::Drawable_Info> list)
{
    api.push_instanced_draw_command_with_array_slice(renderer, drawable, list);
}

//void push_instanced_draw_command_lists_with_drawable(Renderer* renderer, Drawable_Info* drawable, Drawable_Info_List* lists, usize count)
//{
//    api.push_instanced_draw_command_lists_with_drawable(renderer, drawable, lists, count);
//}


void push_draw_command_with_layer_range(Renderer* renderer, Render_Layer_ID render_layer, Render_Layer_Range layer_range)
{
    api.push_draw_command_with_layer_range(renderer, render_layer, layer_range);
}

void push_instanced_draw_command_with_staging_layer(Renderer* renderer, Render_Layer* staging_layer, Drawable_Instance_Data* instance_data)
{
    api.push_instanced_draw_command_with_staging_layer(renderer, staging_layer, instance_data);
}

void push_draw_command_with_drawable_info(Renderer* renderer, Drawable_Info* drawable_info)
{
    api.push_draw_command_with_drawable_info(renderer, drawable_info);
}

void push_pipeline(Renderer* r, uint64 id)
{
    api.push_pipeline(r, id);
}
void push_color_pipeline(Renderer* r)
{
    api.push_color_pipeline(r);
}
void push_color_hsv_pipeline(Renderer* renderer)
{
    api.push_color_hsv_pipeline(renderer);
}
void push_texture_pipeline(Renderer* r)
{
    api.push_texture_pipeline(r);
}
void push_text_pipeline(Renderer* r)
{
    api.push_text_pipeline(r);
}

void push_scissor_rect(Renderer* r, Scissor_Rect rect)
{
    api.push_scissor_rect(r, rect);
}
void push_viewport(Renderer* renderer, Viewport viewport)
{
    api.push_viewport(renderer, viewport);
}

void push_custom_command(Renderer* r, Custom_Command custom)
{
    api.push_custom_command(r, custom);
}

void set_render_pipeline(Renderer* renderer, uint64 ID)
{
    api.set_render_pipeline(renderer, ID);
}
void render_to_texture_sync_begin(Renderer* renderer, sd::Rectangle bounds)
{
    api.render_to_texture_sync_begin(renderer, bounds);
}
void render_to_texture_end(Renderer* renderer, void (*callback)(bool, void*))
{
    api.render_to_texture_end(renderer, callback);
}

void set_depth_write_mode(Renderer* r, sd::DEPTH_WRITE_MODE mode)
{
    api.set_depth_write_mode(r, mode);
}

void Command_List_begin(Renderer* r, Command_List* l)
{
    api.Command_List_begin(r, l);
}
void Command_List_end(Renderer* r, Command_List* l)
{
    api.Command_List_end(r, l);
}

void Command_List_submit(Renderer* r, Command_List* l)
{
    api.Command_List_submit(r, l);
}

void clear_commands(Renderer* renderer)
{
    api.clear_commands(renderer);
}

//void print_commands(Renderer*);

Drawable_Info* pause_path(Renderer* renderer)
{
    return api.pause_path(renderer);
}

sd::Texture_ID texture_id_from_name(sd::Renderer* renderer, const mtt::String& name)
{
    return api.texture_id_from_name(renderer, name);
}

bool texture_is_valid(sd::Renderer* renderer, sd::Texture_ID texture_id)
{
    return api.texture_is_valid(renderer, texture_id);
}


Render_Layer_ID layer_id(Render_Layer* layer)
{
    return api.layer_id(layer);
}

void save(Renderer* renderer)
{
    api.save(renderer);
}
void restore(Renderer* renderer)
{
    api.restore(renderer);
}

Drawable_Info* Drawable_Info_copy(Renderer* renderer, Render_Layer_ID ID, Drawable_Info* info)
{
    return api.Drawable_Info_copy(renderer, ID, info);
}


Rectangle get_display_bounds(void)
{
    return api.get_display_bounds();
}
Rectangle get_native_display_bounds(void)
{
    return api.get_native_display_bounds();
}
float64 get_native_display_scale(void)
{
    return api.get_native_display_scale();
}


sd::Images* images(Renderer* renderer)
{
    return api.images(renderer);
}

bool is_see_through_background(sd::Renderer* r)
{
    return api.is_see_through_background(r);
}
void set_see_through_background(sd::Renderer* r, bool state)
{
    api.set_see_through_background(r, state);
}

void Drawable_Info_print(Drawable_Info* rc)
{
    api.Drawable_Info_print(rc);
}

void set_transform(Drawable_Info* d_info, const mat4& v)
{
    api.set_transform(d_info, v);
}
void set_ptr_transform(Drawable_Info* d_info, const SD_mat4* v)
{
    api.set_ptr_transform(d_info, v);
}
SD_mat4 get_transform(Drawable_Info* d_info)
{
    return api.get_transform(d_info);
}
SD_mat4* get_ptr_transform(Drawable_Info* d_info)
{
    return api.get_ptr_transform(d_info);
}

void Images_init(Renderer* renderer, Images* images, usize max_resident, const Images_Init_Args& args)
{
    api.Images_init(renderer, images, max_resident, args);
}

void images_pick_async(Renderer* renderer, Image_Loader* loader, Image_Pick_Descriptor* desc, bool (*callback)(Image_Load_Result))
{
    api.images_pick_async(renderer, loader, desc, callback);
}

bool images_load_async(Renderer* renderer, Image_Load_Descriptor* desc, bool (*callback)(const Image_Load_Result& result, bool status))
{
    return api.images_load_async(renderer, desc, callback);
}

bool images_load_remote_async(sd::Renderer* renderer, Image_Load_Descriptor* desc, bool (*callback)(const Image_Load_Result& result, bool status))
{
    return api.images_load_remote_async(renderer, desc, callback);
}

bool Images_lookup(sd::Images* images, cstring name, Image** out_img)
{
    return api.Images_lookup(images, name, out_img);
}

sd::Image* Image_create_empty(sd::Images* imgs, const mtt::String& name, const sd::Image_Create_Args& args)
{
    return api.Image_create_empty(imgs, name, args);
}

sd::Image* Image_create_empty_w_size(sd::Images* imgs, const mtt::String& name, uvec2 size, const sd::Image_Create_Args& args, Texture_Handle* out_handle)
{
    return api.Image_create_empty_w_size(imgs, name, size, args, out_handle);
}

void Image_create_from_baked(Images* imgs, cstring name, void* texture, void (*callback)(Image* image))
{
    api.Image_create_from_baked(imgs, name, texture, callback);
}

void Image_save(Images* images, Image* img, const Image_Save_Descriptor& args)
{
    api.Image_save(images, img, args);
}

void Images_for_each(sd::Images* images, void (callback)(sd::Image* img))
{
    api.Images_for_each(images, callback);
}


#ifdef __clang__
void set_transform_intrin(Drawable_Info* d_info, const simd_float4x4& v)
{
    api.set_transform_intrin(d_info, v);
}
simd_float4x4 get_transform_intrin(Drawable_Info* d_info)
{
    return api.get_transform_intrin(d_info);
}
void set_ptr_transform_intrin(Drawable_Info* d_info, const simd_float4x4* v)
{
    api.set_ptr_transform_intrin(d_info, v);
}
#endif

void render_pass_begin(Renderer* r, const Render_Pass_Descriptor& conf)
{
    return api.render_pass_begin(r, conf);
}
void render_pass_begin(Renderer* r)
{
    return api.render_pass_begin_default(r);
}
void render_pass_end(Renderer* r)
{
    return api.render_pass_end(r);
}

void compute_pass_begin(Renderer* r, const Compute_Pass_Descriptor& conf)
{
    api.compute_pass_begin(r, conf);
}
void compute_pass_end(Renderer* r)
{
    api.compute_pass_end(r);
}
void compute_pass_end(Renderer* r, const Compute_Pass_End_Descriptor& desc)
{
    api.compute_pass_end_w_descriptor(r, desc);
}

void dispatch_threadgroups(sd::Renderer* r, vec3 threadgroups_per_grid, vec3 threads_per_threadgroup)
{
    api.dispatch_threadgroups(r, threadgroups_per_grid, threads_per_threadgroup);
}
void dispatch_threads(sd::Renderer* r, vec3 threads_per_grid, vec3 threads_per_threadgroup)
{
    api.dispatch_threads(r, threads_per_grid, threads_per_threadgroup);
}

void* Buffer_contents(sd::Buffer* b)
{
    return api.Buffer_contents(b);
}

usize Buffer_byte_size(sd::Buffer* b)
{
    return api.Buffer_byte_size(b);
}

bool Buffer_is_valid(sd::Buffer* b)
{
    return api.Buffer_is_valid(b);
}

bool Buffer_make(Renderer* renderer, Buffer* buffer, usize byte_count)
{
    return api.Buffer_make(renderer, buffer, byte_count);
}

void Buffer_destroy(Renderer* renderer, Buffer* buffer)
{
    api.Buffer_destroy(renderer, buffer);
}

MTT_NODISCARD sd::Shader_ID rasterization_shader_make_with_builtin_vertex_function(sd::Renderer* renderer, sd::Shader_ID vertex_shader_function_id, cstring fragment_function_src, cstring fragment_function_name)
{
    return api.rasterization_shader_make_with_builtin_vertex_function(renderer, vertex_shader_function_id, fragment_function_src, fragment_function_name);
}
sd::Shader_ID rasterization_shader_replace_with_builtin_vertex_function(sd::Renderer* renderer, sd::Shader_ID vertex_shader_function_id, cstring fragment_function_src, cstring fragment_function_name, Shader_ID shader_id)
{
    return api.rasterization_shader_replace_with_builtin_vertex_function(renderer, vertex_shader_function_id, fragment_function_src, fragment_function_name, shader_id);
}
void rasterization_shader_destroy(sd::Renderer* renderer, Shader_ID shader_id)
{
    api.rasterization_shader_destroy(renderer, shader_id);
}

MTT_NODISCARD sd::Shader_ID compute_shader_make(sd::Renderer* renderer, cstring cstr, cstring main_name)
{
    return api.compute_shader_make(renderer, cstr, main_name);
}

void compute_shader_destroy(sd::Renderer* renderer, Shader_ID shader_id)
{
    api.compute_shader_destroy(renderer, shader_id);
}
sd::Shader_ID compute_shader_replace(sd::Renderer* renderer, cstring src_str, cstring main_name, sd::Shader_ID shader_id)
{
    return api.compute_shader_replace(renderer, src_str, main_name, shader_id);
}

void vertex_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset)
{
    api.vertex_buffer_set_w_offset(r, buffer, slot, byte_offset);
}
void vertex_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot)
{
    api.vertex_buffer_set(r, buffer, slot);
}
void fragment_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset)
{
    api.fragment_buffer_set_w_offset(r, buffer, slot, byte_offset);
}
void fragment_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot)
{
    api.fragment_buffer_set(r, buffer, slot);
}
void compute_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot, usize byte_offset)
{
    api.compute_buffer_set_w_offset(r, buffer, slot, byte_offset);
}
void compute_buffer_set(sd::Renderer* r, Buffer* buffer, uint64 slot)
{
    api.compute_buffer_set(r, buffer, slot);
}


bool compute_shader_info(sd::Renderer* r, sd::Shader_ID shader_id, Compute_Shader_Info* info)
{
    return api.compute_shader_info(r, shader_id, info);
}

void compute_shader_set(sd::Renderer* r, sd::Shader_ID shader_id)
{
    api.compute_shader_set(r, shader_id);
}

SD_vec2 depth_range_default(sd::Renderer* renderer)
{
    return api.depth_range_default(renderer);
}

Blur Blur_make(sd::Renderer* renderer, float32 radius)
{
    return api.Blur_make(renderer, radius);
}
void Blur_destroy(sd::Renderer* renderer, Blur* blur)
{
    api.Blur_destroy(renderer, blur);
}
void compute_set_blur(sd::Renderer* renderer, Blur* blur, Texture_Handle* texture_handle, bool should_wait)
{
    api.compute_set_blur(renderer, blur, texture_handle, should_wait);
}
void compute_set_blur(sd::Renderer* renderer, float32 radius, Texture_Handle* texture_handle, bool should_wait)
{
    api.compute_set_blur_once(renderer, radius, texture_handle, should_wait);
}

void compute_set_sobel(sd::Renderer* renderer, Texture_Handle* texture_handle_dst, Texture_Handle* texture_handle_src)
{
    api.compute_set_sobel(renderer, texture_handle_dst, texture_handle_src);
}

bool Render_Target_make_msaa(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out)
{
    return api.Render_Target_make_msaa(renderer, name, size, flags, rt_out);
}
bool Render_Target_make_no_depth_stencil_msaa(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out)
{
    return api.Render_Target_make_no_depth_stencil_msaa(renderer, name, size, flags, rt_out);
}

bool Render_Target_make(sd::Renderer* renderer, const mtt::String& name, uvec2 size,  uint64 flags, Render_Target* rt_out)
{
    return api.Render_Target_make(renderer, name, size, flags, rt_out);
}
bool Render_Target_make_no_depth_stencil(sd::Renderer* renderer, const mtt::String& name, uvec2 size, uint64 flags, Render_Target* rt_out)
{
    return api.Render_Target_make_no_depth_stencil(renderer, name, size, flags, rt_out);
}

void set_stencil_reference_value(Renderer* renderer, uint32 reference_value)
{
    api.set_stencil_reference_value(renderer, reference_value);
}

mtt::Array_Slice<sd::Index_Type> index_list(Renderer* renderer, sd::Drawable_Info* info, usize size)
{
    return api.index_list(renderer, info, size);
}
void triangle_strip_to_indexed_triangles_in_place(Renderer* renderer, sd::Drawable_Info* info)
{
    api.triangle_strip_to_indexed_triangles_in_place(renderer, info);
}
void set_triangle_indices(sd::Renderer* renderer, mtt::Array_Slice<sd::Index_Type>& index_list, usize idx, Index_Type a, Index_Type b, Index_Type c)
{
    api.set_triangle_indices(renderer, index_list, idx, a, b, c);
}

usize max_texture_dimension_2(sd::Renderer* renderer)
{
    return api.max_texture_dimension_2(renderer);
}

void Image_Atlas_init(Renderer* renderer, Image_Atlas* atlas, usize max_image_dimension_size, uvec2 dimensions)
{
    api.Image_Atlas_init(renderer, atlas, max_image_dimension_size, dimensions);
}

bool Image_Atlas_image_reserve_rectangle(Image_Atlas* atlas, vec2 required_space, sd::Image_Atlas::Region* reserved_space)
{
    return api.Image_Atlas_image_reserve_rectangle(atlas, required_space, reserved_space);
}

void Image_Atlas_clear(Image_Atlas* atlas)
{
    api.Image_Atlas_clear(atlas);
}

usize Image_Atlas_image_count(Image_Atlas* atlas)
{
    return api.Image_Atlas_image_count(atlas);
}

}

void HSVtoRGB(int H, double S, double V, float64 output[3])
{
    sd::api.HSVtoRGB(H, S, V, output);
}

namespace sd {


void Renderer_API_set(sd::Renderer_API* api_in)
{
    api = *api_in;
}


}


namespace mtt {

MTT_API api;

mtt::World* ctx()
{
    return api.MTT_ctx();
}


mtt::Thing_ID Thing_make(mtt::World* world, mtt::Thing_Archetype_ID arch_id, mtt::Thing** out, const mtt::Thing_Make_Args& args)
{
    return api.Thing_make(world, arch_id, out, args);
}
mtt::Thing* Thing_try_get(mtt::World* world, mtt::Thing_ID id)
{
    return api.Thing_try_get(world, id);
}
void Thing_destroy(mtt::Thing* thing)
{
    api.Thing_destroy(thing);
}

mtt::Thing* Thing_copy(mtt::Thing* src)
{
    return api.Thing_copy(src);
}

mtt::Thing_ID get_thing_most_recently_selected_with_touch(mtt::World* world)
{
    return api.get_thing_most_recently_selected_with_touch(world);
}
mtt::Thing* get_thing_ptr_most_recently_selected_with_touch(mtt::World* world)
{
    mtt::Thing_ID id = api.get_thing_most_recently_selected_with_touch(world);
    return mtt::Thing_try_get(world, id);
}

void Thing_set_position(mtt::Thing* thing, vec3 position)
{
    api.Thing_set_position_w_ptr(thing, position);
}

mtt::Thing* Thing_set_position(mtt::World* world, mtt::Thing_ID id, vec3 position)
{
    return api.Thing_set_position_w_id(world, id, position);
}

void set_pose_transform(mtt::Thing* thing, const Mat4* mat)
{
    api.set_pose_transform(thing, mat);
}
void set_pose_transform(mtt::Thing* thing, const Mat4& mat)
{
    api.set_pose_transform_w_ref(thing, mat);
}

Thing* Thing_make_with_collider(World* world, mtt::Rep** rep, mtt::ARCHETYPE arch, mtt::COLLIDER_TYPE collider_type, bool is_in_world, mtt::Collision_System* collision_system)
{
    return api.Thing_make_with_collider(world, rep, arch, collider_type, is_in_world, collision_system);
}

mtt::Thing* Thing_make_with_aabb_corners(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, mtt::Box box, float32 z_position, bool is_in_world, mtt::Collision_System* collision_system)
{
    return api.Thing_make_with_aabb_corners(world, arch, scale_2d, rep_out, box, z_position, is_in_world, collision_system);
}

mtt::Thing* Thing_make_with_aabb_dimensions(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, vec3 in_position, vec2 dimensions, bool is_in_world, mtt::Collision_System* collision_system)
{
    return api.Thing_make_with_aabb_dimensions(world, arch, scale_2d, rep_out, in_position, dimensions, is_in_world, collision_system);
}

mtt::Thing* Thing_make_with_unit_collider(mtt::World* world, mtt::Thing_Archetype_ID arch, vec2 scale_2d, mtt::Rep** rep_out, mtt::COLLIDER_TYPE collider_type, vec3 in_position, bool is_in_world, mtt::Collision_System* collision_system)
{
    return api.Thing_make_with_unit_collider(world, arch, scale_2d, rep_out, collider_type, in_position, is_in_world, collision_system);
}


void push_AABB(Collision_System* sys, Collider* c)
{
    api.push_AABB(sys, c);
}
void push_AABBs(Collision_System* sys, Collider* boxes, usize count, usize layer)
{
    api.push_AABBs(sys, boxes, count, layer);
}
void Collider_remove(Collision_System* sys, usize layer, Collider* c)
{
    api.Collider_remove(sys, layer, c);
}
Collider* Collider_make_aabb(Collision_System* sys)
{
    return api.Collider_make_aabb(sys);
}

MTT_NODISCARD MTT_String_Ref string(cstring str)
{
    return api.string(str);
}
MTT_NODISCARD MTT_String_Ref string(cstring str, MTT_String_Length length)
{
    return api.string_w_len(str, length);
}
bool string_free(MTT_String_Ref& str)
{
    return api.string_free(str);
}
MTT_NODISCARD cstring string_get(MTT_String_Ref& str)
{
    return api.string_get(str);
}

}

//MTT_String_Ref::operator cstring()
//{
//    return mtt::api.string_get(*this);
//}


namespace mtt {

MTT_NODISCARD MTT_String_Ref string_ref_get(cstring str)
{
    return api.string_ref_get(str);
}


void* selected_things(mtt::World* world)
{
    return api.selected_things(world);
}

void Particle_State_update(Particle_System_State* p)
{
    api.Particle_State_update(p);
}

void init_flag(THING_FLAG* src)
{
    api.init_flag(src);
}
void set_flag(THING_FLAG* src, THING_FLAG flag)
{
    api.set_flag(src, flag);
}
void unset_flag(THING_FLAG* src, THING_FLAG flag)
{
    api.unset_flag(src, flag);
}
bool flag_is_set(THING_FLAG* src, THING_FLAG flag)
{
    return api.flag_is_set(src, flag);
}
void set_flag(THING_FLAG* src, THING_FLAG flag, bool state)
{
    api.set_flag_w_state(src, flag, state);
}

void access_write_string(mtt::Thing* thing, const mtt::String& name, const mtt::String& set_to)
{
    api.access_write_string(thing, name, set_to);
}
cstring access_string(mtt::Thing* thing, const mtt::String& name)
{
    return api.access_string(thing, name);
}

void set_logic_proc_for_archetype(mtt::World* world, mtt::Thing_Archetype_ID id, mtt::Logic_Procedure proc)
{
    api.set_logic_proc_for_archetype(world, id, proc);
}

void* access(Thing* thing, const String& tag, MTT_TYPE type)
{
    return api.access(thing, tag, type);
}

void connect_parent_to_child(mtt::World* world, mtt::Thing* parent, mtt::Thing* child)
{
    api.connect_parent_to_child_w_ptrs(world, parent, child);
}
void connect_parent_to_child(mtt::World* world, mtt::Thing_ID parent, mtt::Thing_ID child)
{
    api.connect_parent_to_child_w_ids(world, parent, child);
}

void disconnect_child_from_parent(mtt::World* world, mtt::Thing* child_thing)
{
    api.disconnect_child_from_parent_w_ptr(world, child_thing);
}
void disconnect_child_from_parent(mtt::World* world, mtt::Thing_ID child)
{
    api.disconnect_child_from_parent_w_id(world, child);
}
void disconnect_parent_from_children(mtt::World* world, mtt::Thing* thing)
{
    api.disconnect_parent_from_children(world, thing);
}

void remove_thing_from_hierarchy(mtt::World* world, mtt::Thing* thing)
{
    api.remove_thing_from_hierarchy(world, thing);
}

void flip_left_right(mtt::Thing* thing)
{
    api.flip_left_right(thing);
}

mtt::Thing_ID get_parent_ID(mtt::Thing* thing)
{
    return api.get_parent_ID(thing);
}
mtt::Thing* get_parent(mtt::Thing* thing)
{
    return api.get_parent(thing);
}
mtt::Thing* get_parent_with_archetype(mtt::Thing* thing, mtt::Thing_Archetype_ID arch)
{
    return api.get_parent_with_archetype(thing, arch);
}
void set_option(Thing* thing, uint64 option, uint64 flags)
{
    api.set_option(thing, option, flags);
}

bool is_root(mtt::Thing* thing)
{
    return api.is_root_w_ptr(thing);
}
bool is_root(mtt::World* world, mtt::Thing_ID thing_id)
{
    return api.is_root_w_id(world, thing_id);
}

bool exists_in_other_hierarchy(mtt::Thing* thing, mtt::Thing* other)
{
    return api.exists_in_other_hierarchy(thing, other);
}

bool add_connection(World* world, Thing_ID src, const String& src_tag, Thing_ID dst, const String& dst_tag, Thing** src_thing_out, Thing** dst_thing_out)
{
    return api.add_connection_w_ids(world, src, src_tag, dst, dst_tag, src_thing_out, dst_thing_out);
}

bool add_connection(World* world, Thing* src, const String& src_tag, Thing* dst, const String& dst_tag)
{
    return api.add_connection_w_ptrs(world, src, src_tag, dst, dst_tag);
}

bool remove_all_connections(World* world, Thing_ID thing_id)
{
    return api.remove_all_connections(world, thing_id);
}

void Connection_print(Connection* connection, World* world);

void add_execution_connection(mtt::Thing* thing_src, mtt::Thing* thing_dst)
{
    api.add_execution_connection(thing_src, thing_dst);
}
void remove_execution_connection(mtt::Thing* thing_src, mtt::Thing* thing_dst)
{
    api.remove_execution_connection(thing_src, thing_dst);
}
void remove_incoming_execution_connections(mtt::Thing* thing)
{
    api.remove_incoming_execution_connections(thing);
}
void remove_outgoing_execution_connections(mtt::Thing* thing)
{
    api.remove_outgoing_execution_connections(thing);
}
void remove_all_execution_connections(mtt::Thing* thing)
{
    api.remove_all_execution_connections(thing);
}
void delete_execution_connections(mtt::Thing* thing)
{
    api.delete_execution_connections(thing);
}


void add_selector(mtt::Thing_Archetype* arch, cstring name, const mtt::Procedure& proc)
{
    api.add_selector_w_ptr(arch, name, proc);
}
bool add_selector(mtt::World* world, mtt::Thing_Archetype_ID id, cstring name, const mtt::Procedure& proc)
{
    return api.add_selector_w_id(world, id, name, proc);
}
void add_own_selector(mtt::Thing* thing, cstring name, const mtt::Procedure& proc)
{
    api.add_own_selector(thing, name, proc);
}

void remove_own_selector(mtt::Thing* thing, cstring name)
{
    api.remove_own_selector(thing, name);
}

Procedure_Return_Type selector_invoke(mtt::Thing_Archetype* arch, mtt::Thing* thing, Message* message)
{
    return api.selector_invoke_w_arch(arch, thing, message);
}
Procedure_Return_Type selector_invoke(mtt::Thing* thing, Message* message)
{
    return api.selector_invoke(thing, message);
}

bool thing_group_is_active(mtt::Thing* const thing)
{
    return api.thing_group_is_active(thing);
}
bool thing_group_set_active_state(mtt::Thing* const thing, bool active_state)
{
    return api.thing_group_set_active_state_w_ptr(thing, active_state);
}
bool thing_group_set_active_state(mtt::World* world, mtt::Thing_ID const thing_id, bool active_state)
{
    return api.thing_group_set_active_state_w_id(world, thing_id, active_state);
}


bool input_should_cancel_animation(mtt::Thing* thing)
{
    return api.input_should_cancel_animation(thing);
}

bool is_ancestor_of(mtt::Thing* possible_parent, mtt::Thing* thing)
{
    return api.is_ancestor_of(possible_parent, thing);
}

bool should_be_immobile(mtt::Thing* thing)
{
    return api.should_be_immobile(thing);
}

Result<Evaluation_Output_Entry_Port&> get_in_port(Thing* thing, Port_Input_List* input_list, const String& tag)
{
    return api.get_in_port(thing, input_list, tag);
}
Result<Evaluation_Output_Entry_Port&> get_out_port(Thing* thing, const String& tag)
{
    return api.get_out_port(thing, tag);
}

MTT_NODISCARD
Script_Property_List* Script_Lookup_get_var_with_key(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var)
{
    return api.Script_Lookup_get_var_with_key(lookup, key_ctx, key_var);
}
void Script_Lookup_set_var_group(Script_Lookup& lookup, const mtt::String& key_ctx, Script_Property_Lookup& var_group)
{
    api.Script_Lookup_set_var_group(lookup, key_ctx, var_group);
}
MTT_NODISCARD
Script_Property_Lookup* Script_Lookup_get_var_group(Script_Lookup& lookup, const mtt::String& key_ctx)
{
    return api.Script_Lookup_get_var_group(lookup, key_ctx);
}
void Script_Lookup_set_var_with_key(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var, const Script_Property_List& val)
{
    api.Script_Lookup_set_var_with_key(lookup, key_ctx, key_var, val);
}
void Script_Lookup_set_key(Script_Lookup& lookup, const mtt::String& key_ctx, const mtt::String& key_var)
{
    api.Script_Lookup_set_key(lookup, key_ctx, key_var);
}
MTT_NODISCARD
Script_Property_List* Script_Lookup_get_var_with_key(Script_Instance* s, const mtt::String& key_ctx, const mtt::String& key_var)
{
    return api.Script_Lookup_get_var_with_key_w_script_instance(s, key_ctx, key_var);
}
void Script_Lookup_set_var_with_key(Script_Instance* s, const mtt::String& key_ctx, const mtt::String& key_var, const Script_Property_List& val)
{
    api.Script_Lookup_set_var_with_key_w_script_instance(s, key_ctx, key_var, val);
}



    
}

uint64 MTT_Random(void)
{
    return mtt::api.MTT_Random();
}
sint64 MTT_Random_lower_bounded(sint64 lower_bound_exclusive)
{
    return mtt::api.MTT_Random_lower_bounded(lower_bound_exclusive);
}
sint64 MTT_Random_range(sint64 lower_bound_inclusive, sint64 upper_bound_exclusive)
{
    return mtt::api.MTT_Random_range(lower_bound_inclusive, upper_bound_exclusive);
}
float64 MTT_Random_Float64_value(void)
{
    return mtt::api.MTT_Random_Float64_value();
}
float64 MTT_Random_Float64_value_scaled(float64 scale)
{
    return mtt::api.MTT_Random_Float64_value_scaled(scale);
}
void MTT_Random_Float64_value_within_circle(float64 extent, float64* radius_out, float64* angle_radians_out)
{
    return mtt::api.MTT_Random_Float64_value_within_circle(extent, radius_out, angle_radians_out);
}

void MTT_set_active_logger(struct MTT_Logger* logger)
{
    ::mtt::api.MTT_set_active_logger(logger);
}
struct MTT_Logger* MTT_active_logger(void)
{
    return ::mtt::api.MTT_active_logger();
}
void MTT_set_default_active_logger(void)
{
    ::mtt::api.MTT_set_default_active_logger();
}
void MTT_Logger_init(struct MTT_Logger* logger_out, cstring subsystem, cstring category)
{
    ::mtt::api.MTT_Logger_init(logger_out, subsystem, category);
}
void MTT_Logger_destroy(struct MTT_Logger* logger)
{
    ::mtt::api.MTT_Logger_destroy(logger);
}

void debug_mode_log_enable(bool state)
{
    ::mtt::api.debug_mode_log_enable(state);
}
bool debug_mode_log_is_enabled(void)
{
    return ::mtt::api.debug_mode_log_is_enabled();
}


namespace dt {

void* selections(void)
{
    return mtt::api.selections();
}

dt::DrawTalk* ctx()
{
    return mtt::api.DrawTalk_ctx();
}

}


namespace mtt {

void MTT_API_set(mtt::MTT_API* api_in)
{
    api = *api_in;
}

}

#endif



