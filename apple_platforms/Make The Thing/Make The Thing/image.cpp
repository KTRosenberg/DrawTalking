//
//  image.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/13/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//
#include "image_platform.hpp"


namespace sd {

bool Images_lookup(Images* images, cstring name, Image** out_img)
{
    return Images_lookup_platform(images, name, out_img);
}

void Image_create_from_baked(Images* imgs, cstring name, void* texture, void (*callback)(Image* image))
{
    Image_create_from_baked_platform(imgs, name, texture, callback);
}

void Image_Atlas_image_free_region(Image_Atlas* atlas, sd::Image_Atlas::Region* reserved_space)
{
    atlas->free_regions.push_back(*reserved_space);
}

void Image_Atlas_clear(Image_Atlas* atlas)
{
    atlas->backing.clear();
    atlas->next_available = vec2(0,0);
}

bool Image_Atlas_image_reserve_rectangle(Image_Atlas* atlas, vec2 required_space, sd::Image_Atlas::Region* reserved_space)
{
    float64 max_image_dimension_size = (float64)atlas->max_image_dimension_size;
    uvec2 dimensions = atlas->dimensions;
    
    ASSERT_MSG((required_space[0] <= max_image_dimension_size ||
               required_space[1] <= max_image_dimension_size), "[%f,%f] too big\n", required_space[0], required_space[1]);
    
//    float64 required_area = required_space[0] * required_space[1];
//    if (!atlas->free_regions.empty()) {
//        float64 best_min_area = POSITIVE_INFINITY;
//        usize best_i = atlas->free_regions.size();
//        for (usize i = 0; i < atlas->free_regions.size(); i += 1) {
//            auto& region = atlas->free_regions[i];
//            float64 area = region.rectangle.width * region.rectangle.height;
//
//            if (area < best_min_area &&
//                region.rectangle.width >= required_space[0] &&
//                region.rectangle.height >= required_space[1]) {
//
//                best_min_area = area;
//                best_i = i;
//                if (area == required_area) {
//                    break;
//                }
//            }
//        }
//        if (best_i != atlas->free_regions.size()) {
//            *reserved_space = atlas->free_regions[best_i];
//            atlas->free_regions[best_i] = atlas->free_regions[atlas->free_regions.size() - 1];
//            atlas->free_regions.pop_back();
//            return true;
//        }
//    }
    
    bool completed = false;
    bool create_new_backing = (atlas->backing.empty());
    bool created_new = false;
    do {
        if (create_new_backing) {
            created_new = true;
            atlas->backing.push_back({});
            auto& backing = atlas->backing.back();
            backing.rectangle = (sd::Rectangle){.x = (float64)0, .y = (float64)0, .width = static_cast<float64>(dimensions.x), .height = static_cast<float64>(dimensions.y) };
            backing.max_y_in_row = 0;
            create_new_backing = false;
        }
        
        
        auto& backing = atlas->backing.back();
        
        if (atlas->next_available.x + required_space[0] > dimensions.x) {
            atlas->next_available.x = 0;
            atlas->next_available.y += backing.max_y_in_row + atlas->padding.y;
            backing.max_y_in_row = 0;
        }
        if (atlas->next_available.y + required_space[1] > dimensions.y) {
            create_new_backing = true;
            atlas->next_available.x = 0;
            atlas->next_available.y = 0;
            backing.max_y_in_row = 0;
            continue;
        }
        
        sd::Image_Atlas::Region* region = reserved_space;
        region->idx = atlas->backing.size() - 1;
        region->rectangle.x = atlas->next_available.x;
        region->rectangle.y = atlas->next_available.y;
        region->rectangle.width = required_space[0];
        region->rectangle.height = required_space[1];
        
        atlas->next_available.x += required_space[0] + atlas->padding.x;
        backing.max_y_in_row = m::max(backing.max_y_in_row, required_space[1]);
        
        
        completed = true;
    } while (!completed);
    return created_new;
}

usize Image_Atlas_image_count(Image_Atlas* atlas)
{
    return atlas->backing.size();
}

void Image_Atlas_init(Renderer* renderer, Image_Atlas* atlas, usize max_image_dimension_size, uvec2 dimensions)
{
    
    ASSERT_MSG((dimensions[0] <= max_image_dimension_size &&
               dimensions[1] <= max_image_dimension_size), "[%f,%f] too big, max dimension is %f\n", dimensions[0], dimensions[1], max_image_dimension_size);
    
    *atlas = Image_Atlas();
    atlas->renderer = renderer;
    atlas->max_image_dimension_size = max_image_dimension_size;
    atlas->dimensions = dimensions;
    atlas->backing.clear();
    atlas->next_available = vec2(0,0);
}

}

