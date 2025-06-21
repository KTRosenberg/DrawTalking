//
//  image.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 10/15/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef image_hpp
#define image_hpp

#include "file_system.hpp"
#include "stratadraw.h"
#include "misc.hpp"

namespace sd {

enum TEXTURE_USAGE : uint32 {
    TEXTURE_USAGE_unknown = 0x0000,
    TEXTURE_USAGE_shader_read = 0x0001,
    TEXTURE_USAGE_shader_write = 0x0002,
    TEXTURE_USAGE_render_target = 0x0004,
    TEXTURE_USAGE_pixel_format_view = 0x0010,
};
struct Image {
    void* data = nullptr;
    usize  allocated_size = 0;
    vec2  size = vec2(0, 0);
    uint64 type = 0;
    uint64 format = 0;
    Texture_ID texture_ID = Texture_ID_INVALID;
    Texture_Sampler_ID texture_sampler_ID = Texture_Sampler_ID_INVALID;
    
    mtt::String name = {};
    void (*free)(Image* image) = [](Image*) {};
    
    TEXTURE_USAGE usage = TEXTURE_USAGE_shader_read;
};

struct Image_View {
    Image* image = nullptr;
    vec2 coordinate_offset = vec2(0, 0);
    vec2 dimensions = vec2(0,0);
};



inline static void Image_free(Image* img)
{
    if (img != nullptr && img->free != nullptr) {
       img->free(img);
    }
}

inline static bool Image_is_valid(Image* img)
{
    return (img != nullptr);
}

inline static bool Image_View_is_valid(Image_View* img_view)
{
    return Image_is_valid(img_view->image);
}

inline static vec4 Texture_compute_coordinate_modifiers(vec2 base_size, vec2 offset, vec2 dimensions)
{
    const auto size_inv = 1.0f / base_size;
    return vec4(dimensions * size_inv, offset * size_inv);
}

// non-normalized
inline static Image_View Image_View_make(Image* img, vec2 offset, vec2 dimensions)
{
    ASSERT_MSG(offset.x >= 0 && offset.y >= 0 && (dimensions.x > 0.0f || dimensions.y > 0.0f), "tried to make Image View with invalid arguments: offset=[%f,%f], dimensions=[%f,%f]", offset.x, offset.y, dimensions.x, dimensions.y);
    const auto size_inv = 1.0f / img->size;
    return (Image_View) { .image = img, .coordinate_offset = offset * size_inv, .dimensions = dimensions * size_inv };
}

inline static Image_View Image_View_make(Image* img, vec2 offset)
{
    return Image_View_make(img, offset, img->size);
}

inline static Image_View Image_View_make(Image* img)
{
    return Image_View_make(img, vec2(0,0), img->size);
}


inline static void Drawable_Info_set_from_texture_view(Drawable_Info* d_info, Image_View* img_view)
{
    Image* img_src = img_view->image;
    d_info->set_texture_ID(img_src->texture_ID);
    d_info->set_texture_coordinates_modifiers(vec4(img_view->dimensions / img_src->size, img_view->coordinate_offset));
}


struct Image_Atlas {
    struct Region {
        usize idx = 0;
        sd::Rectangle rectangle = {};
    };
    
    struct Backing {
        sd::Rectangle rectangle;
        float32 max_y_in_row = 0;
    };
    
    sd::Renderer* renderer = nullptr;
    
    std::vector<Backing> backing = {};
    std::vector<Region> free_regions = {};
    
    vec2 next_available = vec2(0,0);
    
    usize max_image_dimension_size = 1;
    uvec2 dimensions = uvec2(1,1);
    vec2 padding = vec2(8,8);
};

void Image_Atlas_clear(Image_Atlas* atlas);

bool Image_Atlas_image_reserve_rectangle(Image_Atlas* atlas, vec2 required_space, sd::Image_Atlas::Region* reserved_space);

void Image_Atlas_init(Renderer* renderer, Image_Atlas* atlas, usize max_image_dimension_size, uvec2 dimensions);

usize Image_Atlas_image_count(Image_Atlas* atlas);


struct Images;

struct Image_Load_Result {
    std::vector<sd::Image*> images;
    
    bool (*handler)(Image_Load_Result);
    
    sd::Images* image_storage;
    
    void* custom_data;
};


struct Image_Loader {
    void* backend;
};

const uint64 PATH_FLAG_CHECK_ALL     = 0;
const uint64 PATH_FLAG_BUILTIN       = (1 << 0);
const uint64 PATH_FLAG_FILE_SYSTEM   = (1 << 1);
const uint64 PATH_FLAG_SYSTEM_REMOTE = (1 << 2);
const uint64 PATH_FLAG_REMOTE        = (1 << 3);

struct Path_Info {
    mtt::String path = "";
    mtt::String extension = "";
    mtt::String name = "";
    uint64 flags = 0;
};

static const uint64 IMAGE_LOAD_DESCRIPTOR_FLAG_NONE           = 0;
static const uint64 IMAGE_LOAD_DESCRIPTOR_FLAG_REMOVE_BG = 1 << 0;
static const uint64 IMAGE_LOAD_DESCRIPTOR_FLAG_READONLY  = 1 << 1;
struct Image_Load_Descriptor {
    void* custom_data;
    std::vector<Path_Info> paths;
    bool require_all = true;
    bool persist_data = false;
    mtt::Net_Connection_Info net_connection_info = {};
    uint64 flags = IMAGE_LOAD_DESCRIPTOR_FLAG_NONE;
};
    
struct Image_Pick_Descriptor {
    usize  arrived;
    usize  min_number_required;
    void*  custom_data;
};


struct Images_Init_Args {
    bool support_bindless = false;
    uint64 default_pixel_format = 0;
    uint64 default_pixel_format_depth_stencil = 0;
};

void Images_init(Renderer* renderer, Images* images, usize max_resident, const Images_Init_Args& args = {});


void images_pick_async(Renderer* renderer, Image_Loader* loader, Image_Pick_Descriptor* desc, bool (*callback)(Image_Load_Result));

bool images_load_async(Renderer* renderer, Image_Load_Descriptor* desc, bool (*callback)(const Image_Load_Result& result, bool status));

bool images_load_remote_async(sd::Renderer* renderer, Image_Load_Descriptor* desc, bool (*callback)(const Image_Load_Result& result, bool status));

bool Images_lookup(sd::Images* images, cstring name, Image** out_img);

struct Image_Create_Args {
    uint64 usage_flags = 0;
    bool for_msaa = false;
    bool for_depth = false;
    uint32 sample_count = 4;
};

sd::Image* Image_create_empty(sd::Images* imgs, const mtt::String& name, const Image_Create_Args& args);
sd::Image* Image_create_empty_w_size(sd::Images* imgs, const mtt::String& name, uvec2 size, const Image_Create_Args& args, Texture_Handle* out_handle);
void Image_create_from_baked(Images* imgs, cstring name, void* texture, void (*callback)(Image* image));


struct Image_Save_Descriptor {
    Path_Info path;
    uint64 flags = 0;
};

void Image_save(Images* images, Image* img, const Image_Save_Descriptor& args);

void Images_for_each(sd::Images* images, void (callback)(sd::Image* img));

bool Texture_lookup(sd::Images* imgs, sd::Texture_ID t_id, Image** out_img, Texture_Handle* out_handle);

}

#endif /* image_hpp */
