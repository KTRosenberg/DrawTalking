//
//  image_platform.mm
//  Make The Thing
//
//  Created by Toby Rosenberg on 10/15/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//


#include "image_platform.hpp"
#include "stratadraw_platform_apple.hpp"
#import "sd_metal_renderer.h"
#include "file_system_platform.hpp"
#include "ShaderTypes.h"

namespace sd {

static NSString* main_directory_path = nil;

/*
 enum TEXTURE_USAGE : uint32 {
     TEXTURE_USAGE_unknown = 0,
     TEXTURE_USAGE_shader_read = 1,
     TEXTURE_USAGE_shader_write = 2,
     TEXTURE_USAGE_render_target = 3,
     TEXTURE_USAGE_pixel_format_view = 4
 };

 */
static constexpr MTLTextureUsage usage_types[] = {
    [TEXTURE_USAGE_unknown] = MTLTextureUsageUnknown,
    [TEXTURE_USAGE_shader_read] = MTLTextureUsageShaderRead,
    [TEXTURE_USAGE_shader_write] = MTLTextureUsageShaderWrite,
    [TEXTURE_USAGE_render_target] = MTLTextureUsageRenderTarget,
    [TEXTURE_USAGE_pixel_format_view] = MTLTextureUsagePixelFormatView,
};

inline static Fragment_Shader_Arguments_Resources* get_buf(Images* imgs)
{
    return (Fragment_Shader_Arguments_Resources*)[Renderer_backend(imgs->renderer) scene_resource_buffer].contents;
}
inline static void set_texture(Fragment_Shader_Arguments_Resources* buf, MTLResourceID resource_id, usize idx)
{
    buf[idx].texture = resource_id;
}

void Images_init(sd::Renderer* renderer, Images* images, usize max_resident, const Images_Init_Args& args)
{
    *images = sd::Images();
    images->renderer = renderer;
    images->backend = Renderer_backend(renderer);
    
    images->support_bindless = args.support_bindless;
    images->default_pixel_format = (MTLPixelFormat)args.default_pixel_format;
    images->default_pixel_format_depth_stencil = (MTLPixelFormat)args.default_pixel_format_depth_stencil;

    //mtt::init(&images->lookup, &desc);
    images->max_resident = max_resident;
    //images->resident_count = 0;
    images->nonresident_count = 0;
    images->count = 0;
    
    //images->textures             = [[NSMutableArray alloc] init];
    
    mtt::init(&images->_textures, sd::Renderer_backend(renderer)->_id_allocator);
    
    //images->id_to_nonresident_texture = [[NSMutableDictionary alloc] initWithCapacity:images->max_resident];
    images->next_avail_id = 0;
    
    renderer->images = images;
    
    mtt::init(&images->_sampler_states, sd::Renderer_backend(renderer)->_id_allocator);

    mtt::File_System_init_platform();
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
    NSString* path = (__bridge NSString*)mtt::File_System_get_main_directory_path_platform();

    if (!path) {
        NSLog(@"error creating directory");
        return;
    }
    
    NSFileManager* file_manager = [NSFileManager defaultManager];
    NSError* error = nil;
    [file_manager createDirectoryAtPath:[path stringByAppendingString:@"/images"]
            withIntermediateDirectories:YES
                             attributes:nil
                                  error:&error];
    if (error != nil) {
        NSLog(@"error creating directory: %@", error);
    } else {
        main_directory_path = [NSString stringWithFormat:@"%@", path];
    }
#else
    NSFileManager* file_manager = [NSFileManager defaultManager];
    
    NSString* path = (__bridge NSString*)mtt::File_System_get_main_directory_path_platform();
    
    NSError * error = nil;
    [file_manager createDirectoryAtPath:[path stringByAppendingString:@"/images"]
            withIntermediateDirectories:YES
                             attributes:nil
                                  error:&error];
    if (error != nil) {
        NSLog(@"error creating directory: %@", error);
    } else {
        main_directory_path = [NSString stringWithFormat:@"%@", path];
    }
#endif
    
    //images->id_to_slot
    //images->slot_to_id
    
    //images->id_to_slot.resize(1 + (images->max_resident * 4));
    //memset(images->id_to_slot.data(), 0, (1 + (images->max_resident * 4)) * sizeof(usize));
    
}

bool max_textures_reached(Images* imgs)
{
    return imgs->count > imgs->max_resident;
}

// TODO: deletion
void Image_create_from_baked_platform(Images* imgs, cstring name, void* texture, void (*callback)(Image* image)) {
    Image* img = Image_create_from_MTLTexture(imgs, name, (__bridge id<MTLTexture>)texture, {.usage_flags = 0});
    callback(img);
}

Image* Image_create_from_MTLTexture(Images* imgs, mtt::String name, id<MTLTexture> texture, const Image_Create_Args& args)
{
    sd::Texture_ID ID = 0;
    usize dense_idx = 0;
        
    ASSERT_MSG(imgs->count <= imgs->dense.size(), "ID is invalid");
    
    sd::Texture_ID* __ = nullptr;
    if (mtt::map_try_get(imgs->name_to_id, name, &__)) {
        
        Image* img = &imgs->id_to_img.find(*__)->second;
        Image_replace_associated_texture(imgs, img, texture);
        texture.label = [[NSString alloc] initWithUTF8String:img->name.c_str()];
        return img;
    }
    
    // out of space
    if (imgs->count == imgs->dense.size()) {
        auto* record = &imgs->dense.emplace_back(Dense{});
        ID = imgs->next_avail_id++;
        while (ID >= imgs->sparse.size()) {
            imgs->sparse.push_back(-1);
        }
        record->ID = ID;
        record->dense_idx = imgs->dense.size() - 1;
        dense_idx = record->dense_idx;
        imgs->count += 1;
        imgs->sparse[record->ID] = record->dense_idx;
        mtt::append(&imgs->_textures, (id)texture);
        
        if (imgs->support_bindless) {
            set_texture(get_buf(imgs), texture.gpuResourceID, dense_idx);
        }
        

    } else {
        auto* record = &imgs->dense[imgs->count];
        assert(record->ID != -1);
        ID = record->ID;
        dense_idx = imgs->count;
        imgs->_textures[dense_idx] = nil;
        imgs->_textures[dense_idx] = (id)texture;
        imgs->count += 1;
        record->dense_idx = dense_idx;
        
        if (imgs->support_bindless) {
            set_texture(get_buf(imgs), texture.gpuResourceID, dense_idx);
        }
    }
    
    
    
    Image* img = nullptr;
    if (mtt::map_try_get(imgs->id_to_img, ID, &img)) {
        // TODO: replace?
        ASSERT_MSG(false, "overwriting texture is unsupported for now!\n");
    } else {
        img = &imgs->id_to_img.emplace(ID, Image{}).first->second;
    }
    
    img->texture_ID = ID;
    img->texture_sampler_ID = sd::DEFAULT_TEXTURE_SAMPLER_ID;
    img->name       = name;
    img->type = 1;
    img->size.x         = texture.width;
    img->size.y         = texture.height;
    img->format         = texture.pixelFormat;
    img->allocated_size = texture.allocatedSize;
    
    imgs->name_to_id.emplace(img->name, ID);
    
    texture.label = [[NSString alloc] initWithUTF8String:img->name.c_str()];
    
    return img;
}

void Image_replace_associated_texture(Images* imgs, Image* img, id<MTLTexture> new_texture)
{
    usize idx = imgs->sparse[img->texture_ID];
    imgs->_textures[idx] = new_texture;
    
    img->size.x         = new_texture.width;
    img->size.y         = new_texture.height;
    img->format         = new_texture.pixelFormat;
    img->allocated_size = new_texture.allocatedSize;
    Image_free(img);
    
    if (imgs->support_bindless) {
        set_texture(get_buf(imgs), new_texture.gpuResourceID, idx);
    }
}

sd::Image* Image_create_empty(sd::Images* imgs, const mtt::String& name, const Image_Create_Args& args)
{
    sd::Texture_ID ID = 0;
    usize dense_idx = 0;
    
    ASSERT_MSG(imgs->count <= imgs->dense.size(), "ID is invalid");
    // out of space
    if (imgs->count == imgs->dense.size()) {
        auto* record = &imgs->dense.emplace_back(Dense{});
        ID = imgs->next_avail_id++;
        while (ID >= imgs->sparse.size()) {
            imgs->sparse.push_back(-1);
        }
        record->ID = ID;
        record->dense_idx = imgs->dense.size() - 1;
        dense_idx = record->dense_idx;
        imgs->count += 1;
        imgs->sparse[record->ID] = record->dense_idx;
        mtt::append(&imgs->_textures, (id)imgs->_textures[1]);
        
        //[imgs->textures addObject:texture];
    // have space left by deleted entry
    } else {
        auto* record = &imgs->dense[imgs->count];
        assert(record->ID != -1);
        ID = record->ID;
        dense_idx = imgs->count;
        imgs->_textures[dense_idx] = nil;
        imgs->count += 1;
    }
    
    
    
    Image* img = nullptr;
    if (mtt::map_try_get(imgs->id_to_img, ID, &img)) {
        // TODO: replace?
        ASSERT_MSG(false, "overwriting texture is unsupported for now!\n");
    } else {
        img = &imgs->id_to_img.emplace(ID, Image{}).first->second;
    }
    
    img->texture_ID = ID;
    img->texture_sampler_ID = sd::DEFAULT_TEXTURE_SAMPLER_ID;
    img->name       = name;
    img->type = 1;
    img->size.x         = 0;
    img->size.y         = 0;
    img->format         = 0;
    img->allocated_size = 0;
    
    imgs->name_to_id.emplace(img->name, ID);
    
    return img;
}


sd::Image* Image_create_empty_w_size(sd::Images* imgs, const mtt::String& name, uvec2 size, const Image_Create_Args& args, Texture_Handle* out_handle)
{
    if (size.x == 0.0f || size.y == 0.0f) {
        return nullptr;
    }
    
    sd::Texture_ID ID = sd::Texture_ID_INVALID;
    
    
    sd::Texture_ID* __ = nullptr;
    Image* img = nullptr;
    if (mtt::map_try_get(imgs->name_to_id, name, &__)) {
        img = &imgs->id_to_img.find(*__)->second;
        ID = img->texture_ID;
    } else {
        usize dense_idx = 0;
        ASSERT_MSG(imgs->count <= imgs->dense.size(), "ID is invalid");
        // out of space
        if (imgs->count == imgs->dense.size()) {
            auto* record = &imgs->dense.emplace_back(Dense{});
            ID = imgs->next_avail_id++;
            while (ID >= imgs->sparse.size()) {
                imgs->sparse.push_back(-1);
            }
            record->ID = ID;
            record->dense_idx = imgs->dense.size() - 1;
            dense_idx = record->dense_idx;
            imgs->count += 1;
            imgs->sparse[record->ID] = record->dense_idx;
            mtt::append(&imgs->_textures, (id)imgs->_textures[1]);
            
            //[imgs->textures addObject:texture];
            // have space left by deleted entry
        } else {
            auto* record = &imgs->dense[imgs->count];
            assert(record->ID != -1);
            ID = record->ID;
            dense_idx = imgs->count;
            imgs->_textures[dense_idx] = nil;
            imgs->count += 1;
        }
        
        img = &imgs->id_to_img.emplace(ID, Image{}).first->second;
    }
    
    img->texture_ID = ID;
    img->texture_sampler_ID = sd::DEFAULT_TEXTURE_SAMPLER_ID;
    img->name       = name;
    img->type = 1;
    img->size.x         = size.x;
    img->size.y         = size.y;
    img->format         = 0;
    img->allocated_size = img->size.x * img->size.y * 4;
    
    MTLTextureDescriptor* texture_desc = [[MTLTextureDescriptor alloc] init];
    texture_desc.pixelFormat  = imgs->default_pixel_format;
    texture_desc.storageMode = MTLStorageModeShared;
    texture_desc.width  = img->size.x;
    texture_desc.height = img->size.y;
    texture_desc.usage = 0;
    if (args.for_msaa) {
        texture_desc.textureType = MTLTextureType2DMultisample;
        texture_desc.sampleCount = args.sample_count;
    }
    if (args.for_depth) {
        texture_desc.pixelFormat = imgs->default_pixel_format_depth_stencil;
    }
    
    uint64 flags = args.usage_flags;
    if (flags == TEXTURE_USAGE_unknown) {
        texture_desc.usage |= usage_types[TEXTURE_USAGE_shader_read];
        texture_desc.usage |= usage_types[TEXTURE_USAGE_shader_write];
    } else {
        if ((flags & TEXTURE_USAGE_shader_read) != 0) {
            texture_desc.usage |= usage_types[TEXTURE_USAGE_shader_read];
        }
        if ((flags & TEXTURE_USAGE_shader_write) != 0) {
            texture_desc.usage |= usage_types[TEXTURE_USAGE_shader_write];
        }
        if ((flags & TEXTURE_USAGE_render_target) != 0) {
            texture_desc.usage |= usage_types[TEXTURE_USAGE_render_target];
            texture_desc.storageMode = MTLStorageModePrivate;
        }
        if ((flags & TEXTURE_USAGE_pixel_format_view) != 0) {
            texture_desc.usage |= usage_types[TEXTURE_USAGE_pixel_format_view];
        }
    }
    
    
    SD_Renderer_Metal_Backend* backend = Renderer_backend(imgs->renderer);
    id<MTLTexture> texture = [[backend active_device] newTextureWithDescriptor:texture_desc];
    if (texture == nil) {
        NSLog(@"ERROR could not create texture");
        return nullptr;
    }
    
    texture.label = [[NSString alloc] initWithUTF8String:img->name.c_str()];

    
    imgs->_textures[ID] = (id)texture;
    
    auto* buf = get_buf(imgs);
    set_texture(buf, texture.gpuResourceID, ID);

    
    imgs->name_to_id.emplace(img->name, ID);
    
    out_handle->id = ID;
    out_handle->ref = (__bridge void*)texture;
    
    return img;
}

bool Images_lookup_platform(sd::Images* images, cstring name, Image** out_img)
{
    sd::Texture_ID* t_id;
    if (mtt::map_try_get(images->name_to_id, name, &t_id)) {
        *out_img = &images->id_to_img.find(*t_id)->second;
        return true;
    }
    
    return false;
}



// direct access
bool Texture_lookup(sd::Images* imgs, sd::Texture_ID t_id, Image** out_img, Texture_Handle* out_handle)
{
    auto find_it = imgs->id_to_img.find(t_id);
    if (find_it == imgs->id_to_img.end()) {
        return false;
    }
    
    *out_img = &imgs->id_to_img.find(t_id)->second;
    out_handle->id = t_id;
    out_handle->ref = (__bridge void*)imgs->_textures[t_id];
    
    return true;
}

void Images_for_each(sd::Images* images, void (callback)(sd::Image* img))
{
    for (auto [key, val] : images->id_to_img) {
        callback(&val);
    }
}

mtt::Dynamic_Array<id>* textures(Images* imgs)
{
    return &imgs->_textures;
}

inline usize resident_texture_count(Images* imgs)
{
    return m::min(imgs->count, imgs->max_resident);
}

bool texture_is_valid(sd::Renderer* renderer, sd::Texture_ID texture_id)
{
    auto* imgs = sd::images(renderer);
    return texture_id < imgs->sparse.size()
            && imgs->sparse[texture_id] != sd::Texture_ID_INVALID;
}

TEXTURE_ENSURE_STATUS texture_ensure_resident(Images* imgs, sd::Texture_ID ID, id<MTLRenderCommandEncoder> render_encoder)
{
    isize idx = imgs->sparse[ID];
    // does not exist
    if (idx == sd::Texture_ID_INVALID) {
        // caller could reload ...
        // TODO: generation bits
        return TEXTURE_ENSURE_STATUS_NOT_LOADED;
    }
    // is resident
    if (idx < imgs->max_resident) {
        return TEXTURE_ENSURE_STATUS_IS_RESIDENT;
    }
    // not resident, must make resident by making an existing texture non-resident
    
    isize resident_idx = imgs->max_resident - 1;
    
    
    // update dense (compact) list
    Dense* dense_resident    = &imgs->dense[resident_idx];
    Dense* dense_nonresident = &imgs->dense[idx];
    
    Texture_ID id_to_make_nonresident = dense_resident->ID;
    Texture_ID id_to_make_resident = dense_nonresident->ID;
    
    dense_resident->ID    = id_to_make_resident;
    dense_nonresident->ID = id_to_make_nonresident;
    
    // update sparse (id to dense mapping) list
    imgs->sparse[id_to_make_resident]    = dense_resident->dense_idx;
    imgs->sparse[id_to_make_nonresident] = dense_nonresident->dense_idx;
    
    // update texture ids (parallel with dense)
    // texture to make resident
    id<MTLTexture> tex_to_swap_in  = imgs->_textures[idx];
    // texture to make non-resident
    id<MTLTexture> tex_to_swap_out = imgs->_textures[resident_idx];
    
    imgs->_textures[idx] = tex_to_swap_out;
    imgs->_textures[resident_idx] = tex_to_swap_in;
    
    
    
    if (imgs->support_bindless) {
        auto* buf = get_buf(imgs);
        set_texture(buf, tex_to_swap_out.gpuResourceID, idx);
        set_texture(buf, tex_to_swap_in.gpuResourceID, resident_idx);
    } else {
        [render_encoder setFragmentTexture:tex_to_swap_in atIndex:resident_idx];
    }
    
    return TEXTURE_ENSURE_STATUS_MADE_RESIDENT;
}


void Image_Loader_init(sd::Renderer* renderer, Image_Loader* loader, Images* images, Image_Loader_Init_Args args)
{
    renderer->image_loader = loader;
}


void images_pick_async(sd::Renderer* renderer, Image_Loader* loader, Image_Pick_Descriptor* desc, bool (*callback)(Image_Load_Result))
{
    
}


bool images_load_remote_async(sd::Renderer* renderer, Image_Load_Descriptor* desc, bool (*callback)(const Image_Load_Result& result, bool status)) {
    if (desc->paths.size() == 0) {
        callback({}, false);
        return false;
    }
    
    static NSURLSession *defaultSession = [NSURLSession sessionWithConfiguration: [NSURLSessionConfiguration     defaultSessionConfiguration]];
    NSURLSession *otherSession = [NSURLSession sessionWithConfiguration: [NSURLSessionConfiguration     defaultSessionConfiguration]];

    @autoreleasepool {
        Image_Loader* loader = renderer->image_loader;
        MTKTextureLoader* tex_loader = (__bridge MTKTextureLoader*)loader->backend;
        
        __block Image_Load_Descriptor saved_desc = *desc;
        __block bool (*callback_to_use)(const Image_Load_Result& result, bool status) = callback;
        __block NSMutableArray* texture_results = [[NSMutableArray alloc] initWithCapacity:desc->paths.size()];
        __block NSMutableArray* tasks = [[NSMutableArray alloc] initWithCapacity:desc->paths.size()];
        __block Image_Load_Result result = {};
        result.images.resize(saved_desc.paths.size());
        result.custom_data = saved_desc.custom_data;
        
        sd::Images* imgs = renderer->images;
        result.image_storage = imgs;
        
        
        __block usize results_wait_count = saved_desc.paths.size();
        __block bool canceled = false;
        
        auto& net_info = saved_desc.net_connection_info;
        mtt::String host_str = "http://" + net_info.ip + ":" + std::to_string(net_info.port);
        __block NSString* host_str_native = [NSString stringWithUTF8String:host_str.c_str()];
        
        usize index = 0;
        for (auto it = saved_desc.paths.begin(); it != saved_desc.paths.end(); ++it) {
            NSURL* url = nil;
            url = [NSURL URLWithString:[NSString stringWithUTF8String:it->path.c_str()]];
            if (it->name.size() == 0) {
                it->name = [[[url URLByDeletingPathExtension] lastPathComponent] UTF8String];
            }
            if (it->extension.size() == 0) {
                it->extension = [[url pathExtension] UTF8String];
            }
            
            auto completion_handler = ^(NSData *data, NSURLResponse *response, NSError *err) {
                @autoreleasepool {
                    
                    if (canceled) {
                        return;
                    }
                    if(err != nil) {
                        canceled = true;
                        for (usize i = 0; i < tasks.count; i += 1) {
                            [((NSURLSessionDataTask*)tasks[i]) cancel];
                        }
                        
                        NSLog(@"Error %@\n", [err localizedDescription]);
                        callback_to_use((Image_Load_Result){.custom_data = saved_desc.custom_data}, false);
                        
                        texture_results = nil;
                        err = nil;
                    } else {
                        
                        
                        results_wait_count -= 1;
                        __block NSData* saved_data = data;
                        sd::Images* imgs = renderer->images;
                        [tex_loader newTextureWithData:data options:@{MTKTextureLoaderOptionGenerateMipmaps:@(NO), MTKTextureLoaderOptionSRGB:@(NO)} completionHandler:^(id<MTLTexture> _Nullable texture, NSError* _Nullable error) {
                            
                            @autoreleasepool {
                                
                                
                                if (error != nil) {
                                    saved_data = nil;
                                    __block NSError* err = error;
                                    canceled = true;
                                    
                                    dispatch_async(dispatch_get_main_queue(), ^{
                                        for (usize i = 0; i < tasks.count; i += 1) {
                                            [((NSURLSessionDataTask*)tasks[i]) cancel];
                                        }
                                        NSLog(@"Error %@\n", [err localizedDescription]);
                                        callback_to_use((Image_Load_Result){.custom_data = saved_desc.custom_data}, false);
                                        err = nil;
                                    });
                                    return;
                                }
                                
                                auto* img = Image_create_from_MTLTexture(imgs, saved_desc.paths[index].name, texture, {.usage_flags = 0});
                                {
                                    img->data = (__bridge_retained void*)saved_data;
                                }
                                result.images[index] = img;
                                
                                if (results_wait_count == 0) {
                                    
                                    

                                    
                                    
                                    callback_to_use(result, true);
                                    texture_results = nil;
                                    
                                    if (saved_desc.persist_data) {
                                        for (usize i = 0; i < result.images.size(); i += 1) {
                                            auto* img = result.images[i];
                                            img->free = [](Image* img) {
                                                @autoreleasepool {
                                                    NSData* data = (__bridge_transfer NSData*)img->data;
                                                    if (data != nil) {
                                                        img->data = nil;
                                                    }
                                                }
                                            };
                                        }
                                    } else {
                                        for (usize i = 0; i < result.images.size(); i += 1) {
                                            auto* img = result.images[i];
                                            @autoreleasepool {
                                                NSData* data = (__bridge_transfer NSData*)img->data;
                                                if (data != nil) {
                                                    img->data = nil;
                                                }
                                            }
                                        }
                                    }
                                }
                                

                                
                                saved_data = nil;
                            }
                        }];
                        
                        data = nil;
                        
                    }
                }
            };
            NSURLSessionDataTask* dataTask = nil;
            
            if ((saved_desc.flags & sd::IMAGE_LOAD_DESCRIPTOR_FLAG_REMOVE_BG) != 0) {
                NSMutableURLRequest* request = [[NSMutableURLRequest alloc] init];
                [request setHTTPMethod:@"GET"];
                
                auto* original = [url absoluteString];
                auto* prefix = original;
                
                // https://en.wikipedia.org/wiki/URL_encoding#Reserved_characters
                NSString* encoded_prefix = [prefix stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet URLQueryAllowedCharacterSet]];
                NSString *encoded = [encoded_prefix stringByReplacingOccurrencesOfString:@"=" withString:@"%3D"];
                encoded = [encoded stringByReplacingOccurrencesOfString:@"?" withString:@"%3F"];
                encoded = [encoded stringByReplacingOccurrencesOfString:@":" withString:@"%3A"];
                encoded = [encoded stringByReplacingOccurrencesOfString:@"/" withString:@"%2F"];
                encoded = [encoded stringByReplacingOccurrencesOfString:@"&" withString:@"%26"];
                
                auto final_encoded = [encoded stringByAppendingFormat:@"%@", @"&model=u2net&a=true&af=240&ab=10&ae=10&om=false&ppm=false"];
                NSString* url_str = [host_str_native stringByAppendingFormat:@"/?url=%@", final_encoded];
                [request setURL:[NSURL URLWithString: url_str]];
                [request setValue: @"application/json" forHTTPHeaderField:@"accept"];


                dataTask = [otherSession dataTaskWithRequest:request completionHandler:completion_handler];
            } else {
                dataTask = [defaultSession dataTaskWithURL:url completionHandler:completion_handler];
            }

            [tasks setObject:dataTask atIndexedSubscript:index];
            
            index += 1;
        }
        for (usize i = 0; i < tasks.count; i += 1) {
            [((NSURLSessionDataTask*)tasks[i]) resume];
        }
    }
    return false;
}
bool images_load_async(sd::Renderer* renderer, Image_Load_Descriptor* desc, bool (*callback)(const Image_Load_Result& result, bool status))
{
    @autoreleasepool {
    //    {
    //        NSString* str = @"https://i.pinimg.com/originals/80/dc/c5/80dcc558353c1998329f0fb0d95e781e.png";
    //
    //        NSURL* EARL = [NSURL URLWithString:str];
    //        NSURL* EARL_NO_EXTENSION = [EARL URLByDeletingPathExtension];
    //        NSString* last_component = [EARL lastPathComponent];
    //        NSString* extension = [EARL pathExtension];
    //        NSString* SIMPLER = [EARL_NO_EXTENSION lastPathComponent];
    //
    //        int BLA = 0;
    //    }
        
        //NSString* resource_path = [[[NSBundle mainBundle] resourcePath] stringByAppendingString:@"/"];

        NSMutableArray* urls = [[NSMutableArray alloc] initWithCapacity:desc->paths.size()];
        
        __block bool (*callback_to_use)(const Image_Load_Result& result, bool status) = callback;
        
        for (auto it = desc->paths.begin(); it != desc->paths.end(); ++it) {
            NSURL* url = nil;
            
            if (it->flags == PATH_FLAG_REMOTE) {
                url = [NSURL URLWithString:[NSString stringWithUTF8String:it->path.c_str()]];

                if (url == nil) {
                    callback_to_use({}, false);
                    return false;
                }
                if (it->name.size() == 0) {
                    it->name = [[[url URLByDeletingPathExtension] lastPathComponent] UTF8String];
                }
                if (it->extension.size() == 0) {
                    it->extension = [[url pathExtension] UTF8String];
                }
                // NSURLSession and NSURLDataTask,
                // then -newTextureWithData:options:error:
                [urls addObject:url];
                continue;
            }
            
            #if TARGET_OS_OSX || TARGET_OS_MACCATALYST

            {
                NSFileManager* file_manager = [NSFileManager defaultManager];
                NSString* path_string = nil;
                if (it->extension.size() > 0) {
                    path_string = [main_directory_path stringByAppendingFormat:@"/%@.%@", [NSString stringWithUTF8String:it->path.c_str()], [NSString stringWithUTF8String:it->extension.c_str()]];
                } else {
                    path_string = [main_directory_path stringByAppendingFormat:@"/%@", [NSString stringWithUTF8String:it->path.c_str()]];
                }
                BOOL file_exists = [file_manager fileExistsAtPath:path_string];
                if (file_exists) {
                    url = [NSURL fileURLWithPath:path_string];
                    [urls addObject:url];
                    continue;
                }
    //            [file_manager urls]
    //            NSArray<NSURL*>* urls = [file_manager URLsForDirectory: inDomains:NSUserDomainMask];
                //main_directory_path
            }
            
            #else
            
            {
                NSFileManager* file_manager = [NSFileManager defaultManager];
                NSString* path_string = nil;
                {
    //                NSArray* dirs = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:[main_directory_path      stringByAppendingString:@"/images"]
    //                                                                                    error:NULL];

    //                [dirs enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
    //                    NSString *filename = (NSString *)obj;
    //                    NSString *extension = [[filename pathExtension] lowercaseString];
    //                    int BP = 0;
    //                }];
                }
                if (it->extension.size() > 0) {
                    NSString* extension_string = [NSString stringWithUTF8String:it->extension.c_str()];
                    path_string = [main_directory_path stringByAppendingFormat:@"/%@.%@", [NSString stringWithUTF8String:it->path.c_str()], extension_string];
                    BOOL file_exists = [file_manager fileExistsAtPath:path_string];
                    if (!file_exists) {
                        path_string = [main_directory_path stringByAppendingFormat:@"/%@.%@", [NSString stringWithUTF8String:it->path.c_str()], [extension_string localizedUppercaseString]];
                    }
                } else {
                    path_string = [main_directory_path stringByAppendingFormat:@"/%@", [NSString stringWithUTF8String:it->path.c_str()]];
                }
                BOOL file_exists = [file_manager fileExistsAtPath:path_string];
                if (file_exists) {
                    url = [NSURL fileURLWithPath:path_string];
                    [urls addObject:url];
                    continue;
                }
                //            [file_manager urls]
                //            NSArray<NSURL*>* urls = [file_manager URLsForDirectory: inDomains:NSUserDomainMask];
                //main_directory_path
            }

            #endif
            
            url = [[NSBundle mainBundle] URLForResource:[NSString stringWithUTF8String:it->path.c_str()]
                                                 withExtension:[NSString stringWithUTF8String:it->extension.c_str()]];
            
            if (url == nil) {
                callback_to_use({}, false);
                return false;
            }
            
            [urls addObject:url];
        }
        
        Image_Loader* loader = renderer->image_loader;
        MTKTextureLoader* tex_loader = (__bridge MTKTextureLoader*)loader->backend;
        
        __block Image_Load_Descriptor saved_desc = *desc;
        
        
        if ((true) || saved_desc.require_all) {
            [tex_loader newTexturesWithContentsOfURLs:urls options:@{MTKTextureLoaderOptionGenerateMipmaps:@(NO), MTKTextureLoaderOptionSRGB:@(NO)} completionHandler:
            ^(NSArray<id<MTLTexture>>* _Nonnull textures, NSError* _Nullable error) {
                // error case
                if (error != nil) {
                    __block NSError* err = error;
                    dispatch_async(dispatch_get_main_queue(), ^{
                        NSLog(@"Error %@\n", [err localizedDescription]);
                        callback_to_use((Image_Load_Result){.custom_data = saved_desc.custom_data}, false);
                        err = nil;
                    });
                    return;
                }
                
                //__block usize count = textures.count;
                dispatch_async(dispatch_get_main_queue(), ^{
                    //assert([NSThread isMainThread]);
                    
                    Image_Load_Result result;
                    result.images.resize(saved_desc.paths.size());
                    result.custom_data = saved_desc.custom_data;
                    
                    sd::Images* imgs = renderer->images;

                    NSEnumerator* e = [textures objectEnumerator];
                    id<MTLTexture> texture;
                    usize idx = 0;
                    while (texture = [e nextObject]) {
                        auto* img = Image_create_from_MTLTexture(imgs, saved_desc.paths[idx].name
                                                                          //+ ((saved_desc.paths[idx].extension.size() > 1) ? "." + saved_desc.paths[idx].extension : "")
                                                                 , texture, { .usage_flags = 0});
                        result.images[idx] = img;
                        
                        idx += 1;
                    }
                    
                    result.image_storage = imgs;
                    
                    callback_to_use(result, true);
                });
            }];
        } else {
    //        - (NSArray <id <MTLTexture>> * __nonnull)newTexturesWithContentsOfURLs:(nonnull NSArray <NSURL *> *)URLs
    //    options:(nullable NSDictionary <MTKTextureLoaderOption, id> *)options
    //    error:(NSError *__nullable *__nullable)error NS_AVAILABLE(10_12, 10_0);
            
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
                NSError* err;
                NSArray <id <MTLTexture>> * __nonnull textures = [tex_loader newTexturesWithContentsOfURLs:urls options:@{MTKTextureLoaderOptionGenerateMipmaps:@(NO), MTKTextureLoaderOptionSRGB:@(NO)} error:&err];
                MTT_UNUSED(textures);
                
    //            dispatch_sync(dispatch_get_main_queue(), ^{
    //
    //            });
            });
        }
        
        return true;
    }
}


void activate_texture(sd::Texture_ID tex_id)
{
    
}

void Image_save(Images* images, Image* img, const Image_Save_Descriptor& args)
{
    NSString* main_path = (__bridge NSString*)mtt::File_System_get_main_directory_path_platform();
    auto* desc = &args;
    @autoreleasepool {
        NSData* to_write_original = (__bridge NSData*)img->data;
        __block NSData* to_write = [NSData dataWithData:to_write_original];
        ASSERT_MSG(to_write != nil, "data must exist\n");
        
        NSFileManager* file_manager = [NSFileManager defaultManager];
        BOOL file_exists = NO;
        usize version_number = 0;
        
        static const mtt::String sep = "__";
        
#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
        NSString* path_string = nil;
        
        do {
            if (desc->path.extension.size() > 0) {
                path_string = [main_path stringByAppendingFormat:@"/%@/%@.%@", [NSString stringWithUTF8String:desc->path.path.c_str()], [NSString stringWithUTF8String: mtt::String(desc->path.name + sep + std::to_string(version_number)).c_str()], [NSString stringWithUTF8String:desc->path.extension.c_str()]];
            } else {
                path_string = [main_path stringByAppendingFormat:@"/%@/%@", [NSString stringWithUTF8String:mtt::String(desc->path.name + sep + std::to_string(version_number)).c_str()], [NSString stringWithUTF8String:desc->path.path.c_str()]];
            }
            
            file_exists = [file_manager fileExistsAtPath:path_string];
            version_number += 1;
        } while (file_exists);
#else
        NSString* path_string = nil;
        
        do {
            const Path_Info* path = &desc->path;
            if (path->extension.size() > 0) {
                NSString* extension_string = [NSString stringWithUTF8String:path->extension.c_str()];
                path_string = [main_path stringByAppendingFormat:@"/%@/%@.%@", [NSString stringWithUTF8String:path->path.c_str()], [NSString stringWithUTF8String:mtt::String(desc->path.name + sep +std::to_string(version_number)).c_str()], extension_string];
                BOOL file_exists = [file_manager fileExistsAtPath:path_string];
                if (!file_exists) {
                    path_string = [main_path stringByAppendingFormat:@"/%@/%@.%@", [NSString stringWithUTF8String:path->path.c_str()], [NSString stringWithUTF8String:mtt::String(desc->path.name + sep +std::to_string(version_number)).c_str()], [extension_string localizedUppercaseString]];
                }
            } else {
                path_string = [main_path stringByAppendingFormat:@"/%@/%@", [NSString stringWithUTF8String:path->path.c_str()], [NSString stringWithUTF8String:mtt::String(desc->path.name + sep + std::to_string(version_number)).c_str()]];
            }
                
            file_exists = [file_manager fileExistsAtPath:path_string];
            version_number += 1;
        } while (file_exists);
#endif
        
    
        NSURL* url = [NSURL fileURLWithPath:path_string];
        __block NSURL* url_to_write = url;
        
        //__block NSString* str = [NSString stringWithUTF8String:desc->to_write.c_str()];
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0), ^{
            @autoreleasepool {
                ASSERT_MSG(!mtt::is_main_thread(), "This shouldn't happen in the main thread");
                NSError* error = nil;
                
                NSData* data = to_write;
                [data writeToURL:url_to_write options:NSDataWritingAtomic error:&error];
                if (error) {
                    NSLog(@"Error image write %@\n", [error localizedDescription]);
                } else {
                    NSLog(@"Image write successful");
                }
            }
        });
        
        
    }
}

}
