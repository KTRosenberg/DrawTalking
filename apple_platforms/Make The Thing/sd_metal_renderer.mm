//
//
//  sd_metal_renderer.m
//  Make The Thing
//
//  Created by Toby Rosenberg on 6/16/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#import "sd_metal_renderer.h"

#include "make_the_thing.h"
#include "make_the_thing_main.h"

#include "memory.hpp"
#include "sparse_array.hpp"
#include "image.hpp"

#include "image_platform.hpp"



#define USE_NVG true
#if (USE_NVG)
#include "nanovg.h"
#include "nanovg_mtl.h"
#endif

#define SD_SUPPORT_BINDLESS_ONLY (false)

#if (MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP)

#define CHECK_SHADER_FUNCTION_NONNULL( function__ , fail_fast__ ) \
if ( function__ == nil) { \
    NSLog(@"Shader function not found!"); \
    if constexpr ( fail_fast__ ) { \
        return; \
    } \
} \

#else

#define CHECK_SHADER_FUNCTION_NONNULL( function__ , fail_fast__ ) \
if ( function__ == nil) { \
    NSLog(@"Shader function not found!"); \
    if constexpr ( fail_fast__ ) { \
        exit(EXIT_FAILURE); \
    } \
} \

#endif



using namespace sd;

// Toby TODO: - support triple buffering for vertex buffers
static const NSUInteger MAX_BUFFERS_IN_FLIGHT = 1;

// Vertex data for an image plane
static const float kImagePlaneVertexData[16] = {
    -1.0, -1.0,  0.0, 1.0,
    1.0, -1.0,  1.0, 1.0,
    -1.0,  1.0,  0.0, 0.0,
    1.0,  1.0,  1.0, 0.0,
};


const float z_dist = -1.0;
const float scale = 1.0;//0.2;

//static const VERTEX_Polygon_Color quad[4] = {
//    {
//        .position  = {-1.0, 1.0, -1.0, 1.0},
//        .color     = {1.0, 0.0, 0.0, 1.0},
//        .tex_coord = {0.0, 0.0}
//    },
//    {
//        .position  = {-1.0, -1.0, -1.0, 1.0},
//        .color     = {0.0, 1.0, 0.0, 0.0},
//        .tex_coord = {0.0, 1.0}
//    },
//    {
//        .position  = {1.0, -1.0, -1.0, 1.0},
//        .color     = {0.0, 1.0, 0.0, 0.0},
//        .tex_coord = {1.0, 1.0}
//    },
//    {
//        .position  = {1.0, 1.0, -1.0, 1.0},
//        .color     = {0.0, 1.0, 0.0, 0.0},
//        .tex_coord = {1.0, 0.0}
//    },
//};
static const VERTEX_Polygon_Color quad[4] = {
    {
        .position  = {0.0, 0.0, z_dist, 1.0},
        .color     = {1.0, 0.0, 0.0, 0.5},
        .tex_coord = {0.0, 0.0}
    },
    {
        .position  = {0.0, 760 * scale, z_dist, 1.0},
        .color     = {0.0, 1.0, 0.0, 0.5},
        .tex_coord = {0.0, 1.0}
    },
    {
        .position  = {1024 * scale, 760 * scale, z_dist, 1.0},
        .color     = {0.0, 1.0, 0.0, 0.5},
        .tex_coord = {1.0, 1.0}
    },
    {
        .position  = {1024 * scale, 0.0, z_dist, 1.0},
        .color     = {0.0, 1.0, 0.0, 0.5},
        .tex_coord = {1.0, 0.0}
    },
};
static const uint32 quad_indices[6] = {
    0,1,2, 2,3,0
};

#if TARGET_OS_OSX || TARGET_OS_MACCATALYST

const size_t SD_DEFAULT_ENTITY_COUNT = (1 << 14);
static const size_t kAlignedSharedUniformsSize = align_up(sizeof(SharedUniforms), std::alignment_of<SharedUniforms>::value);

static const size_t kAlignedEntityUniformsSize = align_up(SD_DEFAULT_ENTITY_COUNT * (sizeof(Instance_Data)), std::alignment_of<Instance_Data>::value);
static const size_t kAlignedSemisharedUniformsSize = align_up(SD_DEFAULT_ENTITY_COUNT * (sizeof(Semishared_Uniforms)), std::alignment_of<Semishared_Uniforms>::value);

#else

const size_t SD_DEFAULT_ENTITY_COUNT = (1 << 14);
static const size_t kAlignedSharedUniformsSize = align_up(sizeof(SharedUniforms), std::alignment_of<SharedUniforms>::value);

static const size_t kAlignedEntityUniformsSize = align_up(SD_DEFAULT_ENTITY_COUNT * (sizeof(Instance_Data)), std::alignment_of<Instance_Data>::value);
static const size_t kAlignedSemisharedUniformsSize = align_up(SD_DEFAULT_ENTITY_COUNT * (sizeof(Semishared_Uniforms)), std::alignment_of<Semishared_Uniforms>::value);

#endif

static bool USE_MULTISAMPLING = true;
static usize SAMPLE_COUNT = (USE_MULTISAMPLING) ? 4 : 1;


struct Default_Texture_Info {
    usize width;
    usize height;
    
    usize byte_count;
    
    NSString* name;
    
    u8* data;
    
    sd::Image img;
};


const usize DEFAULT_TEXTURE_COUNT = sd::BUILTIN_TEXTURE_COUNT;
const usize BYTES_PER_COLOR       = BUILTIN_TEXTURE_BYTES_PER_COLOR;

struct Default_Texture_Info_List {
    Default_Texture_Info textures[DEFAULT_TEXTURE_COUNT];
    usize count;
};

//MPSCopyAllocator MPS_allocator = ^NS_RETURNS_RETAINED id <MTLTexture>(MPSKernel * __nonnull filter, id<MTLCommandBuffer> __nonnull  cmdBuf, id <MTLTexture>  __nonnull sourceTexture)
//{
//    MTLPixelFormat format = sourceTexture.pixelFormat;
//    MTLTextureDescriptor *d = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat: format width: sourceTexture.width height: sourceTexture.height mipmapped: NO];
//    d.usage = sourceTexture.usage;
//
//    id <MTLTexture> result = [cmdBuf.device newTextureWithDescriptor: d];
//
//    return result;
//    // d is autoreleased.
//};

@implementation SD_Renderer_Metal_Backend {
    // MARK: Graphics
    MTKView* _view;
    /// semaphore for drawing
    dispatch_semaphore_t _in_flight_semaphore;
    dispatch_semaphore_t _render_semaphore;
    // the index of the buffer for screen capture
    uint8_t _buffer_idx;
    
    /// main commandQueue
    id<MTLCommandQueue> _command_queue;
    id <MTLCommandBuffer> command_buffer;
    id <MTLCommandBuffer> main_command_buffer;
    
    id<MTLBuffer> _shared_uniform_buffer;
    id<MTLBuffer> _entity_uniform_buffer;
    id<MTLBuffer> _semishared_uniform_buffer;
    
    id<MTLBuffer> active_v_buffer_0;
    
    id<MTLBuffer> _scene_data;
    id<MTLBuffer> _scene_resource_data;
    id<MTLBuffer> _sampler_data;
    BOOL support_bindless;
    
    uint64 _pipeline_mode;
    
    id<MTLRenderPipelineState> _polygon_color_pipeline_state;
    id<MTLDepthStencilState>   _things_depth_stencil_state;
    id<MTLDepthStencilState>   _things_depth_stencil_state_no_depth_write;
    
    id<MTLDepthStencilState>   _things_depth_stencil_state_w_stencil;
    id<MTLDepthStencilState>   _things_depth_stencil_state_no_depth_write_stencil;
    sd::DEPTH_WRITE_MODE depth_write_mode;
    sd::STENCIL_MODE stencil_mode;
    
    
    
    
        // Render pass descriptor to draw to the texture
    struct New_Render_Target {
        MTLRenderPassDescriptor* _renderToTextureRenderPassDescriptor;
        id<MTLTexture>           render_target_multisample;
        id<MTLTexture>           render_target_final;
        id<MTLTexture>           depth_stencil_texture;
        id<MTLCommandBuffer>     command_buffer;
        id<MTLRenderCommandEncoder> render_command_encoder;
    } new_render_target;
    
    MTLRenderPassDescriptor* offscreen_renderpass_descriptor;
    MTLRenderPassDescriptor* offscreen_renderpass_descriptor_msaa;
    
    struct Compute_Pass {
        id<MTLComputeCommandEncoder> encoder;
        bool should_wait;
        id<MTLCommandBuffer> command_buffer;
        bool use_own_command_buffer;
    } compute_pass;
    
    mtt::Dynamic_Array<id> _compute_pipelines;
    mtt::Dynamic_Array<usize> _compute_pipelines_free_list;
    
    id<MTLRenderPipelineState> _polygon_color_hsv_pipeline_state;
    
    // texture support
    id<MTLRenderPipelineState> _polygon_color_texture_pipeline_state;
    
    // TODO:
    id<MTLRenderPipelineState> _polygon_color_additive_pipeline_state;
    id<MTLRenderPipelineState> _polygon_color_hsv_additive_pipeline_state;
    id<MTLRenderPipelineState> _polygon_color_texture_additive_pipeline_state;
    
    id<MTLRenderPipelineState> _polygon_color_no_depth_stencil_pipeline_state;
    
    
    usize max_resident_textures;
    
    MTKTextureLoader*          _texture_loader;
    
    Default_Texture_Info_List  _default_texture_info;
    
    id<MTLSamplerState> sampler_default;
    id<MTLSamplerState> sampler_clamp_to_edge;
    id<MTLSamplerState> sampler_repeat;
    id<MTLSamplerState> sampler_mirror_repeat;
    id<MTLSamplerState> sampler_mirror_clamp_to_edge;
    id<MTLSamplerState> sampler_clamp_to_edge_linear;
    
    // text support
    //id<MTLRenderPipelineState> _text_pipeline_state;
    //id<MTLBuffer>              _fragment_shader_text_argument_buffer;
    //id<MTLArgumentEncoder>     _fragment_shader_text_argument_encoder;
    
    mtt::Dynamic_Array<id> _pipelines;
    
    // Offset within _shared_uniform_buffer to set for the current frame
    uint32 _shared_uniform_buffer_offset;
    // Offset within _shared_uniform_buffer to set for the current frame
    uint32 _entity_uniform_buffer_offset;
    // Offset within _shared_uniform_buffer to set for the current frame
    uint32 _semishared_uniform_buffer_offset;
    
    // Addresses to write shared uniforms to each frame
    void* _shared_uniform_buffer_address;
    // Addresses to write individual entity uniforms to each frame
    void* _entity_uniform_buffer_address;
    // Addresses to write semishared uniforms to each frame
    void* _semishared_uniform_buffer_address;
    
    // MARK: Augmented Reality and Video
    /// ARSession needed for camera retrival
#if (MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP)
    ARSession* _session;
#endif
    /// distortion reduction scaling
    id <MTLBuffer> _textureCoordinates;
    /// the camera recvies frames only as yCbrC frames so conversion is done on the gpu
    id <MTLRenderPipelineState> _captured_image_pipeline_state;
    id <MTLDepthStencilState>   _captured_image_depth_state;
    /// texture cache
    CVMetalTextureCacheRef _captured_image_texture_cache;
    /// texture refrences from the camera y is the yellow part and cb the blue and rc the red and chroma
    CVMetalTextureRef _captured_image_texture_y_ref;
    CVMetalTextureRef _captured_image_texture_cbcr_ref;
    /// the capture buffer for the image
    id <MTLBuffer> _capture_buffers[MAX_BUFFERS_IN_FLIGHT];
    
    id <MTLBuffer> _image_plane_vertex_buffer;
    
    // TODO(Toby): unused
    /// I did start with creating a dispatch queue but actually this is not needed for the compression session
    /// I still left it theret.
    dispatch_queue_t _encoder_queue;
    
    
    // MARK: Viewport
    /// the size of the view e.g view.frame.size
    CGSize _viewport_size;
    BOOL _viewport_size_did_change;
    
    sd::Viewport     viewport;
    sd::Scissor_Rect scissor_rectangle;
    
    // MARK: Time
    float64 _time_seconds_prev;
    
    float64 _time_now_ms;
    float64 _time_now_s;
    float64 _time_sim_accumulator;
    uint64  _time_sim_accumulator_counter;
    float64 _catimeprev;
    float64 _time_sim_elapsed;
    float64 _curr_time;
    
    dispatch_queue_t _render_queue;
    
    id <MTLLibrary> default_shader_library;
    
#if (LOG_SIGNPOST_ENABLED)
    os_log_t _logger;
    os_signpost_id_t _os_signpost_render_update_id;
#endif
    
#if (USE_ARKIT)
    vec3 screen_position_left_eye;
    vec3 screen_position_right_eye;
    vec3 look_at_point;
    // https://stackoverflow.com/questions/55066870/facetracking-in-arkit-how-to-display-the-lookatpoint-on-the-screen
#endif
    
    MPSImageSobel* image_sobel_fx;
}

static MTLStoreAction store_action_render_target = MTLStoreActionUnknown;

static float64 calc_aspect_ratio(void)
{
    CGSize size = (get_native_bounds(get_main_screen())).size;
    return (float64)size.height / (float64)size.width;
}

static float64 get_aspect_ratio(void)
{
    //return mtt_core_platform_ctx()->core.default_aspect_ratio;
    return calc_aspect_ratio();
}




-(nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)view andCorePlatform:(nonnull MTT_Core_Platform*)core_platform;
{
    self = [super init];
    if (self) {
        backing_view = view;
        view.autoResizeDrawable = YES;
        
#if (LOG_SIGNPOST_ENABLED)
        _logger = os_log_create("com.make_the_thing.render_loop", "render loop");
        
        _os_signpost_render_update_id = os_signpost_id_generate(_logger);
#endif
        
#if (USE_ARKIT)
        self->screen_position_left_eye = vec3(0.0f);
        self->screen_position_right_eye = vec3(0.0f);
#endif
        
        //auto offset = view_offset_for_safe_area_platform(backing_view);
#if (MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP)
        UIDeviceOrientation device_orientation = [UIDevice currentDevice].orientation;
        MTT_UNUSED(device_orientation);
//        orientation = (device_orientation == UIDeviceOrientationLandscapeRight) ? UIInterfaceOrientationLandscapeLeft : UIInterfaceOrientationLandscapeRight;
//
        UIInterfaceOrientation interface_orientation;
//        UIInterfaceOrientation interface_orientation = [[[[[UIApplication sharedApplication] windows] firstObject] windowScene] interfaceOrientation];
        
        interface_orientation = [((UIWindowScene*)[[[UIApplication sharedApplication] connectedScenes] anyObject]) interfaceOrientation];
        
        orientation = interface_orientation;
        
        
        _session = nil;
#else
        override_bg_color_with_clear = false;
//        _view.layer.autoresizingMask = kCALayerHeightSizable | kCALayerWidthSizable;
//        _view.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;
//        _view.layer.needsDisplayOnBoundsChange = YES;
//        _view.presentsWithTransaction = YES;
        
        NSArray<NSWindow*>* windows = [[NSApplication sharedApplication] windows];
        for (NSWindow* window in windows) {
            window.delegate = self;
            window.restorable = YES;
        }
        
#endif
        
        _id_allocator.data       = nullptr;
        _id_allocator.allocate   = mem::objc_id_allocate;
        _id_allocator.deallocate = mem::objc_id_deallocate;
        _id_allocator.resize     = mem::objc_id_resize;
        
        /// copy the pointer from the metal device
        _device = view.device;
        /// set thenumber of MetalDrawable objects to the MaxBuffersInFlight variable,
        /// this semaphore locks if there are no completions of these frames
        _in_flight_semaphore = dispatch_semaphore_create(MAX_BUFFERS_IN_FLIGHT);
        _render_semaphore    = dispatch_semaphore_create(0);
        

        _core_platform = core_platform;
        
        renderer = &_core_platform->renderer;
        
        _view = view;
        
        _render_queue = dispatch_queue_create("com.make_the_thing.render", DISPATCH_QUEUE_SERIAL);
        
        _pipeline_mode = sd::PIPELINE_MODE_COLOR;
        
        // TODO: eventually, support custom pipelines
        mtt::init(&_pipelines, _id_allocator, sd::PIPELINE_MODE_COUNT, sd::PIPELINE_MODE_COUNT);
        mtt::init(&_compute_pipelines, _id_allocator);
        mtt::init(&_compute_pipelines_free_list, _core_platform->core.allocator);
        
        
        _time_sim_accumulator = 0;
        _time_sim_accumulator_counter = 0;
        _time_now_s           = 0.0;
        _time_now_ms          = 0.0;
        _time_seconds_prev    = 0.0;
        prev_time_ns   = 0;
        
        
        _core_platform->core.viewport.znear = 0.0;
        _core_platform->core.viewport.zfar = 1.0;
        
        sd::setup();
        sd::Renderer_init(&_core_platform->renderer, &_core_platform->core.allocator);
        _core_platform->renderer.backend = (__bridge void*)self;
        _core_platform->core.default_aspect_ratio = calc_aspect_ratio();
        
        self->image_sobel_fx = [[MPSImageSobel alloc] initWithDevice:_device];
        
        /// continue initialization of Metal
        
        _core_platform->renderer.depths_default = {nextafterf(-1000.0f, -999.0f), nexttoward(1000.0f, 999.0f)};
        [self _loadMetalWithView:view];
        
        
    }
    
    
    return self;
}



- (void)dealloc {
    // release the textures that we got from the ARSession
    CVBufferRelease(_captured_image_texture_y_ref);
    CVBufferRelease(_captured_image_texture_cbcr_ref);
}

- (void)set_size_with_bounds_size:(CGSize)size {
    
}
- (void)drawRectResized:(CGSize)size {
    _viewport_size            = size;
    _viewport_size_did_change = YES;
    _core_platform->core.viewport.x      = 0.0;
    _core_platform->core.viewport.y      = 0.0;
    float64 scale = sd::get_native_display_scale();
    _core_platform->core.viewport.width  = size.width / scale;
    _core_platform->core.viewport.height = size.height / scale;
}

#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)

- (float) titleBarHeight:(NSRect)frame {
    
    NSRect contentRect;
    contentRect = [NSWindow contentRectForFrameRect: frame
                                          styleMask: NSWindowStyleMaskTitled];
    
    return (frame.size.height - contentRect.size.height);
    
}

- (NSSize)windowWillResize:(NSWindow *)sender
                    toSize:(NSSize)frameSize
{
    NSView* main_view = sender.contentView;
    NSRect frame = main_view.frame;

    //f32 ratio = (f32)(frame.size.width)/(f32)(frame.size.height);
    f32 new_height    = frameSize.height;
    f32 new_width     = frameSize.width;// * ratio;
    frame.size.width  = new_width;
    frame.size.height = new_height - [self titleBarHeight:frame];
    
    //[main_view setFrame:frame];
    [_view setFrame:frame];
    //[self drawRectResized:frameSize];
    
    return frameSize;
}
- (void)windowDidMove:(NSNotification *)notification {

}
- (void)windowDidResize:(NSNotification *)notification {
    
}
#endif

- (void) allocate_buffer_resource:(sd::Buffer*_Nonnull)buffer deallocator:(Buffer_Memory_Deallocator_Proc _Nonnull) deallocator {
    @autoreleasepool {
        id<MTLBuffer> device_buffer = [_device newBufferWithBytesNoCopy:buffer->memory
                                                    length:buffer->alloc_bytes
                                                   options:MTLResourceStorageModeShared
                                               deallocator:^(void * _Nonnull memory, NSUInteger byte_length) {
            deallocator(memory, byte_length);
            
//#ifndef NDEBUG
//            NSLog(@"SD deallocated buffer");
//#endif
        }];
        buffer->buffer = CFBridgingRetain(device_buffer);
//#ifndef NDEBUG
//        NSLog(@"SD allocated buffer");
//#endif
    }
}

- (void) allocate_resources_for_layer:(sd::Render_Layer*_Nonnull)render_layer with_label:(NSString*_Nonnull)label deallocator:(Buffer_Memory_Deallocator_Proc _Nonnull) deallocator {
    // MARK: - init vertex and index buffers for this layer
    
    @autoreleasepool {
        
//        MTT_print("ALLOCATING RESOURCES IN METAL BACKEND BEGIN, label=[%s]\n", [label cStringUsingEncoding:NSUTF8StringEncoding]);;
//
        mtt::append(&vertex_buffer_list, (id)[_device newBufferWithBytesNoCopy:render_layer->polygon_color_vertices
                                                                        length:render_layer->polygon_color_vertices_count_bytes
                                                                       options:MTLResourceStorageModeShared
                                                                   deallocator:^(void * _Nonnull memory, NSUInteger byte_length)
        {
          //  MTT_print("MTLBuffer reallocated\n");
            deallocator(memory, byte_length);
        }]);
        ((id<MTLBuffer>)*vertex_buffer_list.last_ptr()).label = [NSString stringWithFormat:@"vertex_buffer_polygons[%@]", label];
        
        mtt::append(&index_buffer_list, (id)[_device newBufferWithBytesNoCopy:render_layer->polygon_color_indices
                                                                       length:render_layer->polygon_color_indices_count_bytes
                                                                      options:MTLResourceStorageModeShared
                                                                  deallocator:^(void * _Nonnull memory, NSUInteger byte_length)
        {
        //    MTT_print("MTLBuffer reallocated\n");
            deallocator(memory, byte_length);
        }]);
        ((id<MTLBuffer>)*index_buffer_list.last_ptr()).label = [NSString stringWithFormat:@"index_buffer_polygons[%@]", label];
        
        //    // MARK: - init paths
        //    _dynamic_paths_vertex_buffer = [_device newBufferWithBytesNoCopy:render_layer->path_color_vertices
        //                                                              length:render_layer->path_color_vertices_count_bytes
        //                                                                options:MTLResourceStorageModeShared
        //                                                            deallocator:nil];
        //    _dynamic_paths_vertex_buffer.label = [NSString stringWithFormat:@"vertex_buffer_paths[%@]", label];
        //
        //    _dynamic_paths_index_buffer = [_device newBufferWithBytesNoCopy:render_layer->path_color_indices
        //                                                             length:render_layer->path_color_indices_count_bytes
        //                                                               options:MTLResourceStorageModeShared
        //                                                           deallocator:nil];
        //    _dynamic_paths_index_buffer.label = [NSString stringWithFormat:@"index_buffer_paths[%@]", label];
        
        //MTT_print("ALLOCATING RESOURCES IN METAL BACKEND END,   label=[%s]\n", [label cStringUsingEncoding:NSUTF8StringEncoding]);
        
    }
}



- (void) reallocate_resources_for_layer:(sd::Render_Layer*_Nonnull)render_layer with_label:(NSString*_Nonnull)label and_index:(usize)index deallocator:(Buffer_Memory_Deallocator_Proc _Nonnull)deallocator {
    //MTT_print("ALLOCATING RESOURCES IN METAL BACKEND BEGIN, label=[%s]\n", [label cStringUsingEncoding:NSUTF8StringEncoding]);
    
    @autoreleasepool {
        vertex_buffer_list[index] = [_device newBufferWithBytesNoCopy:render_layer->polygon_color_vertices
                                                               length:render_layer->polygon_color_vertices_count_bytes
                                                              options:MTLResourceStorageModeShared
                                                          deallocator:^(void * _Nonnull memory, NSUInteger byte_length)
        {
            //MTT_print("MTLBuffer reallocated\n");
            deallocator(memory, byte_length);
        }];
        
        //((id<MTLBuffer>)[_vertex_buffer_list objectAtIndex:index]).label = [NSString stringWithFormat:@"vertex_buffer_polygons[%@]", label];
        
        
        index_buffer_list[index] = [_device newBufferWithBytesNoCopy:render_layer->polygon_color_indices
                                                              length:render_layer->polygon_color_indices_count_bytes
                                                             options:MTLResourceStorageModeShared
                                                         deallocator:^(void * _Nonnull memory, NSUInteger byte_length)
        {
            //MTT_print("MTLBuffer reallocated\n");
            deallocator(memory, byte_length);
        }];
        //((id<MTLBuffer>)[_index_buffer_list objectAtIndex:index]).label = [NSString stringWithFormat:@"index_buffer_polygons[%@]", label];
        
    }
}

- (void) reallocate_vertex_buffer_for_layer:(sd::Render_Layer*_Nonnull)render_layer with_label:(NSString*_Nonnull)label and_index:(usize)index deallocator:(Buffer_Memory_Deallocator_Proc _Nonnull)deallocator {
    //MTT_print("ALLOCATING VERTEX BUFFER IN METAL BACKEND BEGIN, label=[%s]\n", [label cStringUsingEncoding:NSUTF8StringEncoding]);
    
    @autoreleasepool {
        vertex_buffer_list[index] = [_device newBufferWithBytesNoCopy:render_layer->polygon_color_vertices
                                                               length:render_layer->polygon_color_vertices_count_bytes
                                                              options:MTLResourceStorageModeShared
                                                          deallocator:^(void * _Nonnull memory, NSUInteger byte_length)
        {
            //MTT_print("MTLBuffer reallocated\n");
            deallocator(memory, byte_length);
        }];
        
        //((id<MTLBuffer>)[_vertex_buffer_list objectAtIndex:index]).label = [NSString stringWithFormat:@"vertex_buffer_polygons[%@]", label];
    }
}

- (void) reallocate_index_buffer_for_layer:(sd::Render_Layer*_Nonnull)render_layer with_label:(NSString*_Nonnull)label and_index:(usize)index deallocator:(Buffer_Memory_Deallocator_Proc _Nonnull)deallocator {
    //MTT_print("ALLOCATING INDEX BUFFER IN METAL BACKEND BEGIN, label=[%s]\n", [label cStringUsingEncoding:NSUTF8StringEncoding]);
    
    @autoreleasepool {
        index_buffer_list[index] = [_device newBufferWithBytesNoCopy:render_layer->polygon_color_indices
                                                              length:render_layer->polygon_color_indices_count_bytes
                                                             options:MTLResourceStorageModeShared
                                                         deallocator:^(void * _Nonnull memory, NSUInteger byte_length)
        {
           // MTT_print("MTLBuffer reallocated\n");
            deallocator(memory, byte_length);
        }];
        //((id<MTLBuffer>)[_index_buffer_list objectAtIndex:index]).label = [NSString stringWithFormat:@"index_buffer_polygons[%@]", label];
        
    }
}

- (void) deallocate_resources_for_layer:(sd::Render_Layer*_Nonnull)render_layer and_index:(usize)index{
    @autoreleasepool {
        vertex_buffer_list[index] = nil;
        index_buffer_list[index] = nil;
    }
}





// MARK: default color textures
- (usize) load_default_textures:(usize) load_into_idx options:(BOOL)mtl3 {
    @autoreleasepool {
            
        auto* images = self->renderer->images;
        auto* loader = self->renderer->image_loader;
        
    #define COLOR2x2(r, g, b, a) {b,g,r,a, b,g,r,a, b,g,r,a, b,g,r,a}
    #define COLORPIX(r, g, b, a) b,g,r,a
    #define COLORPIXFLOAT(r, g, b, a) (u8)(255*b),(u8)(255*g),(u8)(255*r),(u8)(255*a)
        
        
        CGSize screen_size = get_bounds(get_main_screen()).size;
        
        //HSVtoRGB(0 /* 0-359 */, 1.0, 1.0, 1.0);
        
        
        
    #define TEXTURE2x2(_bytes_per_color, name) 2,2, _bytes_per_color * 2 * 2, name
        _default_texture_info = {
            {
                [sd::TEXTURE_ID_BUILTIN_WHITE] = {
                    TEXTURE2x2(BYTES_PER_COLOR, @"white")
                },
                [sd::TEXTURE_ID_BUILTIN_CLEAR] = {
                    TEXTURE2x2(BYTES_PER_COLOR, @"clear")
                },
                [sd::TEXTURE_ID_BUILTIN_SPECTRUM] = {
                    (usize)screen_size.width, // width
                    2, // height
                    (usize)screen_size.width * 2 * BYTES_PER_COLOR, // bytes
                    @"RGB_spectrum" // name
                },
                [sd::TEXTURE_ID_BUILTIN_BLACK] = {
                    TEXTURE2x2(BYTES_PER_COLOR, @"black")
                },
            },
            DEFAULT_TEXTURE_COUNT
        };

    #define USE_PRIVATE_TEXTURES (1)
    #if !USE_PRIVATE_TEXTURES
        for (usize i = 0; i < DEFAULT_TEXTURE_COUNT; i += 1) {
            _default_texture_info.textures[i].data = (u8*)calloc(_default_texture_info.textures[i].byte_count, sizeof(u8));
            
            if (_default_texture_info.textures[i].data == nullptr) {
                MTT_error("ERROR: could not allocate %llu bytes for default textures\n",
                          _default_texture_info.textures[i].byte_count);
            }
        }
        {
            {
                u8 texture[2*2*BYTES_PER_COLOR] = COLOR2x2(255, 255, 255, 255);
                
                memcpy(_default_texture_info.textures[0].data, texture, sizeof(texture));
            }
            {
                u8 texture[2*2*BYTES_PER_COLOR] = COLOR2x2(0, 0, 0, 0);
                
                memcpy(_default_texture_info.textures[1].data, texture, sizeof(texture));
            }
            {
                Default_Texture_Info* info = &_default_texture_info.textures[2];
                
                float64 pixel_to_hue = (float64)359.0 / (float64)info->width;
                MTT_print("WIDTH screen %f, info %llu\n", get_bounds(get_main_screen()).size.width, info->width);
                u8* tex_data = info->data;
                
                
                for (usize col = 0; col < info->width; col += 1) {
                    
                    vec4 color = HSVtoRGBinplace(pixel_to_hue*col, 1, 1, 1);
                    
                    
                    
                    const u8 color_arr[4] = {
                        COLORPIXFLOAT(color.r, color.g, color.b, color.a)
                    };
                    
                    memcpy(tex_data, color_arr, sizeof(color_arr));
                    
                    tex_data += BYTES_PER_COLOR;
                }
                for (usize row = 1; row < info->height; row += 1) {
                    memcpy(tex_data, tex_data - (BYTES_PER_COLOR * info->width), (BYTES_PER_COLOR * info->width));
                    
                    tex_data += BYTES_PER_COLOR * info->width;
                }
                
                //            u8* data = info->data;
                //            for (usize r = 0; r < info->height; r += 1) {
                //                MTT_print("{");
                //                for (usize c = 0; c < info->width; c += 1) {
                //                    MTT_print("[%u,%u,%u,%u]",
                //                              data[(r * info->width) + c + 2],
                //                              data[(r * info->width) + c + 1],
                //                              data[(r * info->width) + c],
                //                              data[(r * info->width) + c + 3]);
                //                }
                //                MTT_print("}\n");
                //            }
                
                
            }
        }
        
    #undef TEXTURE2x2
    #undef COLOR2x2
    #undef COLORPIX
    #undef COLORPIXFLOAT
        
        MTLTextureDescriptor* texture_desc = [[MTLTextureDescriptor alloc] init];
        
        texture_desc.pixelFormat  = _view.colorPixelFormat;
        
        
        const NSUInteger bytes_per_color_component = 4;
        
        
        for (usize texture_index = load_into_idx; texture_index < DEFAULT_TEXTURE_COUNT; texture_index += 1) {
            
            Default_Texture_Info* tex_info = &_default_texture_info.textures[texture_index];
            
            texture_desc.width        = tex_info->width;
            texture_desc.height       = tex_info->height;
            
            MTLRegion texture_region = MTLRegionMake2D(0, 0, texture_desc.width, texture_desc.height);
            
            NSUInteger bytes_per_row = bytes_per_color_component * texture_desc.width;
            
            
            id<MTLTexture> texture = [_device newTextureWithDescriptor:texture_desc];
            if (texture == nil) {
                NSLog(@"ERROR could not create texture");
            }
            
            texture.label = tex_info->name;
            
            void* image_bytes = (void*)tex_info->data;
            
            [texture replaceRegion:texture_region
                       mipmapLevel:0
                         withBytes:image_bytes
                       bytesPerRow:bytes_per_row];
            
            sd::Image* img = sd::Image_create_from_MTLTexture(images, [tex_info->name cStringUsingEncoding:NSUTF8StringEncoding], texture, {.usage_flags = 0});
            img->free = [](sd::Image* img) {
                free(img->data);
                img->data = NULL;
            };
        }
        
        for (usize i = 0; i < DEFAULT_TEXTURE_COUNT; i += 1) {
            free(_default_texture_info.textures[i].data);
        }
        
    #else
        
        usize buffer_byte_count = 0;
        for (usize i = 0; i < DEFAULT_TEXTURE_COUNT; i += 1) {
            buffer_byte_count += _default_texture_info.textures[i].byte_count;
        }
        
        
        
        id<MTLBuffer> texture_data_buffer = [_device newBufferWithLength:buffer_byte_count options:MTLResourceStorageModeShared];
        u8* buffer_contents = (u8*)texture_data_buffer.contents;
        uintptr buffer_contents_start = (uintptr)texture_data_buffer.contents;
        {
            {
                Default_Texture_Info* info = &_default_texture_info.textures[0];
                info->data = buffer_contents;

                u8 texture[2*2*BYTES_PER_COLOR] = COLOR2x2(255, 255, 255, 255);
                
                memcpy(buffer_contents, texture, sizeof(texture));
                buffer_contents += sizeof(texture);
            }
            {
                Default_Texture_Info* info = &_default_texture_info.textures[1];
                info->data = buffer_contents;
                
                u8 texture[2*2*BYTES_PER_COLOR] = COLOR2x2(0, 0, 0, 0);
                
                memcpy(buffer_contents, texture, sizeof(texture));
                buffer_contents += sizeof(texture);
            }
            {
                Default_Texture_Info* info = &_default_texture_info.textures[2];
                info->data = buffer_contents;
                
                float64 pixel_to_hue = (float64)359.0 / (float64)info->width;
                //MTT_print("WIDTH screen %f, info %llu\n", get_bounds(get_main_screen()).size.width, info->width);
                
                
                for (usize col = 0; col < info->width; col += 1) {
                    
                    vec4 color = HSVtoRGBinplace(pixel_to_hue*col, 1, 1, 1);
                    
                    
                    
                    const u8 color_arr[4] = {
                        COLORPIXFLOAT(color.r, color.g, color.b, color.a)
                    };
                    
                    memcpy(buffer_contents, color_arr, sizeof(color_arr));
                    
                    buffer_contents += BYTES_PER_COLOR;
                }
                for (usize row = 1; row < info->height; row += 1) {
                    memcpy(buffer_contents, buffer_contents - (BYTES_PER_COLOR * info->width), (BYTES_PER_COLOR * info->width));
                    
                    buffer_contents += BYTES_PER_COLOR * info->width;
                }
            }
            {
                Default_Texture_Info* info = &_default_texture_info.textures[3];
                info->data = buffer_contents;
                
                u8 texture[2*2*BYTES_PER_COLOR] = COLOR2x2(0, 0, 0, 255);
                
                memcpy(buffer_contents, texture, sizeof(texture));
                buffer_contents += sizeof(texture);
            }

        }
        
#undef TEXTURE2x2
#undef COLOR2x2
#undef COLORPIX
#undef COLORPIXFLOAT
        
        id<MTLCommandBuffer> cmd_buf = [_command_queue commandBuffer];
        id<MTLBlitCommandEncoder> blits = [cmd_buf blitCommandEncoder];
        
        MTLTextureDescriptor* texture_desc = [[MTLTextureDescriptor alloc] init];
        
        texture_desc.pixelFormat  = _view.colorPixelFormat;
        
        texture_desc.storageMode = MTLStorageModePrivate;
        
        const NSUInteger bytes_per_color_component = 4;
        
        for (usize texture_index = load_into_idx; texture_index < DEFAULT_TEXTURE_COUNT; texture_index += 1) {
            
            Default_Texture_Info* tex_info = &_default_texture_info.textures[texture_index];
            
            texture_desc.width        = tex_info->width;
            texture_desc.height       = tex_info->height;
            
            MTLRegion texture_region = MTLRegionMake2D(0, 0, texture_desc.width, texture_desc.height);
            
            NSUInteger bytes_per_row = bytes_per_color_component * texture_desc.width;
            
            id<MTLTexture> texture = [_device newTextureWithDescriptor:texture_desc];
            if (texture == nil) {
                NSLog(@"ERROR could not create texture");
            }
            
            texture.label = tex_info->name;
            
            //void* image_bytes = (void*)tex_info->data;
            
            
            [blits copyFromBuffer:texture_data_buffer
                     sourceOffset:((uintptr)tex_info->data) - buffer_contents_start
                sourceBytesPerRow:bytes_per_row
              sourceBytesPerImage:texture.allocatedSize
                       sourceSize:MTLSizeMake(texture.width, texture.height, 1)
                        toTexture:texture
                 destinationSlice:0
                 destinationLevel:0
                destinationOrigin:MTLOriginMake(0, 0, 0)];
            
            sd::Image* img = sd::Image_create_from_MTLTexture(images, [tex_info->name cStringUsingEncoding:NSUTF8StringEncoding], texture, {.usage_flags = 0});
            img->data = nil;
            
            tex_info->data = nullptr;
        }
        [blits endEncoding];
        [cmd_buf commit];
        [cmd_buf waitUntilCompleted];
        
        

        
            
    #endif
    #undef USE_PRIVATE_TEXTURES
        
        return DEFAULT_TEXTURE_COUNT;
        
    }
}

- (id<MTLRenderPipelineState>) pipeline_make:(nonnull MTLRenderPipelineDescriptor*) pl_desc
{
    @autoreleasepool {
        NSError* error = nil;
        id<MTLRenderPipelineState> pl_state = [_device newRenderPipelineStateWithDescriptor:pl_desc error:&error];
        if (!pl_state) {
            NSLog(@"Failed to create pipeline state label=[%@], error %@", pl_desc.label, [error localizedDescription]);
            return nil;
        } else {
            return pl_state;
        }
    }
}

- (id<MTLRenderPipelineState>) pipeline_make:(nonnull MTLRenderPipelineDescriptor*) pl_desc with_label:(nonnull NSString*) label
{
    @autoreleasepool {
        pl_desc.label = label;
        NSError* error = nil;
        id<MTLRenderPipelineState> pl_state = [_device newRenderPipelineStateWithDescriptor:pl_desc error:&error];
        if (!pl_state) {
            NSLog(@"Failed to create pipeline state label=[%@], error %@", pl_desc.label, [error localizedDescription]);
            return nil;
        } else {
            return pl_state;
        }
    }
}

- (void)_loadMetalWithView:(nonnull MTKView *)view;
{
    @autoreleasepool {
        
        BOOL supports_metal_3 = NO;
#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
        if (@available(macOS 13.0, *)) {
            if ([_device supportsFamily:MTLGPUFamilyMetal3]) {
                NSLog(@"supports MTLGPUFamilyMetal3");
                supports_metal_3 = YES;
            }
        }
#else
        if (@available(iOS 15.0, *)) {
            if ([_device supportsFamily:MTLGPUFamilyMetal3]) {
                NSLog(@"supports MTLGPUFamilyMetal3");
                supports_metal_3 = YES;
            }
        }
#endif
        
        // MARK: - Command Queue
        
        // Create the command queue
        _command_queue = [_device newCommandQueue];
        
        self->support_bindless = supports_metal_3;

        renderer->max_buffer_size = _device.maxBufferLength;
        
        if ([_device supportsFamily:MTLGPUFamilyApple8]) {
            NSLog(@"supports MTLGPUFamilyApple8");
        } else if ([_device supportsFamily:MTLGPUFamilyApple7]) {
            NSLog(@"supports MTLGPUFamilyApple7");
        } else if ([_device supportsFamily:MTLGPUFamilyApple6]) {
            NSLog(@"supports MTLGPUFamilyApple6");
        } else if ([_device supportsFamily:MTLGPUFamilyApple5]) {
            NSLog(@"supports MTLGPUFamilyApple5");
        } else if ([_device supportsFamily:MTLGPUFamilyApple4]) {
            NSLog(@"supports MTLGPUFamilyApple4");
        } else if ([_device supportsFamily:MTLGPUFamilyApple3]) {
            NSLog(@"supports MTLGPUFamilyApple3 or less, disabling multisampling");
            USE_MULTISAMPLING = false;
        }
        SAMPLE_COUNT = (USE_MULTISAMPLING) ? 4 : 1;
        if (USE_MULTISAMPLING) {
            NSLog(@"MSAA enabled\n");
            store_action_render_target = MTLStoreActionMultisampleResolve;
        } else {
            store_action_render_target = MTLStoreActionStore;
        }
        view.sampleCount = SAMPLE_COUNT;
        
        view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        
        _buffer_idx = 0;
        
        const NSUInteger sharedUniformBufferSize = kAlignedSharedUniformsSize * MAX_BUFFERS_IN_FLIGHT;
        
        _shared_uniform_buffer = [_device newBufferWithLength:sharedUniformBufferSize
                                                      options:MTLResourceStorageModeShared];
        _shared_uniform_buffer.label = @"SharedUniformBuffer";
        _shared_uniform_buffer_offset = 0;
        _shared_uniform_buffer_address = ((uint8_t*)_shared_uniform_buffer.contents);
        
        const NSUInteger entityUniformBufferSize = kAlignedEntityUniformsSize * MAX_BUFFERS_IN_FLIGHT;
        _entity_uniform_buffer = [_device newBufferWithLength:entityUniformBufferSize
                                                      options:MTLResourceStorageModeShared];
        _entity_uniform_buffer.label = @"EntityUniformBuffer";
        _entity_uniform_buffer_offset = 0;
        _entity_uniform_buffer_address = ((uint8_t*)_entity_uniform_buffer.contents);
        
        const NSUInteger semisharedUniformBufferSize = kAlignedSemisharedUniformsSize * MAX_BUFFERS_IN_FLIGHT;
        _semishared_uniform_buffer = [_device newBufferWithLength:semisharedUniformBufferSize
                                                          options:MTLResourceStorageModeShared];
        _semishared_uniform_buffer.label = @"SemisharedUniformBuffer";
        _semishared_uniform_buffer_offset = 0;
        _semishared_uniform_buffer_address = ((uint8_t*)_semishared_uniform_buffer.contents);
        
        
        active_v_buffer_0 = nil;
        
        // Calculate our uniform buffer sizes. We allocate kMaxBuffersInFlight instances for uniform
        //   storage in a single buffer. This allows us to update uniforms in a ring (i.e. triple
        //   buffer the uniforms) so that the GPU reads from one slot in the ring wil the CPU writes
        //   to another. Anchor uniforms should be specified with a max instance count for instancing.
        //   Also uniform storage must be aligned (to 256 bytes) to meet the requirements to be an
        //   argument in the constant address space of our shading functions.
        //    const NSUInteger sharedUniformBufferSize = kAlignedSharedUniformsSize * MAX_BUFFERS_IN_FLIGHT;
        //    const NSUInteger anchorUniformBufferSize = kAlignedInstanceUniformsSize * MAX_BUFFERS_IN_FLIGHT;
        //
        // Load all the shader files with a metal file extension in the project
        default_shader_library = [_device newDefaultLibrary];
        
        NSError *error = nil;
#if (USE_ARKIT)
        // MARK: - AR
        // Create a vertex buffer with our image plane vertex data.
        _image_plane_vertex_buffer = [_device newBufferWithBytes:&kImagePlaneVertexData length:sizeof(kImagePlaneVertexData) options:MTLResourceCPUCacheModeDefaultCache];
        _image_plane_vertex_buffer.label = @"image_plane_vertex_buffer";
        
        
        
        
        id <MTLFunction> capturedImageVertexFunction = [default_shader_library newFunctionWithName:@"capturedImageVertexTransform"];
        id <MTLFunction> capturedImageFragmentFunction = [default_shader_library newFunctionWithName:@"capturedImageFragmentShader"];
        
        // Create a vertex descriptor for our image plane vertex buffer
        MTLVertexDescriptor *image_planeVertexDescriptor = [[MTLVertexDescriptor alloc] init];
        // Positions.
        image_planeVertexDescriptor.attributes[kVertexAttributePosition].format = MTLVertexFormatFloat2;
        image_planeVertexDescriptor.attributes[kVertexAttributePosition].offset = 0;
        image_planeVertexDescriptor.attributes[kVertexAttributePosition].bufferIndex = kBufferIndexMeshPositions;
        
        // Texture coordinates.
        image_planeVertexDescriptor.attributes[kVertexAttributeTexcoord].format = MTLVertexFormatFloat2;
        image_planeVertexDescriptor.attributes[kVertexAttributeTexcoord].offset = 8;
        image_planeVertexDescriptor.attributes[kVertexAttributeTexcoord].bufferIndex = kBufferIndexMeshPositions;
        
        // Position Buffer Layout
        image_planeVertexDescriptor.layouts[kBufferIndexMeshPositions].stride = 16;
        image_planeVertexDescriptor.layouts[kBufferIndexMeshPositions].stepRate = 1;
        image_planeVertexDescriptor.layouts[kBufferIndexMeshPositions].stepFunction = MTLVertexStepFunctionPerVertex;
        
        // Create a pipeline state for rendering the captured image
        MTLRenderPipelineDescriptor *captured_image_PipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        captured_image_PipelineStateDescriptor.label = @"CapturedImagePipeline";
        //captured_image_PipelineStateDescriptor.sampleCount = view.sampleCount;
        captured_image_PipelineStateDescriptor.rasterSampleCount = view.sampleCount;
        captured_image_PipelineStateDescriptor.vertexFunction = capturedImageVertexFunction;
        captured_image_PipelineStateDescriptor.fragmentFunction = capturedImageFragmentFunction;
        captured_image_PipelineStateDescriptor.vertexDescriptor = image_planeVertexDescriptor;
        captured_image_PipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
        captured_image_PipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
        captured_image_PipelineStateDescriptor.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;
        
        
        
        _captured_image_pipeline_state = [self pipeline_make:captured_image_PipelineStateDescriptor];
        
        MTLDepthStencilDescriptor *capturedImageDepthStateDescriptor = [[MTLDepthStencilDescriptor alloc] init];
        capturedImageDepthStateDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
        capturedImageDepthStateDescriptor.depthWriteEnabled    = NO;
        _captured_image_depth_state = [_device newDepthStencilStateWithDescriptor:capturedImageDepthStateDescriptor];
        
        // Create captured image texture cache
        CVMetalTextureCacheCreate(NULL, NULL, _device, NULL, &_captured_image_texture_cache);
#endif
        
        // MARK: - Geometry

        mtt::init(&self->vertex_buffer_list, self->_id_allocator);
        mtt::init(&self->index_buffer_list, self->_id_allocator);
        
        // MARK: - Shader Initialization for Geometry
        id <MTLFunction> polygon_color_vertex_function = [default_shader_library newFunctionWithName:@"vertex_polygon_color"];
        id <MTLFunction> polygon_color_fragment_function = [default_shader_library newFunctionWithName:@"fragment_polygon_color"];
        id <MTLFunction> polygon_color_fragment_hsv_function = [default_shader_library newFunctionWithName:@"fragment_polygon_color_hsv"];
        
        // Create a vertex descriptor for our image plane vertex buffer
        MTLVertexDescriptor *polygon_color_VertexDescriptor = [[MTLVertexDescriptor alloc] init];
        // Positions.
        polygon_color_VertexDescriptor.attributes[kVertexAttributePosition].format = MTLVertexFormatFloat4;
        polygon_color_VertexDescriptor.attributes[kVertexAttributePosition].offset = 0;
        polygon_color_VertexDescriptor.attributes[kVertexAttributePosition].bufferIndex = kBufferIndexMeshPositions;
        
        // Colors.
        polygon_color_VertexDescriptor.attributes[kVertexAttributeColor].format = MTLVertexFormatFloat4;
        polygon_color_VertexDescriptor.attributes[kVertexAttributeColor].offset = 16;
        polygon_color_VertexDescriptor.attributes[kVertexAttributeColor].bufferIndex = kBufferIndexMeshPositions;
        
        // Texture coordinates.
        polygon_color_VertexDescriptor.attributes[kVertexAttributeTexcoord].format = MTLVertexFormatFloat2;
        polygon_color_VertexDescriptor.attributes[kVertexAttributeTexcoord].offset = 32;
        polygon_color_VertexDescriptor.attributes[kVertexAttributeTexcoord].bufferIndex = kBufferIndexMeshPositions;
        
        // Position Buffer Layout
        polygon_color_VertexDescriptor.layouts[kBufferIndexMeshPositions].stride = 48;
        polygon_color_VertexDescriptor.layouts[kBufferIndexMeshPositions].stepRate = 1;
        polygon_color_VertexDescriptor.layouts[kBufferIndexMeshPositions].stepFunction = MTLVertexStepFunctionPerVertex;
        
        
        MTLRenderPipelineDescriptor *polygon_color_PipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        polygon_color_PipelineStateDescriptor.label = @"polygon_color_pipeline";
        polygon_color_PipelineStateDescriptor.rasterSampleCount = view.sampleCount;
        polygon_color_PipelineStateDescriptor.vertexFunction = polygon_color_vertex_function;
        polygon_color_PipelineStateDescriptor.fragmentFunction = polygon_color_fragment_function;
        polygon_color_PipelineStateDescriptor.vertexDescriptor = polygon_color_VertexDescriptor;
        polygon_color_PipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
        polygon_color_PipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
        polygon_color_PipelineStateDescriptor.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;
        // Enable blending / transparency
        MTLRenderPipelineColorAttachmentDescriptor *renderbufferAttachment = polygon_color_PipelineStateDescriptor.colorAttachments[0];
        renderbufferAttachment.rgbBlendOperation = MTLBlendOperationAdd;
        renderbufferAttachment.alphaBlendOperation = MTLBlendOperationAdd;
        renderbufferAttachment.sourceRGBBlendFactor = MTLBlendFactorOne;
        renderbufferAttachment.sourceAlphaBlendFactor = MTLBlendFactorOne;
        renderbufferAttachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        renderbufferAttachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        renderbufferAttachment.blendingEnabled = YES;
        

        
        
        
        polygon_color_PipelineStateDescriptor.label = @"polygon_color_pipeline";
        _polygon_color_pipeline_state = [self pipeline_make:polygon_color_PipelineStateDescriptor];
        _pipelines.set_slot(sd::PIPELINE_MODE_COLOR, ((id)_polygon_color_pipeline_state));
        
        {
            polygon_color_PipelineStateDescriptor.depthAttachmentPixelFormat = MTLPixelFormatInvalid;
            polygon_color_PipelineStateDescriptor.stencilAttachmentPixelFormat = MTLPixelFormatInvalid;
            
            polygon_color_PipelineStateDescriptor.label = @"polygon_color_no_depth_stencil_pipeline";
            
            _polygon_color_no_depth_stencil_pipeline_state = [self pipeline_make:polygon_color_PipelineStateDescriptor];
    
            _pipelines.set_slot(sd::PIPELINE_MODE_COLOR_NO_DEPTH_STENCIL, ((id)_polygon_color_no_depth_stencil_pipeline_state));
            
            polygon_color_PipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
            polygon_color_PipelineStateDescriptor.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;
        }
        

        
        
        polygon_color_PipelineStateDescriptor.label = @"polygon_color_hsv_pipeline";
        polygon_color_PipelineStateDescriptor.fragmentFunction = polygon_color_fragment_hsv_function;
        _polygon_color_hsv_pipeline_state = [self pipeline_make:polygon_color_PipelineStateDescriptor];

        _pipelines.set_slot(sd::PIPELINE_MODE_COLOR_HSV, (id)_polygon_color_hsv_pipeline_state);
        
        MTLRenderPipelineDescriptor *additive_PipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        { // additive versions
            
            {
                additive_PipelineStateDescriptor.rasterSampleCount = view.sampleCount;
                additive_PipelineStateDescriptor.vertexFunction = polygon_color_vertex_function;
                additive_PipelineStateDescriptor.fragmentFunction = polygon_color_fragment_function;
                additive_PipelineStateDescriptor.vertexDescriptor = polygon_color_VertexDescriptor;
                additive_PipelineStateDescriptor.colorAttachments[0].pixelFormat = view.colorPixelFormat;
                additive_PipelineStateDescriptor.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
                additive_PipelineStateDescriptor.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;
                
                MTLRenderPipelineColorAttachmentDescriptor *renderbufferAttachment_additive = additive_PipelineStateDescriptor.colorAttachments[0];
                renderbufferAttachment_additive.rgbBlendOperation = MTLBlendOperationAdd;
                renderbufferAttachment_additive.alphaBlendOperation = MTLBlendOperationAdd;
//                renderbufferAttachment_additive.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
                renderbufferAttachment_additive.sourceRGBBlendFactor = MTLBlendFactorOne;
                renderbufferAttachment_additive.sourceAlphaBlendFactor = MTLBlendFactorOne;
                renderbufferAttachment_additive.destinationRGBBlendFactor = MTLBlendFactorOne;
                renderbufferAttachment_additive.destinationAlphaBlendFactor = MTLBlendFactorOne;
                renderbufferAttachment_additive.blendingEnabled = YES;
            }
            
            additive_PipelineStateDescriptor.label = @"polygon_color_additive_pipeline";
            _polygon_color_additive_pipeline_state = [self pipeline_make:additive_PipelineStateDescriptor];
            _pipelines.set_slot(sd::PIPELINE_MODE_COLOR_ADDITIVE, ((id)_polygon_color_additive_pipeline_state));
            
            
            additive_PipelineStateDescriptor.label = @"polygon_color_hsv_additive_pipeline";
            additive_PipelineStateDescriptor.fragmentFunction = polygon_color_fragment_hsv_function;
            _polygon_color_hsv_additive_pipeline_state = [self pipeline_make:additive_PipelineStateDescriptor];
            _pipelines.set_slot(sd::PIPELINE_MODE_COLOR_HSV_ADDITIVE, (id)_polygon_color_hsv_additive_pipeline_state);
        }

        
        {
            MTLDepthStencilDescriptor *polygon_colorDepthStateDescriptor = [[MTLDepthStencilDescriptor alloc] init];
            polygon_colorDepthStateDescriptor.depthCompareFunction = MTLCompareFunctionLessEqual;
            polygon_colorDepthStateDescriptor.depthWriteEnabled    = YES;
            
            _things_depth_stencil_state = [_device newDepthStencilStateWithDescriptor:polygon_colorDepthStateDescriptor];
            
            polygon_colorDepthStateDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
            polygon_colorDepthStateDescriptor.depthWriteEnabled    = NO;
            _things_depth_stencil_state_no_depth_write = [_device newDepthStencilStateWithDescriptor:polygon_colorDepthStateDescriptor];
        }
        // with stencil
        {
            MTLDepthStencilDescriptor *polygon_colorDepthStateDescriptor = [[MTLDepthStencilDescriptor alloc] init];
            polygon_colorDepthStateDescriptor.depthCompareFunction = MTLCompareFunctionLessEqual;
            polygon_colorDepthStateDescriptor.depthWriteEnabled    = YES;
            polygon_colorDepthStateDescriptor.backFaceStencil.depthStencilPassOperation = MTLStencilOperationReplace;
            polygon_colorDepthStateDescriptor.frontFaceStencil.depthStencilPassOperation = MTLStencilOperationReplace;
            
            _things_depth_stencil_state_w_stencil = [_device newDepthStencilStateWithDescriptor:polygon_colorDepthStateDescriptor];
            
            polygon_colorDepthStateDescriptor.depthCompareFunction = MTLCompareFunctionAlways;
            polygon_colorDepthStateDescriptor.depthWriteEnabled    = NO;
            _things_depth_stencil_state_no_depth_write_stencil = [_device newDepthStencilStateWithDescriptor:polygon_colorDepthStateDescriptor];
        }
        
        
        // MARK: - textures
        
        // MARK:  texture shader
        
        
        
        // MARK: argument buffer
        
        switch (_device.argumentBuffersSupport) {
        case MTLArgumentBuffersTier1: {
            NSLog(@"Metal argument buffer support tier: %@\n", @"tier 1");
            break;
        }
        case MTLArgumentBuffersTier2: {
            NSLog(@"Metal argument buffer support tier: %@\n", @"tier 2");
            break;
        }
        }
        
        
        {
            MTLSamplerDescriptor *samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
            samplerDescriptor.supportArgumentBuffers = YES;
            {
                samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
                samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
                samplerDescriptor.normalizedCoordinates = YES;
                samplerDescriptor.sAddressMode = MTLSamplerAddressModeClampToEdge;
                samplerDescriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
                self->sampler_clamp_to_edge = [_device newSamplerStateWithDescriptor:samplerDescriptor];
                samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
                samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
                self->sampler_clamp_to_edge_linear = [_device newSamplerStateWithDescriptor:samplerDescriptor];
            }
            {
                samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
                samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
                samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
                samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
                self->sampler_repeat = [_device newSamplerStateWithDescriptor:samplerDescriptor];
            }
            {
                samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
                samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
                samplerDescriptor.sAddressMode = MTLSamplerAddressModeMirrorRepeat;
                samplerDescriptor.tAddressMode = MTLSamplerAddressModeMirrorRepeat;
                self->sampler_mirror_repeat = [_device newSamplerStateWithDescriptor:samplerDescriptor];
            }
            {
                samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
                samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
                samplerDescriptor.sAddressMode = MTLSamplerAddressModeMirrorClampToEdge;
                samplerDescriptor.tAddressMode = MTLSamplerAddressModeMirrorClampToEdge;
                self->sampler_mirror_clamp_to_edge = [_device newSamplerStateWithDescriptor:samplerDescriptor];
            }
            self->sampler_default = sampler_clamp_to_edge;
            
            NSError* texture_loader_error = nil;
            // default metalkit texture loader
            _texture_loader = [[MTKTextureLoader alloc] initWithDevice:_device];
            self->image_loader.backend = (__bridge void*)_texture_loader;
            
            NSDictionary* texture_loader_options = @{MTKTextureLoaderOptionGenerateMipmaps: @(YES)};
            
            if (supports_metal_3) {
                max_resident_textures = 4096;
                Images_init(&_core_platform->renderer, &self->images, max_resident_textures, Images_Init_Args { .support_bindless = true, .default_pixel_format = _view.colorPixelFormat, .default_pixel_format_depth_stencil = _view.depthStencilPixelFormat });
                
                Image_Loader_init(self->renderer, &self->image_loader, &self->images, Image_Loader_Init_Args { });
                
                {
                    id<MTLFunction> polygon_color_texture_vertex_function = [default_shader_library newFunctionWithName:@"vertex_polygon_color_texture"];
                    CHECK_SHADER_FUNCTION_NONNULL(polygon_color_texture_vertex_function, true);
                    polygon_color_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                    
                    id<MTLFunction> polygon_color_texture_fragment_function = [default_shader_library newFunctionWithName:@"fragment_polygon_color_texture_with_arg_buffer_MTL3_bindless"];
                    CHECK_SHADER_FUNCTION_NONNULL(polygon_color_texture_fragment_function, true);
                    polygon_color_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                    
                    polygon_color_PipelineStateDescriptor.label = @"polygon_color_texture_bindless_pipeline";
                    _polygon_color_texture_pipeline_state = [self pipeline_make:polygon_color_PipelineStateDescriptor];
                    _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE, (id)_polygon_color_texture_pipeline_state);

                    additive_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                    additive_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                    additive_PipelineStateDescriptor.label = @"polygon_color_texture_bindless_additive_pipeline";
                    _polygon_color_texture_additive_pipeline_state = [self pipeline_make:additive_PipelineStateDescriptor];
                    _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE_ADDITIVE, (id)_polygon_color_texture_additive_pipeline_state);

                    
                    _scene_data = [_device newBufferWithLength:sizeof(Fragment_Shader_Arguments_MTL3) options:MTLResourceStorageModeShared];
                    Fragment_Shader_Arguments_MTL3* args = (Fragment_Shader_Arguments_MTL3*)_scene_data.contents;
                    id<MTLBuffer> resources = [_device newBufferWithLength:sizeof(Fragment_Shader_Arguments_Resources) * max_resident_textures options:MTLResourceStorageModeShared];
                    _scene_resource_data = resources;
                    *args = {
                        .textures = (Fragment_Shader_Arguments_Resources*)resources.gpuAddress,
                        .samplers = {
                            [Texture_Sampler_ID_CLAMP_TO_EDGE] = sampler_clamp_to_edge.gpuResourceID,
                            [Texture_Sampler_ID_REPEAT] = sampler_repeat.gpuResourceID,
                            [Texture_Sampler_ID_MIRROR_REPEAT] = sampler_mirror_repeat.gpuResourceID,
                            [Texture_Sampler_ID_MIRROR_CLAMP_TO_EDGE] = sampler_mirror_clamp_to_edge.gpuResourceID,
                            
                            [Texture_Sampler_ID_CLAMP_TO_EDGE_LINEAR] = sampler_clamp_to_edge_linear.gpuResourceID,
//                            [Texture_Sampler_ID_REPEAT_LINEAR] = sampler_repeat.gpuResourceID,
//                            [Texture_Sampler_ID_MIRROR_REPEAT_LINEAR] = sampler_mirror_repeat.gpuResourceID,
//                            [Texture_Sampler_ID_MIRROR_CLAMP_TO_EDGE_LINEAR] = sampler_mirror_clamp_to_edge.gpuResourceID,
                        }
                    };
                    
                }
                
                [self load_default_textures:0
                                       options:YES];

            }
            else if ([_device supportsFamily:MTLGPUFamilyMac2]) {
                NSLog(@"supports MTLGPUFamilyMac2");
                
                {
                    max_resident_textures = MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_MAC2;
                    
                    Images_init(&_core_platform->renderer, &self->images, max_resident_textures);
                    
                    {
                        id<MTLFunction> polygon_color_texture_vertex_function = [default_shader_library newFunctionWithName:@"vertex_polygon_color_texture"];
                        CHECK_SHADER_FUNCTION_NONNULL(polygon_color_texture_vertex_function, true);
                        polygon_color_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        
                        id<MTLFunction> polygon_color_texture_fragment_function = [default_shader_library newFunctionWithName:@"fragment_polygon_color_texture_f6_or_7"];
                        CHECK_SHADER_FUNCTION_NONNULL(polygon_color_texture_fragment_function, true);
                        polygon_color_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        
                        polygon_color_PipelineStateDescriptor.label = @"polygon_color_texture_pipeline";
                        _polygon_color_texture_pipeline_state = [_device newRenderPipelineStateWithDescriptor:polygon_color_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        
                        
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE, (id)_polygon_color_texture_pipeline_state);
                        
                        additive_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        additive_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        additive_PipelineStateDescriptor.label = @"polygon_color_texture_bindless_additive_pipeline";
                        _polygon_color_texture_additive_pipeline_state = [_device newRenderPipelineStateWithDescriptor:additive_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture additive pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE_ADDITIVE, (id)_polygon_color_texture_additive_pipeline_state);
                    }
                    
                    Image_Loader_init(self->renderer, &self->image_loader, &self->images);
                    
                    
                    [self load_default_textures:0 options:YES];
                }
            
//                if ([_device supportsFamily:MTLGPUFamilyMacCatalyst1]) {
//                    NSLog(@"supports MTLGPUFamilyMacCatalyst1");
//                }
//                if ([_device supportsFamily:MTLGPUFamilyMacCatalyst2]) {
//                    NSLog(@"supports MTLGPUFamilyMacCatalyst2");
//                }
            } else {
            
            // load default textures
            
                if ([_device supportsFamily:MTLGPUFamilyApple7]) {
                    NSLog(@"supports MTLGPUFamilyApple7");
                    
                    max_resident_textures = MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_6_OR_7;
                    
                    Images_init(&_core_platform->renderer, &self->images, max_resident_textures);
                    
                    {
                        id<MTLFunction> polygon_color_texture_vertex_function = [default_shader_library newFunctionWithName:@"vertex_polygon_color_texture"];
                        
                        polygon_color_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        
                        id<MTLFunction> polygon_color_texture_fragment_function = [default_shader_library newFunctionWithName:@"fragment_polygon_color_texture_f6_or_7"];
                        polygon_color_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        
                        polygon_color_PipelineStateDescriptor.label = @"polygon_color_texture_pipeline";
                        _polygon_color_texture_pipeline_state = [_device newRenderPipelineStateWithDescriptor:polygon_color_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        
                        
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE, (id)_polygon_color_texture_pipeline_state);
                        
                        additive_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        additive_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        additive_PipelineStateDescriptor.label = @"polygon_color_texture_bindless_additive_pipeline";
                        _polygon_color_texture_additive_pipeline_state = [_device newRenderPipelineStateWithDescriptor:additive_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture additive pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE_ADDITIVE, (id)_polygon_color_texture_additive_pipeline_state);
                    }
                    
                    Image_Loader_init(self->renderer, &self->image_loader, &self->images);
                    
                    
                    [self load_default_textures:0 options:YES];
                    
                } else if ([_device supportsFamily:MTLGPUFamilyApple6]) {
                    NSLog(@"supports MTLGPUFamilyApple6");
                    
                    max_resident_textures = MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_6_OR_7;
                    
                    Images_init(self->renderer, &self->images, max_resident_textures);
                    
                    {
                        id<MTLFunction> polygon_color_texture_vertex_function = [default_shader_library newFunctionWithName:@"vertex_polygon_color_texture"];
                        
                        polygon_color_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        
                        id<MTLFunction> polygon_color_texture_fragment_function = [default_shader_library newFunctionWithName:@"fragment_polygon_color_texture_f6_or_7"];
                        polygon_color_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        
                        polygon_color_PipelineStateDescriptor.label = @"polygon_color_texture_pipeline";
                        _polygon_color_texture_pipeline_state = [_device newRenderPipelineStateWithDescriptor:polygon_color_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        
                        
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE, (id)_polygon_color_texture_pipeline_state);
                        
                        additive_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        additive_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        additive_PipelineStateDescriptor.label = @"polygon_color_texture_bindless_additive_pipeline";
                        _polygon_color_texture_additive_pipeline_state = [_device newRenderPipelineStateWithDescriptor:additive_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture additive pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE_ADDITIVE, (id)_polygon_color_texture_additive_pipeline_state);
                    }
                    
                    Image_Loader_init(self->renderer, &self->image_loader, &self->images);
                    
                    
                    [self load_default_textures:0 options:YES];
                    
                    
                } else if ([_device supportsFamily:MTLGPUFamilyApple5]) {
                    NSLog(@"supports MTLGPUFamilyApple5");
                    
                    max_resident_textures = MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_5;
                    
                    Images_init(&_core_platform->renderer, &self->images, max_resident_textures);
                    
                    {
                        id<MTLFunction> polygon_color_texture_vertex_function = [default_shader_library newFunctionWithName:@"vertex_polygon_color_texture"];
                        
                        polygon_color_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        
                        id<MTLFunction> polygon_color_texture_fragment_function = [default_shader_library newFunctionWithName:@"fragment_polygon_color_texture_f5"];
                        polygon_color_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        
                        polygon_color_PipelineStateDescriptor.label = @"polygon_color_texture_pipeline";
                        _polygon_color_texture_pipeline_state = [_device newRenderPipelineStateWithDescriptor:polygon_color_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        
                        
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE, (id)_polygon_color_texture_pipeline_state);
                        
                        additive_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        additive_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        additive_PipelineStateDescriptor.label = @"polygon_color_texture_bindless_additive_pipeline";
                        _polygon_color_texture_additive_pipeline_state = [_device newRenderPipelineStateWithDescriptor:additive_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture additive pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE_ADDITIVE, (id)_polygon_color_texture_additive_pipeline_state);
                    }
                    
                    Image_Loader_init(self->renderer, &self->image_loader, &self->images);
                    
                    
                    [self load_default_textures:0 options:YES];
                } else if ([_device supportsFamily:MTLGPUFamilyApple4]) {
                    NSLog(@"supports MTLGPUFamilyApple4");
                    
                    max_resident_textures = MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_4;
                    
                    Images_init(&_core_platform->renderer, &self->images, max_resident_textures);
                    
                    {
                        id<MTLFunction> polygon_color_texture_vertex_function = [default_shader_library newFunctionWithName:@"vertex_polygon_color_texture"];
                        
                        polygon_color_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        
                        id<MTLFunction> polygon_color_texture_fragment_function = [default_shader_library newFunctionWithName:@"fragment_polygon_color_texture_f5"];
                        polygon_color_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        
                        polygon_color_PipelineStateDescriptor.label = @"polygon_color_texture_pipeline";
                        _polygon_color_texture_pipeline_state = [_device newRenderPipelineStateWithDescriptor:polygon_color_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        
                        
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE, (id)_polygon_color_texture_pipeline_state);
                        
                        additive_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        additive_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        additive_PipelineStateDescriptor.label = @"polygon_color_texture_bindless_additive_pipeline";
                        _polygon_color_texture_additive_pipeline_state = [_device newRenderPipelineStateWithDescriptor:additive_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture additive pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE_ADDITIVE, (id)_polygon_color_texture_additive_pipeline_state);
                    }
                    
                    Image_Loader_init(self->renderer, &self->image_loader, &self->images);
                    
                    
                    [self load_default_textures:0 options:YES];
                } else if ([_device supportsFamily:MTLGPUFamilyApple3]) {
                    NSLog(@"supports MTLGPUFamilyApple3");
                    
                    max_resident_textures = MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_3;
                    
                    Images_init(&_core_platform->renderer, &self->images, max_resident_textures);
                    
                    {
                        id<MTLFunction> polygon_color_texture_vertex_function = [default_shader_library newFunctionWithName:@"vertex_polygon_color_texture"];
                        
                        polygon_color_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        
                        id<MTLFunction> polygon_color_texture_fragment_function = [default_shader_library newFunctionWithName:@"fragment_polygon_color_texture_f3"];
                        polygon_color_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        
                        polygon_color_PipelineStateDescriptor.label = @"polygon_color_texture_pipeline";
                        _polygon_color_texture_pipeline_state = [_device newRenderPipelineStateWithDescriptor:polygon_color_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        
                        
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE, (id)_polygon_color_texture_pipeline_state);
                        
                        additive_PipelineStateDescriptor.vertexFunction = polygon_color_texture_vertex_function;
                        additive_PipelineStateDescriptor.fragmentFunction = polygon_color_texture_fragment_function;
                        additive_PipelineStateDescriptor.label = @"polygon_color_texture_bindless_additive_pipeline";
                        _polygon_color_texture_additive_pipeline_state = [_device newRenderPipelineStateWithDescriptor:additive_PipelineStateDescriptor error:&error];
                        if (error) {
                            NSLog(@"Failed to create polygon color texture additive pipeline state, error %@", error);
                        } else {
                            NSLog(@"PIPELINE SUCCESS!!!! %@", _polygon_color_texture_pipeline_state.label);
                        }
                        _pipelines.set_slot(sd::PIPELINE_MODE_TEXTURE_ADDITIVE, (id)_polygon_color_texture_additive_pipeline_state);
                    }
                    
                    Image_Loader_init(self->renderer, &self->image_loader, &self->images);
                    
                    
                    [self load_default_textures:0 options:YES];
                }
            }
            

        }
        
        // MARK: - Render to texture support
        {
            MTLRenderPassDescriptor* _renderToTextureRenderPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
            
            
            
            _renderToTextureRenderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            
            // set when used:
            _renderToTextureRenderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0, 0, 0, 0);
            
            _renderToTextureRenderPassDescriptor.colorAttachments[0].storeAction = (USE_MULTISAMPLING) ? MTLStoreActionMultisampleResolve :
            MTLStoreActionStore;
            
            // must fill-in later
//            {
//                _renderToTextureRenderPassDescriptor.colorAttachments[0].texture = _offscreenTexture;
//                _renderToTextureRenderPassDescriptor.colorAttachments[0].resolveTexture = _renderTargetTexture;
//                _renderToTextureRenderPassDescriptor.depthAttachment = depth;
//                _renderToTextureRenderPassDescriptor.stencilAttachment = stencil;
//            }
            
            self->new_render_target._renderToTextureRenderPassDescriptor = _renderToTextureRenderPassDescriptor;
            
            
            self->offscreen_renderpass_descriptor = [[MTLRenderPassDescriptor alloc] init];
            offscreen_renderpass_descriptor.colorAttachments[0].clearColor = MTLClearColorMake(0,0,0,0);
            offscreen_renderpass_descriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            offscreen_renderpass_descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            offscreen_renderpass_descriptor.colorAttachments[0].texture = nil;
            
            if (USE_MULTISAMPLING) {
                self->offscreen_renderpass_descriptor_msaa = [[MTLRenderPassDescriptor alloc] init];
                offscreen_renderpass_descriptor_msaa.colorAttachments[0].clearColor = MTLClearColorMake(0,0,0,0);
                offscreen_renderpass_descriptor_msaa.colorAttachments[0].loadAction = MTLLoadActionClear;
                offscreen_renderpass_descriptor_msaa.colorAttachments[0].storeAction = MTLStoreActionMultisampleResolve;
                offscreen_renderpass_descriptor_msaa.depthAttachment.clearDepth = 1.0;
                offscreen_renderpass_descriptor_msaa.depthAttachment.texture = nil;
                offscreen_renderpass_descriptor_msaa.depthAttachment.resolveTexture = nil;
                offscreen_renderpass_descriptor_msaa.depthAttachment.loadAction = MTLLoadActionClear;
                offscreen_renderpass_descriptor_msaa.depthAttachment.storeAction = MTLStoreActionMultisampleResolve;
                offscreen_renderpass_descriptor_msaa.stencilAttachment.loadAction = MTLLoadActionClear;
                offscreen_renderpass_descriptor_msaa.stencilAttachment.storeAction = MTLStoreActionMultisampleResolve;
                
            } else {
                offscreen_renderpass_descriptor_msaa = nil;
            }
        }
        

        
        {
            CAMetalLayer* mtl_layer = (CAMetalLayer*)_view.layer;
            MNVGargs mnvgargs = {};
            mnvgargs.command_queue = (__bridge void*)_command_queue;
            mnvgargs.handle_externally = 1;
            mnvgargs.sample_count = (usize32)SAMPLE_COUNT;
            _nvg_ctx = nvgCreateMTL((__bridge void*)mtl_layer, /* NVG_ANTIALIAS | */ /*NVG_STENCIL_STROKES | NVG_TRIPLE_BUFFER | */0, &mnvgargs);
            assert(_nvg_ctx != NULL);
            
            nvgSetGlobalContext(_nvg_ctx);
            
            
            NSString *resources_directory = [[NSBundle mainBundle] resourcePath];
            
            NSString* font_regular = [resources_directory stringByAppendingString:@"/SF-Pro-Display-Regular.otf"];
            const char* const font_regular_path = [font_regular cStringUsingEncoding:NSUTF8StringEncoding];
            auto fontNormal =
            nvgCreateFont(_nvg_ctx, "sans", font_regular_path);
            assert(fontNormal != -1);
            
            NSString* font_bold = [resources_directory stringByAppendingString:@"/SF-Pro-Display-Bold.otf"];
            const char* const font_bold_path = [font_bold cStringUsingEncoding:NSUTF8StringEncoding];
            auto fontBold = nvgCreateFont(_nvg_ctx, "sans-bold", font_bold_path);
            assert(fontBold != -1);
            
            NSString* font_mono = [resources_directory stringByAppendingString:@"/SF-Mono-Regular.otf"];
            const char* const font_mono_path = [font_mono cStringUsingEncoding:NSUTF8StringEncoding];
            auto fontMono = nvgCreateFont(_nvg_ctx, "sans-mono", font_mono_path);
            assert(fontMono != -1);
            
            NSString* font_emoji = [resources_directory stringByAppendingString:@"/NotoEmoji-Regular.ttf"];
            const char* const font_emoji_path = [font_emoji cStringUsingEncoding:NSUTF8StringEncoding];
            auto fontEmoji = nvgCreateFont(_nvg_ctx, "emoji", font_emoji_path);
            assert(fontEmoji
                   != -1);
            nvgAddFallbackFontId(_nvg_ctx, fontNormal, fontEmoji);
            nvgAddFallbackFontId(_nvg_ctx, fontBold, fontEmoji);
            
        }
        
        //    return;
        //
        //#pragma mark Config Compression Session
        //    /// the weird objc way of creating attributes for the texture format
        //    _defaultAttributes = @{ (id)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_32BGRA) };
        //
        //    /// creates the compression session which we use to compress the frames from Metal
        //    VTCompressionSessionCreate(kCFAllocatorMalloc,
        //                               view.frame.size.width,
        //                               view.frame.size.height,
        //                               kCMVideoCodecType_H264,
        //                               NULL,
        //                               (__bridge CFDictionaryRef _Nullable)(_defaultAttributes),
        //                               kCFAllocatorMalloc,
        //                               CompressionCallback,
        //                               (__bridge void * _Nullable)(self),
        //                               &_compression_session);
        //
        //    /// unsure wether we really need this here or not did find it's slower to use the dispatch qeue
        //    ///_encoderQueue = dispatch_queue_create("com.platin21.encoderQeueu", NULL);
        //
        //#pragma mark Frame Capture Buffer Creation
        //    /// set the intial index for the buffers
        //    _buffer_idx = 0;
        //    for(int i = 0; i < 3; i += 1) {
        //      /// create buffers for storing the screen image
        //       _capture_buffers[i] = [_device newBufferWithLength: view.frame.size.width * view.frame.size.height * 4
        //                                                 options: MTLResourceStorageModeShared];
        //    }
        //
        //#pragma mark Depth Stencil Creation
        //    /// depth stencil stuff not used
        //    view.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
        //    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm;
        //    view.sampleCount = 1;
        //
        //#pragma mark Standard Metal Initialization
        //
        //  /// create buffer for texture coordinates
        //    _textureCoordinates = [_device newBufferWithLength: sizeof(Uniforms) options: MTLResourceStorageModeShared];
        //
        //    /// standard texture coordinates
        //    const float textureData[] = {
        //        1.0, 0.0,
        //        0.0, 0.0,
        //        1.0, 1.0,
        //        0.0, 1.0,
        //    };
        //
        //    /// get contents of buffer and copy texture coordinates into it.
        //    Uniforms* transformData = (Uniforms*)[_textureCoordinates contents];
        //    memcpy(&transformData->textureCoordinates[0], &textureData[0], sizeof(textureData));
        //
        //
        //    /// get the compiled shader library that is in the bundle when xcode compiles it creates this .metallib file which is loaded here
        //    id <MTLLibrary> defaultLibrary = [_device newDefaultLibrary];
        //
        //    /// create vertex shader for image drawing
        //    id <MTLFunction> arImageVertexShader   = [defaultLibrary newFunctionWithName:@"arImageVertexShader"];
        //    /// create fragment shader for image drawing
        //    id <MTLFunction> arImageFragmentShader = [defaultLibrary newFunctionWithName:@"arImageFragmentShader"];
        //
        //    /// create a render pipline state descriptor,
        //    /// used for defining attachments, vertex data layout needs to be supplied
        //    /// unnecessary for uniform data
        //    MTLRenderPipelineDescriptor* pipStateDesc = [MTLRenderPipelineDescriptor new];
        //    pipStateDesc.label = @"ArImagePipeline";
        //    pipStateDesc.sampleCount = view.sampleCount;
        //    pipStateDesc.vertexFunction = arImageVertexShader;
        //    pipStateDesc.fragmentFunction = arImageFragmentShader;
        //    pipStateDesc.colorAttachments[0].pixelFormat = view.colorPixelFormat;
        //    pipStateDesc.depthAttachmentPixelFormat = view.depthStencilPixelFormat;
        //    pipStateDesc.stencilAttachmentPixelFormat = view.depthStencilPixelFormat;
        //
        //    /// validate the pipline state
        //    NSError* error = NULL;
        //    _captured_image_pipeline_state = [_device newRenderPipelineStateWithDescriptor:pipStateDesc error: &error];
        //    if (!_captured_image_pipeline_state) {
        //        NSLog(@"Failed to create pipeline state, error %@", error);
        //    }
        //
        //    /// create a depth stencil buffer - unused, but left as an example
        //    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
        //    depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
        //    depthStateDesc.depthWriteEnabled = NO;
        //    _captured_image_depth_state = [_device newDepthStencilStateWithDescriptor:depthStateDesc];
        //
        //    _command_queue = [_device newCommandQueue];
        //
        //#pragma mark Texture Cache for Camera
        //
        //    /// create a texture cache will make the copy of the textures faster
        //    CVMetalTextureCacheCreate(kCFAllocatorMalloc,
        //                              NULL,
        //                              _device,
        //                              NULL,
        //                              &_captured_image_texture_cache);
        //
        //}
        //
        //- (CVMetalTextureRef)_createTextureRefFromPixelBuffer:(CVPixelBufferRef)pixelBuffer andPlane:(NSInteger)planeIndex {
        //
        //    /// get the width an height of the pixelBuffer
        //    const size_t width = CVPixelBufferGetWidthOfPlane(pixelBuffer, planeIndex);
        //    const size_t height = CVPixelBufferGetHeightOfPlane(pixelBuffer, planeIndex);
        //
        //    /// this is the real return value
        //    CVMetalTextureRef mtlTextureRef = nil;
        //
        //    /// create CVMetalTexture is basically a metal texture from a CVPixelBuffer
        //    CVReturn status = CVMetalTextureCacheCreateTextureFromImage(NULL,
        //                                                                _captured_image_texture_cache,
        //                                                                pixelBuffer,
        //                                                                NULL,
        //                                                                planeIndex == 0 ? MTLPixelFormatR8Unorm : MTLPixelFormatRG8Unorm,
        //                                                                width,
        //                                                                height,
        //                                                                planeIndex,
        //                                                                &mtlTextureRef);
        //    /// check if the status was a sucess if not set the thing to nil and still return
        //    if (status != kCVReturnSuccess) {
        //        CVBufferRelease(mtlTextureRef);
        //        mtlTextureRef = nil;
        //    }
        //
        //    return mtlTextureRef;
    }
}

- (void)_handle_ARKit_and_transform
{
    //    /// get the current frame from arkit
    //    ARFrame* currentFrame = [_session currentFrame];
    //    if(!currentFrame) {
    //      dispatch_semaphore_signal(_in_flight_semaphore);
    //      return;
    //    }
    //
    //    /// get the current image from the ARFrame
    //    CVPixelBufferRef pxlBuffer = currentFrame.capturedImage;
    //
    //    /// the camera always has two planes else we deal with a exotic hardware
    //    if (CVPixelBufferGetPlaneCount(pxlBuffer) < 2) {
    //        return;
    //    }
    //
    //    /// relase the old textures
    //    CVBufferRelease(_captured_image_texture_y_ref);
    //    CVBufferRelease(_captured_image_texture_cbcr_ref);
    //
    //    /// call doTranform which does the image transform
    //    [self doTransformFromRealCameraToDisplay:currentFrame];
    //
    //    /// create a texture with y and cbrc components from the camera image
    //    _captured_image_texture_y_ref    = [self _createTextureRefFromPixelBuffer: pxlBuffer andPlane: 0];
    //    _captured_image_texture_cbcr_ref = [self _createTextureRefFromPixelBuffer: pxlBuffer andPlane: 1];
}

#if (MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP)
- (void)doTransformFromRealCameraToDisplay:(nonnull ARFrame*)frame {
    
    /// get the affine transform for the camera from the ARFrame
    CGAffineTransform displayToCameraTransform = CGAffineTransformInvert([frame displayTransformForOrientation:orientation viewportSize: _viewport_size]);
    
    /// get the pointer to our fake uniform data this might actually not need to be updated every frame
    Uniforms* transformData = (Uniforms*)[_textureCoordinates contents];
    for (NSInteger index = 0; index < 4; index += 2) {
        
        // create a cg point for applying the transform
        CGPoint textureCoord = CGPointMake(transformData->textureCoordinates[index],
                                           transformData->textureCoordinates[index + 1]);
        
        /// transform texture coordinates
        CGPoint transformedCoord = CGPointApplyAffineTransform(textureCoord, displayToCameraTransform);
        
        /// store the texture coordinates back
        transformData->textureCoordinates[index]     = transformedCoord.x;
        transformData->textureCoordinates[index + 1] = transformedCoord.y;
    }
}
#endif


- (CVMetalTextureRef)_createTextureFromPixelBuffer:(CVPixelBufferRef)pixelBuffer pixelFormat:(MTLPixelFormat)pixelFormat planeIndex:(NSInteger)planeIndex {
    
    const size_t width = CVPixelBufferGetWidthOfPlane(pixelBuffer, planeIndex);
    const size_t height = CVPixelBufferGetHeightOfPlane(pixelBuffer, planeIndex);
    
    CVMetalTextureRef mtlTextureRef = nil;
    CVReturn status = CVMetalTextureCacheCreateTextureFromImage(NULL, _captured_image_texture_cache, pixelBuffer, NULL, pixelFormat, width, height, planeIndex, &mtlTextureRef);
    if (status != kCVReturnSuccess) {
        CVBufferRelease(mtlTextureRef);
        mtlTextureRef = nil;
    }
    
    return mtlTextureRef;
}

#if (MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP)
- (void)_updateCapturedImageTexturesWithFrame:(ARFrame *)frame {
    // Create two textures (Y and CbCr) from the provided frame's captured image
    CVPixelBufferRef pixelBuffer = frame.capturedImage;
    
    if (CVPixelBufferGetPlaneCount(pixelBuffer) < 2) {
        return;
    }
    
    CVBufferRelease(_captured_image_texture_y_ref);
    CVBufferRelease(_captured_image_texture_cbcr_ref);
    _captured_image_texture_y_ref = [self _createTextureFromPixelBuffer:pixelBuffer pixelFormat:MTLPixelFormatR8Unorm planeIndex:0];
    _captured_image_texture_cbcr_ref = [self _createTextureFromPixelBuffer:pixelBuffer pixelFormat:MTLPixelFormatRG8Unorm planeIndex:1];
}
#endif

#if (MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP)
- (void)_updateImagePlaneWithFrame:(ARFrame *)frame {
    // Update the texture coordinates of our image plane to aspect fill the viewport
    CGAffineTransform displayToCameraTransform = CGAffineTransformInvert([frame displayTransformForOrientation:orientation viewportSize:_viewport_size]);
    
    float *vertexData = (float*)[_image_plane_vertex_buffer contents];
    for (NSInteger index = 0; index < 4; index++) {
        NSInteger textureCoordIndex = 4 * index + 2;
        CGPoint textureCoord = CGPointMake(kImagePlaneVertexData[textureCoordIndex], kImagePlaneVertexData[textureCoordIndex + 1]);
        CGPoint transformedCoord = CGPointApplyAffineTransform(textureCoord, displayToCameraTransform);
        vertexData[textureCoordIndex] = transformedCoord.x;
        vertexData[textureCoordIndex + 1] = transformedCoord.y;
    }
}
#endif

- (void)_drawCapturedImageWithCommandEncoder:(id<MTLRenderCommandEncoder>)renderEncoder {
    if (_captured_image_texture_y_ref == nil || _captured_image_texture_cbcr_ref == nil) {
        return;
    }
    
    // Push a debug group allowing us to identify render commands in the GPU Frame Capture tool
    [renderEncoder pushDebugGroup:@"DrawCapturedImage"];
    
    // Set render command encoder state
    [renderEncoder setCullMode:MTLCullModeNone];
    [renderEncoder setRenderPipelineState:_captured_image_pipeline_state];
    [renderEncoder setDepthStencilState:_captured_image_depth_state];
    
    // Set mesh's vertex buffers
    [renderEncoder setVertexBuffer:_image_plane_vertex_buffer offset:0 atIndex:kBufferIndexMeshPositions];
    
    // Set any textures read/sampled from our render pipeline
    [renderEncoder setFragmentTexture:CVMetalTextureGetTexture(_captured_image_texture_y_ref) atIndex:kTextureIndexY];
    [renderEncoder setFragmentTexture:CVMetalTextureGetTexture(_captured_image_texture_cbcr_ref) atIndex:kTextureIndexCbCr];
    
    // Draw each submesh of our mesh
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    
    [renderEncoder popDebugGroup];
    
}

- (void)_updateUniforms {
    _shared_uniform_buffer_offset     = (kAlignedSharedUniformsSize * _buffer_idx);
    _entity_uniform_buffer_offset     = (kAlignedEntityUniformsSize * _buffer_idx);
    _semishared_uniform_buffer_offset = (kAlignedSemisharedUniformsSize * _buffer_idx);
    
    _shared_uniform_buffer_address = ((uint8_t*)_shared_uniform_buffer.contents) + _shared_uniform_buffer_offset;
    
    _entity_uniform_buffer_address = ((uint8_t*)_entity_uniform_buffer.contents) + _entity_uniform_buffer_offset;
    
    _semishared_uniform_buffer_address = ((uint8_t*)_semishared_uniform_buffer.contents) + _semishared_uniform_buffer_offset;
    
    SharedUniforms *uniforms = (SharedUniforms *)_shared_uniform_buffer_address;
    
    
    //mat4 proj = m::ortho(-1.0, 1.0, -1.0, 1.0, 0.0, 1000.0);
    
    
    mat4 proj = m::ortho(0.0, get_bounds(get_main_screen()).size.width,
                           get_bounds(get_main_screen()).size.height, 0.0, -1000.0, 1000.0);
    
    
    
    //    proj = m::perspective(-m::radians(45.0f), -(float)UIScreen.mainScreen.bounds.size.width / (float)UIScreen.mainScreen.bounds.size.height, 0.001f, 1000.0f);
    
    memcpy(&uniforms->projection_matrix, m::value_ptr(proj), sizeof(proj));
    
    uniforms->time_seconds = _curr_time;
    uniforms->aspect_ratio = get_aspect_ratio();
    
}
- (void)_updateSharedUniforms {
    
    _shared_uniform_buffer_offset     = (kAlignedSharedUniformsSize * _buffer_idx);
    _entity_uniform_buffer_offset     = (kAlignedEntityUniformsSize * _buffer_idx);
    _semishared_uniform_buffer_offset = (kAlignedSemisharedUniformsSize * _buffer_idx);
    
    _shared_uniform_buffer_address = ((uint8_t*)_shared_uniform_buffer.contents) + _shared_uniform_buffer_offset;
    
    _entity_uniform_buffer_address = ((uint8_t*)_entity_uniform_buffer.contents) + _entity_uniform_buffer_offset;
    
    _semishared_uniform_buffer_address = ((uint8_t*)_semishared_uniform_buffer.contents) + _semishared_uniform_buffer_offset;
    
    // Update the shared uniforms of the frame
    SharedUniforms *uniforms = (SharedUniforms *)_shared_uniform_buffer_address;
    
    //    mat4 view = sd::get_view(&_core_platform->renderer);
    //
    //    memcpy(&uniforms->viewMatrix, m::value_ptr(view), sizeof(view));
    
    //mat4 proj = m::ortho(-1.0, 1.0, -1.0, 1.0, 0.0, 1000.0);
    mat4 proj = m::ortho(0.0, get_bounds(get_main_screen()).size.width,
                           get_bounds(get_main_screen()).size.height, 0.0, -1000.0, 1000.0);
    
    //    proj = m::perspective(-m::radians(45.0f), -(float)UIScreen.mainScreen.bounds.size.width / (float)UIScreen.mainScreen.bounds.size.height, 0.001f, 1000.0f);
    
    
    memcpy(&uniforms->projection_matrix, m::value_ptr(proj), sizeof(proj));
    
    // Set up lighting for the scene using the ambient intensity if provided
    float ambientIntensity = 1.0;
    
    
    vector_float3 ambientLightColor = { 0.5, 0.5, 0.5 };
    uniforms->ambientLightColor = ambientLightColor * ambientIntensity;
    
    vector_float3 directionalLightDirection = { 0.0, 0.0, -1.0 };
    directionalLightDirection = vector_normalize(directionalLightDirection);
    uniforms->directionalLightDirection = directionalLightDirection;
    
    vector_float3 directionalLightColor = { 0.6, 0.6, 0.6};
    uniforms->directionalLightColor = directionalLightColor * ambientIntensity;
    
    uniforms->materialShininess = 30;
    
    uniforms->time_seconds = _curr_time;
    uniforms->aspect_ratio = get_aspect_ratio();
}

#if (MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP)
- (void)_updateSharedUniformsWithFrame:(ARFrame *)frame {
    // Update the shared uniforms of the frame
    SharedUniforms *uniforms = (SharedUniforms *)_shared_uniform_buffer_address;
    
    uniforms->view_matrix = [frame.camera viewMatrixForOrientation:orientation];
    uniforms->projection_matrix = [frame.camera projectionMatrixForOrientation:orientation viewportSize:_viewport_size zNear:0.001 zFar:1000];
    
    // Set up lighting for the scene using the ambient intensity if provided
    float ambientIntensity = 1.0;
    
    if (frame.lightEstimate) {
        ambientIntensity = frame.lightEstimate.ambientIntensity / 1000;
    }
    
    vector_float3 ambientLightColor = { 0.5, 0.5, 0.5 };
    uniforms->ambientLightColor = ambientLightColor * ambientIntensity;
    
    vector_float3 directionalLightDirection = { 0.0, 0.0, -1.0 };
    directionalLightDirection = vector_normalize(directionalLightDirection);
    uniforms->directionalLightDirection = directionalLightDirection;
    
    vector_float3 directionalLightColor = { 0.6, 0.6, 0.6};
    uniforms->directionalLightColor = directionalLightColor * ambientIntensity;
    
    uniforms->materialShininess = 30;
    
    uniforms->time_seconds = _curr_time;
    uniforms->aspect_ratio = (float64)UIScreen.mainScreen.nativeBounds.size.height / (float64)UIScreen.mainScreen.nativeBounds.size.width;
}
#endif

- (void)update:(nonnull MTKView*)view {
    
#if (LOG_SIGNPOST_ENABLED)
    os_signpost_interval_begin(_logger, _os_signpost_render_update_id, "animation update");
    os_signpost_interval_end(_logger, _os_signpost_render_update_id, "animation wait");
#endif
    // Wait to ensure only kMaxBuffersInFlight are getting proccessed by any stage in the Metal
    //   pipeline (App, Metal, Drivers, GPU, etc)
    dispatch_semaphore_wait(_in_flight_semaphore, DISPATCH_TIME_FOREVER);
    
    // Add completion hander which signal _inFlightSemaphore when Metal and the GPU has fully
    //   finished proccssing the commands we're encoding this frame.  This indicates when the
    //   dynamic buffers, that we're writing to this frame, will no longer be needed by Metal
    //   and the GPU.
    __block dispatch_semaphore_t block_semaphore = _in_flight_semaphore;
    
    main_command_buffer = [_command_queue commandBuffer];
    command_buffer = main_command_buffer;
    command_buffer.label = @"draw_commands";
    // Retain our CVMetalTextureRefs for the duration of the rendering cycle. The MTLTextures
    //   we use from the CVMetalTextureRefs are not valid unless their parent CVMetalTextureRefs
    //   are retained. Since we may release our CVMetalTextureRef ivars during the rendering
    //   cycle, we must retain them separately here.
    
    
#if (USE_ARKIT)
    
    mtt::Augmented_Reality_Context* ar_ctx = &_core_platform->core.ar_ctx;
    const bool ar_is_active = ar_ctx->is_active;
    const bool ar_freeze_frame_when_off = ar_ctx->freeze_frame_when_off;
    
    if (ar_is_active || ar_freeze_frame_when_off) {
        CVBufferRef captured_image_texture_y_ref = CVBufferRetain(_captured_image_texture_y_ref);
        CVBufferRef captured_image_texture_CbCr_ref = CVBufferRetain(_captured_image_texture_cbcr_ref);
        
        [command_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            
            CVBufferRelease(captured_image_texture_y_ref);
            CVBufferRelease(captured_image_texture_CbCr_ref);
            
            dispatch_semaphore_signal(block_semaphore);
            
        }];
    } else {
        [command_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            dispatch_semaphore_signal(block_semaphore);
        }];
    }
#else
    [command_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(block_semaphore);
    }];
#endif
    
#if (MTT_FRAME_ASYNC == 1)
    
    __block dispatch_semaphore_t block_render_data_semaphore = _render_semaphore;
    
    dispatch_async(_render_queue, ^{
        // MARK: - begin the frame
#endif
        if constexpr ((USE_NVG)) {
            nvgBeginFrame(_nvg_ctx, _viewport_size.width, _viewport_size.height, get_native_scale(get_main_screen()));
            
            nvgTextScale(_nvg_ctx, 4.0);
            
            {
                static float m4x4_identity[16] = {
                    1.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 1.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f,
                };
                nvgSetViewTransform(_nvg_ctx, m4x4_identity);
            }
        }
        //#if (USE_ARKIT)
        //
        //        ARFrame *currentFrame = _session.currentFrame;
        //        for (ARAnchor* anchor in _session.currentFrame.anchors) {
        //            if ([anchor isKindOfClass:[ARFaceAnchor class]]) {
        //
        //
        //                ARFaceAnchor* face_anchor = (ARFaceAnchor*)anchor;
        //
        //                simd_float3 point = face_anchor.lookAtPoint;
        //                ARCamera* ar_cam = _session.currentFrame.camera;
        //
        //                auto orientation = [[[[UIApplication sharedApplication] windows][0] windowScene] interfaceOrientation];
        //
        //                CGPoint cgpoint = [ar_cam projectPoint:point orientation: orientation viewportSize:self->_viewport_size];
        //
        //
        //
        //                _core_platform->core.look_at_point = vec3(cgpoint.x, cgpoint.y, 0.0f);
        //
        //                break;
        //            }
        //        }
        //#endif
        
        auto* core = &self->_core_platform->core;
        uint64 current_time_ns = mtt_time_nanoseconds();
        
        
        
        uint64 time_delta_ns =  mtt_time_delta_nanoseconds(self->prev_time_ns, current_time_ns);
        self->prev_time_ns = current_time_ns;
        
        core->time_seconds_prev = self->_curr_time;
        self->_curr_time = mtt_ns_to_s(current_time_ns);
        
        if (MTT_Core_active_pause_state(core)) {
            core->time_sim_paused_elapsed_ns += time_delta_ns;
            core->time_sim_paused_elapsed_s = mtt_ns_to_s(core->time_sim_paused_elapsed_ns);
        } else {
            core->time_sim_accumulator_counter_ns += time_delta_ns;
        }
        
        core->time_nanoseconds_prev = core->time_nanoseconds;
        core->time_nanoseconds = current_time_ns;
        core->time_delta_nanoseconds = time_delta_ns;
        
        core->time_delta_seconds = mtt_ns_to_s(time_delta_ns);
        
        core->time_seconds = self->_curr_time;
        
        core->elapsed = [[NSProcessInfo processInfo] systemUptime];
        
        if (_viewport_size_did_change) {
            MTT_on_resize(self->_core_platform, vec2(self->_core_platform->core.viewport.width, self->_core_platform->core.viewport.height));
        }
        
        //MTT_print("time sim accu before: %llu\n", core->time_sim_accumulator_counter_ns);
        MTT_on_frame(self->_core_platform);
        //MTT_print("time sim accu after: %llu\n", core->time_sim_accumulator_counter_ns);
        
        
        
#if (LOG_SIGNPOST_ENABLED)
        os_signpost_interval_begin(self->_logger, self->_os_signpost_render_update_id, "input end of frame");
#endif
        Input_end_of_frame(&self->_core_platform->core.input);
#if (LOG_SIGNPOST_ENABLED)
        os_signpost_interval_end(self->_logger, self->_os_signpost_render_update_id, "input end of frame");
#endif
        
#if (MTT_FRAME_ASYNC == 1)
        dispatch_semaphore_signal(block_render_data_semaphore);
    });
    
    dispatch_semaphore_wait(_render_semaphore, DISPATCH_TIME_FOREVER);
#endif
    
    @autoreleasepool {
        
        
        // Create a new command buffer for each renderpass to the current drawable

        
        
        
        // Obtain a renderPassDescriptor generated from the view's drawable textures
        MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
        
        
        // If we've gotten a renderPassDescriptor we can render to the drawable, otherwise we'll skip
        //   any rendering this frame because we have no drawable to draw to
        if (renderPassDescriptor != nil) MTT_LIKELY {
            
            // MARK: - update with ARKit frame
            
            
            
            //id<CAMetalDrawable> drawable = [view currentDrawable];
            //renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
            renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPassDescriptor.colorAttachments[0].clearColor =
#if (MTT_PLATFORM == MTT_PLATFORM_TYPE_MACOS_DESKTOP)
            (override_bg_color_with_clear) ? MTLClearColorMake(0.0, 0.0, 0.0, 0.0) :
#endif
            MTLClearColorMake(
                                                                                    _core_platform->renderer.background_color[0],
                                                                                    _core_platform->renderer.background_color[1],
                                                                                    _core_platform->renderer.background_color[2],
                                                                                    _core_platform->renderer.background_color[3]
                                                                                    );
            
            /*
             renderPassDescriptor.colorAttachments[0].storeAction = (USE_MULTISAMPLING) ?
             MTLStoreActionStoreAndMultisampleResolve :
             MTLStoreActionStore;
             */
            renderPassDescriptor.colorAttachments[0].storeAction = store_action_render_target;
            
            
            sd::Renderer& renderer = _core_platform->renderer;
            
            usize i_uniform_offset = 0;
            Instance_Data* per_entity_data = (Instance_Data*)_entity_uniform_buffer_address;
            
            usize i_semishared_uniform_offset = 0;
            Semishared_Uniforms* projection_view_data =
            (Semishared_Uniforms*)_semishared_uniform_buffer_address;
            
            memcpy(&projection_view_data->view_matrix, m::value_ptr(_core_platform->core.view_transform), sizeof(_core_platform->core.view_transform));
            
            memcpy(&projection_view_data->projection_matrix,
                   m::value_ptr(Mat4(1.0f)),
                   sizeof(Mat4));
            
            id<MTLRenderCommandEncoder> renderEncoder;
            id<MTLRenderCommandEncoder> main_render_encoder;
            
            [self _updateUniforms];
            
            for (usize cmd_list_idx = 0; cmd_list_idx < renderer.cmd_list_count; cmd_list_idx += 1) {
                sd::Command_List& cmd_list = renderer.cmd_lists[cmd_list_idx];
                const usize cmd_list_size = cmd_list.size();
                for (usize cmd_idx = 0; cmd_idx < cmd_list_size; cmd_idx += 1)
                {
                    
                    sd::Render_Layer* render_layer = nullptr;
                    sd::Render_Layer_ID layer_idx = 0;
                    sd::Render_Layer_ID buffer_layer_idx = 0;
                    sd::Render_Layer_Range layer_range;
                    
                    sd::Command* cmd = &_core_platform->renderer.cmd_lists[cmd_list_idx].commands[cmd_idx];
                    switch (cmd->type) {
                    case sd::COMMAND_TYPE_RENDER_PASS_BEGIN: {
                        if (cmd->render_pass.id != 0) {
                            sd::Render_Target* rt = &cmd->render_pass.color_targets[0];
                            
                            MTLRenderPassDescriptor* descriptor;
                            if (USE_MULTISAMPLING) {
                                if (Render_Target_is_msaa(rt)) {
                                    id<MTLTexture> target_msaa = (__bridge id<MTLTexture>)cmd->render_pass.color_targets[0].target_msaa.ref;
                                    id<MTLTexture> target_resolved = (__bridge id<MTLTexture>)cmd->render_pass.color_targets[0].target.ref;
                                    
                                    id<MTLTexture> depth_stencil_msaa = (__bridge id<MTLTexture>)cmd->render_pass.color_targets[0].target_depth_stencil_msaa.ref;
                                    id<MTLTexture> depth_stencil_resolved = (__bridge id<MTLTexture>)cmd->render_pass.color_targets[0].target_depth_stencil.ref;
                                    
                                    offscreen_renderpass_descriptor_msaa.colorAttachments[0].texture = target_msaa;
                                    offscreen_renderpass_descriptor_msaa.colorAttachments[0].resolveTexture = target_resolved;
                                    
                                    offscreen_renderpass_descriptor_msaa.colorAttachments[0].loadAction = (MTLLoadAction)cmd->render_pass.color_targets[0].target_load;
                                    offscreen_renderpass_descriptor_msaa.colorAttachments[0].storeAction = (MTLStoreAction)cmd->render_pass.color_targets[0].target_store;
                                    
                                    offscreen_renderpass_descriptor_msaa.depthAttachment.texture = depth_stencil_msaa;
                                    offscreen_renderpass_descriptor_msaa.depthAttachment.resolveTexture = depth_stencil_resolved;
                                    offscreen_renderpass_descriptor_msaa.stencilAttachment.texture = depth_stencil_msaa;
                                    offscreen_renderpass_descriptor_msaa.stencilAttachment.resolveTexture = depth_stencil_resolved;
                                    offscreen_renderpass_descriptor_msaa.depthAttachment.loadAction = (MTLLoadAction)cmd->render_pass.color_targets[0].depth_load;
                                    offscreen_renderpass_descriptor_msaa.depthAttachment.storeAction = (MTLStoreAction)cmd->render_pass.color_targets[0].depth_store;
                                    offscreen_renderpass_descriptor_msaa.stencilAttachment.loadAction = (MTLLoadAction)cmd->render_pass.color_targets[0].stencil_load;
                                    offscreen_renderpass_descriptor_msaa.stencilAttachment.storeAction = (MTLStoreAction)cmd->render_pass.color_targets[0].stencil_store;
                                    
                                    descriptor = offscreen_renderpass_descriptor_msaa;
                                } else {
                                    id<MTLTexture> target = (__bridge id<MTLTexture>)cmd->render_pass.color_targets[0].target.ref;
                                    id<MTLTexture> depth_stencil_target = (__bridge id<MTLTexture>)cmd->render_pass.color_targets[0].target_depth_stencil.ref;
                                    
                                    offscreen_renderpass_descriptor.colorAttachments[0].texture = target;
                                    
                                    offscreen_renderpass_descriptor.colorAttachments[0].loadAction = (MTLLoadAction)cmd->render_pass.color_targets[0].target_load;
                                    offscreen_renderpass_descriptor.colorAttachments[0].storeAction = (MTLStoreAction)cmd->render_pass.color_targets[0].target_store;
                                    
                                    offscreen_renderpass_descriptor.depthAttachment.texture = depth_stencil_target;
                                    offscreen_renderpass_descriptor.stencilAttachment.texture = depth_stencil_target;
                                    
                                    offscreen_renderpass_descriptor.depthAttachment.loadAction = (MTLLoadAction)cmd->render_pass.color_targets[0].depth_load;
                                    offscreen_renderpass_descriptor.depthAttachment.storeAction = (MTLStoreAction)cmd->render_pass.color_targets[0].depth_store;
                                    offscreen_renderpass_descriptor.stencilAttachment.loadAction = (MTLLoadAction)cmd->render_pass.color_targets[0].stencil_load;
                                    offscreen_renderpass_descriptor.stencilAttachment.storeAction = (MTLStoreAction)cmd->render_pass.color_targets[0].stencil_store;

                                    descriptor = offscreen_renderpass_descriptor;
                                }
                            } else {
                                id<MTLTexture> target = (__bridge id<MTLTexture>)cmd->render_pass.color_targets[0].target.ref;
                                id<MTLTexture> depth_stencil_target = (__bridge id<MTLTexture>)cmd->render_pass.color_targets[0].target_depth_stencil.ref;
                                
                                offscreen_renderpass_descriptor.colorAttachments[0].texture = target;
                                
                                offscreen_renderpass_descriptor.colorAttachments[0].loadAction = (MTLLoadAction)cmd->render_pass.color_targets[0].target_load;
                                offscreen_renderpass_descriptor.colorAttachments[0].storeAction = (MTLStoreAction)cmd->render_pass.color_targets[0].target_store;
                                if (offscreen_renderpass_descriptor.colorAttachments[0].storeAction == MTLStoreActionMultisampleResolve) {
                                    offscreen_renderpass_descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
                                } else if (offscreen_renderpass_descriptor.colorAttachments[0].storeAction == MTLStoreActionStoreAndMultisampleResolve) {
                                    offscreen_renderpass_descriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
                                }
                                
                                offscreen_renderpass_descriptor.depthAttachment.texture = depth_stencil_target;
                                offscreen_renderpass_descriptor.stencilAttachment.texture = depth_stencil_target;
                                
                                offscreen_renderpass_descriptor.depthAttachment.loadAction = (MTLLoadAction)cmd->render_pass.color_targets[0].depth_load;
                                offscreen_renderpass_descriptor.depthAttachment.storeAction = (MTLStoreAction)cmd->render_pass.color_targets[0].depth_store;
                                if (offscreen_renderpass_descriptor.depthAttachment.storeAction == MTLStoreActionMultisampleResolve) {
                                    offscreen_renderpass_descriptor.depthAttachment.storeAction = MTLStoreActionStore;
                                } else if (offscreen_renderpass_descriptor.depthAttachment.storeAction == MTLStoreActionStoreAndMultisampleResolve) {
                                    offscreen_renderpass_descriptor.depthAttachment.storeAction = MTLStoreActionStore;
                                }
                                
                                offscreen_renderpass_descriptor.stencilAttachment.loadAction = (MTLLoadAction)cmd->render_pass.color_targets[0].stencil_load;
                                offscreen_renderpass_descriptor.stencilAttachment.storeAction = (MTLStoreAction)cmd->render_pass.color_targets[0].stencil_store;
                                if (offscreen_renderpass_descriptor.stencilAttachment.storeAction == MTLStoreActionMultisampleResolve) {
                                    offscreen_renderpass_descriptor.stencilAttachment.storeAction = MTLStoreActionStore;
                                } else if (offscreen_renderpass_descriptor.stencilAttachment.storeAction == MTLStoreActionStoreAndMultisampleResolve) {
                                    offscreen_renderpass_descriptor.stencilAttachment.storeAction = MTLStoreActionStore;
                                }

                                descriptor = offscreen_renderpass_descriptor;
                            }
                            
                            
                            
                            //renderEncoder = main_render_encoder;
                            renderEncoder = [command_buffer renderCommandEncoderWithDescriptor:descriptor];
                            

                            //NSLog(@"%zu %zu\n", kAlignedSharedUniformsSize, sizeof(SharedUniforms));
                            [renderEncoder pushDebugGroup:[@"draw polygons id=" stringByAppendingFormat:@"%llu", cmd->render_pass.id]];
                            
                            [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
                            [renderEncoder setCullMode:MTLCullModeNone];
                            
                            //            [renderEncoder setRenderPipelineState:_polygon_color_pipeline_state];
                            [renderEncoder setDepthStencilState:_things_depth_stencil_state];
                            self->depth_write_mode = sd::DEPTH_WRITE_MODE_ENABLED;
                            self->stencil_mode = sd::STENCIL_MODE_DISABLED;
                            
                            [renderEncoder setVertexBuffer:_shared_uniform_buffer offset:_shared_uniform_buffer_offset atIndex:kBufferIndexSharedUniforms];
                            [renderEncoder setVertexBuffer:_entity_uniform_buffer offset:_entity_uniform_buffer_offset atIndex:kBufferIndexEntityUniforms];
                            [renderEncoder setVertexBuffer:_semishared_uniform_buffer offset:_semishared_uniform_buffer_offset atIndex:kBufferIndexSemisharedUniforms];
                            
                            self->active_v_buffer_0 = nil;
                            
                            if (SD_SUPPORT_BINDLESS_ONLY || self->support_bindless) {
                                [renderEncoder setFragmentBuffer:_scene_data offset:0 atIndex:MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS];
                                
                                [renderEncoder useResource:_scene_resource_data usage:MTLResourceUsageRead stages:MTLRenderStageFragment];
                                
                                mtt::Dynamic_Array<id>* textures = sd::textures(&images);
                                [renderEncoder useResources:textures->begin_ptr() count:textures->size() usage:MTLResourceUsageRead stages:MTLRenderStageFragment];
                            }
                            
                            
                            //            [renderEncoder setRenderPipelineState:_polygon_color_texture_pipeline_state];
                            
                            else {
                                mtt::Dynamic_Array<id>* textures = sd::textures(&images);
                                [renderEncoder setFragmentTextures:textures->begin_ptr() withRange:NSMakeRange(0, MIN(textures->count, max_resident_textures))];
                                
                                
                                //[renderEncoder setFragmentSamplerState:sampler_clamp_to_edge atIndex:0];
                                //[renderEncoder setFragmentSamplerState:sampler_repeat atIndex:1];
                            }
                            //            [renderEncoder setFragmentTexture:__textures[2] atIndex:0];
                            //            [renderEncoder setScissorRect:{
                            //                .x      = (uint64)(0),
                            //                .y      = (uint64)(0),
                            //                .width  = (uint64)(UIScreen.mainScreen.nativeBounds.size.height),
                            //                .height = (uint64)(UIScreen.mainScreen.nativeBounds.size.width)
                            //            }];
                            
                            _core_platform->core.view_transform = sd::get_view(&_core_platform->renderer);
                            
                            
                            
                            //sd::print_commands(&_core_platform->renderer);
                            
                            

                            
                            //            i_semishared_uniform_offset += 1;
                            //            projection_view_data += 1;
                            
                            
                            self->_pipeline_mode = sd::PIPELINE_MODE_NONE;
                            
                            {
                                auto* viewport = &_core_platform->core.viewport;
                                auto scale = sd::get_native_display_scale();
                                MTLViewport vp = (MTLViewport){viewport->x * scale, viewport->y * scale, viewport->width * scale, viewport->height * scale, viewport->znear, viewport->zfar};
                                [renderEncoder setViewport:vp];
                                simd_float2 dimensions = simd_make_float2((float32)vp.width, (float32)vp.height);
                                Push_Uniforms u_in = {};
                                memcpy(&u_in.dimensions, &dimensions, sizeof(u_in.dimensions));
                                [renderEncoder setFragmentBytes:&u_in length:sizeof(Push_Uniforms) atIndex:MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS_TARGET_PARAMS];
                                
                                self->scissor_rectangle.x = viewport->x;
                                self->scissor_rectangle.y = viewport->y;
                                self->scissor_rectangle.width = viewport->width;
                                self->scissor_rectangle.height = viewport->height;
                                
                                [renderEncoder setScissorRect:{
                                    .x      = static_cast<NSUInteger>(self->scissor_rectangle.x * scale),
                                    .y      = static_cast<NSUInteger>(self->scissor_rectangle.y * scale),
                                    .width  = static_cast<NSUInteger>(self->scissor_rectangle.width * scale),
                                    .height = static_cast<NSUInteger>(self->scissor_rectangle.height * scale)
                                }];
                            }
                            
                            
                            if constexpr (USE_NVG) {
                                mnvgSetCommandBuffer(_nvg_ctx, (__bridge void*)command_buffer);
                                mnvgSetRenderEncoder(_nvg_ctx, (__bridge void*)renderEncoder);
                                mnvgSetDefaultDepthStencilState(_nvg_ctx, (__bridge void*)_things_depth_stencil_state);
                            }
                        } else {
                            // Create a render command encoder so we can render into something
                            renderEncoder = [command_buffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
                            renderEncoder.label = @"render encoder default";
                            
                            main_render_encoder = renderEncoder;
                            
                            renderPassDescriptor = nil;
                            
                #if ((MTT_PLATFORM != MTT_PLATFORM_TYPE_MACOS_DESKTOP) && USE_ARKIT)
                            ARFrame *currentFrame = nil;
                            if (ar_is_active) {
                                currentFrame = _session.currentFrame;
                                if (currentFrame) {
                                    //[self _updateSharedUniformsWithFrame:currentFrame];
                                    
                                    [self _updateSharedUniforms];
                                    //[self _updateAnchorsWithFrame:currentFrame];
                                    [self _updateCapturedImageTexturesWithFrame:currentFrame];
                                    
                                    if (_viewport_size_did_change) {
                                        
                                        
                                        [self _updateImagePlaneWithFrame:currentFrame];
                                    }
                                    
                                    
                                    //                [ar_cam projectPoint:[session] orientation:[[UIApplication sharedApplication] statusBarOrientation] viewportSize:self->_viewport_size];
                                    
                                    
                                    
                                    
                                    
                                }
                                [self _drawCapturedImageWithCommandEncoder:renderEncoder];
                            } else if (ar_freeze_frame_when_off) {
                                [self _drawCapturedImageWithCommandEncoder:renderEncoder];
                            } else {
                                [self _updateUniforms];
                            }
                            //            MTT_print("AR active state: %s freeze? %s\n", _core_platform->core.ar_ctx.is_active ? "on" : "off", _core_platform->core.ar_ctx.freeze_frame_when_off ? "yes" : "no");
                            //[self _drawAnchorGeometryWithCommandEncoder:renderEncoder];
                #else
                            
                #endif
                            
                            renderEncoder = main_render_encoder;

                            //NSLog(@"%zu %zu\n", kAlignedSharedUniformsSize, sizeof(SharedUniforms));
                            [renderEncoder pushDebugGroup:@"draw polygons"];
                            
                            [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
                            [renderEncoder setCullMode:MTLCullModeNone];
                            
                            //            [renderEncoder setRenderPipelineState:_polygon_color_pipeline_state];
                            [renderEncoder setDepthStencilState:_things_depth_stencil_state];
                            self->depth_write_mode = sd::DEPTH_WRITE_MODE_ENABLED;
                            self->stencil_mode = sd::STENCIL_MODE_DISABLED;
                            
                            [renderEncoder setVertexBuffer:_shared_uniform_buffer offset:_shared_uniform_buffer_offset atIndex:kBufferIndexSharedUniforms];
                            [renderEncoder setVertexBuffer:_entity_uniform_buffer offset:_entity_uniform_buffer_offset atIndex:kBufferIndexEntityUniforms];
                            [renderEncoder setVertexBuffer:_semishared_uniform_buffer offset:_semishared_uniform_buffer_offset atIndex:kBufferIndexSemisharedUniforms];
                            
                            self->active_v_buffer_0 = nil;
                            
                            if (SD_SUPPORT_BINDLESS_ONLY || self->support_bindless) {
                                [renderEncoder setFragmentBuffer:_scene_data offset:0 atIndex:MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS];
                                
                                [renderEncoder useResource:_scene_resource_data usage:MTLResourceUsageRead stages:MTLRenderStageFragment];
                                
                                mtt::Dynamic_Array<id>* textures = sd::textures(&images);
                                [renderEncoder useResources:textures->begin_ptr() count:textures->size() usage:MTLResourceUsageRead stages:MTLRenderStageFragment];
                            }
                            
                            
                            //            [renderEncoder setRenderPipelineState:_polygon_color_texture_pipeline_state];
                            
                            else {
                                mtt::Dynamic_Array<id>* textures = sd::textures(&images);
                                [renderEncoder setFragmentTextures:textures->begin_ptr() withRange:NSMakeRange(0, MIN(textures->count, max_resident_textures))];
                                
                                
                                //[renderEncoder setFragmentSamplerState:sampler_clamp_to_edge atIndex:0];
                                //[renderEncoder setFragmentSamplerState:sampler_repeat atIndex:1];
                            }
                            //            [renderEncoder setFragmentTexture:__textures[2] atIndex:0];
                            //            [renderEncoder setScissorRect:{
                            //                .x      = (uint64)(0),
                            //                .y      = (uint64)(0),
                            //                .width  = (uint64)(UIScreen.mainScreen.nativeBounds.size.height),
                            //                .height = (uint64)(UIScreen.mainScreen.nativeBounds.size.width)
                            //            }];
                            
                            _core_platform->core.view_transform = sd::get_view(&_core_platform->renderer);
                            
                            
                            
                            //sd::print_commands(&_core_platform->renderer);
                            
                            

                            
                            //            i_semishared_uniform_offset += 1;
                            //            projection_view_data += 1;
                            
                            
                            self->_pipeline_mode = sd::PIPELINE_MODE_NONE;
                            
                            {
                                auto* viewport = &_core_platform->core.viewport;
                                auto scale = sd::get_native_display_scale();
                                MTLViewport vp = (MTLViewport){viewport->x * scale, viewport->y * scale, viewport->width * scale, viewport->height * scale, viewport->znear, viewport->zfar};
                                [renderEncoder setViewport:vp];
                                simd_float2 dimensions = simd_make_float2((float32)vp.width, (float32)vp.height);
                                Push_Uniforms u_in = {};
                                memcpy(&u_in.dimensions, &dimensions, sizeof(u_in.dimensions));
                                [renderEncoder setFragmentBytes:&u_in length:sizeof(Push_Uniforms) atIndex:MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS_TARGET_PARAMS];
                                
                                self->scissor_rectangle.x = viewport->x;
                                self->scissor_rectangle.y = viewport->y;
                                self->scissor_rectangle.width = viewport->width;
                                self->scissor_rectangle.height = viewport->height;
                                
                                [renderEncoder setScissorRect:{
                                    .x      = static_cast<NSUInteger>(self->scissor_rectangle.x * scale),
                                    .y      = static_cast<NSUInteger>(self->scissor_rectangle.y * scale),
                                    .width  = static_cast<NSUInteger>(self->scissor_rectangle.width * scale),
                                    .height = static_cast<NSUInteger>(self->scissor_rectangle.height * scale)
                                }];
                            }
                            
                            
                            if constexpr (USE_NVG) {
                                mnvgSetCommandBuffer(_nvg_ctx, (__bridge void*)command_buffer);
                                mnvgSetRenderEncoder(_nvg_ctx, (__bridge void*)renderEncoder);
                                mnvgSetDefaultDepthStencilState(_nvg_ctx, (__bridge void*)_things_depth_stencil_state);
                            }
                            
                        }
                        continue;
                    }
                    case sd::COMMAND_TYPE_RENDER_PASS_END: {
                        [renderEncoder popDebugGroup];
                        
                        // We're done encoding commands
                        [renderEncoder endEncoding];
                        continue;
                    }
                    case sd::COMMAND_TYPE_COMPUTE_PASS_BEGIN: {
                        compute_pass.use_own_command_buffer = cmd->compute_pass.descriptor.use_own_command_buffer;
                        id<MTLCommandBuffer> cmd_buf = (cmd->compute_pass.descriptor.use_own_command_buffer) ? [_command_queue commandBuffer] : command_buffer;
                        MTLComputePassDescriptor* desc = [MTLComputePassDescriptor computePassDescriptor];
                        switch (cmd->compute_pass.descriptor.type) {
                            case sd::COMPUTE_DISPATCH_TYPE_CONCURRENT:
                                desc.dispatchType = MTLDispatchTypeConcurrent;
                                break;
                            case sd::COMPUTE_DISPATCH_TYPE_SERIAL:
                                desc.dispatchType = MTLDispatchTypeSerial;
                                break;
                        }
                        
                        id<MTLComputeCommandEncoder> encoder = [cmd_buf computeCommandEncoderWithDescriptor:desc];
                        compute_pass.command_buffer = cmd_buf;
                        compute_pass.encoder = encoder;
                        [compute_pass.encoder pushDebugGroup:@"compute encoder"];
                        
                        [encoder setBuffer:_scene_data offset:0 atIndex:0];
                        
                        [encoder useResource:_scene_resource_data usage:MTLResourceUsageRead|MTLResourceUsageWrite];
                        
                        mtt::Dynamic_Array<id>* textures = sd::textures(&images);
                        [encoder useResources:textures->begin_ptr() count:textures->size() usage:MTLResourceUsageRead|MTLResourceUsageWrite];
                        
                        
                        continue;
                    }
                    case sd::COMMAND_TYPE_COMPUTE_BUFFER_SET: {
                        [compute_pass.encoder setBuffer:
                         (__bridge id<MTLBuffer>)cmd->compute_pass.buffer_set.buffer.buffer
                                                offset:0 atIndex: cmd->compute_pass.buffer_set.slot];
                        continue;
                    }
                    case sd::COMMAND_TYPE_COMPUTE_SHADER_SET: {
                        [compute_pass.encoder setComputePipelineState:(id<MTLComputePipelineState>)_compute_pipelines[cmd->compute_pass.shader_set.shader_id]];
                        continue;
                    }
                    case sd::COMMAND_TYPE_COMPUTE_SET_BLUR: {
                        id<MTLCommandBuffer> cmd_buf = command_buffer//[_command_queue commandBuffer]
                        ;
                        id<MTLTexture> texture = (__bridge id<MTLTexture>)cmd->compute_pass.blur_set.texture_handle.ref;
                        MPSImageGaussianBlur *blur;
                        if (cmd->compute_pass.blur_set.blur.handle == nullptr) {
                             blur = [[MPSImageGaussianBlur alloc] initWithDevice: _device sigma:cmd->compute_pass.blur_set.blur.radius];
                        } else {
                            blur = (__bridge MPSImageGaussianBlur *)cmd->compute_pass.blur_set.blur.handle;
                        }
                        BOOL result = [blur encodeToCommandBuffer: cmd_buf inPlaceTexture: &texture fallbackCopyAllocator:nil];
                        if (!result) {
                            
                        }
                        //[cmd_buf commit];
//                        if (cmd->compute_pass.blur_set.should_wait) {
//                            [cmd_buf waitUntilCompleted];
//                        }
                        continue;
                    }
                    case sd::COMMAND_TYPE_COMPUTE_SET_SOBEL: {
                        id<MTLCommandBuffer> cmd_buf = command_buffer;
                        
                        id<MTLTexture> texture_src = (__bridge id<MTLTexture>)cmd->compute_pass.sobel_set.texture_handle_src.ref;
                        id<MTLTexture> texture_dst = (__bridge id<MTLTexture>)cmd->compute_pass.sobel_set.texture_handle_dst.ref;
                        
                        [image_sobel_fx encodeToCommandBuffer:cmd_buf sourceTexture:texture_src destinationTexture:texture_dst];
                        continue;
                    }
                    case sd::COMMAND_TYPE_COMPUTE_DISPATCH_NON_UNIFORM_GRID: {
                        /*
                         NSUInteger w = pipelineState.threadExecutionWidth;
                         NSUInteger h = pipelineState.maxTotalThreadsPerThreadgroup / w;
                         MTLSize threadsPerThreadgroup = MTLSizeMake(w, h, 1);
                         
                         MTLSize threadsPerGrid = MTLSizeMake(texture.width, texture.height, 1);
                             
                         [computeCommandEncoder dispatchThreads: threadsPerGrid
                                                threadsPerThreadgroup: threadsPerThreadgroup];
                         */
                        const uvec3 tptg = cmd->compute_pass.dispatch.nonuniform_grid_args.threads_per_threadgroup;
                        MTLSize threadsPerThreadgroup = MTLSizeMake(tptg[0], tptg[1], tptg[2]);
                        const uvec3 tpg = cmd->compute_pass.dispatch.nonuniform_grid_args.threads_per_grid;
                        MTLSize threadsPerGrid = MTLSizeMake(tpg[0], tpg[1], tpg[2]);
                        
                        [compute_pass.encoder dispatchThreads: threadsPerGrid threadsPerThreadgroup: threadsPerThreadgroup];
                        continue;
                    }
                    case sd::COMMAND_TYPE_COMPUTE_DISPATCH_UNIFORM_GRID: {
//                        NSUInteger w = compute_pass.shader.threadExecutionWidth;
//                        NSUInteger h = compute_pass.shader.maxTotalThreadsPerThreadgroup / w;
//                        MTLSize threadsPerThreadgroup = MTLSizeMake(w, h, 1);
//
//                        MTT_error("not implemented fully\n");
                        continue;
                    }
                    case sd::COMMAND_TYPE_COMPUTE_PASS_END: {
                        [compute_pass.encoder popDebugGroup];
                        [compute_pass.encoder endEncoding];
                        
                        if (compute_pass.use_own_command_buffer) {
                            if (cmd->compute_pass.end_pass.should_wait) {
                                [compute_pass.command_buffer commit];
                                [compute_pass.command_buffer waitUntilCompleted];
                                
                                if (cmd->compute_pass.end_pass.callback != nullptr) {
                                    cmd->compute_pass.end_pass.callback(cmd->compute_pass.end_pass.state);
                                }
                            } else {
//                                if (cmd->compute_pass.end_pass.callback != nullptr) {
//                                    __block void* state = cmd->compute_pass.end_pass.state;
//                                    __block auto* cb = cmd->compute_pass.end_pass.callback;
//                                    [compute_pass.command_buffer addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull) {
//                                        cb(state);
//                                    }];
//                                }
//                                [compute_pass.command_buffer commit];
                            }
                        } else {
                            ASSERT_MSG(!cmd->compute_pass.end_pass.should_wait || cmd->compute_pass.end_pass.callback != nullptr, "cannot wait for compute pass within command buffer shared with other passes\n");
                        }
                        
                        compute_pass.encoder = nil;
                        compute_pass.command_buffer = nil;
                        continue;
                    }
                    case sd::COMMAND_TYPE_DRAW: {
                        layer_idx = cmd->draw.layer_id;
                        buffer_layer_idx = cmd->draw.buffer_layer_id;
                        layer_range = cmd->draw.layer_range;
                        render_layer = &_core_platform->renderer.render_layers[layer_idx];
                        
                        break;
                    }
                    case sd::COMMAND_TYPE_PROJECTION_VIEW_TRANSFORM: {
                        memcpy(&projection_view_data->view_matrix, m::value_ptr(cmd->projection_view_transform.view), sizeof(cmd->projection_view_transform.view));
                        
                        memcpy(&projection_view_data->projection_matrix,
                               m::value_ptr(cmd->projection_view_transform.projection),
                               sizeof(cmd->projection_view_transform.projection));
                        
                        [renderEncoder setVertexBufferOffset:_semishared_uniform_buffer_offset + (i_semishared_uniform_offset * sizeof(Semishared_Uniforms)) atIndex:kBufferIndexSemisharedUniforms];
                        
                        projection_view_data += 1;
                        
                        i_semishared_uniform_offset += 1;
                        
                        continue;
                    }
                    case sd::COMMAND_TYPE_PIPELINE: {
                        // TODO
                        const uint64 pipeline_id = cmd->pipeline_id;
                        if (pipeline_id == self->_pipeline_mode) {
                            continue;
                        }
                        
                        self->_pipeline_mode = pipeline_id;
                        
                        if (pipeline_id != sd::PIPELINE_MODE_NONE) {
                            [renderEncoder setRenderPipelineState:self->_pipelines[pipeline_id]];
                        }
                        
                        //                                [renderEncoder setVertexBuffer:((id<MTLBuffer>)[self->_vertex_buffer_list
                        //                                                                                objectAtIndex:layer_idx]) offset:0 atIndex:0];
                        
                        continue;
                    }
                    case sd::COMMAND_TYPE_COLOR_PIPELINE: {
                        if (self->_pipeline_mode == sd::PIPELINE_MODE_COLOR) {
                            continue;
                        }
                        
                        self->_pipeline_mode = sd::PIPELINE_MODE_COLOR;
                        
                        [renderEncoder setRenderPipelineState:_polygon_color_pipeline_state];
                        
                        
                        continue;
                    }
                    case sd::COMMAND_TYPE_COLOR_HSV_PIPELINE: {
                        if (self->_pipeline_mode == sd::PIPELINE_MODE_COLOR_HSV) {
                            continue;
                        }
                        
                        self->_pipeline_mode = sd::PIPELINE_MODE_COLOR_HSV;
                        
                        [renderEncoder setRenderPipelineState:_polygon_color_hsv_pipeline_state];
                        
                        
                        continue;
                    }
                    case sd::COMMAND_TYPE_TEXTURE_PIPELINE: {
                        if (self->_pipeline_mode == sd::PIPELINE_MODE_TEXTURE) {
                            continue;
                        }
                        
                        self->_pipeline_mode = sd::PIPELINE_MODE_TEXTURE;
                        
                        [renderEncoder setRenderPipelineState:_polygon_color_texture_pipeline_state];
                        
                        continue;
                    }
                    case sd::COMMAND_TYPE_TEXT_PIPELINE: {
//                        if (self->_pipeline_mode == sd::PIPELINE_MODE_TEXT) {
//                            continue;
//                        }
//
//                        self->_pipeline_mode = sd::PIPELINE_MODE_TEXT;
//
//                        [renderEncoder setRenderPipelineState:_text_pipeline_state];
                        
                        continue;
                    }
                        // TODO: set viewport command
                        //                            case sd::COMMAND_TYPE_VIEWPORT: {
                        //                                const sd::Viewport viewport = cmd->viewport;
                        //
                        //                                MTLViewport vp;
                        //                                [renderEncoder setViewport:{
                        //                                    .x      = viewport.x,
                        //                                    .y      = viewport.y,
                        //                                    .width  = MIN((viewport.width + viewport.x) * (uint64)(UIScreen.mainScreen.nativeScale), (uint64)(UIScreen.mainScreen.nativeBounds.size.height)),
                        //                                    .height = MIN((viewport.height + viewport.y) * (uint64)(UIScreen.mainScreen.nativeScale), (uint64)(UIScreen.mainScreen.nativeBounds.size.width))
                        //                                }];
                        //
                        //                                continue;
                        //                            }
                    case sd::COMMAND_TYPE_SCISSOR_RECTANGLE: {
                        const sd::Scissor_Rect scissor_rectangle = cmd->scissor_rectangle;
                        auto bounds = get_native_bounds();
                        auto scale = get_native_scale();
                        
                        if (bounds.size.width < scissor_rectangle.x || bounds.size.height < scissor_rectangle.y) {
                            continue;
                        }
                        
                        MTLScissorRect rect = {};
                        rect.x = (uint64)(scissor_rectangle.x * scale);
                        rect.y = (uint64)(scissor_rectangle.y * scale);
                        uint64 w = scissor_rectangle.width;
                        uint64 h = scissor_rectangle.height;
                        if (scissor_rectangle.x + scissor_rectangle.width > bounds.size.width) {
                            w = bounds.size.width - scissor_rectangle.x;
                        }
                        if (scissor_rectangle.y + scissor_rectangle.height > bounds.size.height) {
                            h = bounds.size.height - scissor_rectangle.y;
                        }
                        rect.width = (uint64)(w * scale);
                        rect.height = (uint64)(h * scale);
                        [renderEncoder setScissorRect:rect];
                        
                        continue;
                    }
                    case sd::COMMAND_TYPE_VIEWPORT: {
                        const auto& vp_cmd = cmd->viewport;
                        auto bounds = get_native_bounds();
                        //auto scale = get_native_scale();
                        
                        auto scale = sd::get_native_display_scale();
                        
                        MTLViewport vp = {};
                        vp.originX = (vp_cmd.x * scale);
                        vp.originY = (vp_cmd.y * scale);
                        vp.width = (vp_cmd.width * scale);
                        vp.height = (vp_cmd.height * scale);
                        vp.znear = vp_cmd.znear;
                        vp.zfar = vp_cmd.zfar;
                        [renderEncoder setViewport:vp];
                        simd_float2 dimensions = simd_make_float2((float32)vp.width, (float32)vp.height);
                        Push_Uniforms u_in = {};
                        memcpy(&u_in.dimensions, &dimensions, sizeof(u_in.dimensions));
                        [renderEncoder setFragmentBytes:&u_in length:sizeof(Push_Uniforms) atIndex:MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS_TARGET_PARAMS];
                        
                        continue;
                    }
                    case sd::COMMAND_TYPE_SET_STENCIL_REFERENCE_VALUE: {
                        [renderEncoder setStencilReferenceValue:cmd->stencil_reference_value];
                        continue;
                    }
                    case sd::COMMAND_TYPE_SET_DEPTH_MODE: {
                        switch (cmd->depth_mode) {
                        case sd::DEPTH_WRITE_MODE_ENABLED: {
                            if (self->depth_write_mode != sd::DEPTH_WRITE_MODE_ENABLED) {
                                self->depth_write_mode = sd::DEPTH_WRITE_MODE_ENABLED;
                                [renderEncoder setDepthStencilState:_things_depth_stencil_state];
                                if constexpr (USE_NVG) {
                                    mnvgSetDefaultDepthStencilState(_nvg_ctx, (__bridge void*)_things_depth_stencil_state);
                                }
                            }
                            break;
                        }
                        case sd::DEPTH_WRITE_MODE_DISABLED: {
                            if (self->depth_write_mode != sd::DEPTH_WRITE_MODE_DISABLED) {
                                self->depth_write_mode = sd::DEPTH_WRITE_MODE_DISABLED;
                                [renderEncoder setDepthStencilState:_things_depth_stencil_state_no_depth_write];
                                if constexpr (USE_NVG) {
                                    mnvgSetDefaultDepthStencilState(_nvg_ctx, (__bridge void*)_things_depth_stencil_state_no_depth_write);
                                }
                            }
                            break;
                        }
                        }
                        continue;
                    }
                    case sd::COMMAND_TYPE_CUSTOM: {
                        
                        cmd->custom_command.handler((void*)cmd->custom_command.state, (void*)cmd->custom_command.data);
                        
                        [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
                        [renderEncoder setCullMode:MTLCullModeNone];
                        auto* viewport = &_core_platform->core.viewport;
                        auto scale = sd::get_native_display_scale();
                        [renderEncoder setViewport:(MTLViewport)
                         {viewport->x * scale, viewport->y * scale, viewport->width * scale, viewport->height * scale, viewport->znear, viewport->zfar}];
                        [renderEncoder setScissorRect:{
                            .x      = self->scissor_rectangle.x,
                            .y      = self->scissor_rectangle.y,
                            .width  = (uint64)((self->scissor_rectangle.width + scissor_rectangle.x) * scale),
                            .height = (uint64)((self->scissor_rectangle.height + scissor_rectangle.y) * scale)
                        }];
                        self->active_v_buffer_0 = nil;
                        
                        if (!self->support_bindless) {
                            {
                                mtt::Dynamic_Array<id>* textures = sd::textures(&images);
                                [renderEncoder setFragmentTextures:textures->begin_ptr() withRange:NSMakeRange(0, MIN(textures->count, max_resident_textures))];
                            }
                        }
                        
                        
                        continue;
                    }
                    case sd::COMMAND_TYPE_DRAW_INSTANCED: {
                        {
                            sd::Drawable_Info* const rc = cmd->draw_instanced.instance_drawable;
                            
                            layer_idx = cmd->draw_instanced.layer_id;
                            //layer_range = cmd->draw_instanced.layer_range;
                            buffer_layer_idx = cmd->draw_instanced.buffer_layer_id;
                            render_layer = &_core_platform->renderer.render_layers[layer_idx];
                            
                            //const usize idx_count = cmd->draw_instanced.layer_range.count;
                            //sd::Drawable_Info* dr_list = cmd->draw_instanced.instance_list;
                            usize uniforms_added = 0;
                            //if (/*idx_count > 0*/)
                            mtt::Array_Slice<sd::Drawable_Info> list = *sd::Draw_Instanced_Command_get_array(&cmd->draw_instanced);
                            
                            usize instance_count = 0;
                            
                            //for (usize i = layer_range.first_index; i < layer_range.count; i += 1) {
                            
                            usize list_idx = 0;
                            //                                for (auto* it = MTT_List_begin(list); it != MTT_List_end(list); MTT_List_advance(&it)) {
                            const usize list_count = list.size();
                            if (list_count == 0) {
                                continue;
                            }
                            for (; list_idx != list_count; list_idx += 1) {
                                
                                
                                //                                    sd::Drawable_Info* drawable = static_cast<sd::Drawable_Info*>(MTT_List_data_ptr(list, it));
                                sd::Drawable_Info* drawable = &list[list_idx];
                                
                                instance_count += 1;
                                
                                *per_entity_data = drawable->instance_data;
                                
                                
                                per_entity_data += 1;
                                
                                uniforms_added += 1;
                            }
                            
                            id<MTLBuffer> v_buffer_to_set = (id<MTLBuffer>)vertex_buffer_list[buffer_layer_idx];
                            if (self->active_v_buffer_0 != v_buffer_to_set) {
                                self->active_v_buffer_0 = v_buffer_to_set;
                                [renderEncoder setVertexBuffer:v_buffer_to_set offset:0 atIndex:0];
                            }
                            
                            //                       [renderEncoder setVertexBufferOffset:_entity_uniform_buffer_offset + ((i_uniform_offset + i) * sizeof(InstanceUniforms)) atIndex:kBufferIndexEntityUniforms];
                            
                            
                            
                            //                         [renderEncoder setVertexBufferOffset:_entity_uniform_buffer_offset + ((i_uniform_offset + i) * sizeof(InstanceUniforms)) atIndex:kBufferIndexEntityUniforms];
                            
                            // MARK: handle instanced data not using textures
                            
                            if (self->_pipeline_mode != sd::PIPELINE_MODE_TEXTURE || !sd::max_textures_reached(renderer.images)) {
                                [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                                          indexCount:rc->index_count()
                                                           indexType:MTLIndexTypeUInt32
                                                         indexBuffer:((id<MTLBuffer>)index_buffer_list[buffer_layer_idx])
                                                   indexBufferOffset:rc->i_buffer_offset * sizeof(sd::Index_Type)
                                                       instanceCount:instance_count
                                                          baseVertex:rc->base_vertex
                                                        baseInstance:(i_uniform_offset)];
                                i_uniform_offset +=
                                uniforms_added;
                                continue;
                            }
                            
                            // MARK: handle instanced data using textures
                            
                            list_idx = 0;
                            
                            sd::Texture_ID tex_id =  (list[0]).get_texture_ID();
                            
                            if (sd::max_textures_reached(renderer.images)) {
                                
                            
                                switch (sd::texture_ensure_resident(renderer.images, tex_id, renderEncoder)) {
                                case sd::TEXTURE_ENSURE_STATUS_NOT_LOADED: {
                                    ASSERT_MSG(false, "ERROR: texture reloading not yet implemented!\n");
                                    break;
                                }
                                case sd::TEXTURE_ENSURE_STATUS_MADE_RESIDENT: {
                                    // ok
                                    break;
                                }
                                case sd::TEXTURE_ENSURE_STATUS_IS_RESIDENT: {
                                    // ok
                                    break;
                                }
                                }
                                
                                
                                usize instances_to_draw = 1;
                                usize instance_uniform_offset = 0;
                                
                                do {
                                    //MTT_List_advance(&inst);
                                    list_idx += 1;
                                    
                                    for ( ; list_idx != list_count; list_idx += 1) {
//                                        sd::Drawable_Info* info = (static_cast<sd::Drawable_Info*>(MTT_List_data_ptr(list, inst)));
                                        sd::Drawable_Info* info = &list[list_idx];
                                        if (info->get_texture_ID() == tex_id) {
                                            instances_to_draw += 1;
                                            continue;
                                        }
                                        
                                        tex_id = info->get_texture_ID();
                                        
                                        switch (sd::texture_ensure_resident(renderer.images, tex_id, renderEncoder)) {
                                        case sd::TEXTURE_ENSURE_STATUS_NOT_LOADED: {
                                            ASSERT_MSG(false, "ERROR: texture reloading not yet implemented!\n");
                                            break;
                                        }
                                        case sd::TEXTURE_ENSURE_STATUS_MADE_RESIDENT: {
                                            // ok
                                            goto RENDER_INSTANCED_TEXTURED;
                                        }
                                        case sd::TEXTURE_ENSURE_STATUS_IS_RESIDENT: {
                                            // ok
                                            break;
                                        }
                                        }
                                    }
                                    // MARK: label render
                                    RENDER_INSTANCED_TEXTURED:
                                    
                                    [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                                              indexCount:rc->index_count()
                                                               indexType:MTLIndexTypeUInt32
                                                             indexBuffer:((id<MTLBuffer>)index_buffer_list [buffer_layer_idx])
                                                       indexBufferOffset:rc->i_buffer_offset * sizeof(sd::Index_Type)
                                                           instanceCount:instances_to_draw
                                                              baseVertex:rc->base_vertex
                                                            baseInstance:(i_uniform_offset + instance_uniform_offset)];
                                    
                                    instance_uniform_offset += instances_to_draw;
                                    instances_to_draw = 1;
                                    
                                } while (list_idx != list_count);
                            
                            } else {
                                /*
                                usize instances_to_draw = 1;
                                usize instance_uniform_offset = 0;
                                
                                do {
                                    //MTT_List_advance(&inst);
                                    list_idx += 1;
                                    
                                    for ( ; list_idx != list_count; list_idx += 1) {
                                        sd::Drawable_Info* info = &list[list_idx];
                                        if (info->texture_ID == tex_id) {
                                            instances_to_draw += 1;
                                            continue;
                                        }
                                        
                                        tex_id = info->texture_ID;
                                        
                                    }
                                    
                                    [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                                              indexCount:rc->index_count()
                                                               indexType:MTLIndexTypeUInt32
                                                             indexBuffer:((id<MTLBuffer>)index_buffer_list [layer_idx])
                                                       indexBufferOffset:rc->i_buffer_offset * sizeof(sd::Index_Type)
                                                           instanceCount:instances_to_draw
                                                              baseVertex:rc->base_vertex
                                                            baseInstance:(i_uniform_offset + instance_uniform_offset)];
                                    
                                    instance_uniform_offset += instances_to_draw;
                                    instances_to_draw = 1;
                                    
                                } while (list_idx != list_count);
                                 
                                 */
                                
//                                    [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
//                                                              indexCount:rc->index_count()
//                                                               indexType:MTLIndexTypeUInt32
//                                                             indexBuffer:((id<MTLBuffer>)index_buffer_list[layer_idx])
//                                                       indexBufferOffset:rc->i_buffer_offset * sizeof(sd::Index_Type)
//                                                           instanceCount:instance_count
//                                                              baseVertex:rc->base_vertex
//                                                            baseInstance:(i_uniform_offset)];
                            
                            }
                            
                        
                            i_uniform_offset +=
                            uniforms_added;
                        }
                        
                        
                        continue;
                    }
                    case sd::COMMAND_TYPE_RENDER_TO_TEXTURE_BEGIN: {
                        //renderEncoder =
                        auto* cmd_viewport = &cmd->viewport;
                        [self bake_to_texture_begin:cmd_viewport->width height:cmd_viewport->height];
                        renderEncoder = new_render_target.render_command_encoder;
                        [renderEncoder setViewport:(MTLViewport)
                         {cmd_viewport->x, cmd_viewport->y, cmd_viewport->width, cmd_viewport->height, cmd_viewport->znear, cmd_viewport->zfar}];
                        [renderEncoder setCullMode:MTLCullModeNone];
                        
                        continue;
                    }
                    case sd::COMMAND_TYPE_RENDER_TO_TEXTURE_END: {
                        [self bake_to_texture_end:cmd->callback];
                        renderEncoder = main_render_encoder;
                        continue;
                    }
                    default: { continue; }
                    }
                    
                    {
                        const usize idx_count = layer_range.count;
                        if (idx_count <= 0) {
                            continue;
                        }
                        auto* drawable_info_list = render_layer->drawable_info_list;
                        usize skipped_count = 0;
                        for (usize i = layer_range.first_index; i < layer_range.count; i += 1) {
                            sd::Drawable_Info* const rc = &render_layer->drawable_info_list[i];

                            if (!rc->is_enabled || rc->i_buffer_count == 0) MTT_UNLIKELY {
                                //per_entity_data += 1;
                                skipped_count += 1;
                                continue;
                            }
                            
                            *per_entity_data = drawable_info_list[i].instance_data;
                            
                            per_entity_data += 1;
                        }
                        
                        id<MTLBuffer> v_buffer_to_set = (id<MTLBuffer>)vertex_buffer_list[buffer_layer_idx];
                        if (self->active_v_buffer_0 != v_buffer_to_set) {
                            self->active_v_buffer_0 = v_buffer_to_set;
                            [renderEncoder setVertexBuffer:v_buffer_to_set offset:0 atIndex:0];
                        }
                        
                        //                       [renderEncoder setVertexBufferOffset:_entity_uniform_buffer_offset + ((i_uniform_offset + i) * sizeof(InstanceUniforms)) atIndex:kBufferIndexEntityUniforms];
                        skipped_count = 0;
                        if (_pipeline_mode != sd::PIPELINE_MODE_TEXTURE) {
                            for (usize i = layer_range.first_index; i < layer_range.count; i += 1) {
                                
                                sd::Drawable_Info* const rc = &render_layer->drawable_info_list[i];
                                if (!rc->is_enabled || rc->i_buffer_count == 0) MTT_UNLIKELY {
                                    skipped_count += 1;
                                    continue;
                                }
                                
                                //                         [renderEncoder setVertexBufferOffset:_entity_uniform_buffer_offset + ((i_uniform_offset + i) * sizeof(InstanceUniforms)) atIndex:kBufferIndexEntityUniforms];
                                
                                [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                                          indexCount:rc->index_count()
                                                           indexType:MTLIndexTypeUInt32
                                                         indexBuffer:((id<MTLBuffer>)index_buffer_list[buffer_layer_idx])
                                                   indexBufferOffset:rc->i_buffer_offset * sizeof(sd::Index_Type)
                                                       instanceCount:1
                                                          baseVertex:rc->base_vertex
                                                        baseInstance:(i_uniform_offset + i - skipped_count)];
                                
                            }
                        } else {
                            
                            if (!sd::max_textures_reached(renderer.images)) MTT_LIKELY {
                                for (usize i = layer_range.first_index; i < layer_range.count; i += 1) {
                                    
                                    sd::Drawable_Info* const rc = &render_layer->drawable_info_list[i];
                                    if (!rc->is_enabled || rc->i_buffer_count == 0) MTT_UNLIKELY {
                                        skipped_count += 1;
                                        continue;
                                    }
                                    
                                    //                         [renderEncoder setVertexBufferOffset:_entity_uniform_buffer_offset + ((i_uniform_offset + i) * sizeof(InstanceUniforms)) atIndex:kBufferIndexEntityUniforms];
                                    
                                    [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                                              indexCount:rc->index_count()
                                                               indexType:MTLIndexTypeUInt32
                                                             indexBuffer:((id<MTLBuffer>)index_buffer_list [buffer_layer_idx])
                                                       indexBufferOffset:rc->i_buffer_offset * sizeof(sd::Index_Type)
                                                           instanceCount:1
                                                              baseVertex:rc->base_vertex
                                                            baseInstance:(i_uniform_offset + i - skipped_count)];
                                    
                                }
                            } else {
                                for (usize i = layer_range.first_index; i < layer_range.count; i += 1) {
                                    
                                    sd::Drawable_Info* const rc = &render_layer->drawable_info_list[i];
                                    if (!rc->is_enabled || rc->i_buffer_count == 0) MTT_UNLIKELY {
                                        skipped_count += 1;
                                        continue;
                                    }
                                    
                                    //                         [renderEncoder setVertexBufferOffset:_entity_uniform_buffer_offset + ((i_uniform_offset + i) * sizeof(InstanceUniforms)) atIndex:kBufferIndexEntityUniforms];
                                    
                                    switch (sd::texture_ensure_resident(renderer.images, rc->get_texture_ID(), renderEncoder)) {
                                    case sd::TEXTURE_ENSURE_STATUS_NOT_LOADED: {
                                        ASSERT_MSG(false, "ERROR: texture reloading not yet implemented!\n");
                                        break;
                                    }
                                    case sd::TEXTURE_ENSURE_STATUS_MADE_RESIDENT: {
                                        // ok
                                        break;
                                    }
                                    case sd::TEXTURE_ENSURE_STATUS_IS_RESIDENT: {
                                        // ok
                                        break;
                                    }
                                    }
                                    
                                    
                                    [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                                              indexCount:rc->index_count()
                                                               indexType:MTLIndexTypeUInt32
                                                             indexBuffer:((id<MTLBuffer>)index_buffer_list [buffer_layer_idx])
                                                       indexBufferOffset:rc->i_buffer_offset * sizeof(sd::Index_Type)
                                                           instanceCount:1
                                                              baseVertex:rc->base_vertex
                                                            baseInstance:(i_uniform_offset + i - skipped_count)];
                                    
                                }
                            }
                        }
                        
                        i_uniform_offset +=
                        render_layer->drawable_info_list_count - skipped_count;
                    }
                    

                    //+
                    //   render_layer->render_commands_path_count
                    ;
                    
                }
            }
            
//            for (usize i = 0; i < self->renderer->render_layer_count; i += 1) {
//                [self->vertex_buffer_list[i] didModifyRange:NSMakeRange(0, self->renderer->render_layers[i].polygon_color_vertices_count_bytes)];
//                [self->index_buffer_list[i] didModifyRange:NSMakeRange(0, self->renderer->render_layers[i].polygon_color_indices_count_bytes)];
//            }

            

            
            renderEncoder = nil;
            main_render_encoder = nil;
        }
        
        
        // Schedule a present once the framebuffer is complete using the current drawable
        command_buffer = main_command_buffer;
        [command_buffer presentDrawable:view.currentDrawable afterMinimumDuration:MTT_TIMESTEP];
        // Finalize rendering here & push the command buffer to the GPU
        [command_buffer commit];
        
        command_buffer = nil;
        main_command_buffer = nil;
        _viewport_size_did_change = NO;
        
        _buffer_idx += 1;
        if (_buffer_idx == MAX_BUFFERS_IN_FLIGHT) {
            _buffer_idx = 0;
        }
        
    }
    
#if (LOG_SIGNPOST_ENABLED)
    os_signpost_interval_end(_logger, _os_signpost_render_update_id, "animation update");
    os_signpost_interval_begin(_logger, _os_signpost_render_update_id, "animation wait");
#endif
}
- (void)drawInMTKView:(nonnull MTKView*)view
{
    [self update:view];
    
    //
    //
    //    //TODO(Toby): remove
    //    /// this semaphore waits forever if no of the two frames go handeld
    //    dispatch_semaphore_wait(_in_flight_semaphore, DISPATCH_TIME_FOREVER);
    //
    //    /// here we create the command buffer
    //    id <MTLCommandBuffer> command_buffer = [_command_queue commandBuffer];
    //    command_buffer.label = @"render frame"; /// we add a label which is nice when we view the buffer in the gpu capture
    //
    //    /// create a block variable for the semaphore so we can access it from there
    //    __block dispatch_semaphore_t block_sema = _in_flight_semaphore;
    //
    //    /// increment the retain count of the image refrences so that they don't get deleted by ARC
    //    CVBufferRef cImageTextureYRef    = CVBufferRetain(_captured_image_texture_y_ref);
    //    CVBufferRef cImageTextureCbCrRef = CVBufferRetain(_captured_image_texture_cbcr_ref);
    //
    //    /// Our callback handler for a finished frame
    //    [command_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
    //         dispatch_semaphore_signal(block_sema);
    //        #pragma mark Release Camera Image
    //
    //         /// we call release to give the memory for both buffers back as they are not used anymore
    //         CVBufferRelease(cImageTextureYRef);
    //         CVBufferRelease(cImageTextureCbCrRef);
    //
    //
    //
    //        #pragma mark Image Compression
    //        // I'm unsure if this makes sense to move to a other thread
    //        // i did use a dispatch queue before but it doesn't make any diffrence
    //
    //        NSUInteger currentIdx = self->_buffer_idx; // store current image index
    //
    //        CVImageBufferRef rndFrame;
    //
    //        /// create a cv pixel bffer from the metalbuffer
    //        CVPixelBufferCreateWithBytes(kCFAllocatorMalloc,
    //                                     self->_viewport_size.width,
    //                                     self->_viewport_size.height,
    //                                     kCVPixelFormatType_32BGRA,
    //                                     [self->_capture_buffers[currentIdx] contents],
    //                                     self->_viewport_size.width * 4,
    //                                     NULL,
    //                                     NULL,
    //                                     (__bridge CFDictionaryRef _Nullable)(self->_defaultAttributes),
    //                                     &rndFrame);
    //
    //          /// start the encoding of a frame from the compression session
    //          VTCompressionSessionEncodeFrame(self->_compression_session,
    //                                          rndFrame,
    //                                          CMTimeMake(0, 0), // dunno if we should use a valid time here
    //                                          kCMTimeInvalid,
    //                                          NULL,
    //                                          (void*)currentIdx, // we can use this for reordering of the frames
    //                                          NULL);
    //
    //          /// relases the frame that needs to be renderd the frame can still exists because of the retain count
    //          CFRelease(rndFrame);
    //
    //     }];
    //
    //    /// here we do the arkit creation of the buffers and the transformation of the texture coordinates
    //    [self _handle_ARKit_and_transform];
    //
    //    /// we get a render pass descriptor from the view this can also be manually allocated when you do your own metal view
    //    MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
    //
    //    /// don't draw if we don't have a render pass descriptor
    //    if (renderPassDescriptor != nil) {
    //
    //      /// store the current size of the main view which is equal to the display size
    //      _viewport_size = view.frame.size;
    //
    //#pragma mark Draw ARKit
    //
    //        /// check if we have both textures if not just return we don't render here anymore
    //        if(_captured_image_texture_cbcr_ref == nil || _captured_image_texture_y_ref == nil) {
    //            return;
    //        }
    //
    //        /// create the render encoder for the AR image drawing
    //        id <MTLRenderCommandEncoder> renderARImageEncoder =
    //        [command_buffer renderCommandEncoderWithDescriptor: renderPassDescriptor];
    //        renderARImageEncoder.label = @"ARImage RenderEncoder"; /// add a label for debugging
    //        /// add a debug group for nice folding of the render command encoder
    //        [renderARImageEncoder pushDebugGroup:@"Draw ARImage"];
    //        {
    //            /// set culling to non we don't want this when we draw the image of the camera
    //            [renderARImageEncoder setCullMode:MTLCullModeNone];
    //
    //            /// use our prevalidated piplinestate
    //            [renderARImageEncoder setRenderPipelineState: _captured_image_pipeline_state];
    //
    //            /// add the depth buffer
    //            [renderARImageEncoder setDepthStencilState: _captured_image_depth_state];
    //
    //            /// get the metal textures from the texture ref
    //            [renderARImageEncoder setFragmentTexture: CVMetalTextureGetTexture(_captured_image_texture_y_ref)
    //                                           atIndex: TextureIndexY];
    //
    //            [renderARImageEncoder setFragmentTexture: CVMetalTextureGetTexture(_captured_image_texture_cbcr_ref)
    //                                           atIndex: TextureIndexCbRc];
    //
    //            /// set the transform uniforms it's all at index zero and buffer zero
    //            [renderARImageEncoder setVertexBuffer:_textureCoordinates offset:0 atIndex:0];
    //
    //            /// draw the camera image via triangle strip
    //            [renderARImageEncoder drawPrimitives: MTLPrimitiveTypeTriangleStrip
    //                         vertexStart: 0
    //                         vertexCount: 4];
    //
    //            // MARK: TODO(Toby) draw more here
    //
    //            /// stop the encoding of the render command encoder we could add more here but i didn't
    //            [renderARImageEncoder endEncoding];
    //        }
    //        /// pop the debug group again
    //        [renderARImageEncoder popDebugGroup];
    //
    //#pragma mark Image Capture
    //
    //        /// create a blit command encoder it is just used for copying buffers
    //        /// if you move this after your rendering it also adds that data to the capture buffer
    //        id <MTLBlitCommandEncoder> blit_AR_image_encoder = [command_buffer blitCommandEncoder];
    //
    //        /// here we create a debug group again
    //        [blit_AR_image_encoder pushDebugGroup: @"BlitCommandEncoder"];
    //        {
    //
    //            /// here we do the copy of the buffers from drawable to captureBuffer
    //            [blit_AR_image_encoder copyFromTexture: view.currentDrawable.texture
    //                          sourceSlice: 0
    //                          sourceLevel: 0
    //                         sourceOrigin: MTLOriginMake(0, 0, 0)
    //                           sourceSize: MTLSizeMake(view.frame.size.width, view.frame.size.height, 1)
    //                             toBuffer: _capture_buffers[_buffer_idx]
    //                    destinationOffset: 0
    //               destinationBytesPerRow: view.frame.size.width * 4 destinationBytesPerImage:0];
    //        }
    //        /// pop the debug group
    //        [blit_AR_image_encoder popDebugGroup];
    //
    //        /// stop the encoding now no new commands can be send to the blit encoder
    //        [blit_AR_image_encoder endEncoding];
    //
    //    }
    //
    //    /// draw the drawable to the screen
    //    [command_buffer presentDrawable: view.currentDrawable];
    //
    //    /// commit all commands to the commandQeueu
    //    [command_buffer commit];
    //
    //    /// increment triple buffer counter
    //    _buffer_idx += 1;
    //    if (_buffer_idx > 2) {
    //        _buffer_idx = 0;
    //    }
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    [self drawRectResized:size];
}

-(void)deinit {
    sd::Renderer_deinit(&_core_platform->renderer);
    mtt::deinit(&vertex_buffer_list);
    mtt::deinit(&index_buffer_list);
    nvgDeleteMTL(_nvg_ctx);
    
}

-(void)pause {
    [_view setPaused:YES];
}
-(void)resume {
    self->prev_time_ns = mtt_time_nanoseconds();
    [_view setPaused:NO];
}

- (Boolean) supports_multiple_viewports {
    return [_device supportsFamily: MTLGPUFamilyMac2 ] ||
    [_device supportsFamily: MTLGPUFamilyApple5 ];
}

- (MTKTextureLoader*) get_texture_loader {
    return self->_texture_loader;
}
- (void) make_render_target:(usize) width height:(usize)height {

}

- (void)bake_to_texture_begin:(usize) width height:(usize)height {
    @autoreleasepool {
        
        MTLRenderPassDepthAttachmentDescriptor* depth = [[MTLRenderPassDepthAttachmentDescriptor alloc] init];
        MTLRenderPassStencilAttachmentDescriptor* stencil = [[MTLRenderPassStencilAttachmentDescriptor alloc] init];
        
        {
            MTLTextureDescriptor *texDescriptor = [[MTLTextureDescriptor alloc] init];
            
            
            if (USE_MULTISAMPLING) {
                texDescriptor.textureType = MTLTextureType2DMultisample;
                texDescriptor.width = width;
                texDescriptor.height = height;
                texDescriptor.pixelFormat = _view.colorPixelFormat;
                //texDescriptor.storageMode = MTLStorageModeShared;
                texDescriptor.storageMode = MTLStorageModePrivate;
                texDescriptor.usage = MTLTextureUsageRenderTarget |
                MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite | MTLTextureUsagePixelFormatView;
                texDescriptor.sampleCount = 4;
                
                self->new_render_target.render_target_multisample = [_device newTextureWithDescriptor:texDescriptor];
            }
            texDescriptor.textureType = MTLTextureType2D;
            texDescriptor.width = width;
            texDescriptor.height = height;
            texDescriptor.pixelFormat = _view.colorPixelFormat;
            //texDescriptor.storageMode = MTLStorageModeShared;
            texDescriptor.storageMode = MTLStorageModePrivate;
            texDescriptor.usage =
            MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
            texDescriptor.sampleCount = 1;
            
            self->new_render_target.render_target_final = [_device newTextureWithDescriptor:texDescriptor];
            
            

            {
                MTLTextureDescriptor* depthDescriptor = [[MTLTextureDescriptor alloc] init];
                depthDescriptor.textureType = (USE_MULTISAMPLING) ? MTLTextureType2DMultisample : MTLTextureType2D;
                depthDescriptor.pixelFormat = MTLPixelFormatDepth32Float_Stencil8;
                depthDescriptor.width  = width;
                depthDescriptor.height = height;
                depthDescriptor.mipmapLevelCount = 1;
                depthDescriptor.storageMode = MTLStorageModePrivate;
                depthDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
                depthDescriptor.sampleCount = (USE_MULTISAMPLING) ? 4 : 1;
                
                id<MTLTexture> depth_stencil_texture = [_device newTextureWithDescriptor:depthDescriptor];
                
                depth.clearDepth = 1.0;
                depth.loadAction = MTLLoadActionClear;
                depth.storeAction = MTLStoreActionStore;
                
                
                depth.texture = depth_stencil_texture;
                

                stencil.loadAction = MTLLoadActionClear;
                stencil.storeAction = MTLStoreActionStore;
                stencil.texture = depth_stencil_texture;
            }
        }
        
        MTLRenderPassDescriptor* _renderToTextureRenderPassDescriptor = self->new_render_target._renderToTextureRenderPassDescriptor;
        
  // TODO: need a resolve texture
        {
            if (USE_MULTISAMPLING) {
                id<MTLTexture> _offscreenTexture = self->new_render_target.render_target_multisample;
                
                _renderToTextureRenderPassDescriptor.colorAttachments[0].texture = _offscreenTexture;
                
                id<MTLTexture> _renderTargetTexture = self->new_render_target.render_target_final;
                
                _renderToTextureRenderPassDescriptor.colorAttachments[0].resolveTexture = _renderTargetTexture;
                
            } else {
                id<MTLTexture> _renderTargetTexture = self->new_render_target.render_target_final;
                
                _renderToTextureRenderPassDescriptor.colorAttachments[0].texture = _renderTargetTexture;
            }
            _renderToTextureRenderPassDescriptor.depthAttachment = depth;
            _renderToTextureRenderPassDescriptor.stencilAttachment = stencil;
        }
    
        self->new_render_target.command_buffer = [_command_queue commandBuffer];
        self->command_buffer = self->new_render_target.command_buffer;
        
        {
            id <MTLRenderCommandEncoder> renderEncoder = [self->command_buffer  renderCommandEncoderWithDescriptor:_renderToTextureRenderPassDescriptor];
            renderEncoder.label = @"bake to texture";
            self->new_render_target.render_command_encoder = renderEncoder;
        }
    }
}
- (void)bake_to_texture_end:(void(*)(bool, void*)) callback
{
    @autoreleasepool {
        [self->new_render_target.command_buffer commit];
        // TODO: it should be in the background, but this requires some more duplicate resources
        [self->new_render_target.command_buffer waitUntilCompleted];
        self->new_render_target.command_buffer = nil;
        self->new_render_target.render_command_encoder = nil;
        self->new_render_target.depth_stencil_texture = nil;
        self->new_render_target.render_target_final = nil;
        self->new_render_target.render_target_multisample = nil;
        callback(true, ( __bridge void*)self->new_render_target.render_target_multisample);
        self->command_buffer = self->main_command_buffer;
    }
}

- (Shader_ID) compute_shader_make_from_builtin:(NSString*_Nonnull) name
{
    @autoreleasepool {
        NSError* error = nil;
        
        id <MTLLibrary> source_library = default_shader_library;
        
        id<MTLFunction> main_function = [source_library newFunctionWithName:name];
        if (main_function == nil) {
            NSLog(@"Failed to find the desired function.");
            return Shader_ID_INVALID;
        }
        
        id<MTLComputePipelineState> pipeline = [_device newComputePipelineStateWithFunction: main_function error:&error];
        if (error) {
            NSLog(@"Error occurs after %s: %s",
                   __PRETTY_FUNCTION__, [[error localizedDescription] UTF8String]);
            return Shader_ID_INVALID;
        }

        
        uint64 next = _compute_pipelines.size();
        mtt::append(&_compute_pipelines, (id)pipeline);
        
        return next;
    }
}

- (Shader_ID) compute_shader_make:(NSString*_Nonnull) src with_function_name:(NSString*_Nonnull) function_name
{
    @autoreleasepool {
        NSError* error = nil;
        
        MTLCompileOptions* compile_opts = [[MTLCompileOptions alloc] init];
        compile_opts.fastMathEnabled = YES;
        
        id<MTLLibrary> source_library = [_device newLibraryWithSource:src options:compile_opts error:&error];
        
        if (error) {
            NSLog(@"Error occurs after %s: %s",
                   __PRETTY_FUNCTION__, [[error localizedDescription] UTF8String]);
            return Shader_ID_INVALID;
        }
        
        id<MTLFunction> main_function = [source_library newFunctionWithName:function_name];
        if (main_function == nil) {
            NSLog(@"Failed to find the desired function.");
            return Shader_ID_INVALID;
        }
        
        id<MTLComputePipelineState> pipeline = [_device newComputePipelineStateWithFunction: main_function error:&error];
        if (error) {
            NSLog(@"Error occurs after %s: %s",
                   __PRETTY_FUNCTION__, [[error localizedDescription] UTF8String]);
            return Shader_ID_INVALID;
        }

        uint64 next = 0;
        if (_compute_pipelines_free_list.empty()) {
            next = _compute_pipelines.size();
            mtt::append(&_compute_pipelines, (id)pipeline);
        } else {
            next = _compute_pipelines_free_list.back();
            mtt::pop(&_compute_pipelines_free_list);
            _compute_pipelines[next] = (id)pipeline;
        }
        
        return next;
    }
}

- (void) compute_shader_destroy:(Shader_ID)shader_id
{
    _compute_pipelines_free_list.push_back(shader_id);
    _compute_pipelines[shader_id] = nil;
}

- (Shader_ID) compute_shader_replace:(NSString*_Nonnull) src with_function_name:(NSString*_Nonnull) function_name with_shader_id:(Shader_ID)shader_id
{
    if ([self compute_shader_lookup_with_id:shader_id] == nil) {
        return [self compute_shader_make:src with_function_name:function_name];
    }
    
    @autoreleasepool {
        NSError* error = nil;
        
        MTLCompileOptions* compile_opts = [[MTLCompileOptions alloc] init];
        compile_opts.fastMathEnabled = YES;
        
        id<MTLLibrary> source_library = [_device newLibraryWithSource:src options:compile_opts error:&error];
        
        if (error) {
            NSLog(@"Error occurs after %s: %s",
                   __PRETTY_FUNCTION__, [[error localizedDescription] UTF8String]);
            return Shader_ID_INVALID;
        }
        
        id<MTLFunction> main_function = [source_library newFunctionWithName:function_name];
        if (main_function == nil) {
            NSLog(@"Failed to find the desired function.");
            return Shader_ID_INVALID;
        }
        
        id<MTLComputePipelineState> pipeline = [_device newComputePipelineStateWithFunction: main_function error:&error];
        if (error) {
            NSLog(@"Error occurs after %s: %s",
                   __PRETTY_FUNCTION__, [[error localizedDescription] UTF8String]);
            return Shader_ID_INVALID;
        }
        _compute_pipelines[shader_id] = nil;
        _compute_pipelines[shader_id] = (id)pipeline;
    }
    
    return shader_id;
}

- (id<MTLComputePipelineState>) compute_shader_lookup_with_id:(sd::Shader_ID) shader_id
{
    if (shader_id >= _compute_pipelines.size()) {
        return nil;
    }
    return _compute_pipelines[shader_id];
}

- (id<MTLDevice>) active_device
{
    return self->_device;
}


#if !TARGET_OS_OSX

- (ARSession*) ARSession_get {
    return self->_session;
}
- (void) ARSession_set:(ARSession*) session {
    self->_session = session;
}


#endif

- (id<MTLBuffer>) scene_argument_buffer {
    return self->_scene_data;
}
- (id<MTLBuffer>) scene_resource_buffer {
    return self->_scene_resource_data;
}


@end


vec4 view_offset_for_safe_area_platform(Platform_View* _Nonnull view)
{
#if !TARGET_OS_OSX
    if (@available(iOS 11.0, *))
#endif
    {
        CGFloat topPadding    = view.safeAreaInsets.top;
        CGFloat bottomPadding = view.safeAreaInsets.bottom;
        CGFloat leftPadding   = view.safeAreaInsets.left;
        CGFloat rightPadding   = view.safeAreaInsets.right;

        return vec4(topPadding, bottomPadding, leftPadding, rightPadding);
    }
    
    return vec4(1.0f);
}



