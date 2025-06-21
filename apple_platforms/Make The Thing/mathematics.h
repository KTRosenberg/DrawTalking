#ifndef MATHEMATICS_H
#define MATHEMATICS_H
#include "types_common.h"

#define GLM_FORCE_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
//#define GLM_FORCE_INLINE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_INTRINSICS

#if defined(__ARM_NEON__)
#define GLM_FORCE_NEON
#endif

#ifdef MATHEMATICS_LOCAL_INCLUDE

#include "glm/glm.hpp"
#ifndef MATHEMATICS_DONT_USE_ALIGNED
#include "glm/gtc/type_aligned.hpp"
#endif
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

//#include "glm/gtx/transform.hpp"
//#include "glm/gtx/transform2.hpp"
#include "glm/gtx/scalar_multiplication.hpp"
#include "glm/gtx/rotate_vector.hpp"



//#include "glm/gtx/compatibility.hpp"
#include "glm/ext/scalar_integer.hpp"
#include "glm/gtx/euler_angles.hpp"
//#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"
//#include "glm/gtx/norm.hpp"
#include "glm/gtx/matrix_decompose.hpp"

//#include "glm/gtx/string_cast.hpp"
//#include "glm/gtx/transform2.hpp"
//#include "glm/gtx/spline.hpp"

#else

#include <glm/glm.hpp>
#ifndef MATHEMATICS_DONT_USE_ALIGNED
#include <glm/gtc/type_aligned.hpp>
#endif
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/transform2.hpp>

#endif

#ifndef MATHEMATICS_DONT_USE_ALIGNED
typedef glm::aligned_vec2  Vector2;
typedef glm::aligned_highp_dvec2 Vector2_64;
typedef glm::tvec2<uint64, glm::aligned_highp> UVector2_64;
typedef glm::aligned_vec3  Vector3;
typedef glm::aligned_vec4  Vector4;
typedef glm::aligned_ivec2 IntVector2;
typedef glm::aligned_ivec3 IntVector3;
typedef glm::aligned_ivec4 IntVector4;
typedef glm::aligned_uvec2 UintVector2;
typedef glm::aligned_uvec3 UintVector3;
typedef glm::aligned_uvec4 UintVector4;
typedef glm::aligned_mat3  Matrix3;
typedef glm::aligned_mat4  Matrix4;
typedef glm::qua<float, glm::aligned_highp> Quaternion;
#else
typedef glm::vec2  Vector2;
typedef glm::aligned_highp_dvec2 Vector2_64;
typedef glm::tvec2<uint64> UVector2_64;
typedef glm::vec3  Vector3;
typedef glm::vec4  Vector4;
typedef glm::ivec2 IntVector2;
typedef glm::ivec3 IntVector3;
typedef glm::ivec4 IntVector4;
typedef glm::uvec2 UintVector2;
typedef glm::uvec3 UintVector3;
typedef glm::uvec4 UintVector4;
typedef glm::mat3  Matrix3;
typedef glm::mat4  Matrix4;
typedef glm::qua<float, glm::highp> Quaternion;
#endif

typedef Vector2    Vec2;
typedef Vector2_64 Vec2_64;
typedef UVector2_64 UVec2_64;
typedef Vector3    Vec3;
typedef Vector4    Vec4;
typedef IntVector2 iVec2;
typedef IntVector3 iVec3;
typedef IntVector4 iVec4;
typedef UintVector2 uVec2;
typedef UintVector3 uVec3;
typedef UintVector4 uVec4;
typedef Matrix3    Mat3;
typedef Matrix4    Mat4;
typedef Quaternion Quat;

typedef Vector2    vec2;
typedef Vector2_64 vec2_64;
typedef UVector2_64 uvec2_64;
typedef Vector3    vec3;
typedef Vector4    vec4;
typedef IntVector2 ivec2;
typedef IntVector3 ivec3;
typedef IntVector4 ivec4;
typedef UintVector2 uvec2;
typedef UintVector3 uvec3;
typedef UintVector4 uvec4;
typedef Matrix3    mat3;
typedef Matrix4    mat4;
typedef Quaternion quat;

// typedef Vector2    float2;
// typedef Vector3    float3;
// typedef Vector4    float4;
// typedef IntVector2 int2;
// typedef IntVector3 int3;
// typedef IntVector4 int4;

// unaligned

typedef glm::vec2  Vector2_unaligned;
typedef glm::vec3  Vector3_unaligned;
typedef glm::vec4  Vector4_unaligned;
typedef glm::ivec2 IntVector2_unaligned;
typedef glm::ivec3 IntVector3_unaligned;
typedef glm::ivec4 IntVector4_unaligned;
typedef glm::uvec2 UintVector2_unaligned;
typedef glm::uvec3 UintVector3_unaligned;
typedef glm::uvec4 UintVector4_unaligned;
typedef glm::mat3  Matrix3_unaligned;
typedef glm::mat4  Matrix4_unaligned;
typedef glm::qua<float, glm::highp> Quaternion_unaligned;

typedef Vector2_unaligned    Vec2_ua;
typedef Vector3_unaligned    Vec3_ua;
typedef Vector4_unaligned    Vec4_ua;
typedef IntVector2_unaligned iVec2_ua;
typedef IntVector3_unaligned iVec3_ua;
typedef IntVector4_unaligned iVec4_ua;
typedef UintVector2_unaligned uVec2_ua;
typedef UintVector3_unaligned uVec3_ua;
typedef UintVector4_unaligned uVec4_ua;
typedef Matrix3_unaligned    Mat3_ua;
typedef Matrix4_unaligned    Mat4_ua;
typedef Quaternion_unaligned Quat_ua;

typedef Vector2_unaligned    vec2_ua;
typedef Vector3_unaligned    vec3_ua;
typedef Vector4_unaligned    vec4_ua;
typedef IntVector2_unaligned ivec2_ua;
typedef IntVector3_unaligned ivec3_ua;
typedef IntVector4_unaligned ivec4_ua;
typedef UintVector2_unaligned uvec2_ua;
typedef UintVector3_unaligned uvec3_ua;
typedef UintVector4_unaligned uvec4_ua;
typedef Matrix3_unaligned    mat3_ua;
typedef Matrix4_unaligned    mat4_ua;
typedef Quaternion_unaligned quat_ua;

// typedef Vector2_unaligned    float2_ua;
// typedef Vector3_unaligned    float3_ua;
// typedef Vector4_unaligned    float4_ua;
// typedef IntVector2_unaligned int2_ua;
// typedef IntVector3_unaligned int3_ua;
// typedef IntVector4_unaligned int4_ua;

typedef glm::tvec4<u8> U8Vector4;
typedef U8Vector4      u8Vec4;
typedef U8Vector4      u8vec4;

typedef glm::tvec2<u16> U16Vector2;
typedef U16Vector2      u16Vec2;
typedef U16Vector2      u8vec2;

struct Quad {
    vec2 tl;
    vec2 bl;
    vec2 br;
    vec2 tr;
};

#include <limits>

#define POSITIVE_INFINITY (std::numeric_limits<f64>::infinity())
#define NEGATIVE_INFINITY (-POSITIVE_INFINITY)

#include <complex>




namespace m {

using glm::translate;
using glm::rotate;
using glm::scale;
using glm::ortho;
using glm::perspective;

using glm::inverse;
using glm::toMat4;
using glm::value_ptr;
using glm::eulerAngles;
using glm::dot;
using glm::length2;
//using glm::to_string;
using glm::dot;
using glm::cross;
using glm::length;
using glm::transpose;

using glm::decompose;
using glm::normalize;

using glm::clamp;

using glm::sign;
using glm::distance;

using glm::ceil;
using glm::floor;


template<typename T>
inline T min(T val_a, T val_b)
{
    return glm::min(val_a, val_b);
}

inline float32 min(float32 val_a, float32 val_b)
{
    return glm::min(val_a, val_b);
}

inline float64 min(float64 val_a, float64 val_b)
{
    return glm::min(val_a, val_b);
}

inline int32 min(int32 val_a, int32 val_b)
{
    return glm::min(val_a, val_b);
}

inline int64 min(int64 val_a, int64 val_b)
{
    return glm::min(val_a, val_b);
}

template<typename T>
inline T max(T val_a, T val_b)
{
    return glm::max(val_a, val_b);
}

inline static float32 max_dimension(vec2 v)
{
    return m::max(v.x, v.y);
}
inline static float64 max_dimension(vec2_64 v)
{
    return m::max(v.x, v.y);
}
inline static void max_min_dimensions(vec2 v, vec2* max_min)
{
    *max_min = (v.x >= v.y) ? v : vec2(v.y, v.x);
}

inline static float32 min_dimension(vec2 v)
{
    return m::min(v.x, v.y);
}
inline static float64 min_dimension(vec2_64 v)
{
    return m::min(v.x, v.y);
}


template <typename T>
T abs(T val)
{
    return glm::abs(val);
}


inline float32 cos(float32 val)
{
    return glm::cos(val);
}
inline float64 cos(float64 val)
{
    return glm::cos(val);
}

inline float32 sin(float32 val)
{
    return glm::sin(val);
}
inline float64 sin(float64 val)
{
    return glm::sin(val);
}

inline float32 sinpi(float32 val)
{
    return simd::sinpi(val);
}
inline float32 cospi(float32 val)
{
    return simd::sinpi(val);
}

inline void sincos(float32 x, float32 *sin, float32 *cos)
{
    return simd::sincos(x, sin, cos);
}
inline void sincospi(float32 x, float32 *sin, float32 *cos)
{
    return simd::sincospi(x, sin, cos);
}

inline float64 sinpi(float64 val)
{
    return simd::sinpi(val);
}
inline float64 cospi(float64 val)
{
    return simd::sinpi(val);
}

inline void sincos(float64 val, float64 *sin, float64 *cos)
{
    return simd::sincos(val, sin, cos);
}
inline void sincospi(float64 val, float64 *sin, float64 *cos)
{
    return simd::sincospi(val, sin, cos);
}


inline f32 lerp(f32 a, f32 b, f32 t)
{
    //return (1 - t) * a + t * b;
    return glm::lerp(a, b, t);
}

inline f64 lerp(f64 a, f64 b, f64 t)
{
    //return (1 - t) * a + t * b;
    return glm::lerp(a, b, t);
}

inline vec2 lerp(vec2 a, vec2 b, f32 t)
{
    //    return {
    //        lerp(a[0], b[0], t),
    //        lerp(a[1], b[1], t)
    //    };
    
    return glm::lerp(a, b, t);
}

inline vec3 lerp(vec3 a, vec3 b, f32 t)
{
    //    return {
    //        lerp(a[0], b[0], t),
    //        lerp(a[1], b[1], t),
    //        lerp(a[2], b[2], t)
    //    };
    return glm::lerp(a, b, t);
}





#define SIN01_RETURN_VAL ((m::sin(val) + 1.0) / 2.0)
inline float64 sin01(float64 val)
{
    return SIN01_RETURN_VAL;
}

inline float32 sin01(float32 val)
{
    return SIN01_RETURN_VAL;
}
#undef SIN01_RETURN_VAL

#define COS01_RETURN_VAL ((m::cos(val) + 1.0) / 2.0)
inline float64 cos01(float64 val)
{
    return COS01_RETURN_VAL;
}

inline float32 cos01(float32 val)
{
    return COS01_RETURN_VAL;
}
#undef COS01_RETURN_VAL

constexpr bool is_powerof2(uint64 N)
{
    return N && ((N & (N - 1)) == 0);
}

constexpr uint64 next_powerof2_ge(uint64 n)
{
    if (is_powerof2(n)) {
        return n;
    }
    
    return glm::nextPowerOfTwo(n);
}

constexpr bool is_pow_2_greater_equal_4(usize N)
{
    return N >= 4 && is_powerof2(N);
}




template <typename T=float32>
inline T smoothstep(T x)
{
    return glm::smoothstep<T>(0, 1, x);
}

template <typename T=float32>
inline T inverse_smoothstep(T x)
{
    return 0.5 - m::sin(glm::asin(1.0-2.0*x)/3.0);
}

inline float32 sqrt(float32 val)
{
    return glm::sqrt(val);
}

inline float64 sqrt(float64 val)
{
    return glm::sqrt(val);
}

inline vec2 vec2_zero()
{
    return vec2(0.0f);
}
inline vec3 vec3_zero()
{
    return vec3(0.0f);
}
inline vec3 vec3_one()
{
    return vec3(1.0f);
}
inline vec4 vec4_zero()
{
    return vec4(0.0f);
}
inline mat4 mat4_identity()
{
    return mat4(1.0f);
}

inline quat quat_identity()
{
    return quat(static_cast<float32>(1), static_cast<float32>(0), static_cast<float32>(0), static_cast<float32>(0));
}

inline float32 circular_rotation(float32 radius, float32 displacement)
{
    return displacement / radius;
}




inline float32 circular_rotation(float32 radius, vec2 initial_position, vec2 final_position)
{
    return circular_rotation(radius, m::distance(initial_position, final_position));
}


#define MTT_HALF_PI_32 (glm::half_pi<f32>())
#define MTT_PI_32 (glm::pi<f32>())
#define MTT_TAU_32 (glm::two_pi<f32>())
#define MTT_TWO_PI_32 (MTT_TAU_32)

#define MTT_HALF_PI_64 (glm::half_pi<f64>())
#define MTT_PI_64 (glm::pi<f64>())
#define MTT_TAU_64 (glm::two_pi<f64>())
#define MTT_TWO_PI_64 (MTT_TAU_64)
#define MTT_2_PI_64 (MTT_TAU_64)

#define MTT_HALF_PI (MTT_HALF_PI_64)
#define MTT_PI (MTT_PI_64)
#define MTT_TAU (MTT_TAU_64)
#define MTT_TWO_PI (MTT_TWO_PI_64)
#define MTT_2_PI (MTT_TWO_PI_64)

//const f32 PI32  = MTT_PI32;
//const f32 TAU32 = MTT_TAU32;
//const f64 PI64  = MTT_PI64;
//const f64 TAU64 = MTT_TAU64;
//const f32 PI  = MTT_PI;
//const f64 TAU = MTT_TAU;

inline f32 atan2pos_32(f64 y, f64 x)
{
    f32 val = glm::atan2<f32>(-y, x);
    
    return (val < 0) ? val + 2 * glm::pi<f64>() : val;
}
inline f64 atan2pos_64(f64 y, f64 x)
{
    f64 val = glm::atan2<f64>(-y, x);
    
    return (val < 0) ? val + 2 * glm::pi<f64>() : val;
}



template <typename T>
inline T atan2(T y, T x) { return glm::atan2<T>(y, x); }

inline f32 dist_squared(Vec3 v, Vec3 w)
{
    f32 dx = v.x - w.x;
    f32 dy = v.y - w.y;
    f32 dz = v.z - w.z;
    return (dx * dx) + (dy * dy) + (dz * dz);
}

inline f32 dist_squared(Vec2 v, Vec2 w)
{
    f32 dx = v.x - w.x;
    f32 dy = v.y - w.y;
    return (dx * dx) + (dy * dy);
}

inline f32 dist(Vec3 v, Vec3 w)
{
    return m::distance(v, w);
}

inline f32 dist(Vec2 v, Vec2 w)
{
    return m::distance(v, w);
}

inline f64 dist_to_segment_squared(Vec3 v, Vec3 w, Vec3 p)
{
    const f64 l2 = dist_squared(v, w);
    if (l2 == 0.0) {
        return dist_squared(p, v);
    }
    
    
    const f64 t = m::max(0.0, m::min(1.0, glm::dot(p - v, w - v) / l2));
    return dist_squared(p, Vec3(v.x + t * (w.x - v.x), v.y + t * (w.y - v.y), 0.0));
}

inline f32 dist_to_segment_squared(Vec2 v, Vec2 w, Vec2 p)
{
    const f32 l2 = dist_squared(v, w);
    if (l2 == 0.0) {
        return dist_squared(p, v);
    }
    
    const f32 t = m::max(0.0f, m::min(1.0f, glm::dot(p - v, w - v) / l2));
    return dist_squared(p, Vec2(v.x + t * (w.x - v.x), v.y + t * (w.y - v.y)));
}

inline f64 dist_to_segment(Vec3 v, Vec3 w, Vec3 p)
{
    return glm::sqrt(dist_to_segment_squared(v, w, p));
}

inline float64 angular_velocity(float64 radians, float64 time_delta)
{
    return radians / time_delta;
}

inline Vec2 angular_impulse(float64 angular_velocity, Vec2 center, Vec2 point)
{
    return -angular_velocity * Vec2(-(point.y - center.y), (point.x - center.x));
}


inline Mat4 rotate_around(vec3 orientation, vec3 p)
{
    Mat4 m_out = Mat4(1.0f);
    m_out = m::translate(m_out, p);
    
    Mat4 rot =  glm::eulerAngleXYZ(orientation.y, orientation.x, orientation.z);
    
    m_out = m_out * rot;
    
    m_out = m::translate(m_out, -p);
    
    return m_out;
    
}

inline mat4 rotate_around_with_matrix(const mat4& m, const mat4& rot_global, const mat4& rot_local, vec3 center_offset)
{
    return rot_global * m * m::translate(m::mat4_identity(), -center_offset) * rot_local * m::translate(m::mat4_identity(), center_offset);
}
inline mat4 rotate_around_with_matrix2(const mat4& m, const mat4& rot_global, const mat4& rot_local, vec3 center_offset)
{
    return rot_global * m * m::translate(m::mat4_identity(), -center_offset) * rot_local * m::translate(m::mat4_identity(), center_offset);
}

inline vec2 rotate_around_2D(vec2 center, float32 angle_radians, vec2 p)
{
    const float32 cx = center.x;
    const float32 cy = center.y;
    float32 cos_val = m::cos(angle_radians);
    float32 sin_val = m::sin(angle_radians);
    return vec2(cos_val * (p.x - cx) - sin_val * (p.y - cy) + cx,
                sin_val * (p.x - cx) + cos_val * (p.y - cy) + cy);
}

inline static float32 positive_angle(float32 angle)
{
    return (float32)((angle < 0) ? (float64)angle + MTT_2_PI_64 : angle);
}

inline static float64 positive_angle(float64 angle)
{
    return (angle < 0) ? angle + MTT_2_PI_64 : angle;
}



//inline vec3 rotate_around(vec3 out_point, vec3 around_point, vec3 orientation)
//{
//    float s = m::sin(angle);
//    float c = m::cos(angle);
//
//    // translate point back to origin:
//    p.x -= cx;
//    p.y -= cy;
//
//    // rotate point
//    float xnew = p.x * c - p.y * s;
//    float ynew = p.x * s + p.y * c;
//
//    // translate point back:
//    p.x = xnew + cx;
//    p.y = ynew + cy;
//    return p;
//}

inline void Vec2_print(Vec2 v)
{
    MTT_print("[%f,%f]\n", v[0], v[1]);
}
inline void Vec3_print(Vec3 v)
{
    MTT_print("[%f,%f,%f]\n", v[0], v[1], v[2]);
}
inline void Vec4_print(Vec4 v)
{
    MTT_print("[%f,%f,%f,%f]\n", v[0], v[1], v[2], v[3]);
}




static const vec3 UP2D    = vec3(0.0f, -1.0f, 0.0f);
static const vec3 DOWN2D  = vec3(0.0f, 1.0f, 0.0f);
static const vec3 LEFT2D  = vec3(-1.0f, 0.0f, 0.0f);
static const vec3 RIGHT2D = vec3(1.0f, 0.0f, 0.0f);



inline float64 determinant_matrix3x3(mat3& m)
{
    float64 determinant  = m[0][0] * (m[1][1]*m[2][2] - m[1][2] * m[2][1]);
    determinant -= m[0][1] * (m[1][0]*m[2][2] - m[1][2] * m[2][0]);
    determinant += m[0][2] * (m[1][0]*m[2][1] - m[1][1] * m[2][0]);
    
    return determinant;
}
inline float64 determinant_matrix3x3_last_column_ones(mat3& m)
{
    const auto& A = m[0][0];
    const auto& B = m[0][1];
    //const auto& C = m[0][2];
    const auto& D = m[1][0];
    const auto& E = m[1][1];
    //const auto& F = m[1][2];
    const auto& G = m[2][0];
    const auto& H = m[2][1];
    //const auto& I = m[2][2];
    
    float64 determinant  = ((D*H) - (G*E));
    determinant -= ((A*H) - (G*B));
    determinant += ((A*E) - (D*B));
    
    return determinant;
}


inline void scale_adjoint_matrix3x3(mat3& a, float64 s, mat3& m)
{
    a[0][0] = (s) * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
    a[1][0] = (s) * (m[1][2] * m[2][0] - m[1][0] * m[2][2]);
    a[2][0] = (s) * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    
    a[0][1] = (s) * (m[0][2] * m[2][1] - m[0][1] * m[2][2]);
    a[1][1] = (s) * (m[0][0] * m[2][2] - m[0][2] * m[2][0]);
    a[2][1] = (s) * (m[0][1] * m[2][0] - m[0][0] * m[2][1]);
    
    a[0][2] = (s) * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);
    a[1][2] = (s) * (m[0][2] * m[1][0] - m[0][0] * m[1][2]);
    a[2][2] = (s) * (m[0][0] * m[1][1] - m[0][1] * m[1][0]);
}


inline mat3 inverse_matrix3x3(mat3& m)
{
    float64 determinant = determinant_matrix3x3(m);
    
    float64 factor = 1.0 / (determinant);
    
    mat3 out_m_inverse;
    scale_adjoint_matrix3x3(out_m_inverse, factor, m);
    
    return out_m_inverse;
}

inline mat3 inverse_matrix3x3_last_column_ones(mat3& m)
{
    float64 determinant = determinant_matrix3x3_last_column_ones(m);
    
    float64 factor = 1.0 / (determinant);
    
    mat3 out_m_inverse;
    scale_adjoint_matrix3x3(out_m_inverse, factor, m);
    
    return out_m_inverse;
}


struct Rotation_Toward_Out {
    float32 value;
    float32 sign;
    float32 dot_product;
    vec3 cross_product;
};
inline Rotation_Toward_Out rotation_toward(vec3 dst, vec3 src, float32 scalar, float32 dt)
{
    float32 dot_product = m::dot(dst, src);
    vec3 cr = m::cross(src, dst);
    float32 sign = m::dot(cr, vec3(0.0f,0.0f,1.0f)) < 0 ? -1.0f : 1.0f;
    float32 rot_by = sign * scalar * m::distance(dst, src) * dt;
    
    return {
        .value = rot_by,
        .sign = sign,
        
        .dot_product = dot_product,
        .cross_product = cr,
    };
}

using complex_float32 = std::complex<float32>;
using complex_float64 = std::complex<float64>;

inline Quad quad_expand(vec2 tl, vec2 bl, vec2 br, vec2 tr)
{
    vec4 axes = m::normalize(vec4(tl - br, tr - bl));
    
    Quad q;
    q.tl = tl + vec2(axes[0], axes[1]);
    q.bl = bl - vec2(axes[2], axes[3]);
    q.br = br - vec2(axes[0], axes[1]);
    q.tr = tr + vec2(axes[2], axes[3]);
    
    return q;
}

inline Quad quad_expand(vec2 tl, vec2 bl, vec2 br, vec2 tr, float32 radius)
{
    vec4 axes = m::normalize(vec4(tl - br, tr - bl)) * radius;
    
    Quad q;
    q.tl = tl + vec2(axes[0], axes[1]);
    q.bl = bl - vec2(axes[2], axes[3]);
    q.br = br - vec2(axes[0], axes[1]);
    q.tr = tr + vec2(axes[2], axes[3]);
    
    return q;
}

inline Quad quad_expand(vec2 tl, vec2 bl, vec2 br, vec2 tr, float32 radius, vec4* axes_out)
{
    vec4 axes = m::normalize(vec4(tl - br, tr - bl)) * radius;
    
    Quad q;
    q.tl = tl + vec2(axes[0], axes[1]);
    q.bl = bl - vec2(axes[2], axes[3]);
    q.br = br - vec2(axes[0], axes[1]);
    q.tr = tr + vec2(axes[2], axes[3]);
    
    *axes_out = axes;
    
    return q;
}






//static bool decompose(mat4 const& ModelMatrix, vec3 & Scale, quat & Orientation, vec3 & Translation, vec3 & Skew, vec4 & Perspective)
//{
//    using T = float32;
//    constexpr const float32 epsilon = std::numeric_limits<float32>::epsilon();
//    using length_t = int;
//
//    auto length = [](vec3 const& v) {
//        vec3 tmp(v*v);
//        return sqrt(tmp.x + tmp.y + tmp.z);
//    };
//
//    auto scale = [length](vec3 const& v, float32 desiredLength) -> vec3
//    {
//        return v * desiredLength / length(v);
//    };
//
//    auto combine = [](
//        vec3 const& a,
//        vec3 const& b,
//        T ascl, T bscl) -> vec3
//    {
//        return (a * ascl) + (b * bscl);
//    };
//
//
//
//    mat4 LocalMatrix(ModelMatrix);
//
//    // Normalize the matrix.
//    if(glm::epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon))
//        return false;
//
//    for(length_t i = 0; i < 4; ++i)
//    for(length_t j = 0; j < 4; ++j)
//        LocalMatrix[i][j] /= LocalMatrix[3][3];
//
//    // perspectiveMatrix is used to solve for perspective, but it also provides
//    // an easy way to test for singularity of the upper 3x3 component.
//    mat4 PerspectiveMatrix(LocalMatrix);
//
//    for(length_t i = 0; i < 3; i++)
//        PerspectiveMatrix[i][3] = static_cast<T>(0);
//    PerspectiveMatrix[3][3] = static_cast<T>(1);
//
//
//
//    /// TODO: Fixme!
//    if(glm::epsilonEqual(determinant(PerspectiveMatrix), static_cast<T>(0), epsilon))
//        return false;
//
//    // First, isolate perspective.  This is the messiest.
//    if(
//       glm::epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon) ||
//       glm::epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon) ||
//       glm::epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon))
//    {
//        // rightHandSide is the right hand side of the equation.
//        vec4 RightHandSide;
//        RightHandSide[0] = LocalMatrix[0][3];
//        RightHandSide[1] = LocalMatrix[1][3];
//        RightHandSide[2] = LocalMatrix[2][3];
//        RightHandSide[3] = LocalMatrix[3][3];
//
//        // Solve the equation by inverting PerspectiveMatrix and multiplying
//        // rightHandSide by the inverse.  (This is the easiest way, not
//        // necessarily the best.)
//        mat4 InversePerspectiveMatrix = m::inverse(PerspectiveMatrix);//   inverse(PerspectiveMatrix, inversePerspectiveMatrix);
//        mat4 TransposedInversePerspectiveMatrix = m::transpose(InversePerspectiveMatrix);//   transposeMatrix4(inversePerspectiveMatrix, transposedInversePerspectiveMatrix);
//
//        Perspective = TransposedInversePerspectiveMatrix * RightHandSide;
//        //  v4MulPointByMatrix(rightHandSide, transposedInversePerspectiveMatrix, perspectivePoint);
//
//        // Clear the perspective partition
//        LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
//        LocalMatrix[3][3] = static_cast<T>(1);
//    }
//    else
//    {
//        // No perspective.
//        Perspective = vec4(0, 0, 0, 1);
//    }
//
//    // Next take care of translation (easy).
//    Translation = vec3(LocalMatrix[3]);
//    LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);
//
//    vec3 Row[3], Pdum3;
//
//    // Now get scale and shear.
//    for(length_t i = 0; i < 3; ++i)
//    for(length_t j = 0; j < 3; ++j)
//        Row[i][j] = LocalMatrix[i][j];
//
//    // Compute X scale factor and normalize first row.
//    Scale.x = length(Row[0]);// v3Length(Row[0]);
//
//    Row[0] = scale(Row[0], static_cast<T>(1));
//
//    // Compute XY shear factor and make 2nd row orthogonal to 1st.
//    Skew.z = dot(Row[0], Row[1]);
//    Row[1] = combine(Row[1], Row[0], static_cast<T>(1), -Skew.z);
//
//    // Now, compute Y scale and normalize 2nd row.
//    Scale.y = length(Row[1]);
//    Row[1] = scale(Row[1], static_cast<T>(1));
//    Skew.z /= Scale.y;
//
//    // Compute XZ and YZ shears, orthogonalize 3rd row.
//    Skew.y = glm::dot(Row[0], Row[2]);
//    Row[2] = combine(Row[2], Row[0], static_cast<T>(1), -Skew.y);
//    Skew.x = glm::dot(Row[1], Row[2]);
//    Row[2] = combine(Row[2], Row[1], static_cast<T>(1), -Skew.x);
//
//    // Next, get Z scale and normalize 3rd row.
//    Scale.z = length(Row[2]);
//    Row[2] = scale(Row[2], static_cast<T>(1));
//    Skew.y /= Scale.z;
//    Skew.x /= Scale.z;
//
//    // At this point, the matrix (in rows[]) is orthonormal.
//    // Check for a coordinate system flip.  If the determinant
//    // is -1, then negate the matrix and the scaling factors.
//    Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
//    if(dot(Row[0], Pdum3) < 0)
//    {
//        for(length_t i = 0; i < 3; i++)
//        {
//            Scale[i] *= static_cast<T>(-1);
//            Row[i] *= static_cast<T>(-1);
//        }
//    }
//
//    // Now, get the rotations out, as described in the gem.
//
//    // FIXME - Add the ability to return either quaternions (which are
//    // easier to recompose with) or Euler angles (rx, ry, rz), which
//    // are easier for authors to deal with. The latter will only be useful
//    // when we fix https://bugs.webkit.org/show_bug.cgi?id=23799, so I
//    // will leave the Euler angle code here for now.
//
//    // ret.rotateY = asin(-Row[0][2]);
//    // if (cos(ret.rotateY) != 0) {
//    //     ret.rotateX = atan2(Row[1][2], Row[2][2]);
//    //     ret.rotateZ = atan2(Row[0][1], Row[0][0]);
//    // } else {
//    //     ret.rotateX = atan2(-Row[2][0], Row[1][1]);
//    //     ret.rotateZ = 0;
//    // }
//
//    int i, j, k = 0;
//    T root, trace = Row[0].x + Row[1].y + Row[2].z;
//    if(trace > static_cast<T>(0))
//    {
//        root = sqrt(trace + static_cast<T>(1.0));
//        Orientation.w = static_cast<T>(0.5) * root;
//        root = static_cast<T>(0.5) / root;
//        Orientation.x = root * (Row[1].z - Row[2].y);
//        Orientation.y = root * (Row[2].x - Row[0].z);
//        Orientation.z = root * (Row[0].y - Row[1].x);
//    } // End if > 0
//    else
//    {
//        static int Next[3] = {1, 2, 0};
//        i = 0;
//        if(Row[1].y > Row[0].x) i = 1;
//        if(Row[2].z > Row[i][i]) i = 2;
//        j = Next[i];
//        k = Next[j];
//
//#           ifdef GLM_FORCE_QUAT_DATA_XYZW
//            int off = 0;
//#           else
//            int off = 1;
//#           endif
//
//        root = sqrt(Row[i][i] - Row[j][j] - Row[k][k] + static_cast<T>(1.0));
//
//        Orientation[i + off] = static_cast<T>(0.5) * root;
//        root = static_cast<T>(0.5) / root;
//        Orientation[j + off] = root * (Row[i][j] + Row[j][i]);
//        Orientation[k + off] = root * (Row[i][k] + Row[k][i]);
//        Orientation.w = root * (Row[j][k] - Row[k][j]);
//    } // End if <= 0
//
//    return true;
//}



//using glm::to_string;
const float64 sqrt3_over_2 = m::sqrt(3.0) / 2.0;
inline void triangle_equilateral_points(vec3 origin, sint32 height_sign, float32 side, vec3* vh, vec3* vl, vec3* vr)
{
    float32 height = height_sign * side * sqrt3_over_2;
    float32 side_half = height_sign * side * 0.5;
    *vh = origin + vec3(0, height, 0);
    *vl = origin - vec3(side_half, 0, 0);
    *vr = origin + vec3(side_half, 0, 0);
}

inline void triangle_equilateral_points(vec2 origin, sint32 height_sign, float32 side, vec2* vh, vec2* vl, vec2* vr)
{
    float32 height = height_sign * side * sqrt3_over_2;
    float32 side_half = height_sign * side * 0.5;
    *vh = origin + vec2(0, height);
    *vl = origin - vec2(side_half, 0);
    *vr = origin + vec2(side_half, 0);
}


}

#endif
