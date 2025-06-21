//
//  ShaderTypes.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/12/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

//
//  Header containing types and enum constants shared between Metal shaders and C/ObjC source
//
#ifndef ShaderTypes_h
#define ShaderTypes_h

#include "shared_shader_types.hpp"

#include <simd/simd.h>

#ifdef __METAL_VERSION__



#define NS_ENUM(_type, _name) enum _name : _type _name; enum _name : _type
#define NSInteger metal::int32_t

#define DEFAULT_TEXTURE_ADDRESS address::clamp_to_edge

#define TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE address::clamp_to_edge
#define TEXTURE_ADDRESS_MODE_REPEAT address::repeat
#define DEFAULT_TEXTURE_ADDRESS_MODE address::clamp_to_edge

#define DEVICE device


#else

template<typename T>
struct texture2d : public MTLResourceID {
public:
    texture2d(MTLResourceID v) : MTLResourceID(v) {}
};

struct sampler : public MTLResourceID {
public:
    sampler(MTLResourceID v) : MTLResourceID(v) {}
};


typedef uint16_t half;

#define DEVICE


#import <Foundation/Foundation.h>


#define TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE MTLSamplerAddressModeClampToEdge
#define TEXTURE_ADDRESS_MODE_REPEAT MTLSamplerAddressModeRepeat
#define DEFAULT_TEXTURE_ADDRESS_MODE MTLSamplerAddressModeClampToEdge



#endif





//typedef enum COLOR_SPACE {
//    COLOR_SPACE_RGB,
//    COLOR_SPACE_HSV,
//    COLOR_SPACE_COUNT
//} COLOR_SPACE;

typedef NS_ENUM(NSInteger, BufferIndex)
{
    BufferIndexMeshPositions = 0,
    BufferIndexMeshGenerics  = 1,
    BufferIndexUniforms      = 2
};

typedef NS_ENUM(NSInteger, VertexAttribute)
{
    VertexAttributePosition  = 0,
    VertexAttributeTexcoord  = 1,
};

typedef NS_ENUM(NSInteger, TextureIndex)
{
    TextureIndexColor    = 0,
    TextureIndexCbRc     = 1,
    TextureIndexY        = 2
};

typedef struct
{
  float textureCoordinates[4 * 2];
} Uniforms;


#if defined(__METAL_VERSION__)
#define SD_Metal_Attribute(arg) arg
#else
#define SD_Metal_Attribute(__VA_ARGS__)
#endif


// Buffer index values shared between shader and C code to ensure Metal shader buffer inputs match
//   Metal API buffer set calls
typedef enum BufferIndices {
    kBufferIndexMeshPositions    = 0,
    kBufferIndexMeshGenerics     = 1,
    kBufferIndexInstanceUniforms = 2,
    kBufferIndexSharedUniforms   = 3,
    kBufferIndexEntityUniforms   = 4,
    kBufferIndexSemisharedUniforms = 5,
    kBufferIndexOffsets = 6,
} BufferIndices;

// Attribute index values shared between shader and C code to ensure Metal shader vertex
//   attribute indices match the Metal API vertex descriptor attribute indices
typedef enum VertexAttributes {
    kVertexAttributePosition  = 0,
    kVertexAttributeTexcoord  = 1,
    kVertexAttributeNormal    = 2,
    kVertexAttributeColor     = 3,
    kVertexAttributeColorSpace = 4
} VertexAttributes;

// Texture index values shared between shader and C code to ensure Metal shader texture indices
//   match indices of Metal API texture set calls
typedef enum TextureIndices {
    kTextureIndexColor    = 0,
    kTextureIndexY        = 1,
    kTextureIndexCbCr     = 2
} TextureIndices;

// Structure shared between shader and C code to ensure the layout of shared uniform data accessed in
//    Metal shaders matches the layout of uniform data set in C code


typedef struct
DATA_ALIGN_CONSTANT
{
    // Camera Uniforms
    matrix_float4x4 projection_matrix;
    // for AR
    matrix_float4x4 view_matrix;
    
    // Lighting Properties
    vector_float3 ambientLightColor;
    vector_float3 directionalLightDirection;
    vector_float3 directionalLightColor;
    float materialShininess;
    float time_seconds;
    float aspect_ratio;
} SharedUniforms;

typedef struct
DATA_ALIGN_DEVICE
{
    vector_float4 diffuse;
    vector_float4 specular;
} Instance_Data_Materials;

// Structure shared between shader and C code to ensure the layout of instance uniform data accessed in
//    Metal shaders matches the layout of uniform data set in C code



typedef struct
DATA_ALIGN_CONSTANT
{
    matrix_float4x4 view_matrix;
    matrix_float4x4 projection_matrix;
} Semishared_Uniforms;





// TR

typedef struct VERTEX_Color {
    simd_float4 position  SD_Metal_Attribute([[attribute(kVertexAttributePosition)]]);
    simd_float4 color     SD_Metal_Attribute([[attribute(kVertexAttributeColor)]]);
    simd_float2 tex_coord SD_Metal_Attribute([[attribute(kVertexAttributeTexcoord)]]);
    //float color_space       MTT_Metal_Attribute([[attribute(kVertexAttributeColorSpace)]]);
} VERTEX_Polygon_Color;

#ifndef __METAL_VERSION__
inline static void VERTEX_Polygon_Color_printf(VERTEX_Polygon_Color* vertex)
{
    printf("[%f, %f, %f, %f], [%f, %f, %f, %f], [%f, %f]\n",
           ( vertex )->position[0], \
           ( vertex )->position[1], \
           ( vertex )->position[2], \
           ( vertex )->position[3], \
           ( vertex )->color[0], \
           ( vertex )->color[1], \
           ( vertex )->color[2], \
           ( vertex )->color[3], \
           ( vertex )->tex_coord[0], \
           ( vertex )->tex_coord[1]
    );
}
#endif

typedef struct VERTEX_Path_Color {
    vector_float4 position  SD_Metal_Attribute([[attribute(kVertexAttributePosition)]]);
    vector_float4 color     SD_Metal_Attribute([[attribute(kVertexAttributeColor)]]);
    vector_float2 tex_coord SD_Metal_Attribute([[attribute(kVertexAttributeTexcoord)]]);
} VERTEX_Path_Color;

typedef enum MTT_Fragment_Buffer_Index
{
    MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS = 0,
    MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS_TARGET_PARAMS = 1,
  //  MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS_1,
} MTT_FRAGMENT_BUFFER_INDEX;

typedef enum MTT_ARGUMENT_BUFFER_ID {
    MTT_ARGUMENT_BUFFER_ID_TEXTURES   = 0
    //AAPLArgumentBufferIDExampleBuffers   = 100,
    //AAPLArgumentBufferIDExampleConstants = 200
} MTT_ARGUMENT_BUFFER_ID;

typedef enum MTT_MAX_NUM_ARGUMENTS {
    //MTT_MAX_NumBufferArguments  = 30,
    MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_3      = 31,
    MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_4      = 96,
    MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_5      = 96,
    MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_6_OR_7 = 128,
    MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_MAC1   = 128,
    MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_MAC2   = 128
} MTT_MAX_NUM_ARGUMENTS;


#ifdef __METAL_VERSION__
template <typename T> T sin01(T value) {
    return (metal::sin(value) + 1.0) / 2.0;
}

struct Fragment_Shader_Arguments_Resources {
    texture2d<half, access::sample> texture;
};

struct Fragment_Shader_Arguments_MTL3 {
    device Fragment_Shader_Arguments_Resources* textures;
    metal::array<sampler, 8> samplers;
};

#else

template <typename T> T sin01(T value) {
    return (m::sin(value) + 1.0) / 2.0;
}

struct Fragment_Shader_Arguments_Resources {
    MTLResourceID  texture;
};

struct Fragment_Shader_Arguments_MTL3 {
    Fragment_Shader_Arguments_Resources* textures;
    MTLResourceID samplers[8];
};



#endif


/*
 - (void)drawIndexedPrimitives:(MTLPrimitiveType)primitiveType indexCount:(NSUInteger)indexCount indexType:(MTLIndexType)indexType indexBuffer:(id <MTLBuffer>)indexBuffer indexBufferOffset:(NSUInteger)indexBufferOffset instanceCount:(NSUInteger)instanceCount baseVertex:(NSInteger)baseVertex baseInstance:(NSUInteger)baseInstance API_AVAILABLE(macos(10.11), ios(9.0));
 */
typedef struct {
    uint32_t indexCount;
    //uint32_t indexBufferOffset;
    uint32_t instanceCount;
    uint32_t baseVertex;
    uint32_t baseInstance;
} Draw_Args_Indexed;


#endif /* ShaderTypes_h */
