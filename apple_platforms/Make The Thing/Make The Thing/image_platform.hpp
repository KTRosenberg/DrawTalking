//
//  image_platform.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 3/6/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

#ifndef image_platform_hpp
#define image_platform_hpp

#include "image.hpp"



namespace sd {

inline usize resident_texture_count(Images* imgs);



typedef enum TEXTURE_ENSURE_STATUS {
    TEXTURE_ENSURE_STATUS_NOT_LOADED,
    TEXTURE_ENSURE_STATUS_MADE_RESIDENT,
    TEXTURE_ENSURE_STATUS_IS_RESIDENT,
} TEXTURE_ENSURE_STATUS;


#if defined(__OBJC__)

}

@class SD_Renderer_Metal_Backend;

namespace sd {

struct MTL_Texture {
    id<MTLTexture> texture;
    ~MTL_Texture()
    {
        this->texture = nil;
    }
};

mtt::Dynamic_Array<id>* textures(Images* imgs);

struct Dense {
    sd::Texture_ID ID;
    usize dense_idx;
};



struct Images {
    mtt::Map<mtt::String, sd::Texture_ID>      name_to_id;
    mtt::Map_Stable<sd::Texture_ID, sd::Image>  id_to_img;
    
    //NSMutableArray*                   textures;
    
    mtt::Dynamic_Array<id> _textures;
    
    std::vector<isize>                         sparse = std::vector<isize>();
    std::vector<Dense>                          dense = std::vector<Dense>();
    //NSMutableDictionary*        id_to_nonresident_texture;
    
    usize max_resident;
    
    sd::Renderer* renderer;
    
    sd::Texture_ID next_avail_id;
    //std::vector<sd::Texture_ID> free_ids;
    
    //usize resident_count;
    usize nonresident_count;
    usize count;
    
    mtt::Dynamic_Array<id> _sampler_states;
    
    bool support_bindless = false;
    
    SD_Renderer_Metal_Backend* backend;
    
    /*
     textureDescriptor.storageMode = MTLStorageModePrivate;
 #endif  // TARGET_OS_SIMULATOR
     tex->tex = [_metalLayer.device newTextureWithDescriptor:textureDescriptor];
     
     if (data != NULL) {
         NSUInteger bytesPerRow;
         if (tex->type == NVG_TEXTURE_RGBA) {
             bytesPerRow = width * 4;
         } else {
             bytesPerRow = width;
         }
         
         if (textureDescriptor.storageMode == MTLStorageModePrivate) {
             const NSUInteger kBufferSize = bytesPerRow * height;
             id<MTLBuffer> buffer = [_metalLayer.device
                                     newBufferWithLength:kBufferSize
                                     options:MTLResourceStorageModeShared];
             memcpy([buffer contents], data, kBufferSize);
             
             id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
             id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer
                                                             blitCommandEncoder];
             [blitCommandEncoder copyFromBuffer:buffer
                                   sourceOffset:0
                              sourceBytesPerRow:bytesPerRow
                            sourceBytesPerImage:kBufferSize
                                     sourceSize:MTLSizeMake(width, height, 1)
                                      toTexture:tex->tex
                               destinationSlice:0
                               destinationLevel:0
                              destinationOrigin:MTLOriginMake(0, 0, 0)];
             
             [blitCommandEncoder endEncoding];
             [commandBuffer commit];
             [commandBuffer waitUntilCompleted];
         } else {
             [tex->tex replaceRegion:MTLRegionMake2D(0, 0, width, height)
                         mipmapLevel:0
                           withBytes:data
                         bytesPerRow:bytesPerRow];
         }
         
         if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) {
             id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
             id<MTLBlitCommandEncoder> encoder = [commandBuffer blitCommandEncoder];
             [encoder generateMipmapsForTexture:tex->tex];
             [encoder endEncoding];
             [commandBuffer commit];
             [commandBuffer waitUntilCompleted];
         }
     }
     
     MTLSamplerDescriptor* samplerDescriptor = [MTLSamplerDescriptor new];
     if (imageFlags & NVG_IMAGE_NEAREST) {
         samplerDescriptor.minFilter = MTLSamplerMinMagFilterNearest;
         samplerDescriptor.magFilter = MTLSamplerMinMagFilterNearest;
         if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS)
             samplerDescriptor.mipFilter = MTLSamplerMipFilterNearest;
     } else {
         samplerDescriptor.minFilter = MTLSamplerMinMagFilterLinear;
         samplerDescriptor.magFilter = MTLSamplerMinMagFilterLinear;
         if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS)
             samplerDescriptor.mipFilter = MTLSamplerMipFilterLinear;
     }
     
     if (imageFlags & NVG_IMAGE_REPEATX) {
         samplerDescriptor.sAddressMode = MTLSamplerAddressModeRepeat;
     } else {
         samplerDescriptor.sAddressMode = MTLSamplerAddressModeClampToEdge;
     }
     
     if (imageFlags & NVG_IMAGE_REPEATY) {
         samplerDescriptor.tAddressMode = MTLSamplerAddressModeRepeat;
     } else {
         samplerDescriptor.tAddressMode = MTLSamplerAddressModeClampToEdge;
     }
     
     tex->sampler = [_metalLayer.device
                     newSamplerStateWithDescriptor:samplerDescriptor];
     
     return tex->id;
     */
    MTLPixelFormat default_pixel_format;
    MTLPixelFormat default_pixel_format_depth_stencil;
};

//
Image* Image_create_from_MTLTexture(Images* imgs, mtt::String name, id<MTLTexture> texture, const sd::Image_Create_Args& args);


void Image_replace_associated_texture(Images* imgs, Image* img, id<MTLTexture> new_texture);

bool max_textures_reached(Images* imgs);

TEXTURE_ENSURE_STATUS texture_ensure_resident(Images* imgs, sd::Texture_ID ID, id<MTLRenderCommandEncoder> render_encoder);



#endif
void Image_create_from_baked_platform(Images* imgs, cstring name, void* texture, void (*callback)(Image* image));
bool Images_lookup_platform(sd::Images* images, cstring name, Image** out_img);

struct Image_Loader_Init_Args {
};

void Image_Loader_init(sd::Renderer* renderer, Image_Loader* loader, Images* images, Image_Loader_Init_Args args = {});

}

#endif /* image_platform_hpp */
