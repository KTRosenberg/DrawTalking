//
//  camera.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 10/3/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef camera_hpp
#define camera_hpp

#include "stratadraw.h"

namespace mtt {

enum CAMERA_FLAGS {
    CAMERA_FLAGS_ENABLE_TRANSLATION = (1 << 0),
    CAMERA_FLAGS_ENABLE_ROTATION = (1 << 1),
    CAMERA_FLAGS_ENABLE_SCALE = (1 << 2)
};



struct Camera {
    vec3 position;
    vec3 orientation;
    mat4 view_transform;
    mat4 cam_transform;
    CAMERA_FLAGS flags;
};

static inline bool camera_flags_check(Camera* cam, CAMERA_FLAGS flags)
{
    return (cam->flags & flags) == flags;
}

Camera Camera_make(void);

void Camera_init(Camera* cam);
void Camera_reset(Camera* cam);

Mat4 calc_view_matrix(Camera* cam);
Mat4 calc_camera_matrix(Camera* cam);

Mat4 set_view_with_matrix(Camera* cam, Mat4& mat);

Mat4 set_view_with_preinverted_matrix(Camera* cam, Mat4& mat);

vec3 transform_point(Camera* cam, vec3 p);
vec2 transform_point(Camera* cam, vec2 p);

Mat4 orthographic_projection_matrix(float64 left, float64 right, float64 bottom, float64 top, float64 z_near, float64 z_far);

Mat4 orthographic_projection_matrix_scale(float64 left, float64 right, float64 bottom, float64 top, float64 z_near, float64 z_far, float64 scale, sd::Viewport*);

Mat4 inverse_matrix(Mat4& transform);

void pan_camera_at_edge(sd::Viewport& viewport, mtt::Camera& cam, vec3& view_position, vec2 canvas_pos);

}



#endif /* camera_hpp */
