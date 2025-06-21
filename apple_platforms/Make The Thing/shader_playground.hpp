//
//  shader_playground.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/24/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef SHADER_PLAYGROUND
#define SHADER_PLAYGROUND

#include "shared_shader_types.hpp"

//typedef struct
//DATA_ALIGN
//{
//    SD_mat4 model_matrix;
//    SD_vec4 color_factor;
//    SD_vec4 color_addition;
//    int32_t texture_index;
//    int16_t texture_sampler_index;
//    SD_vec4 texture_coordinates_modifiers;
//    SD_vec2 texture_animation_speed;
//} Instance_Data;

namespace sd {

namespace playground {
typedef struct
DATA_ALIGN
{
    SD_mat4 model_matrix;
    SD_vec4 color_factor;
    SD_vec4 color_addition;
    int32_t texture_index;
    SD_vec4 texture_coordinates_modifiers;
    SD_vec2 texture_animation_speed;
    float32 time_seconds;
} Input_Data;

#define SHADER_PROLOGUE_SUB(x) #x
#define SHADER_PROLOGUE(x) "#line " SHADER_PROLOGUE_SUB(x) "\n"

static cstring program = SHADER_PROLOGUE(__LINE__) R"(
#include <metal_stdlib>
using namespace metal;

#include <TargetConditionals.h>

typedef simd_float2 SD_vec2;
typedef simd_double2 SD_vec2_64;
typedef simd_float3 SD_vec3;
typedef simd_float4 SD_vec4;
typedef float4x4 SD_mat4;
typedef float float32;

namespace sd {
typedef SD_vec2 vec2;
typedef SD_vec2_64 vec2_64;
typedef SD_vec3 vec3;
typedef SD_vec4 vec4;
typedef SD_mat4 mat4;
}

typedef SD_vec2 vec2;
typedef SD_vec2_64 vec2_64;
typedef SD_vec3 vec3;
typedef SD_vec4 vec4;
typedef SD_mat4 mat4;

#define const

#ifdef TARGET_OS_OSX
#define DATA_ALIGN __attribute__((aligned (256)))
#else
#define DATA_ALIGN  __attribute__((aligned (16)))
#endif

struct Fragment_Shader_Arguments_Resources {
    texture2d<float, access::write> texture;
};

struct Fragment_Shader_Arguments_MTL3 {
    device Fragment_Shader_Arguments_Resources* textures;
    metal::array<sampler, 8> samplers;
};

typedef struct
DATA_ALIGN
{
    SD_mat4 model_matrix;
    SD_vec4 color_factor;
    SD_vec4 color_addition;
    int32_t texture_index;
    SD_vec4 texture_coordinates_modifiers;
    SD_vec2 texture_animation_speed;
    float32 time_seconds;
    float32 aspect_ratio;
} Input_Data;

#define COORD_DIVIDE(size, res) float2((float)size.x / (float)res.x, (float)size.y / (float)res.y)

#define LIGHTNING_TEST (0)

#if LIGHTNING_TEST
/* discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 */
vec3 random3(vec3 c) {
    float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
    vec3 r;
    r.z = fract(512.0*j);
    j *= .125;
    r.x = fract(5
12.0*j);
    j *= .125;
    r.y = fract(512.0*j);
    return r-0.5;
}

/* skew constants for 3d simplex functions */
constant float F3 =  0.3333333;
constant float G3 =  0.1666667;

/* 3d simplex noise */
float simplex3d(vec3 p) {
     /* 1. find current tetrahedron T and it's four vertices */
     /* s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices */
     /* x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices*/
     
     /* calculate s and x */
     vec3 s = floor(p + dot(p, vec3(F3)));
     vec3 x = p - s + dot(s, vec3(G3));
     
     /* calculate i1 and i2 */
     vec3 e = step(vec3(0.0), x - x.yzx);
     vec3 i1 = e*(1.0 - e.zxy);
     vec3 i2 = 1.0 - e.zxy*(1.0 - e);
         
     /* x1, x2, x3 */
     vec3 x1 = x - i1 + G3;
     vec3 x2 = x - i2 + 2.0*G3;
     vec3 x3 = x - 1.0 + 3.0*G3;
     
     /* 2. find four surflets and store them in d */
     vec4 w, d;
     
     /* calculate surflet weights */
     w.x = dot(x, x);
     w.y = dot(x1, x1);
     w.z = dot(x2, x2);
     w.w = dot(x3, x3);
     
     /* w fades from 0.6 at the center of the surflet to 0.0 at the margin */
     w = max(0.6 - w, 0.0);
     
     /* calculate surflet components */
     d.x = dot(random3(s), x);
     d.y = dot(random3(s + i1), x1);
     d.z = dot(random3(s + i2), x2);
     d.w = dot(random3(s + 1.0), x3);
     
     /* multiply d by w^4 */
     w *= w;
     w *= w;
     d *= w;
     
     /* 3. return the sum of the four surflets */
     return dot(d, vec4(52.0));
}

float noise(vec3 m) {
    return   0.5333333*simplex3d(m)
            +0.2666667*simplex3d(2.0*m)
            +0.1333333*simplex3d(4.0*m)
            +0.0666667*simplex3d(8.0*m);
}

#endif

#define FIRE_1_TEST (1)
#if FIRE_1_TEST
float rand(vec2 n) {
    return fract(sin(cos(dot(n, vec2(12.9898,12.1414)))) * 83758.5453);
}

float noise(vec2 n) {
    vec2 d = vec2(0.0, 1.0);
    vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
    return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

float fbm(vec2 n) {
    float total = 0.0, amplitude = 1.0;
    for (int i = 0; i <5; i++) {
        total += noise(n) * amplitude;
        n += n*1.7;
        amplitude *= 0.47;
    }
    return total;
}
#endif

kernel void main_entry(device Fragment_Shader_Arguments_MTL3* fragment_shader_args [[ buffer(0) ]],
                        constant Input_Data& data [[buffer(1)]],
                        uint2 index [[thread_position_in_grid]],
                        uint2 size [[threads_per_grid]]
)
{
    texture2d<float, access::write> tex = fragment_shader_args->textures[data.texture_index].texture;
    float t_width = (float)tex.get_width();
    float t_height = (float)tex.get_height();
    float t_width_r = (t_width  / 1920.0f) * 4;
    float t_height_r = (t_height / 1080.0f) * 4;
#define TIME_SCROLL_1 (0)

    float2 v_position = float2((float)index.x / (float)t_width, (float)index.y / (float)t_height);
#if TIME_SCROLL_1
    v_position = fmod(v_position + data.time_seconds, float2(1.0));
#endif
    
    
    //v_position.x *= t_height / t_width;

#if LIGHTNING_TEST

    vec2 uv = float2((float)index.x / (float)t_width, (float)index.y / (float)t_height);
    uv = uv * 2. -1.;
    //uv.x*=t_width_r;
    uv.y*=t_height_r;

    vec2 p = vec2((float)index.x/t_width, (float)index.y/t_height) * vec2(1.0, t_height_r);
    vec3 p3 = vec3(p, data.time_seconds*0.4);

    float intensity = noise(vec3(p3*12.0+12.0));

    float t = clamp((uv.x * -uv.x * 0.16) + 0.15, 0., 1.);
    float y = abs(intensity * -t + uv.y);

    float g = pow(y, 0.2);

    vec3 col = vec3(1.70, 1.48, 1.78);
    col = col * -g + col;
    col = col * col;
    col = col * col;

    fragment_shader_args->textures[data.texture_index].texture.write(
    float4(col.x, col.y, col.z, 1.0), index);
return;
#elif FIRE_1_TEST
    vec3 c1 = vec3(0.5, 0.0, 0.1);
    vec3 c2 = vec3(0.9, 0.1, 0.0);
    vec3 c3 = vec3(0.2, 0.1, 0.7);
    vec3 c4 = vec3(1.0, 0.9, 0.1);
    vec3 c5 = vec3(0.1);
    vec3 c6 = vec3(0.9);

    float iTime = data.time_seconds;

    vec2 speed = vec2(0.1, 0.9);
    float shift = 1.327+sin(iTime*2.0)/2.4;
    float alpha = 1.0;
    
    float dist = 3.5-sin(iTime*0.4)/1.89;

    vec2 p = float2((float)index.x, 0.5 * (t_height - (float)index.y)) * dist / float2(t_width, t_height);
p.x *= t_width_r;
p.y *= t_height_r;
    p += sin(p.yx*4.0+vec2(.2,-.3)*iTime)*0.04;
    p += sin(p.yx*8.0+vec2(.6,+.1)*iTime)*0.01;

    p.x -= iTime/1.1;
    float q = fbm(p - iTime * 0.3+1.0*sin(iTime+0.5)/2.0);
    float qb = fbm(p - iTime * 0.4+0.1*cos(iTime)/2.0);
    float q2 = fbm(p - iTime * 0.44 - 5.0*cos(iTime)/2.0) - 6.0;
    float q3 = fbm(p - iTime * 0.9 - 10.0*cos(iTime)/15.0)-4.0;
    float q4 = fbm(p - iTime * 1.4 - 20.0*sin(iTime)/14.0)+2.0;
    q = (q + qb - .4 * q2 -2.0*q3  + .6*q4)/3.8;
    vec2 r = vec2(fbm(p + q /2.0 + iTime * speed.x - p.x - p.y), fbm(p + q - iTime * speed.y));
    vec3 c = mix(c1, c2, fbm(p + r)) + mix(c3, c4, r.x) - mix(c5, c6, r.y);

    vec3 color = vec3(1.0/(pow(c+1.61,vec3(4.0))) * cos(shift * (float)index.y / t_height));
    color=vec3(1.0,.2,.05)/(pow((r.y+r.y)* max(.0,p.y)+0.1, 4.0));;
    color = color/(1.0+max(vec3(0),color));
    
    fragment_shader_args->textures[data.texture_index].texture.write(
    float4(color.x, color.y, color.z, color.x + color.y + color.z), index);
return;
#endif
    
    fragment_shader_args->textures[data.texture_index].texture.write(
    float4(v_position.x, v_position.y, v_position.x * v_position.y, 1.0), index);
//        fragment_shader_args->textures[data.texture_index].texture.write(
//        float4(1.0f, 0.0, 0.0, 1.0), index);
}

)";

static cstring program_grayscale = SHADER_PROLOGUE(__LINE__) R"(
#include <metal_stdlib>
using namespace metal;

#include <TargetConditionals.h>

typedef simd_float2 SD_vec2;
typedef simd_double2 SD_vec2_64;
typedef simd_float3 SD_vec3;
typedef simd_float4 SD_vec4;
typedef float4x4 SD_mat4;
typedef float float32;

namespace sd {
typedef SD_vec2 vec2;
typedef SD_vec2_64 vec2_64;
typedef SD_vec3 vec3;
typedef SD_vec4 vec4;
typedef SD_mat4 mat4;
}

typedef SD_vec2 vec2;
typedef SD_vec2_64 vec2_64;
typedef SD_vec3 vec3;
typedef SD_vec4 vec4;
typedef SD_mat4 mat4;

#define const

#ifdef TARGET_OS_OSX
#define DATA_ALIGN __attribute__((aligned (256)))
#else
#define DATA_ALIGN  __attribute__((aligned (16)))
#endif

struct Fragment_Shader_Arguments_Resources {
    texture2d<float, access::read_write> texture;
};

struct Fragment_Shader_Arguments_MTL3 {
    device Fragment_Shader_Arguments_Resources* textures;
    metal::array<sampler, 8> samplers;
};

typedef struct
DATA_ALIGN
{
    SD_mat4 model_matrix;
    SD_vec4 color_factor;
    SD_vec4 color_addition;
    int32_t texture_index;
    SD_vec4 texture_coordinates_modifiers;
    SD_vec2 texture_animation_speed;
    float32 time_seconds;
    float32 aspect_ratio;
} Input_Data;

#define COORD_DIVIDE(size, res) float2((float)size.x / (float)res.x, (float)size.y / (float)res.y)

// Rec. 709 luma values for grayscale image conversion
constant float3 kRec709Luma = float3(0.2126, 0.7152, 0.0722);

// Grayscale compute kernel
kernel void grayscale(device Fragment_Shader_Arguments_MTL3* fragment_shader_args [[ buffer(0) ]],
                        constant Input_Data& data [[buffer(1)]],
                        uint2 gid [[thread_position_in_grid]],
                        uint2 size [[threads_per_grid]])
{
//    // Check if the pixel is within the bounds of the output texture
//    if((gid.x >= outTexture.get_width()) || (gid.y >= outTexture.get_height()))
//    {
//        // Return early if the pixel is out of bounds
//        return;
//    }

    texture2d<float, access::read_write> tex = fragment_shader_args->textures[data.texture_index].texture;
    float4 inColor  = tex.read(gid);
    float  gray     = dot(inColor.rgb, kRec709Luma);
    tex.write(float4(gray, gray, gray, inColor.a), gid);
}

)";

}

}

#endif
