//
//  sd_metal_renderer.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/16/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef sd_metal_renderer_h
#define sd_metal_renderer_h

#import <simd/simd.h>


#import "camera_types.h"
#import "Matrix.h"


#import "ShaderTypes.h"

#include "Make_The_Thing-Swift.h"



#include "stratadraw_platform_apple.hpp"

#include "mtt_core_platform.h"

#include "augmented_reality_platform.hpp"


@class MTLTextureSwift;
//#include <os/signpost.h>






@interface SD_Renderer_Metal_Backend : NSObject <MTKViewDelegate
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
, NSWindowDelegate
#endif
> {
    /// basic metal device
    @public id<MTLDevice> _device;
    uint64 prev_time_ns;
    @public MTT_Core_Platform* _core_platform;
    @public mem::Allocator _id_allocator;
    @public mtt::Dynamic_Array<id> vertex_buffer_list;
    @public mtt::Dynamic_Array<id> index_buffer_list;
    
    sd::Renderer* renderer;
    sd::Image_Loader image_loader;
    @public sd::Images images;
    
    @public NVGcontext* _nvg_ctx;
    
#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
@public bool override_bg_color_with_clear;
#else
@public UIInterfaceOrientation orientation;
#endif
}

-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view andCorePlatform:(nonnull MTT_Core_Platform*)core_platform;

- (void)set_size_with_bounds_size:(CGSize)size;
- (void)drawRectResized:(CGSize)size;


- (void)update:(nonnull MTKView*)view;

- (void)deinit;


typedef void(*Buffer_Memory_Deallocator_Proc)(void* _Nonnull, usize);

- (void) allocate_buffer_resource:(sd::Buffer*_Nonnull)buffer deallocator:(Buffer_Memory_Deallocator_Proc _Nonnull) deallocator;
- (void) allocate_resources_for_layer:(sd::Render_Layer*_Nonnull)render_layer with_label:(NSString*_Nonnull)label deallocator:(Buffer_Memory_Deallocator_Proc _Nonnull)deallocator;

- (void) reallocate_vertex_buffer_for_layer:(sd::Render_Layer*_Nonnull)render_layer with_label:(NSString*_Nonnull)label and_index:(usize)index deallocator:(Buffer_Memory_Deallocator_Proc _Nonnull)deallocator;
- (void) reallocate_index_buffer_for_layer:(sd::Render_Layer*_Nonnull)render_layer with_label:(NSString*_Nonnull)label and_index:(usize)index deallocator:(Buffer_Memory_Deallocator_Proc _Nonnull)deallocator;
- (void) reallocate_resources_for_layer:(sd::Render_Layer*_Nonnull)render_layer with_label:(NSString*_Nonnull)label and_index:(usize)index deallocator:(Buffer_Memory_Deallocator_Proc _Nonnull)deallocator;
- (void) deallocate_resources_for_layer:(sd::Render_Layer*_Nonnull)render_layer and_index:(usize)index;

- (MTKTextureLoader*_Nonnull) get_texture_loader;

- (id<MTLBuffer>_Nonnull) scene_argument_buffer;
- (id<MTLBuffer>_Nonnull) scene_resource_buffer;

- (void) make_render_target:(usize) width height:(usize)height;
- (void)bake_to_texture_begin:(usize) width height:(usize)height;
- (void)bake_to_texture_end:(void(*_Nonnull)(bool, void*_Nonnull)) callback;


- (sd::Shader_ID) compute_shader_make_from_builtin:(NSString*_Nonnull) name;
- (sd::Shader_ID) compute_shader_make:(NSString*_Nonnull) src with_function_name:(NSString*_Nonnull) function_name;
- (id<MTLComputePipelineState>_Nullable) compute_shader_lookup_with_id:(sd::Shader_ID) shader_id;

- (void) compute_shader_destroy:(sd::Shader_ID)shader_id;

- (sd::Shader_ID) compute_shader_replace:(NSString*_Nonnull) src with_function_name:(NSString*_Nonnull) function_name with_shader_id:(sd::Shader_ID)shader_id;

- (void) pause;
- (void) resume;

- (Boolean) supports_multiple_viewports;

- (id<MTLDevice>_Nonnull) active_device;

#if MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP
- (float) titleBarHeight:(NSRect)frame;
#else

- (ARSession*) ARSession_get;
- (void) ARSession_set:(ARSession*) session;

#endif

@end

vec4 view_offset_for_safe_area_platform(Platform_View* _Nonnull view);

static inline SD_Renderer_Metal_Backend* _Nonnull SD_Renderer_backend(sd::Renderer* _Nonnull renderer)
{
    return (__bridge SD_Renderer_Metal_Backend*)renderer->backend;
}

#endif /* sd_metal_renderer_h */
