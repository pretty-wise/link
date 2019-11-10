/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/math/matrix4x4.h"
#include "base/math/vec3.h"
#include "base/math/math.h"
#include "base/core/macro.h"

namespace Base {

// axis angle rotation: Q = cos(angle/2) + sin(angle/2) * (x*i + y*j + z*k)
// rotation difference: R = B * A'

template <class T> class TQuaternion {
public:
  static const TQuaternion Zero, Identity;

  inline TQuaternion();
  inline TQuaternion(const TQuaternion &);
  inline TQuaternion(T x, T y, T z, T w);
  inline TQuaternion(T angle, const TVec3<T> &vec);

  inline TQuaternion &operator+=(const TQuaternion &);
  inline TQuaternion &operator-=(const TQuaternion &);
  inline TQuaternion &operator*=(const TQuaternion &);
  inline TQuaternion &operator*=(T);
  inline TQuaternion &operator/=(T);

  inline TQuaternion operator+() const;
  inline TQuaternion operator-() const;

  inline TQuaternion operator+(const TQuaternion &) const;
  inline TQuaternion operator-(const TQuaternion &) const;

  // rotates the quaternion by rhs quaternion's rotation. remember - quaternion
  // multiplication is not commutative!
  inline TQuaternion operator*(const TQuaternion &) const;
  inline TQuaternion operator*(T) const;
  inline TVec3<T> operator*(const TVec3<T> &) const;
  inline TQuaternion operator/(T) const;

  inline bool operator==(const TQuaternion &) const;
  inline bool operator!=(const TQuaternion &) const;

  inline float squaredLength() const;
  inline float length() const;
  inline float dot(const TQuaternion &q) const;
  inline TQuaternion inverse() const;
  inline void normalize();

  // conjugate quaternion represents the opposite rotation (negate x, y, z
  // terms).
  inline TQuaternion conjugate() const;

  // rotation of a vector by a quaternion.
  inline TVec3<T> rotate(const TVec3<T> &vec) const;

  inline void setIdentity();

  inline void toMatrix(TMatrix4x4<T> &out) const;
  //	inline void							toMatrix(TMatrix3x3<T>& out)
  //const;

  void fromAxisAngle(T angle, T x, T y, T z);
  void fromAxisAngle(T angle, const TVec3<T> &vec);

  void toAxisAngle(T &angle, T &x, T &y, T &z);
  void toAxisAngle(T &angle, TVec3<T> &vec);

  static TQuaternion FromAxisAngle(T angle, T x, T y, T z);
  static TQuaternion FromAxisAngle(T angle, const TVec3<T> &vec);

  static TQuaternion FromAxisRotationX(T angle);
  static TQuaternion FromAxisRotationY(T angle);
  static TQuaternion FromAxisRotationZ(T angle);

  // spherical linear interpolation used for rotations.
  static inline TQuaternion SLERP(const TQuaternion<T> &a,
                                  const TQuaternion<T> &b, T factor);

  // linear interpolation. unlike SLERP, this does not rotate at constant speed.
  static inline TQuaternion NLERP(const TQuaternion<T> &a,
                                  const TQuaternion<T> &b, T factor);

public:
  union {
    struct {
      T x, y, z, w;
    };
    T coords[4];
  };
};

typedef TQuaternion<float> Quaternion;

template <class T>
inline TQuaternion<T>::TQuaternion()
    : x(0), y(0), z(0), w(1) {}

template <class T>
inline TQuaternion<T>::TQuaternion(const TQuaternion<T> &rhs)
    : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) {}

template <class T>
inline TQuaternion<T>::TQuaternion(T _x, T _y, T _z, T _w)
    : x(_x), y(_y), z(_z), w(_w) {}

template <class T>
inline TQuaternion<T>::TQuaternion(T _degree, const TVec3<T> &vec) {
  float angle = (_degree / 180.0f) * 3.1415f;
  float coef = sin(angle / 2.0f);

  this->w = cos(angle / 2.0f);
  this->x = vec.x * coef;
  this->y = vec.y * coef;
  this->z = vec.z * coef;
}

template <class T>
inline TQuaternion<T> &TQuaternion<T>::operator+=(const TQuaternion<T> &rhs) {
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  w += rhs.w;
  return *this;
}

template <class T>
inline TQuaternion<T> &TQuaternion<T>::operator-=(const TQuaternion<T> &rhs) {
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  w -= rhs.w;
  return *this;
}

template <class T>
inline TQuaternion<T> &TQuaternion<T>::operator*=(const TQuaternion<T> &rhs) {
  Math::Quaternion::multiply<T>(this->coords, this->coords, rhs.coords);
  return *this;
}

template <class T> inline TQuaternion<T> &TQuaternion<T>::operator*=(T v) {
  x *= v;
  y *= v;
  z *= v;
  w *= v;
  return *this;
}

template <class T> inline TQuaternion<T> &TQuaternion<T>::operator/=(T v) {
  x /= v.x;
  y /= v.y;
  z /= v.z;
  w /= v.w;
  return *this;
}

template <class T> inline TQuaternion<T> TQuaternion<T>::operator+() const {
  return TQuaternion<T>(*this);
}

template <class T> inline TQuaternion<T> TQuaternion<T>::operator-() const {
  return TQuaternion<T>(-x, -y, -z, -w);
}

template <class T>
inline TQuaternion<T> TQuaternion<T>::operator+(const TQuaternion &rhs) const {
  TQuaternion<T> res(*this);
  res += rhs;
  return res;
}

template <class T>
inline TQuaternion<T> TQuaternion<T>::operator-(const TQuaternion &rhs) const {
  TQuaternion<T> res(*this);
  res -= rhs;
  return res;
}

template <class T>
inline TQuaternion<T> TQuaternion<T>::operator*(const TQuaternion &rhs) const {
  TQuaternion<T> res;
  Math::Quaternion::multiply<float>(&res.coords[0], &this->coords[0],
                                    &rhs.coords[0]);
  return res;
}

template <class T> inline TQuaternion<T> TQuaternion<T>::operator*(T v) const {
  TQuaternion<T> res(*this);
  res *= v;
  return res;
}

template <class T> inline TQuaternion<T> TQuaternion<T>::operator/(T v) const {
  TQuaternion<T> res(*this);
  res /= v;
  return res;
}

template <class T>
inline bool TQuaternion<T>::operator==(const TQuaternion &rhs) const {
  return memcmp(coords, rhs.coords, 4 * sizeof(T)) == 0;
}

template <class T>
inline bool TQuaternion<T>::operator!=(const TQuaternion &rhs) const {
  return memcmp(coords, rhs.coords, 4 * sizeof(T)) != 0;
}

template <class T> inline float TQuaternion<T>::squaredLength() const {
  return x * x + y * y + z * z + w * w;
}

template <class T> inline float TQuaternion<T>::length() const {
  return Math::Sqrt<T>(x * x + y * y + z * z + w * w);
}

template <class T>
inline float TQuaternion<T>::dot(const TQuaternion &q) const {
  return Math::Quaternion::dot(this->coords, q.coords);
}

template <class T> inline TQuaternion<T> TQuaternion<T>::inverse() const {
  TQuaternion<T> ret;
  Math::Quaternion::inverse(ret.coords, this->coords);
  return ret;
}

template <class T> inline void TQuaternion<T>::normalize() {
  T len = length();
  T invLength = ((T)1) / len;

  x *= invLength;
  y *= invLength;
  z *= invLength;
  w *= invLength;
}

template <class T> inline TQuaternion<T> TQuaternion<T>::conjugate() const {
  return TQuaternion<T>(-x, -y, -z, w);
}

template <class T> inline void TQuaternion<T>::setIdentity() {
  x = y = z = 0.0f;
  w = 1.0f;
}

template <class T>
inline TVec3<T> TQuaternion<T>::rotate(const TVec3<T> &vec) const {
  // Given a vector u = (x0,y0,z0) and a unit length quaternion
  // q = <x,y,z,w>, the vector v = (x1,y1,z1) which represents the
  // rotation of u by q is v = q*u*q^{-1} where * indicates quaternion
  // multiplication and where u is treated as the quaternion <x0,y0,z0,0>.
  // Note that q^{-1} = <-x,-y,-z,w>, so no real work is required to
  // invert q.	Now
  //
  //	 q*u*q^{-1} = q*<x0,y0,z0,0>*q^{-1}
  //		 = q*(x0*i+y0*j+z0*k)*q^{-1}
  //		 = x0*(q*i*q^{-1})+y0*(q*j*q^{-1})+z0*(q*k*q^{-1})
  //
  // As 3-vectors, q*i*q^{-1}, q*j*q^{-1}, and 2*k*q^{-1} are the columns
  // of the rotation matrix computed in Quaternion<Real>::ToRotationMatrix.
  // The vector v is obtained as the product of that rotation matrix with
  // vector u.	As such, the quaternion representation of a rotation
  // matrix requires less space than the matrix and more time to compute
  // the rotated vector.	Typical space-time tradeoff...

  TMatrix4x4<T> rot;
  toMatrix(rot);
  return rot * vec;
}

template <class T>
inline void TQuaternion<T>::toMatrix(TMatrix4x4<T> &out) const {
  // Math::Quaternion::toMatrix(&out->matrix[0][0], this->coords);
  T twoX = ((T)2) * x;
  T twoY = ((T)2) * y;
  T twoZ = ((T)2) * z;
  T twoWX = twoX * w;
  T twoWY = twoY * w;
  T twoWZ = twoZ * w;
  T twoXX = twoX * x;
  T twoXY = twoY * x;
  T twoXZ = twoZ * x;
  T twoYY = twoY * y;
  T twoYZ = twoZ * y;
  T twoZZ = twoZ * z;

  out.matrix[0] = (T)1 - (twoYY + twoZZ);
  out.matrix[4] = twoXY - twoWZ;
  out.matrix[8] = twoXZ + twoWY;
  out.matrix[1] = twoXY + twoWZ;
  out.matrix[5] = (T)1 - (twoXX + twoZZ);
  out.matrix[9] = twoYZ - twoWX;
  out.matrix[2] = twoXZ - twoWY;
  out.matrix[6] = twoYZ + twoWX;
  out.matrix[10] = (T)1 - (twoXX + twoYY);

  out.matrix[12] = (T)0;
  out.matrix[13] = (T)0;
  out.matrix[14] = (T)0;
  out.matrix[15] = (T)1;

  // TODO: potrzebne to?
  out.matrix[3] = (T)0;
  out.matrix[7] = (T)0;
  out.matrix[11] = (T)0;
}

template <class T>
inline void TQuaternion<T>::toMatrix(TMatrix3x3<T> &out) const {
  T twoX = ((T)2.f) * x;
  T twoY = ((T)2.f) * y;
  T twoZ = ((T)2.f) * z;
  T twoWX = twoX * w;
  T twoWY = twoY * w;
  T twoWZ = twoZ * w;
  T twoXX = twoX * x;
  T twoXY = twoY * x;
  T twoXZ = twoZ * x;
  T twoYY = twoY * y;
  T twoYZ = twoZ * y;
  T twoZZ = twoZ * z;

  out.matrix[0][0] = (T)1.f - (twoYY + twoZZ);
  out.matrix[0][1] = twoXY - twoWZ;
  out.matrix[0][2] = twoXZ + twoWY;
  out.matrix[1][0] = twoXY + twoWZ;
  out.matrix[1][1] = (T)1.f - (twoXX + twoZZ);
  out.matrix[1][2] = twoYZ - twoWX;
  out.matrix[2][0] = twoXZ - twoWY;
  out.matrix[2][1] = twoYZ + twoWX;
  out.matrix[2][2] = (T)1.f - (twoXX + twoYY);
}
* /

    template <class T>
    void TQuaternion<T>::toAxisAngle(T &radians, T &out_x, T &out_y, T &out_z) {
  // The quaternion representing the rotation is
  //	 q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

  float sqrLength = x * x + y * y + z * z;

  if(sqrLength > 0.0f) {
    radians = 2.0f * acosf(w);
    float invLength = 1.0f / sqrtf(sqrLength);
    out_x = x * invLength;
    out_y = y * invLength;
    out_z = z * invLength;
  } else {
    // Angle is 0 (mod 2*pi), so any axis will do.
    radians = 0.0f;
    out_x = 1.0f;
    out_y = 0.0f;
    out_z = 0.0f;
  }
}

template <class T> void TQuaternion<T>::toAxisAngle(T &radians, TVec3<T> &vec) {
  toAxisAngle(radians, vec.x, vec.y, vec.z);
}

template <class T>
TQuaternion<T> TQuaternion<T>::SLERP(const TQuaternion<T> &a,
                                     const TQuaternion<T> &b, T factor) {
  BASE_ASSERT(Math::Clamp<float>(factor, 0.f, 1.f) == factor,
              "factor range out of bounds <0,1>");
  T w1, w2;

  T directionScale = 1.f;

  float omega, cosom, sinom;

  // calc cosine
  cosom = a.dot(b);

  // adjust signs (if necessary) - interpolation direction
  if(cosom < 0.0f) {
    cosom = -cosom;
    w2 = -1.0f;
  } else
    w2 = 1.0f;

  if((1.0 - cosom) > 0.001f) {
    // standard case (slerp)
    omega = Math::Acos<T>(cosom);
    sinom = Math::Sin<T>(omega);
    w1 = Math::Sin<T>((1.0f - factor) * omega) / sinom;
    w2 *= Math::Sin<T>(factor * omega) / sinom;
  } else {
    // "from" and "to" quaternions are very close
    //	... so we can do a linear interpolation
    w1 = 1.0f - factor;
    w2 *= factor;
  }

  return TQuaternion<T>(a * w1 + b * w2);

  /* this calculation did't include rotation direction
   T cosTheta = a.dot(b);
   T theta = Math::acos<T>(cosTheta);
   T sinTheta = Math::sin<T>(theta);

   if(sinTheta > 0.001f )
   {
   w1 = Math::sin<T>( (1.0f - factor) * theta ) / sinTheta;
   w2 = Math::sin<T>( factor * theta) / sinTheta;
   }
   else
   {
   // CQuat a ~= CQuat b
   w1 = 1.0f - factor;
   w2 = factor;
   }

   return TQuaternion<T>(a * w1 + b * w2);
   */
}

template <class T>
TQuaternion<T> TQuaternion<T>::NLERP(const TQuaternion<T> &a,
                                     const TQuaternion<T> &b, T factor) {
  BASE_ASSERT(Math::Clamp<float>(factor, 0.f, 1.f) == factor,
              "factor range out of bounds <0,1>");
  float w1 = 1.f - factor;

  TQuaternion<T> result; // = TQuaternion<T>(a * w1 + b * factor);

  // determine interpolation direction.
  float inner = a.dot(b);

  if(inner < 0.f)
    result = a * w1 - b * factor;
  // result = a - (b+a)*factor; maybe faster?
  else
    result = a * w1 + b * factor;
  // result = a + (b-a)*factor; maybe faster?

  result.normalize();
  return result;
}

template <class T>
void TQuaternion<T>::fromAxisAngle(T radians, T in_x, T in_y, T in_z) {
  // The quaternion representing the rotation is
  //	 q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

  T mod = Math::Sin<T>(radians / 2);
  T len = Math::Sqrt<T>(in_x * in_x + in_y * in_y + in_z * in_z);
  x = mod * in_x / len;
  y = mod * in_y / len;
  z = mod * in_z / len;
  w = Math::Cos<T>(radians / 2);
}

template <class T>
void TQuaternion<T>::fromAxisAngle(T radians, const TVec3<T> &vec) {
  fromAxisAngle(radians, vec.x, vec.y, vec.z);
}

template <class T>
TQuaternion<T> TQuaternion<T>::FromAxisAngle(T radians, T in_x, T in_y,
                                             T in_z) {
  // assert:	axis[] is unit length
  //
  // The quaternion representing the rotation is
  //	 q = cos(A/2)+sin(A/2)*(x*i+y*j+z*k)

  T halfAngle = 0.5f * radians;
  T sn = Math::Sin(halfAngle);

  return TQuaternion<T>(sn * in_x, sn * in_y, sn * in_z, Math::Cos(halfAngle));
}

template <class T>
TQuaternion<T> TQuaternion<T>::FromAxisAngle(T radians, const TVec3<T> &vec) {
  return FromAxisAngle(radians, vec.x, vec.y, vec.z);
}

template <class T> TQuaternion<T> TQuaternion<T>::FromAxisRotationX(T angle) {
  angle *= 0.5f;
  return TQuaternion<T>(Math::Sin(angle), 0.f, 0.f, Math::Cos(angle));
}

template <class T> TQuaternion<T> TQuaternion<T>::FromAxisRotationY(T angle) {
  angle *= 0.5f;
  return TQuaternion<T>(0.f, Math::Sin(angle), 0.f, Math::Cos(angle));
}

template <class T> TQuaternion<T> TQuaternion<T>::FromAxisRotationZ(T angle) {
  angle *= 0.5f;
  return TQuaternion<T>(0.f, 0.f, Math::Sin(angle), Math::Cos(angle));
}

template <class T>
inline TVec3<T> TQuaternion<T>::operator*(const TVec3<T> &vec) const {
  // V' = Q * V * Q^-1

  TQuaternion<T> res =
      *this * TQuaternion<T>(vec.x, vec.y, vec.z, 0.f) * this->conjugate();

  return TVec3<T>(res.x, res.y, res.z);
}

}; // namespace Base
