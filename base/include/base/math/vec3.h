/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/macro.h"
#include "base/math/math.h"

#include <string> // memcpy

namespace Base {

template <class T> class TVec3 {
public:
  static const TVec3 Zero;
  static const TVec3 Identity;

  static const TVec3 Unit_Y;
  static const TVec3 Unit_Z;
  static const TVec3 Unit_X;

  inline TVec3();
  inline TVec3(const TVec3 &);
  inline TVec3(T x, T y, T z);
  inline TVec3(const T *array);

  inline TVec3 &operator+=(const TVec3 &);
  inline TVec3 &operator-=(const TVec3 &);
  inline TVec3 &operator*=(T);
  inline TVec3 &operator/=(T);

  inline TVec3 operator+() const;
  inline TVec3 operator-() const;

  inline TVec3 operator+(const TVec3 &) const;
  inline TVec3 operator-(const TVec3 &) const;

  friend TVec3 operator*(T, const TVec3 &);

  inline TVec3 operator*(T) const;
  inline TVec3 operator/(T) const;

  inline void set(T x, T y, T z);

  inline bool operator==(const TVec3 &) const;
  inline bool operator!=(const TVec3 &) const;

  inline T length() const;
  void normalize();

  /**
   * @param vector u
   * @return vector orthogonal to u and *this
   */
  inline TVec3 cross(const TVec3 &u) const;

  /**
   * the dot product:
   * u * v = 0, vectors are orthogonal (perpendicular ;)
   * u * v > 0, vectors make acute angle (< 90)
   * u * v < 0, vectors make obtuse angle(> 90)
   */
  inline T dot(const TVec3 &v) const;

  /**
   * linear interpolation of two vectors.
   * @return intermediate vector between a and b. factor ranges <0,1>
   */
  static inline TVec3 LERP(const TVec3 &a, const TVec3 &b, float factor);

public:
  union {
    struct {
      T x, y, z;
    };
    T coords[3];
  };
};

typedef TVec3<float> Vec3;

template <class T> inline TVec3<T>::TVec3() {
  memset(&coords, 0x00, 3 * sizeof(T));
}

template <class T> inline TVec3<T>::TVec3(const TVec3<T> &v) {
  memcpy(&coords, &v.coords, 3 * sizeof(T));
}

template <class T>
inline TVec3<T>::TVec3(T _x, T _y, T _z)
    : x(_x), y(_y), z(_z) {}

template <class T>
inline TVec3<T>::TVec3(const T *array)
    : x(array[0]), y(array[1]), z(array[2]) {}

template <class T> inline TVec3<T> &TVec3<T>::operator+=(const TVec3 &v) {
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}

template <class T> inline TVec3<T> &TVec3<T>::operator-=(const TVec3 &v) {
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}

template <class T> inline TVec3<T> &TVec3<T>::operator*=(T a) {
  x *= a;
  y *= a;
  z *= a;
  return *this;
}

template <class T> inline TVec3<T> &TVec3<T>::operator/=(T a) {
  x /= a;
  y /= a;
  z /= a;
  return *this;
}

template <class T> inline TVec3<T> TVec3<T>::operator+() const { return *this; }

template <class T> inline TVec3<T> TVec3<T>::operator-() const {
  return TVec3(-x, -y, -z);
}

template <class T> inline TVec3<T> TVec3<T>::operator+(const TVec3 &v) const {
  return TVec3<T>(x + v.x, y + v.y, z + v.z);
}

template <class T> inline TVec3<T> TVec3<T>::operator-(const TVec3 &v) const {
  return TVec3<T>(x - v.x, y - v.y, z - v.z);
}

template <class T> inline TVec3<T> TVec3<T>::operator*(T a) const {
  return TVec3<T>(x * a, y * a, z * a);
}

template <class T> inline TVec3<T> TVec3<T>::operator/(T a) const {
  T inverse = 1 / a; // dzielenie wolniejsze.
  return TVec3<T>(x * inverse, y * inverse, z * inverse);
}

template <class T> inline void TVec3<T>::set(T _x, T _y, T _z) {
  x = _x;
  y = _y;
  z = _z;
}

template <class T> inline bool TVec3<T>::operator==(const TVec3 &v) const {
  return x == v.x && y == v.y && z == v.z;
}

template <class T> inline bool TVec3<T>::operator!=(const TVec3 &v) const {
  return x != v.x || y != v.y || z != v.z;
}

template <class T> inline T TVec3<T>::length() const {
  return Math::Sqrt(x * x + y * y + z * z);
}

template <class T> void TVec3<T>::normalize() {
  BASE_ASSERT(x || y || z);
  T inv_len = 1 / length();
  x *= inv_len;
  y *= inv_len;
  z *= inv_len;
}

template <class T> inline T TVec3<T>::dot(const TVec3 &v) const {
  return x * v.x + y * v.y + z * v.z;
}

template <class T> inline TVec3<T> TVec3<T>::cross(const TVec3 &v) const {
  return TVec3<T>(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

template <class T>
inline TVec3<T> TVec3<T>::LERP(const TVec3 &a, const TVec3 &b, float factor) {
  BASE_ASSERT(Math::Clamp<float>(factor, 0.f, 1.f) == factor,
              "factor range out of bounds <0,1>");

  // LERP(A,B,f) = (1-f)A + fB = [ (1-f)Ax + fBx, (1-f)Ay + fBy, (1-f)Az + fBz ]

  return TVec3<T>((1 - factor) * a.x + factor * b.x,
                  (1 - factor) * a.y + factor * b.y,
                  (1 - factor) * a.z + factor * b.z);
}

}; // namespace Base
