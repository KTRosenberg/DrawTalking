//
//  Shaders.metal
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/12/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//


#include <metal_stdlib>

using namespace metal;

#import "ShaderTypes.h"

#include <metal_stdlib>
#include <metal_math>
#include <simd/simd.h>

// Include header shared between this Metal shader code and C code executing Metal API command

typedef struct {
    float2 position [[attribute(kVertexAttributePosition)]];
    float2 texCoord [[attribute(kVertexAttributeTexcoord)]];
} ImageVertex;


typedef struct {
    float4 position [[position]];
    float2 texCoord;
} ImageColorInOut;


// Captured image vertex function
vertex ImageColorInOut capturedImageVertexTransform(ImageVertex in [[stage_in]]) {
    ImageColorInOut out;
    
    // Pass through the image vertex's position
    out.position = float4(in.position, 0.0, 1.0);
    
    // Pass through the texture coordinate
    out.texCoord = in.texCoord;
    
    return out;
}

// Captured image fragment function
fragment float4 capturedImageFragmentShader(ImageColorInOut in [[stage_in]],
                                            texture2d<float, access::sample> capturedImageTextureY [[ texture(kTextureIndexY) ]],
                                            texture2d<float, access::sample> capturedImageTextureCbCr [[ texture(kTextureIndexCbCr) ]]) {
    
    constexpr sampler colorSampler(mip_filter::linear,
                                   mag_filter::linear,
                                   min_filter::linear);
    
    const float4x4 ycbcrToRGBTransform = float4x4(
        float4(+1.0000f, +1.0000f, +1.0000f, +0.0000f),
        float4(+0.0000f, -0.3441f, +1.7720f, +0.0000f),
        float4(+1.4020f, -0.7141f, +0.0000f, +0.0000f),
        float4(-0.7010f, +0.5291f, -0.8860f, +1.0000f)
    );
    
    // Sample Y and CbCr textures to get the YCbCr color at the given texture coordinate
    float4 ycbcr = float4(capturedImageTextureY.sample(colorSampler, in.texCoord).r,
                          capturedImageTextureCbCr.sample(colorSampler, in.texCoord).rg, 1.0);
    
    // Return converted RGB color
    return ycbcrToRGBTransform * ycbcr;
}


typedef struct {
    float3 position [[attribute(kVertexAttributePosition)]];
    float2 texCoord [[attribute(kVertexAttributeTexcoord)]];
    half3 normal    [[attribute(kVertexAttributeNormal)]];
} Vertex;


typedef struct {
    float4 position [[position]];
    float4 color;
    half3  eyePosition;
    half3  normal;
} ColorInOut;


// Anchor geometry vertex function
vertex ColorInOut anchorGeometryVertexTransform(Vertex in [[stage_in]],
                                                constant SharedUniforms &sharedUniforms [[ buffer(kBufferIndexSharedUniforms) ]],
                                                constant Instance_Data *instanceUniforms [[ buffer(kBufferIndexInstanceUniforms) ]],
                                                ushort vid [[vertex_id]],
                                                ushort iid [[instance_id]]) {
    ColorInOut out;
    
    // Make position a float4 to perform 4x4 matrix math on it
    float4 position = float4(in.position, 1.0);
    
    float4x4 modelMatrix = instanceUniforms[iid].model_matrix;
    float4x4 modelViewMatrix = sharedUniforms.view_matrix * modelMatrix;
    
    // Calculate the position of our vertex in clip space and output for clipping and rasterization
    out.position = sharedUniforms.projection_matrix * modelViewMatrix * position;
    
    // Color each face a different color
    ushort colorID = vid / 4 % 6;
    out.color = colorID == 0 ? float4(0.0, 1.0, 0.0, 1.0) // Right face
              : colorID == 1 ? float4(1.0, 0.0, 0.0, 1.0) // Left face
              : colorID == 2 ? float4(0.0, 0.0, 1.0, 1.0) // Top face
              : colorID == 3 ? float4(1.0, 0.5, 0.0, 1.0) // Bottom face
              : colorID == 4 ? float4(1.0, 1.0, 0.0, 1.0) // Back face
              : float4(1.0, 1.0, 1.0, 1.0); // Front face
    
    // Calculate the positon of our vertex in eye space
    out.eyePosition = half3((modelViewMatrix * position).xyz);
    
    // Rotate our normals to world coordinates
    float4 normal = modelMatrix * float4(in.normal.x, in.normal.y, in.normal.z, 0.0f);
    out.normal = normalize(half3(normal.xyz));
    
    return out;
}

// Anchor geometry fragment function
fragment float4 anchorGeometryFragmentLighting(ColorInOut in [[stage_in]],
                                               constant SharedUniforms &uniforms [[ buffer(kBufferIndexSharedUniforms) ]]) {
    
    float3 normal = float3(in.normal);
    
    // Calculate the contribution of the directional light as a sum of diffuse and specular terms
    float3 directionalContribution = float3(0);
    {
        // Light falls off based on how closely aligned the surface normal is to the light direction
        float nDotL = saturate(dot(normal, -uniforms.directionalLightDirection));
        
        // The diffuse term is then the product of the light color, the surface material
        // reflectance, and the falloff
        float3 diffuseTerm = uniforms.directionalLightColor * nDotL;
        
        // Apply specular lighting...
        
        // 1) Calculate the halfway vector between the light direction and the direction they eye is looking
        float3 halfwayVector = normalize(-uniforms.directionalLightDirection - float3(in.eyePosition));
        
        // 2) Calculate the reflection angle between our reflection vector and the eye's direction
        float reflectionAngle = saturate(dot(normal, halfwayVector));
        
        // 3) Calculate the specular intensity by multiplying our reflection angle with our object's
        //    shininess
        float specularIntensity = saturate(powr(reflectionAngle, uniforms.materialShininess));
        
        // 4) Obtain the specular term by multiplying the intensity by our light's color
        float3 specularTerm = uniforms.directionalLightColor * specularIntensity;
        
        // Calculate total contribution from this light is the sum of the diffuse and specular values
        directionalContribution = diffuseTerm + specularTerm;
    }
    
    // The ambient contribution, which is an approximation for global, indirect lighting, is
    // the product of the ambient light intensity multiplied by the material's reflectance
    float3 ambientContribution = uniforms.ambientLightColor;
    
    // Now that we have the contributions our light sources in the scene, we sum them together
    // to get the fragment's lighting value
    float3 lightContributions = ambientContribution + directionalContribution;
    
    // We compute the final color by multiplying the sample from our color maps by the fragment's
    // lighting value
    float3 color = in.color.rgb * lightContributions;
    
    // We use the color we just computed and the alpha channel of our
    // colorMap for this fragment's alpha value
    return float4(color, in.color.w);
}


// color geometry

//float4 HSVtoRGB(float4 c)
//{
//    return float4(mix(saturate((abs(fract(c.x + float3(1,2,3)/3) * 6 - 3) - 1)),1,c.y) * c.z, c.a);
//}

float3 HSVtoRGB(float3 c)
{
    float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

//float4 HSVtoRGB(float4 hsv)
//{
//    int H       = hsv[0];
//    float S     = hsv[1];
//    float V     = hsv[2];
//    float alpha = hsv[3];
//
//    float C = S * V;
//    float X = C * (1 - abs(fmod(H / 60.0, 2) - 1));
//    float m = V - C;
//    float Rs, Gs, Bs;
//
//    if(H >= 0 && H < 60) {
//        Rs = C;
//        Gs = X;
//        Bs = 0;
//    }
//    else if(H >= 60 && H < 120) {
//        Rs = X;
//        Gs = C;
//        Bs = 0;
//    }
//    else if(H >= 120 && H < 180) {
//        Rs = 0;
//        Gs = C;
//        Bs = X;
//    }
//    else if(H >= 180 && H < 240) {
//        Rs = 0;
//        Gs = X;
//        Bs = C;
//    }
//    else if(H >= 240 && H < 300) {
//        Rs = X;
//        Gs = 0;
//        Bs = C;
//    }
//    else {
//        Rs = C;
//        Gs = 0;
//        Bs = X;
//    }
//
//    return float4(Rs + m,
//           Gs + m,
//           Bs + m,
//           alpha);
//}




typedef struct {
    float4 position [[position, invariant]];
    float4 color;
    float4 v_position;
    float4 v_position_flat [[flat]];
    float2 v_canvas;
    float2 tex_coord;
    short   texture_index   [[flat]];
    short   texture_sampler_index [[flat]];
    short   texture_mult_index [[flat]];
    short   texture_add_index [[flat]];
} FRAGMENT_Polygon_Color;

vertex FRAGMENT_Polygon_Color vertex_polygon_color(uint vid [[vertex_id]],
                                                   uint entity_index [[instance_id]],
                                                   uint bvid [[base_vertex]],
                                                   //VERTEX_Polygon_Color in [[stage_in]],
                                                   const device VERTEX_Polygon_Color* vertex_list [[ buffer(0)]],
                                                   constant SharedUniforms& shared_uniforms [[ buffer(kBufferIndexSharedUniforms) ]],
                                                   const device Instance_Data* entity_uniforms [[ buffer(kBufferIndexEntityUniforms) ]],
                                                   constant Semishared_Uniforms& semishared_uniforms [[ buffer(kBufferIndexSemisharedUniforms) ]],
                                                   const device Draw_Args_Indexed* offsets [[buffer(kBufferIndexOffsets)]]
                                                   )
{
    FRAGMENT_Polygon_Color out;
    
    out.position   = semishared_uniforms.projection_matrix *
    semishared_uniforms.view_matrix *
    entity_uniforms[entity_index].model_matrix *
                     //in.position;
    vertex_list[vid].position;
    
    out.v_position = out.position;
    out.v_position.x *= shared_uniforms.aspect_ratio;
    
    out.v_position_flat = out.v_position;
    //float2 out.v_canvas = float2((out.v_position.x + 1) * 0.5, 1 - (out.v_position.y + 1) * 0.5);
    //float2 out.v_canvas = float2((out.v_position.x * 0.5) + 0.5, 0.5 - (out.v_position.y * 0.5));
    out.v_canvas = float2((out.position.x * 0.5) + 0.5, 0.5 - (out.position.y * 0.5));
    out.color = (vertex_list[vid].color * entity_uniforms[entity_index].color_factor) + entity_uniforms[entity_index].color_addition;

    out.texture_index = entity_uniforms[entity_index].texture_index;
    out.texture_sampler_index = entity_uniforms[entity_index].texture_sampler_index;
    out.texture_mult_index = entity_uniforms[entity_index].texture_mult_index;
    
    return out;
}

fragment float4 fragment_polygon_color(FRAGMENT_Polygon_Color in [[stage_in]],
                                       device Fragment_Shader_Arguments_MTL3* fragment_shader_args [[ buffer(MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS) ]]) {
    float4 out = in.color;
    
    float4 mult_color = float4(fragment_shader_args->textures[in.texture_mult_index].texture.sample(fragment_shader_args->samplers[in.texture_sampler_index],                                                   in.v_canvas));
    out *= mult_color.a;
    
    
    out.r = out.r*out.a;
    out.g = out.g*out.a;
    out.b = out.b*out.a;
    return out;
}

fragment float4 fragment_polygon_color_hsv(FRAGMENT_Polygon_Color in [[stage_in]],
                                           device Fragment_Shader_Arguments_MTL3* fragment_shader_args [[ buffer(MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS) ]]) {
    float3 out = HSVtoRGB(in.color.xyz);
    
    float4 mult_color = float4(fragment_shader_args->textures[in.texture_mult_index].texture.sample(fragment_shader_args->samplers[in.texture_sampler_index],                                                   in.v_canvas));
    out *= mult_color.a;
    
    out.r = out.r*in.color.a;
    out.g = out.g*in.color.a;
    out.b = out.b*in.color.a;
    return float4(out, in.color.a);
}

// MARK: - textured

vertex FRAGMENT_Polygon_Color vertex_polygon_color_texture(uint vid [[vertex_id]],
                                                           uint entity_index [[instance_id]],
                                                           uint bvid [[base_vertex]],
                                                           //VERTEX_Polygon_Color in [[stage_in]],
                                                           const device VERTEX_Polygon_Color* vertex_list [[ buffer(0)]],
                                                           constant SharedUniforms& shared_uniforms [[ buffer(kBufferIndexSharedUniforms) ]],
                                                           const device Instance_Data* entity_uniforms [[ buffer(kBufferIndexEntityUniforms) ]],
                                                           constant Semishared_Uniforms& semishared_uniforms [[ buffer(kBufferIndexSemisharedUniforms) ]],
                                                           const device Draw_Args_Indexed* offsets [[buffer(kBufferIndexOffsets)]]
                                                           )
{
    FRAGMENT_Polygon_Color out;
    
    out.position   = semishared_uniforms.projection_matrix *
    semishared_uniforms.view_matrix *
    entity_uniforms[entity_index].model_matrix *
    //in.position;
    vertex_list[vid].position;
    
    out.v_position = out.position;
    out.v_position.x *= shared_uniforms.aspect_ratio;
    
    out.v_position_flat = out.v_position;
    
    //float2 out.v_canvas = float2((out.v_position.x + 1) * 0.5, 1 - (out.v_position.y + 1) * 0.5);
    //float2 out.v_canvas = float2((out.v_position.x * 0.5) + 0.5, 0.5 - (out.v_position.y * 0.5));
    out.v_canvas = float2((out.position.x * 0.5) + 0.5, 0.5 - (out.position.y * 0.5));
    
    out.color = (vertex_list[vid].color * entity_uniforms[entity_index].color_factor) + entity_uniforms[entity_index].color_addition;
    
    out.tex_coord  = (vertex_list[vid].tex_coord * (float2(entity_uniforms[entity_index].texture_coordinates_modifiers[0], entity_uniforms[entity_index].texture_coordinates_modifiers[1])))
        + float2(entity_uniforms[entity_index].texture_coordinates_modifiers[2], entity_uniforms[entity_index].texture_coordinates_modifiers[3])
        + (entity_uniforms[entity_index].texture_animation_speed * shared_uniforms.time_seconds);
    
    out.texture_index = entity_uniforms[entity_index].texture_index;
    out.texture_sampler_index = entity_uniforms[entity_index].texture_sampler_index;
    out.texture_mult_index = entity_uniforms[entity_index].texture_mult_index;
    out.texture_add_index = entity_uniforms[entity_index].texture_add_index;

    return out;
}

typedef struct Fragment_Shader_Arguments_GPU_FAMILY_5 {
    metal::array<texture2d<half, access::sample>, MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_5> textures  [[ id(MTT_ARGUMENT_BUFFER_ID_TEXTURES ) ]];
} Fragment_Shader_Arguments_GPU_FAMILY_5;

// TODO support newer families for more textures
typedef struct Fragment_Shader_Arguments_GPU_FAMILY_6_OR_7 {
    metal::array<texture2d<half, access::sample>, MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_6_OR_7> textures  [[ id(MTT_ARGUMENT_BUFFER_ID_TEXTURES ) ]];
} Fragment_Shader_Arguments_GPU_FAMILY_6_OR_7;


fragment float4 fragment_polygon_color_texture_with_arg_buffer_family5(FRAGMENT_Polygon_Color in [[stage_in]],
                                               device Fragment_Shader_Arguments_GPU_FAMILY_5& fragment_shader_args [[ buffer(MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS) ]])
{
    constexpr sampler texture_sampler(coord::normalized,
                                      mag_filter::nearest,
                                      min_filter::nearest,
                                      DEFAULT_TEXTURE_ADDRESS_MODE);
        
    
    float4 out = in.color * float4(fragment_shader_args.textures[in.texture_index].sample(texture_sampler, in.tex_coord));
    out.r = out.r*out.a;
    out.g = out.g*out.a;
    out.b = out.b*out.a;
    return out;
}


fragment float4 fragment_polygon_color_texture_f6_or_7(FRAGMENT_Polygon_Color in [[stage_in]],
                                                  metal::array<texture2d<half, access::sample>, MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_6_OR_7> textures [[ texture(0) ]])
{
    constexpr sampler texture_sampler(coord::normalized,
                                      mag_filter::nearest,
                                      min_filter::nearest,
                                      DEFAULT_TEXTURE_ADDRESS_MODE);
        
    
    float4 out = in.color * float4(textures[in.texture_index].sample(texture_sampler, in.tex_coord));
    out.r = out.r*out.a;
    out.g = out.g*out.a;
    out.b = out.b*out.a;
    return out;
    
}

fragment float4 fragment_polygon_color_texture_f5(FRAGMENT_Polygon_Color in [[stage_in]],
                                               metal::array<texture2d<half, access::sample>, MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_5> textures [[ texture(0) ]])
{
    constexpr sampler texture_sampler(coord::normalized,
                mag_filter::nearest,
                min_filter::nearest,
                DEFAULT_TEXTURE_ADDRESS_MODE);
        
    
    float4 out = in.color * float4(textures[in.texture_index].sample(texture_sampler, in.tex_coord));
    out.r = out.r*out.a;
    out.g = out.g*out.a;
    out.b = out.b*out.a;
    return out;
}

fragment float4 fragment_polygon_color_texture_f3(FRAGMENT_Polygon_Color in [[stage_in]],
                                                  metal::array<texture2d<half, access::sample>, MTT_MAX_NUM_TEXTURE_ARGUMENTS_GPU_FAMILY_3> textures [[ texture(0) ]])
{
    constexpr sampler texture_sampler(coord::normalized,
                                      mag_filter::nearest,
                                      min_filter::nearest,
                                      DEFAULT_TEXTURE_ADDRESS_MODE);
    
    
    float4 out = in.color * float4(textures[in.texture_index].sample(texture_sampler, in.tex_coord));
    out.r = out.r*out.a;
    out.g = out.g*out.a;
    out.b = out.b*out.a;
    return out;
}

fragment float4 fragment_polygon_color_texture_with_arg_buffer_MTL3_bindless(FRAGMENT_Polygon_Color in [[stage_in]],
                                                                            device Fragment_Shader_Arguments_MTL3* fragment_shader_args [[ buffer(MTT_FRAGMENT_BUFFER_INDEX_ARGUMENTS) ]])
{
//    constexpr sampler texture_sampler(coord::normalized,
//                                      mag_filter::nearest,
//                                      min_filter::nearest,
//                                      DEFAULT_TEXTURE_ADDRESS_MODE);
    
    //float2 uv = float2((in.v_position.x + 1) * 0.5, 1 - (in.v_position.y + 1) * 0.5);
    //float2 uv = float2((in.v_position.x * 0.5) + 0.5, 0.5 - (in.v_position.y * 0.5));
    //float2 uv = in.v_canvas;
    float4 out = in.color * float4(fragment_shader_args->textures[in.texture_index].texture.sample(fragment_shader_args->samplers[in.texture_sampler_index], in.tex_coord));
    float4 mult_color = float4(fragment_shader_args->textures[in.texture_mult_index].texture.sample(fragment_shader_args->samplers[in.texture_sampler_index],                                                   in.v_canvas));
//    float4 add_color = float4(fragment_shader_args->textures[in.texture_add_index].texture.sample(fragment_shader_args->samplers[in.texture_sampler_index],                                                   in.v_canvas));
    
//    out.rgb += add_color.rgb * add_color.a;
    out *= mult_color.a;
    
    out.r = out.r*out.a;
    out.g = out.g*out.a;
    out.b = out.b*out.a;
    
    return out;
}

