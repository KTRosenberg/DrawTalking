//
//  shared_shader_types.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 1/21/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef shared_shader_types_hpp
#define shared_shader_types_hpp

#include <simd/simd.h>

#include <TargetConditionals.h>

#ifdef TARGET_OS_OSX
#define DATA_ALIGN __attribute__((aligned (256)))
#else
#define DATA_ALIGN  __attribute__((aligned (16)))
#endif

#ifdef __METAL_VERSION__

typedef simd_float2 SD_vec2;
typedef simd_double2 SD_vec2_64;
typedef simd_float3 SD_vec3;
typedef simd_float4 SD_vec4;
typedef matrix_float4x4 SD_mat4;

namespace sd {
typedef SD_vec2 vec2;
typedef SD_vec2_64 vec2_64;
typedef SD_vec3 vec3;
typedef SD_vec4 vec4;
typedef SD_mat4 mat4;
}

#else


//#ifdef __clang__
//
//typedef simd_float2 SD_vec2;
//typedef simd_float3 SD_vec3;
//typedef simd_float4 SD_vec4;
//typedef matrix_float4x4 SD_mat4;
//
//static inline SD_mat4 SD_mat4_identity() { return matrix_identity_float4x4; }
//
//#else

typedef vec2 SD_vec2;
typedef vec2_64 SD_vec2_64;
typedef vec3 SD_vec3;
typedef vec4 SD_vec4;
typedef mat4 SD_mat4;

namespace sd {
typedef SD_vec2 vec2;
typedef SD_vec2_64 vec2_64;
typedef SD_vec3 vec3;
typedef SD_vec4 vec4;
typedef SD_mat4 mat4;
}

static inline SD_mat4 SD_mat4_identity() { return m::mat4_identity(); }

//#endif

static_assert(sizeof(SD_vec2) == sizeof(simd_float2), "");
static_assert(alignof(SD_vec2) == alignof(simd_float2), "");

static_assert(sizeof(SD_vec2_64) == sizeof(simd_double2), "");
static_assert(alignof(SD_vec2_64) == alignof(simd_double2), "");

static_assert(sizeof(SD_vec3) == sizeof(simd_float3), "");
static_assert(alignof(SD_vec3) == alignof(simd_float3), "");

static_assert(sizeof(SD_vec4) == sizeof(simd_float4), "");
static_assert(alignof(SD_vec4) == alignof(simd_float4), "");

static_assert(sizeof(SD_mat4) == sizeof(matrix_float4x4), "");
static_assert(alignof(SD_mat4) == alignof(matrix_float4x4), "");



#endif


typedef struct
DATA_ALIGN
{
    SD_mat4 model_matrix;
    SD_vec4 color_factor;
    SD_vec4 color_addition;
    int32_t texture_index;
    int16_t texture_sampler_index;
    int32_t texture_mult_index;
    int32_t texture_add_index;
    SD_vec4 texture_coordinates_modifiers;
    SD_vec2 texture_animation_speed;
} Instance_Data;

typedef struct
DATA_ALIGN
{
    SD_vec2 dimensions;
} Push_Uniforms;



#endif /* shared_shader_types_hpp */
