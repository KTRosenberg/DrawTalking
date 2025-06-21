//
//  camera.cpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 10/3/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#include "camera.hpp"

namespace mtt {

Camera Camera_make(void)
{
    Camera cam;
    Camera_init(&cam);

    return cam;
};

void Camera_init(Camera* cam)
{
    cam->position         = vec3(0.0f);
    cam->orientation      = vec3(0.0f);
    cam->cam_transform   = mat4(1.0f);
    cam->view_transform = mat4(1.0f);
    cam->flags = (CAMERA_FLAGS)(CAMERA_FLAGS_ENABLE_TRANSLATION |
    CAMERA_FLAGS_ENABLE_ROTATION |
    CAMERA_FLAGS_ENABLE_SCALE);
}

void Camera_reset(Camera* cam)
{
    cam->position         = vec3(0.0f);
    cam->orientation      = vec3(0.0f);
    cam->cam_transform   = mat4(1.0f);
    cam->view_transform = mat4(1.0f);
}



Mat4 calc_view_matrix(Camera* cam)
{
    Mat4 mat = cam->cam_transform;//calc_camera_matrix(cam);
    cam->view_transform = inverse_matrix(mat);
    return cam->view_transform;
}

Mat4 calc_camera_matrix(Camera* cam)
{
    mat4 rot = m::rotate_around(
                                  cam->orientation,
                                   -cam->position
    );
    
    // translation
    cam->cam_transform = m::translate(rot, cam->position);
    return cam->view_transform;
}

Mat4 set_view_with_matrix(Camera* cam, Mat4& mat)
{
    return cam->view_transform = m::inverse(mat);
}

Mat4 set_view_with_preinverted_matrix(Camera* cam, Mat4& mat)
{
    return cam->view_transform = mat;
}

vec3 transform_point(Camera* cam, vec3 p)
{
    const vec4 result = m::inverse(cam->view_transform) * vec4(p, 1.0f);
    return vec3(result.x, result.y, result.z);
}

vec2 transform_point(Camera* cam, vec2 p)
{
    const vec4 result = m::inverse(cam->view_transform) * vec4(p, 0.0f, 1.0f);
    return vec2(result.x, result.y);
}

Mat4 orthographic_projection_matrix(float64 left, float64 right, float64 bottom, float64 top, float64 z_near, float64 z_far)
{
    return m::ortho(left, right, bottom, top, z_near, z_far);
}

Mat4 orthographic_projection_matrix_scale(float64 left, float64 right, float64 bottom, float64 top, float64 z_near, float64 z_far, float64 scale, sd::Viewport* out)
{
//    float64 cx = (right - left) / 2.0;
//    float64 cy = (bottom - top) / 2.0;
    
    float64 w = m::abs(right - left);
    float64 h = m::abs(bottom - top);
    

    
    float64 w_new = w * scale;
    float64 h_new = h * scale;
    
    float64 tx = (w_new - w) / 2.0;
    float64 ty = (h_new - h) / 2.0;
    
    float64 L = (left   + tx);
    float64 R = (right  - tx);
    float64 B = (bottom - ty);
    float64 T = (top    + ty);
    
    out->width  = w_new;
    out->height = h_new;
    out->x      = L;
    out->y      = T;
    
    return m::ortho(L, R, B, T, z_near, z_far);
}

Mat4 inverse_matrix(Mat4& transform)
{
    return m::inverse(transform);
}

void pan_camera_at_edge(sd::Viewport& viewport, mtt::Camera& cam, vec3& view_position, vec2 canvas_pos)
{
    float32 trans_inc = 1024.0f * MTT_TIMESTEP;
    vec3 trans = vec3(0.0f);
    bool changed = false;
    if (viewport.width - canvas_pos.x < 32) {
        changed = true;
        trans.x -= trans_inc;
    } else if (canvas_pos.x < 32) {
        changed = true;
        trans.x += trans_inc;
    }
    if (viewport.height - canvas_pos.y < 32) {
        changed = true;
        trans.y -= trans_inc;
    } else if (canvas_pos.y < 32) {
        changed = true;
        trans.y += trans_inc;
    }
    if (changed) {
        view_position -= trans;
        cam.cam_transform = cam.cam_transform * m::translate(Mat4(1.0f), -trans);
        mtt::calc_view_matrix(&cam);
    }
}

}
