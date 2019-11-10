/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "base/math/vec4.h"
#include "base/math/vec3.h"

namespace Base {

/* Translation Matrix
[ 1 0 0 Tx ]
[ 0 1 0 Ty ]
[ 0 0 1 Tz ]
[ 0 0 0 1	]

** Scaling Matrix
[ Sx 0	0	0 ]
[ 0	Sy 0	0 ]
[ 0	0	Sz 0 ]
[ 0	0	0	1 ]

** Rotation Matrices. Rotate 'd' radians around x-, y-, z-axes respectively.
** Around X.			** Around Y.
** Around Z.
[ 1 0		0		 0 ]		[ cosd	0 sind 0
]			[ cosd -sind 0 0 ]
[ 0 cosd -sind 0 ]		[ 0		 1 0		0 ]
[ sind cosd	0 0 ]
[ 0 sind cosd	0 ]		[ -sind 0 cosd 0 ]
[ 0		0		 1 0 ]
[ 0 0		0		 1 ]		[ 0		 0
0		1 ]			[ 0		0		 0 1 ]

*/

// matrix is stored row-ordered (DirectX style). i.e. matrix[0 - 3] is the first
// row, so matrix[3] is Tx.

template <class T> class TMatrix4x4 {
public:
  static const TMatrix4x4 Identity;

public:
  inline TMatrix4x4();
  inline TMatrix4x4(const TMatrix4x4 &m);

  // creates matrix from row-ordered T-array.
  inline TMatrix4x4(const T *p);

  // creates matrix - row-order.
  inline TMatrix4x4(T a00, T a10, T a20, T a30, T a01, T a11, T a21, T a31,
                    T a02, T a12, T a22, T a32, T a03, T a13, T a23, T a33);

  // accessors
  inline T &operator()(u8 r, u8 c);
  inline T operator()(u8 r, u8 c) const;
  inline T &get(u8 r, u8 c);
  inline T get(u8 r, u8 c) const;

  // assignment operators
  inline TMatrix4x4 &operator+=(const TMatrix4x4 &);
  inline TMatrix4x4 &operator-=(const TMatrix4x4 &);
  inline TMatrix4x4 &operator*=(const TMatrix4x4 &);
  inline TMatrix4x4 &operator*=(T);
  inline TMatrix4x4 &operator/=(T);

  inline TVec4<T> operator*(const TVec4<T> &) const;
  inline TVec3<T> operator*(const TVec3<T> &) const;

  // unary operators
  inline TMatrix4x4 operator+() const;
  inline TMatrix4x4 operator-() const;

  // binary operators
  inline TMatrix4x4 operator+(const TMatrix4x4 &) const;
  inline TMatrix4x4 operator-(const TMatrix4x4 &) const;
  inline TMatrix4x4 operator*(const TMatrix4x4 &) const;
  inline TMatrix4x4 operator*(T) const;
  inline TMatrix4x4 operator/(T) const;

  friend TMatrix4x4 operator*(T, const TMatrix4x4 &);

  inline bool operator==(const TMatrix4x4 &) const;
  inline bool operator!=(const TMatrix4x4 &) const;

  inline TMatrix4x4 &makeIdentity();
  inline TMatrix4x4 &makeZero();

  inline TMatrix4x4 &MakeRotation(const Vec3 &vAxis, float fAngle);

  // creates translation matrix.
  inline TMatrix4x4 &MakeTranslation(float x, float y, float z);

  // cretes scale matrix.
  inline TMatrix4x4 &MakeScale(float x, float y, float z);

  inline TMatrix4x4 &InverseIt();

  inline TMatrix4x4 &transposeIt();

  inline void ToArray(float *array) const;
  inline void ToColumnOrderedArray(float *array) const;

  void print() const {
    BASE_LOG("[ %f %f %f %f ]\n", _00, _01, _02, _03);
    BASE_LOG("[ %f %f %f %f ]\n", _10, _11, _12, _13);
    BASE_LOG("[ %f %f %f %f ]\n", _20, _21, _22, _23);
    BASE_LOG("[ %f %f %f %f ]\n", _30, _31, _32, _33);
  }

  static TMatrix4x4 CreateViewMatrix(const Vec3 &vPosition,
                                     const Vec3 &vRightVector,
                                     const Vec3 &vUpVector,
                                     const Vec3 &vFrontVector);
  static TMatrix4x4 CreateProjectionMatrix(float fZNear, float fZFar,
                                           float fLeft, float fRight, float fUp,
                                           float fDown);

  /**
   * makes the matrix identity matrix
   */
  // inline void						identity();
  inline void getTranspose(TMatrix4x4 &out) const;

public:
  union {
    struct {
      T _00, _10, _20, _30;
      T _01, _11, _21, _31;
      T _02, _12, _22, _32;
      T _03, _13, _23, _33;
    };
    T matrix[16];
  };
};

typedef TMatrix4x4<float> Matrix4x4;

template <class T> TMatrix4x4<T>::TMatrix4x4() {
  // memset(&matrix, 0x00, 4*4*sizeof(T));
  matrix[0] = (T)0;
  matrix[1] = (T)0;
  matrix[2] = (T)0;
  matrix[3] = (T)0;
  matrix[4] = (T)0;
  matrix[5] = (T)0;
  matrix[6] = (T)0;
  matrix[7] = (T)0;
  matrix[8] = (T)0;
  matrix[9] = (T)0;
  matrix[10] = (T)0;
  matrix[11] = (T)0;
  matrix[12] = (T)0;
  matrix[13] = (T)0;
  matrix[14] = (T)0;
  matrix[15] = (T)0;
}

template <class T> inline TMatrix4x4<T>::TMatrix4x4(const TMatrix4x4<T> &m) {
  // memcpy(&matrix, &m.matrix, 4*4*sizeof(T));
  matrix[0] = m.matrix[0];
  matrix[1] = m.matrix[1];
  matrix[2] = m.matrix[2];
  matrix[3] = m.matrix[3];
  matrix[4] = m.matrix[4];
  matrix[5] = m.matrix[5];
  matrix[6] = m.matrix[6];
  matrix[7] = m.matrix[7];
  matrix[8] = m.matrix[8];
  matrix[9] = m.matrix[9];
  matrix[10] = m.matrix[10];
  matrix[11] = m.matrix[11];
  matrix[12] = m.matrix[12];
  matrix[13] = m.matrix[13];
  matrix[14] = m.matrix[14];
  matrix[15] = m.matrix[15];
}

template <class T>
inline TMatrix4x4<T>::TMatrix4x4(T a00, T a10, T a20, T a30, T a01, T a11,
                                 T a21, T a31, T a02, T a12, T a22, T a32,
                                 T a03, T a13, T a23, T a33) {
  _00 = a00;
  _01 = a01;
  _02 = a02;
  _03 = a03;
  _10 = a10;
  _11 = a11;
  _12 = a12;
  _13 = a13;
  _20 = a20;
  _21 = a21;
  _22 = a22;
  _23 = a23;
  _30 = a30;
  _31 = a31;
  _32 = a32;
  _33 = a33;
}

template <class T> inline TMatrix4x4<T>::TMatrix4x4(const T *p) {
  // p must be row-ordered.

  // memcpy(&matrix, (const void*)a, 4*4*sizeof(T));
  matrix[0] = p[0];
  matrix[1] = p[1];
  matrix[2] = p[2];
  matrix[3] = p[3];
  matrix[4] = p[4];
  matrix[5] = p[5];
  matrix[6] = p[6];
  matrix[7] = p[7];
  matrix[8] = p[8];
  matrix[9] = p[9];
  matrix[10] = p[10];
  matrix[11] = p[11];
  matrix[12] = p[12];
  matrix[13] = p[13];
  matrix[14] = p[14];
  matrix[15] = p[15];
}

template <class T> inline T &TMatrix4x4<T>::operator()(u8 r, u8 c) {
  return matrix[r + c * 4];
}

template <class T> inline T TMatrix4x4<T>::operator()(u8 r, u8 c) const {
  return matrix[r + c * 4];
}

template <class T> inline T &TMatrix4x4<T>::get(u8 r, u8 c) {
  return matrix[r + c * 4];
}

template <class T> inline T TMatrix4x4<T>::get(u8 r, u8 c) const {
  return matrix[r + c * 4];
}

template <class T>
inline TMatrix4x4<T> &TMatrix4x4<T>::operator+=(const TMatrix4x4 &m) {
  matrix[0] += m.matrix[0];
  matrix[1] += m.matrix[1];
  matrix[2] += m.matrix[2];
  matrix[3] += m.matrix[3];
  matrix[4] += m.matrix[4];
  matrix[5] += m.matrix[5];
  matrix[6] += m.matrix[6];
  matrix[7] += m.matrix[7];
  matrix[8] += m.matrix[8];
  matrix[9] += m.matrix[9];
  matrix[10] += m.matrix[10];
  matrix[11] += m.matrix[11];
  matrix[12] += m.matrix[12];
  matrix[13] += m.matrix[13];
  matrix[14] += m.matrix[14];
  matrix[15] += m.matrix[15];

  return *this;
}

template <class T>
inline TMatrix4x4<T> &TMatrix4x4<T>::operator-=(const TMatrix4x4 &m) {
  matrix[0] -= m.matrix[0];
  matrix[1] -= m.matrix[1];
  matrix[2] -= m.matrix[2];
  matrix[3] -= m.matrix[3];
  matrix[4] -= m.matrix[4];
  matrix[5] -= m.matrix[5];
  matrix[6] -= m.matrix[6];
  matrix[7] -= m.matrix[7];
  matrix[8] -= m.matrix[8];
  matrix[9] -= m.matrix[9];
  matrix[10] -= m.matrix[10];
  matrix[11] -= m.matrix[11];
  matrix[12] -= m.matrix[12];
  matrix[13] -= m.matrix[13];
  matrix[14] -= m.matrix[14];
  matrix[15] -= m.matrix[15];
  return *this;
}

template <class T>
inline TMatrix4x4<T> &TMatrix4x4<T>::operator*=(const TMatrix4x4 &m) {
  TMatrix4x4<T> tthis(*this);

  matrix[0] = tthis.matrix[0] * m.matrix[0] + tthis.matrix[1] * m.matrix[4] +
              tthis.matrix[2] * m.matrix[8] + tthis.matrix[3] * m.matrix[12];

  matrix[1] = tthis.matrix[0] * m.matrix[1] + tthis.matrix[1] * m.matrix[5] +
              tthis.matrix[2] * m.matrix[9] + tthis.matrix[3] * m.matrix[13];

  matrix[2] = tthis.matrix[0] * m.matrix[2] + tthis.matrix[1] * m.matrix[6] +
              tthis.matrix[2] * m.matrix[10] + tthis.matrix[3] * m.matrix[14];

  matrix[3] = tthis.matrix[0] * m.matrix[3] + tthis.matrix[1] * m.matrix[7] +
              tthis.matrix[2] * m.matrix[11] + tthis.matrix[3] * m.matrix[15];

  matrix[4] = tthis.matrix[4] * m.matrix[0] + tthis.matrix[5] * m.matrix[4] +
              tthis.matrix[6] * m.matrix[8] + tthis.matrix[7] * m.matrix[12];

  matrix[5] = tthis.matrix[4] * m.matrix[1] + tthis.matrix[5] * m.matrix[5] +
              tthis.matrix[6] * m.matrix[9] + tthis.matrix[7] * m.matrix[13];

  matrix[6] = tthis.matrix[4] * m.matrix[2] + tthis.matrix[5] * m.matrix[6] +
              tthis.matrix[6] * m.matrix[10] + tthis.matrix[7] * m.matrix[14];

  matrix[7] = tthis.matrix[4] * m.matrix[3] + tthis.matrix[5] * m.matrix[7] +
              tthis.matrix[6] * m.matrix[11] + tthis.matrix[7] * m.matrix[15];

  matrix[8] = tthis.matrix[8] * m.matrix[0] + tthis.matrix[9] * m.matrix[4] +
              tthis.matrix[10] * m.matrix[8] + tthis.matrix[11] * m.matrix[12];

  matrix[9] = tthis.matrix[8] * m.matrix[1] + tthis.matrix[9] * m.matrix[5] +
              tthis.matrix[10] * m.matrix[9] + tthis.matrix[11] * m.matrix[13];

  matrix[10] = tthis.matrix[8] * m.matrix[2] + tthis.matrix[9] * m.matrix[6] +
               tthis.matrix[10] * m.matrix[10] +
               tthis.matrix[11] * m.matrix[14];

  matrix[11] = tthis.matrix[8] * m.matrix[3] + tthis.matrix[9] * m.matrix[7] +
               tthis.matrix[10] * m.matrix[11] +
               tthis.matrix[11] * m.matrix[15];

  matrix[12] = tthis.matrix[12] * m.matrix[0] + tthis.matrix[13] * m.matrix[4] +
               tthis.matrix[14] * m.matrix[8] + tthis.matrix[15] * m.matrix[12];

  matrix[13] = tthis.matrix[12] * m.matrix[1] + tthis.matrix[13] * m.matrix[5] +
               tthis.matrix[14] * m.matrix[9] + tthis.matrix[15] * m.matrix[13];

  matrix[14] = tthis.matrix[12] * m.matrix[2] + tthis.matrix[13] * m.matrix[6] +
               tthis.matrix[14] * m.matrix[10] +
               tthis.matrix[15] * m.matrix[14];

  matrix[15] = tthis.matrix[12] * m.matrix[3] + tthis.matrix[13] * m.matrix[7] +
               tthis.matrix[14] * m.matrix[11] +
               tthis.matrix[15] * m.matrix[15];

  return *this;
}

template <class T> inline TMatrix4x4<T> &TMatrix4x4<T>::operator*=(T scalar) {
  matrix[0] *= scalar;
  matrix[1] *= scalar;
  matrix[2] *= scalar;
  matrix[3] *= scalar;
  matrix[4] *= scalar;
  matrix[5] *= scalar;
  matrix[6] *= scalar;
  matrix[7] *= scalar;
  matrix[8] *= scalar;
  matrix[9] *= scalar;
  matrix[10] *= scalar;
  matrix[11] *= scalar;
  matrix[12] *= scalar;
  matrix[13] *= scalar;
  matrix[14] *= scalar;
  matrix[15] *= scalar;
  return *this;
}

template <class T> inline TMatrix4x4<T> &TMatrix4x4<T>::operator/=(T scalar) {
  matrix[0] /= scalar;
  matrix[1] /= scalar;
  matrix[2] /= scalar;
  matrix[3] /= scalar;
  matrix[4] /= scalar;
  matrix[5] /= scalar;
  matrix[6] /= scalar;
  matrix[7] /= scalar;
  matrix[8] /= scalar;
  matrix[9] /= scalar;
  matrix[10] /= scalar;
  matrix[11] /= scalar;
  matrix[12] /= scalar;
  matrix[13] /= scalar;
  matrix[14] /= scalar;
  matrix[15] /= scalar;
  return *this;
}

template <class T> inline TMatrix4x4<T> TMatrix4x4<T>::operator+() const {
  return TMatrix4x4<T>(*this);
}

template <class T> inline TMatrix4x4<T> TMatrix4x4<T>::operator-() const {
  return TMatrix4x4<T>(*this) * -1;
}

template <class T>
inline TMatrix4x4<T> TMatrix4x4<T>::operator+(const TMatrix4x4 &m) const {
  return TMatrix4x4<T>(_00 + m._00, _01 + m._01, _02 + m._02, _03 + m._03W,
                       _10 + m._10, _11 + m._11, _12 + m._12, _13 + m._13,
                       _20 + m._20, _21 + m._21, _22 + m._22, _23 + m._23,
                       _30 + m._30, _31 + m._31, _32 + m._32, _33 + m._33);
}

template <class T>
inline TMatrix4x4<T> TMatrix4x4<T>::operator-(const TMatrix4x4 &m) const {
  return TMatrix4x4<T>(matrix[0] - m.matrix[0], matrix[1] - m.matrix[1],
                       matrix[2] - m.matrix[2], matrix[3] - m.matrix[3],
                       matrix[4] - m.matrix[4], matrix[5] - m.matrix[5],
                       matrix[6] - m.matrix[6], matrix[7] - m.matrix[7],
                       matrix[8] - m.matrix[8], matrix[9] - m.matrix[9],
                       matrix[10] - m.matrix[10], matrix[11] - m.matrix[11],
                       matrix[12] - m.matrix[12], matrix[13] - m.matrix[13],
                       matrix[14] - m.matrix[14], matrix[15] - m.matrix[15]);
}

template <class T>
inline TMatrix4x4<T> TMatrix4x4<T>::operator*(const TMatrix4x4 &m) const {
  // A*B
  return TMatrix4x4<T>(
      // _00
      matrix[0] * m.matrix[0] + matrix[1] * m.matrix[4] +
          matrix[2] * m.matrix[8] + matrix[3] * m.matrix[12],

      // _10
      matrix[0] * m.matrix[1] + matrix[1] * m.matrix[5] +
          matrix[2] * m.matrix[9] + matrix[3] * m.matrix[13],

      // _20
      matrix[0] * m.matrix[2] + matrix[1] * m.matrix[6] +
          matrix[2] * m.matrix[10] + matrix[3] * m.matrix[14],

      // _30
      matrix[0] * m.matrix[3] + matrix[1] * m.matrix[7] +
          matrix[2] * m.matrix[11] + matrix[3] * m.matrix[15],

      // _01
      matrix[4] * m.matrix[0] + matrix[5] * m.matrix[4] +
          matrix[6] * m.matrix[8] + matrix[7] * m.matrix[12],

      // _11
      matrix[4] * m.matrix[1] + matrix[5] * m.matrix[5] +
          matrix[6] * m.matrix[9] + matrix[7] * m.matrix[13],

      // _21
      matrix[4] * m.matrix[2] + matrix[5] * m.matrix[6] +
          matrix[6] * m.matrix[10] + matrix[7] * m.matrix[14],

      // _31
      matrix[4] * m.matrix[3] + matrix[5] * m.matrix[7] +
          matrix[6] * m.matrix[11] + matrix[7] * m.matrix[15],

      // _02
      matrix[8] * m.matrix[0] + matrix[9] * m.matrix[4] +
          matrix[10] * m.matrix[8] + matrix[11] * m.matrix[12],

      // _12
      matrix[8] * m.matrix[1] + matrix[9] * m.matrix[5] +
          matrix[10] * m.matrix[9] + matrix[11] * m.matrix[13],

      // _22
      matrix[8] * m.matrix[2] + matrix[9] * m.matrix[6] +
          matrix[10] * m.matrix[10] + matrix[11] * m.matrix[14],

      // _ 32
      matrix[8] * m.matrix[3] + matrix[9] * m.matrix[7] +
          matrix[10] * m.matrix[11] + matrix[11] * m.matrix[15],

      // _03
      matrix[12] * m.matrix[0] + matrix[13] * m.matrix[4] +
          matrix[14] * m.matrix[8] + matrix[15] * m.matrix[12],

      // _13
      matrix[12] * m.matrix[1] + matrix[13] * m.matrix[5] +
          matrix[14] * m.matrix[9] + matrix[15] * m.matrix[13],

      // _23
      matrix[12] * m.matrix[2] + matrix[13] * m.matrix[6] +
          matrix[14] * m.matrix[10] + matrix[15] * m.matrix[14],

      // _33
      matrix[12] * m.matrix[3] + matrix[13] * m.matrix[7] +
          matrix[14] * m.matrix[11] + matrix[15] * m.matrix[15]);

  /*
   return TMatrix4x4<T>
   (
   // _11
   m.matrix[ 0] * matrix[ 0] +
   m.matrix[ 1] * matrix[ 4] +
   m.matrix[ 2] * matrix[ 8] +
   m.matrix[ 3] * matrix[12],

   // _12
   m.matrix[ 4] * matrix[ 0] +
   m.matrix[ 5] * matrix[ 4] +
   m.matrix[ 6] * matrix[ 8] +
   m.matrix[ 7] * matrix[12],

   // _13
   m.matrix[ 8] * matrix[ 0] +
   m.matrix[ 9] * matrix[ 4] +
   m.matrix[10] * matrix[ 8] +
   m.matrix[11] * matrix[12],

   // _14
   m.matrix[12] * matrix[ 0] +
   m.matrix[13] * matrix[ 4] +
   m.matrix[14] * matrix[ 8] +
   m.matrix[15] * matrix[12],

   // _21
   m.matrix[ 0] * matrix[ 1] +
   m.matrix[ 1] * matrix[ 5] +
   m.matrix[ 2] * matrix[ 9] +
   m.matrix[ 3] * matrix[13],

   // _22
   m.matrix[ 4] * matrix[ 1] +
   m.matrix[ 5] * matrix[ 5] +
   m.matrix[ 6] * matrix[ 9] +
   m.matrix[ 7] * matrix[13],

   // _23
   m.matrix[ 8] * matrix[ 1] +
   m.matrix[ 9] * matrix[ 5] +
   m.matrix[10] * matrix[ 9] +
   m.matrix[11] * matrix[13],

   // _24
   m.matrix[12] * matrix[ 1] +
   m.matrix[13] * matrix[ 5] +
   m.matrix[14] * matrix[ 9] +
   m.matrix[15] * matrix[13],

   // _31
   m.matrix[ 0] * matrix[ 2] +
   m.matrix[ 1] * matrix[ 6] +
   m.matrix[ 2] * matrix[10] +
   m.matrix[ 3] * matrix[14],

   // _32
   m.matrix[ 4] * matrix[ 2] +
   m.matrix[ 5] * matrix[ 6] +
   m.matrix[ 6] * matrix[10] +
   m.matrix[ 7] * matrix[14],

   // _33
   m.matrix[ 8] * matrix[ 2] +
   m.matrix[ 9] * matrix[ 6] +
   m.matrix[10] * matrix[10] +
   m.matrix[11] * matrix[14],

   // _34
   m.matrix[12] * matrix[ 2] +
   m.matrix[13] * matrix[ 6] +
   m.matrix[14] * matrix[10] +
   m.matrix[15] * matrix[14],

   // _41
   m.matrix[ 0] * matrix[ 3] +
   m.matrix[ 1] * matrix[ 7] +
   m.matrix[ 2] * matrix[11] +
   m.matrix[ 3] * matrix[15],

   // _42
   m.matrix[ 4] * matrix[ 3] +
   m.matrix[ 5] * matrix[ 7] +
   m.matrix[ 6] * matrix[11] +
   m.matrix[ 7] * matrix[15],

   // _ 43
   m.matrix[ 8] * matrix[ 3] +
   m.matrix[ 9] * matrix[ 7] +
   m.matrix[10] * matrix[11] +
   m.matrix[11] * matrix[15],

   // _44
   m.matrix[12] * matrix[ 3] +
   m.matrix[13] * matrix[ 7] +
   m.matrix[14] * matrix[11] +
   m.matrix[15] * matrix[15]
   );

   */
}

template <class T>
inline TMatrix4x4<T> TMatrix4x4<T>::operator*(T scalar) const {
  return TMatrix4x4<T>(
      scalar * matrix[0], scalar * matrix[1], scalar * matrix[2],
      scalar * matrix[3], scalar * matrix[4], scalar * matrix[5],
      scalar * matrix[6], scalar * matrix[7], scalar * matrix[8],
      scalar * matrix[9], scalar * matrix[10], scalar * matrix[11],
      scalar * matrix[12], scalar * matrix[13], scalar * matrix[14],
      scalar * matrix[15]);
}

template <class T>
inline TMatrix4x4<T> TMatrix4x4<T>::operator/(T scalar) const {
  BASE_ASSERT(scalar != 0, "0 division");
  T invScalar = ((T)1) / scalar;
  return TMatrix4x4<T>(
      invScalar * matrix[0], invScalar * matrix[1], invScalar * matrix[2],
      invScalar * matrix[3], invScalar * matrix[4], invScalar * matrix[5],
      invScalar * matrix[6], invScalar * matrix[7], invScalar * matrix[8],
      invScalar * matrix[9], invScalar * matrix[10], invScalar * matrix[11],
      invScalar * matrix[12], invScalar * matrix[13], invScalar * matrix[14],
      invScalar * matrix[15]);
}

template <class T>
inline bool TMatrix4x4<T>::operator==(const TMatrix4x4 &m) const {
  return memncmp(&matrix, &m.matrix, 4 * 4 * sizeof(T)) ? true : false;
  // todo: close compare
}

template <class T>
inline void TMatrix4x4<T>::getTranspose(TMatrix4x4 &out) const {
  out.matrix[0] = matrix[0];
  out.matrix[1] = matrix[4];
  out.matrix[2] = matrix[8];
  out.matrix[3] = matrix[12];

  out.matrix[4] = matrix[1];
  out.matrix[5] = matrix[5];
  out.matrix[6] = matrix[9];
  out.matrix[7] = matrix[13];

  out.matrix[8] = matrix[2];
  out.matrix[9] = matrix[6];
  out.matrix[10] = matrix[10];
  out.matrix[11] = matrix[14];

  out.matrix[12] = matrix[3];
  out.matrix[13] = matrix[7];
  out.matrix[14] = matrix[11];
  out.matrix[15] = matrix[15];
}

template <class T> inline TMatrix4x4<T> &TMatrix4x4<T>::makeIdentity() {
  matrix[0] = (T)1;
  matrix[1] = (T)0;
  matrix[2] = (T)0;
  matrix[3] = (T)0;
  matrix[4] = (T)0;
  matrix[5] = (T)1;
  matrix[6] = (T)0;
  matrix[7] = (T)0;
  matrix[8] = (T)0;
  matrix[9] = (T)0;
  matrix[10] = (T)1;
  matrix[11] = (T)0;
  matrix[12] = (T)0;
  matrix[13] = (T)0;
  matrix[14] = (T)0;
  matrix[15] = (T)1;

  return *this;
}

template <class T> inline TMatrix4x4<T> &TMatrix4x4<T>::makeZero() {
  // memset(&matrix, 0x00, 4*4*sizeof(T));
  matrix[0] = (T)0;
  matrix[1] = (T)0;
  matrix[2] = (T)0;
  matrix[3] = (T)0;
  matrix[4] = (T)0;
  matrix[5] = (T)0;
  matrix[6] = (T)0;
  matrix[7] = (T)0;
  matrix[8] = (T)0;
  matrix[9] = (T)0;
  matrix[10] = (T)0;
  matrix[11] = (T)0;
  matrix[12] = (T)0;
  matrix[13] = (T)0;
  matrix[14] = (T)0;
  matrix[15] = (T)0;
  return *this;
}

// template<class T> inline
// TMatrix4x4<T>& TMatrix4x4<T>::makeTranslation(const TVec3<T>& vec)
//{
//	makeIdentity();
//	matrix[0][3] = vec.x;
//	matrix[1][3] = vec.y;
//	matrix[2][3] = vec.z;
//	return *this;
//}

template <class T>
inline TMatrix4x4<T> &TMatrix4x4<T>::MakeTranslation(float x, float y,
                                                     float z) {
  makeIdentity();
  matrix[12] = x;
  matrix[13] = y;
  matrix[14] = z;
  return *this;
}

template <class T>
inline TMatrix4x4<T> &TMatrix4x4<T>::MakeScale(float x, float y, float z) {
  makeIdentity();
  matrix[0] = x;
  matrix[5] = y;
  matrix[10] = z;
  return *this;
}

template <class T> inline TMatrix4x4<T> &TMatrix4x4<T>::transposeIt() {
  T temp;

  temp = _01;
  _01 = _10;
  _10 = temp;
  temp = _02;
  _02 = _20;
  _20 = temp;
  temp = _03;
  _03 = _30;
  _30 = temp;
  temp = _12;
  _12 = _21;
  _21 = temp;
  temp = _13;
  _13 = _31;
  _31 = temp;
  temp = _23;
  _23 = _32;
  _32 = temp;

  return *this;
}

template <class T> inline TMatrix4x4<T> &TMatrix4x4<T>::InverseIt() {
  T fA0 = matrix[0] * matrix[5] - matrix[1] * matrix[4];
  T fA1 = matrix[0] * matrix[6] - matrix[2] * matrix[4];
  T fA2 = matrix[0] * matrix[7] - matrix[3] * matrix[4];
  T fA3 = matrix[1] * matrix[6] - matrix[2] * matrix[5];
  T fA4 = matrix[1] * matrix[7] - matrix[3] * matrix[5];
  T fA5 = matrix[2] * matrix[7] - matrix[3] * matrix[6];
  T fB0 = matrix[8] * matrix[13] - matrix[9] * matrix[12];
  T fB1 = matrix[8] * matrix[14] - matrix[10] * matrix[12];
  T fB2 = matrix[8] * matrix[15] - matrix[11] * matrix[12];
  T fB3 = matrix[9] * matrix[14] - matrix[10] * matrix[13];
  T fB4 = matrix[9] * matrix[15] - matrix[11] * matrix[13];
  T fB5 = matrix[10] * matrix[15] - matrix[11] * matrix[14];

  T fDet =
      fA0 * fB5 - fA1 * fB4 + fA2 * fB3 + fA3 * fB2 - fA4 * fB1 + fA5 * fB0;

  T fInvDet = T(1) / fDet;
  T temp[16];

  temp[0] = (+matrix[5] * fB5 - matrix[6] * fB4 + matrix[7] * fB3) * fInvDet;
  temp[1] = (-matrix[1] * fB5 + matrix[2] * fB4 - matrix[3] * fB3) * fInvDet;
  temp[2] = (+matrix[13] * fA5 - matrix[14] * fA4 + matrix[15] * fA3) * fInvDet;
  temp[3] = (-matrix[9] * fA5 + matrix[10] * fA4 - matrix[11] * fA3) * fInvDet;
  temp[4] = (-matrix[4] * fB5 + matrix[6] * fB2 - matrix[7] * fB1) * fInvDet;
  temp[5] = (+matrix[0] * fB5 - matrix[2] * fB2 + matrix[3] * fB1) * fInvDet;
  temp[6] = (-matrix[12] * fA5 + matrix[14] * fA2 - matrix[15] * fA1) * fInvDet;
  temp[7] = (+matrix[8] * fA5 - matrix[10] * fA2 + matrix[11] * fA1) * fInvDet;
  temp[8] = (+matrix[4] * fB4 - matrix[5] * fB2 + matrix[7] * fB0) * fInvDet;
  temp[9] = (-matrix[0] * fB4 + matrix[1] * fB2 - matrix[3] * fB0) * fInvDet;
  temp[10] =
      (+matrix[12] * fA4 - matrix[13] * fA2 + matrix[15] * fA0) * fInvDet;
  temp[11] = (-matrix[8] * fA4 + matrix[9] * fA2 - matrix[11] * fA0) * fInvDet;
  temp[12] = (-matrix[4] * fB3 + matrix[5] * fB1 - matrix[6] * fB0) * fInvDet;
  temp[13] = (+matrix[0] * fB3 - matrix[1] * fB1 + matrix[2] * fB0) * fInvDet;
  temp[14] =
      (-matrix[12] * fA3 + matrix[13] * fA1 - matrix[14] * fA0) * fInvDet;
  temp[15] = (+matrix[8] * fA3 - matrix[9] * fA1 + matrix[10] * fA0) * fInvDet;

  memcpy(matrix, temp, 16 * sizeof(T));
  /*
   T a0 = matrix[ 0] * matrix[ 5] - matrix[ 4] * matrix[ 1];
   T a1 = matrix[ 0] * matrix[ 9] - matrix[ 8] * matrix[ 1];
   T a2 = matrix[ 0] * matrix[13] - matrix[12] * matrix[ 1];
   T a3 = matrix[ 4] * matrix[ 9] - matrix[ 8] * matrix[ 5];
   T a4 = matrix[ 4] * matrix[13] - matrix[12] * matrix[ 5];
   T a5 = matrix[ 8] * matrix[13] - matrix[12] * matrix[ 9];
   T b0 = matrix[ 2] * matrix[ 7] - matrix[ 6] * matrix[ 3];
   T b1 = matrix[ 2] * matrix[11] - matrix[10] * matrix[ 3];
   T b2 = matrix[ 2] * matrix[15] - matrix[14] * matrix[ 3];
   T b3 = matrix[ 6] * matrix[11] - matrix[10] * matrix[ 7];
   T b4 = matrix[ 6] * matrix[15] - matrix[14] * matrix[ 7];
   T b5 = matrix[10] * matrix[15] - matrix[14] * matrix[11];

   T det = a0*b5 - a1*b4 + a2*b3 + a3*b2 - a4*b1 + a5*b0;

   Matrix4x4 inverse;
   inverse.matrix[ 0] = + matrix[ 5]*b5 - matrix[ 9]*b4 + matrix[13]*b3;
   inverse.matrix[ 1] = - matrix[ 1]*b5 + matrix[ 9]*b2 - matrix[13]*b1;
   inverse.matrix[ 2] = + matrix[ 1]*b4 - matrix[ 5]*b2 + matrix[13]*b0;
   inverse.matrix[ 3] = - matrix[ 1]*b3 + matrix[ 5]*b1 - matrix[ 9]*b0;
   inverse.matrix[ 4] = - matrix[ 4]*b5 + matrix[ 8]*b4 - matrix[12]*b3;
   inverse.matrix[ 5] = + matrix[ 0]*b5 - matrix[ 8]*b2 + matrix[12]*b1;
   inverse.matrix[ 6] = - matrix[ 0]*b4 + matrix[ 4]*b2 - matrix[12]*b0;
   inverse.matrix[ 7] = + matrix[ 0]*b3 - matrix[ 4]*b1 + matrix[ 8]*b0;
   inverse.matrix[ 8] = + matrix[ 7]*a5 - matrix[11]*a4 + matrix[15]*a3;
   inverse.matrix[ 9] = - matrix[ 3]*a5 + matrix[11]*a2 - matrix[15]*a1;
   inverse.matrix[10] = + matrix[ 3]*a4 - matrix[ 7]*a2 + matrix[15]*a0;
   inverse.matrix[11] = - matrix[ 3]*a3 + matrix[ 7]*a1 - matrix[11]*a0;
   inverse.matrix[12] = - matrix[ 6]*a5 + matrix[10]*a4 - matrix[14]*a3;
   inverse.matrix[13] = + matrix[ 2]*a5 - matrix[10]*a2 + matrix[14]*a1;
   inverse.matrix[14] = - matrix[ 2]*a4 + matrix[ 6]*a2 - matrix[14]*a0;
   inverse.matrix[15] = + matrix[ 2]*a3 - matrix[ 6]*a1 + matrix[10]*a0;

   T invDet = ((T)1)/det;
   inverse.matrix[ 0] *= invDet;
   inverse.matrix[ 1] *= invDet;
   inverse.matrix[ 2] *= invDet;
   inverse.matrix[ 3] *= invDet;
   inverse.matrix[ 4] *= invDet;
   inverse.matrix[ 5] *= invDet;
   inverse.matrix[ 6] *= invDet;
   inverse.matrix[ 7] *= invDet;
   inverse.matrix[ 8] *= invDet;
   inverse.matrix[ 9] *= invDet;
   inverse.matrix[10] *= invDet;
   inverse.matrix[11] *= invDet;
   inverse.matrix[12] *= invDet;
   inverse.matrix[13] *= invDet;
   inverse.matrix[14] *= invDet;
   inverse.matrix[15] *= invDet;

   *this = inverse;

   */
  return *this;
}

template <class T>
inline TVec3<T> TMatrix4x4<T>::operator*(const TVec3<T> &p) const {
  return TVec3<T>(matrix[0] * p.x + matrix[1] * p.y + matrix[2] * p.z,

                  matrix[4] * p.x + matrix[5] * p.y + matrix[6] * p.z,

                  matrix[8] * p.x + matrix[9] * p.y + matrix[10] * p.z);
}

template <class T>
inline TMatrix4x4<T> &TMatrix4x4<T>::MakeRotation(const Vec3 &vAxis,
                                                  float fAngle) {
  T cs = cosf(fAngle);
  T sn = sinf(fAngle);
  T oneMinusCos = 1.0f - cs;
  T x2 = vAxis.x * vAxis.x;
  T y2 = vAxis.y * vAxis.y;
  T z2 = vAxis.z * vAxis.z;
  T xym = vAxis.x * vAxis.y * oneMinusCos;
  T xzm = vAxis.x * vAxis.z * oneMinusCos;
  T yzm = vAxis.y * vAxis.z * oneMinusCos;
  T xSin = vAxis.x * sn;
  T ySin = vAxis.y * sn;
  T zSin = vAxis.z * sn;

  matrix[0] = x2 * oneMinusCos + cs;
  matrix[1] = xym - zSin;
  matrix[2] = xzm + ySin;
  matrix[3] = 0.0f;
  matrix[4] = xym + zSin;
  matrix[5] = y2 * oneMinusCos + cs;
  matrix[6] = yzm - xSin;
  matrix[7] = 0.0f;
  matrix[8] = xzm - ySin;
  matrix[9] = yzm + xSin;
  matrix[10] = z2 * oneMinusCos + cs;
  matrix[11] = 0.0f;
  matrix[12] = 0.0f;
  matrix[13] = 0.0f;
  matrix[14] = 0.0f;
  matrix[15] = 1.0f;

  return *this;
}

template <class T>
TMatrix4x4<T> TMatrix4x4<T>::CreateViewMatrix(const Vec3 &vPosition,
                                              const Vec3 &vRightVector,
                                              const Vec3 &vUpVector,
                                              const Vec3 &vFrontVector) {
  return TMatrix4x4<T>(vRightVector.x, vRightVector.y, vRightVector.z,
                       -vPosition.dot(vRightVector), vUpVector.x, vUpVector.y,
                       vUpVector.z, -vPosition.dot(vUpVector), vFrontVector.x,
                       vFrontVector.y, vFrontVector.z,
                       -vPosition.dot(vFrontVector), 0.0f, 0.0f, 0.0f, 1.0f);
}

template <class T>
TMatrix4x4<T> TMatrix4x4<T>::CreateProjectionMatrix(float fZNear, float fZFar,
                                                    float fLeft, float fRight,
                                                    float fUp, float fDown) {
  float dMin = fZNear;
  float dMax = fZFar;
  float uMin = fDown;
  float uMax = fUp;
  float rMin = fLeft;
  float rMax = fRight;

  float invDDiff = 1.0f / (dMax - dMin);
  float invUDiff = 1.0f / (uMax - uMin);
  float invRDiff = 1.0f / (rMax - rMin);
  float sumRMinRMaxInvRDiff = (rMin + rMax) * invRDiff;
  float sumUMinUMaxInvUDiff = (uMin + uMax) * invUDiff;
  // float sumDMinDMaxInvDDiff = (dMin + dMax)*invDDiff;

  float twoDMinInvRDiff = 2.0f * dMin * invRDiff;
  float twoDMinInvUDiff = 2.0f * dMin * invUDiff;
  float dMaxInvDDiff = dMax * invDDiff;
  float dMinDMaxInvDDiff = dMin * dMaxInvDDiff;
  // float twoDMinDMaxInvDDiff = 2.0f*dMinDMaxInvDDiff;

  return TMatrix4x4<T>(twoDMinInvRDiff, 0.0f, -sumRMinRMaxInvRDiff, 0.0f, 0.0f,
                       twoDMinInvUDiff, -sumUMinUMaxInvUDiff, 0.0f, 0.0f, 0.0f,
                       dMaxInvDDiff, -dMinDMaxInvDDiff, 0.0f, 0.0f, 1.0f, 0.0f);
}

template <class T>
inline TVec4<T> TMatrix4x4<T>::operator*(const TVec4<T> &p) const {

  return TVec4<T>(
      matrix[0] * p.x + matrix[1] * p.y + matrix[2] * p.z + matrix[3] * p.w,

      matrix[4] * p.x + matrix[5] * p.y + matrix[6] * p.z + matrix[07] * p.w,

      matrix[8] * p.x + matrix[9] * p.y + matrix[10] * p.z + matrix[11] * p.w,

      matrix[12] * p.x + matrix[13] * p.y + matrix[14] * p.z +
          matrix[15] * p.w);
  /*
   return TVec4<T>(
   matrix[ 0]*p.x + matrix[ 4]*p.y + matrix[ 8]*p.z + matrix[ 12]*p.w,

   matrix[ 1]*p.x + matrix[ 5]*p.y + matrix[ 9]*p.z + matrix[13]*p.w,

   matrix[ 2]*p.x + matrix[ 6]*p.y + matrix[10]*p.z + matrix[14]*p.w,

   matrix[ 3]*p.x + matrix[ 7]*p.y + matrix[11]*p.z + matrix[15]*p.w);
   */
}

template <class T> inline void TMatrix4x4<T>::ToArray(float *array) const {

  for(u32 i = 0; i < 16; ++i)
    array[i] = matrix[i];
  // or use memcpy: memcpy( array, matrix, 16 * sizeof(float) );
}

template <class T>
inline void TMatrix4x4<T>::ToColumnOrderedArray(float *array) const {
  array[0] = matrix[0];
  array[1] = matrix[4];
  array[2] = matrix[8];
  array[3] = matrix[12];
  array[4] = matrix[1];
  array[5] = matrix[5];
  array[6] = matrix[9];
  array[7] = matrix[13];
  array[8] = matrix[2];
  array[9] = matrix[6];
  array[10] = matrix[10];
  array[11] = matrix[14];
  array[12] = matrix[3];
  array[13] = matrix[7];
  array[14] = matrix[11];
  array[15] = matrix[15];
}

} // namespace Base
