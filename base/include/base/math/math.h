/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "base/core/assert.h"

#include <stdlib.h> // rand
#include <math.h>
#include <time.h>
#include <string> //memcpy

namespace Base {
namespace Math {

//! Constants.
static const float PI = float(3.14159265358979323846f);
static const float TWO_PI = 2 * PI;

//! Bit Operations.

#define ROUND_UP_32(value, alignment)                                          \
  ((((s32)(value) + (alignment - 1)) & ~(alignment - 1)))
#define ROUND_DOWN_32(value, alignment) ((s32)(value) &= (s32)(alignment - 1))

#define BIT_SET_ON(val, bit) ((val) |= (1 << (bit)))

#define BIT_SET_OFF(val, bit) ((val) ^= (1 << (bit)))

#define BIT_IS_ON(val, bit) ((val) & (1 << (bit)))

#define BIT32(x) (1 << (x))
#define BIT64(x) (1ULL << (x))

// marks mask bit in value as set.
#define BIT_MASK_ON(val, mask) ((val) |= (mask))

/// marks mask bit in value as not set.
#define BIT_MASK_OFF(val, mask) ((val) &= ~(mask))

/// toggle mask bit.
#define BIT_MASK_TOGGLE(val, mask) ((val) ^= (mask))

/// checks whether all bits of mask are set.
#define BIT_MASK_IS_ON(val, mask) (((val) & (mask)) == (mask))

/// checks whether any bit of mask is set.
#define BIT_MASK_IS_ANY_ON(val, mask) ((val) & (mask))

//---------------------------------------------------------------------------------------

void SRand();

u32 Rand(u32 uMax);

//! returns random floating point number from <0, 1>
float RandFloat();

//! returns a random floating point number from <0, fMax>
float RandFloat(float fMax);

//! returns a random floating point number from <fMin, fMax>
float RandFloatRange(float fMin, float fMax);

template <class T> inline T Pow(T base, T exp);

template <class T> inline T Log(T val);


template <class T> inline T Pow(T base, T exp) { return pow(base, exp); }
template <class T> inline T Log(T val) { return logf(val); }
template <class T> inline T Sqrt(T a) { return sqrtf((float)a); }
template <class T> inline T Cos(T angle) { return cosf((float)angle); }
template <class T> inline T Sin(T angle) { return sinf((float)angle); }
template <class T> inline T Abs(T value) { return fabsf(value); }
template <class T> inline T Acos(T angle) { return acosf(angle); }
template <class T> inline T Asin(T angle) { return asinf(angle); }
template <class T> inline T Deg2rad(T degree) {
  return degree * 0.0174532925199432957692f;
}
template <class T> inline T Rad2deg(T radian) {
  return radian * 57.29577951308232087684f;
}
template <class T> inline T Clamp(T value, T min, T max) {
  return value < min ? min : value > max ? max : value;
}
template <class T> inline T Mod(T value, T modulus) {
  return fmodf(value, modulus);
}
template <class T> inline T Floor(T value) { return floorf(value); }
template <class T> inline T Ceil(T value) { return ceilf(value); }
template <class T> inline T Lerp(T a, T b, T factor) {
  return (1.f - factor) * a + factor * b;
}
inline void SRand() { srand((s32)time(NULL)); }
inline u32 Rand(u32 uMax) { return rand() % uMax; }
inline float RandFloat() { return float(rand()) / float(RAND_MAX); }
inline float RandFloat(float fMax) {
  return float(rand()) / float(RAND_MAX) * fMax;
}
inline float RandFloatRange(float fMin, float fMax) {
  BASE_ASSERT(fMin <= fMax, "minimum value higher than maximum");
  return (fMax - fMin) * RandFloat() + fMin;
}
/*
namespace Matrix4x4{

template<class T> inline
T determinant(const T* m)
{
        T fA0 = m[ 0]*m[ 5] - m[ 1]*m[ 4];
        T fA1 = m[ 0]*m[ 6] - m[ 2]*m[ 4];
        T fA2 = m[ 0]*m[ 7] - m[ 3]*m[ 4];
        T fA3 = m[ 1]*m[ 6] - m[ 2]*m[ 5];
        T fA4 = m[ 1]*m[ 7] - m[ 3]*m[ 5];
        T fA5 = m[ 2]*m[ 7] - m[ 3]*m[ 6];
        T fB0 = m[ 8]*m[13] - m[ 9]*m[12];
        T fB1 = m[ 8]*m[14] - m[10]*m[12];
        T fB2 = m[ 8]*m[15] - m[11]*m[12];
        T fB3 = m[ 9]*m[14] - m[10]*m[13];
        T fB4 = m[ 9]*m[15] - m[11]*m[13];
        T fB5 = m[10]*m[15] - m[11]*m[14];

        return fA0*fB5-fA1*fB4+fA2*fB3+fA3*fB2-fA4*fB1+fA5*fB0;
}

template<class T> inline
void identity(T* m)
{
        m[0 ] = T(1.0);	m[1 ] = T(0.0);	m[2 ] = T(0.0);	m[3 ] = T(0.0);

        m[4 ] = T(0.0);	m[5 ] = T(1.0);	m[6 ] = T(0.0);	m[7 ] = T(0.0);

        m[8 ] = T(0.0);	m[9 ] = T(0.0);	m[10] = T(1.0);	m[11] = T(0.0);

        m[12] = T(0.0);	m[13] = T(0.0);	m[14] = T(0.0);	m[15] = T(1.0);
}

template<class T> inline
void translation(T* out_mat, T x, T y, T z)
{
        Matrix4x4::identity(out_mat);

        out_mat[12] = x;
        out_mat[13] = y;
        out_mat[14] = z;
}

template<class T> inline
void scale(T* out_mat, T x, T y, T z)
{
        Matrix4x4::identity(out_mat);

        out_mat[0] = x;
        out_mat[5] = y;
        out_mat[10] = z;
}

template<class T> inline
void transpose(T* out_mat, const T* in_mat)
{
        for(u8 i = 0; i < 4; ++i)
        {
                out_mat[4 * i + i] = in_mat[4 * i + i];
                for(u8 j = i + 1; j < 4; ++j)
                {
                        out_mat[4 * i + j] = in_mat[4 * j + i];
                        out_mat[4 * j + i] = in_mat[4 * i + j];
                }
        }
}

} //namespace Matrix4x4
*/
/*
namespace Quaternion {

template <class T> inline void multiply(T *out, const T *in1, const T *in2) {
  out[3] =
      in1[3] * in2[3] - in1[0] * in2[0] - in1[1] * in2[1] - in1[2] * in2[2];
  out[0] =
      in1[3] * in2[0] + in1[0] * in2[3] + in1[1] * in2[2] - in1[2] * in2[1];
  out[1] =
      in1[3] * in2[1] + in1[1] * in2[3] + in1[2] * in2[0] - in1[0] * in2[2];
  out[2] =
      in1[3] * in2[2] + in1[2] * in2[3] + in1[0] * in2[1] - in1[1] * in2[0];
}

template <class T> inline void inverse(T *out, const T *in) {

  // Quaternion::normalize<T>(out, in);
  // out[0] = -out[0];
  // out[1] = -out[1];
  // out[2] = -out[2];
  // out[3] = out[3];

  T norm = in[0] * in[0] + in[1] * in[1] + in[2] * in[2] + in[3] * in[3];

  if(norm > (T)0.0) {
    T fInvNorm = ((T)1.0) / norm;
    out[0] = -in[0] * fInvNorm;
    out[1] = -in[1] * fInvNorm;
    out[2] = -in[2] * fInvNorm;
    out[3] = in[3] * fInvNorm;
  } else {
    // return an invalid result to flag the error
    for(u32 i = 0; i < 4; i++) {
      out[i] = (T)0.0;
    }
  }
}

template <class T>
inline void multiplyVec3(T *out_vec, const T *in_quat, const T *in_vec) {
  // v = Q * quaternion(vec3, (0)) * Q^-1

  T inversed[4];
  Quaternion::inverse<T>(inversed, in_quat);

  T vector[4];
  memcpy(vector, in_vec, 3 * sizeof(T));
  vector[3] = 0;

  T temp[4];

  Quaternion::multiply<T>(temp, in_quat, vector);

  T res[4];

  Quaternion::multiply<T>(res, temp, inversed);

  out_vec[0] = res[0];
  out_vec[1] = res[1];
  out_vec[2] = res[2];
}

template<class T> inline
void normalize(T* out, const T* in)
{
        T tmp = T(1) / Math::Vector4::length<T>(in);
        out[0] = in[0] * tmp;
        out[1] = in[1] * tmp;
        out[2] = in[2] * tmp;
        out[3] = in[3] * tmp;
}

template <class T> inline T dot(T *in1, T *in2) {
  return in1[0] * in2[0] + in1[1] * in2[1] + in1[2] * in2[2] + in1[3] * in2[3];
}

template <class T> inline void toMatrix(T *out_mat, const T *in_quat) {
  T twoX = ((T)2) * in_quat[0];
  T twoY = ((T)2) * in_quat[1];
  T twoZ = ((T)2) * in_quat[2];
  T twoWX = twoX * in_quat[3];
  T twoWY = twoY * in_quat[3];
  T twoWZ = twoZ * in_quat[3];
  T twoXX = twoX * in_quat[0];
  T twoXY = twoY * in_quat[0];
  T twoXZ = twoZ * in_quat[0];
  T twoYY = twoY * in_quat[1];
  T twoYZ = twoZ * in_quat[1];
  T twoZZ = twoZ * in_quat[1];

  out_mat[0] = (T)1 - (twoYY + twoZZ);
  out_mat[1] = twoXY - twoWZ;
  out_mat[2] = twoXZ + twoWY;
  out_mat[3] = (T)0;

  out_mat[4] = twoXY + twoWZ;
  out_mat[5] = (T)1 - (twoXX + twoZZ);
  out_mat[6] = twoYZ - twoWX;
  out_mat[7] = (T)0;

  out_mat[8] = twoXZ - twoWY;
  out_mat[9] = twoYZ + twoWX;
  out_mat[10] = (T)1 - (twoXX + twoYY);
  out_mat[11] = (T)0;

  out_mat[12] = (T)0;
  out_mat[13] = (T)0;
  out_mat[14] = (T)0;
  out_mat[15] = (T)1;
}

template <class T> inline void rotationX(T *out_quat, T angle) {
  angle = Math::Deg2rad(angle);
  angle *= 0.5f;

  out_quat[0] = Math::Sin<float>(angle);
  out_quat[1] = 0.0f;
  out_quat[2] = 0.0f;
  out_quat[3] = Math::Cos<float>(angle);
}

template <class T> inline void rotationY(T *out_quat, T angle) {
  angle = Math::Deg2rad(angle);
  angle *= 0.5f;

  out_quat[0] = 0.0f;
  out_quat[1] = Math::Sin<float>(angle);
  out_quat[2] = 0.0f;
  out_quat[3] = Math::Cos<float>(angle);
}

template <class T> inline void rotationZ(T *out_quat, T angle) {
  angle = Math::Deg2rad(angle);
  angle *= 0.5f;

  out_quat[0] = 0.0f;
  out_quat[1] = 0.0f;
  out_quat[2] = Math::Sin<float>(angle);
  out_quat[3] = Math::Cos<float>(angle);
}

template <class T> void rotationFromAxisAngle(T *out, T radians, T *axis) {
  T mod = Math::Sin<T>(radians / 2);
  T len =
      Math::Sqrt<T>(axis[0] * axis[0] + axis[1] * axis[1] + axis[2] * axis[2]);
  out[0] = mod * axis[0] / len;
  out[1] = mod * axis[1] / len;
  out[2] = mod * axis[2] / len;
  out[3] = Math::Cos<T>(radians / 2);
}

} // namespace Quaternion
*/
} // namespace Math
} // namespace Base
