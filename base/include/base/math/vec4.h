/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include <iostream>

namespace Base {

template <class T> class TVec4 {
public:
  inline TVec4();
  inline TVec4(const TVec4 &);
  inline TVec4(T x, T y, T z, T w);
  inline TVec4(const T *array);

  inline TVec4 &operator+=(const TVec4 &);
  inline TVec4 &operator-=(const TVec4 &);
  inline TVec4 &operator*=(T);
  inline TVec4 &operator/=(T);

  inline TVec4 operator+();
  inline TVec4 operator-();

  inline TVec4 operator+(const TVec4 &) const;
  inline TVec4 operator-(const TVec4 &) const;

  friend TVec4 operator*(T, const TVec4 &);

  inline TVec4 operator*(T) const;
  inline TVec4 operator/(T) const;

  inline void set(T x, T y, T z, T w);

  inline bool operator==(const TVec4 &) const;
  inline bool operator!=(const TVec4 &) const;

  inline T length() const;
  void normalize();

public:
  union {
    struct {
      T x, y, z, w;
    };
    T coords[4];
  };
};

template <class T> inline TVec4<T>::TVec4() {}

template <class T>
inline TVec4<T>::TVec4(const TVec4 &v)
    : x(v.x), y(v.y), z(v.z), w(v.w) {}

template <class T>
inline TVec4<T>::TVec4(T _x, T _y, T _z, T _w)
    : x(_x), y(_y), z(_z), w(_w) {}

template <class T>
inline TVec4<T>::TVec4(const T *array)
    : x(array[0]), y(array[1]), z(array[2]), w(array[3]) {}

template <class T> inline TVec4<T> &TVec4<T>::operator+=(const TVec4 &v) {
  x += v.x;
  y += v.y;
  z += v.z;
  w += v.w;
  return *this;
}

template <class T> inline TVec4<T> &TVec4<T>::operator-=(const TVec4 &v) {
  x -= v.x;
  y -= v.y;
  z -= v.z;
  w -= v.w;
  return *this;
}

template <class T> inline TVec4<T> &TVec4<T>::operator*=(T a) {
  x *= a;
  y *= a;
  z *= a;
  w *= a;
  return *this;
}

template <class T> inline TVec4<T> &TVec4<T>::operator/=(T a) {
  x /= a;
  y /= a;
  z /= a;
  w /= a;
  return *this;
}

template <class T> inline TVec4<T> TVec4<T>::operator+() { return *this; }

template <class T> inline TVec4<T> TVec4<T>::operator-() {
  return TVec4(-x, -y, -z, -w);
}

template <class T> inline TVec4<T> TVec4<T>::operator+(const TVec4 &v) const {
  return TVec4<T>(x + v.x, y + v.y, z + v.z, w + v.w);
}

template <class T> inline TVec4<T> TVec4<T>::operator-(const TVec4 &v) const {
  return TVec4<T>(x - v.x, y - v.y, z - v.z, w - v.w);
}

template <class T> inline TVec4<T> TVec4<T>::operator*(T a) const {
  return TVec4<T>(x * a, y * a, z * a, w * a);
}

template <class T> inline TVec4<T> TVec4<T>::operator/(T a) const {
  T inverse = 1 / a; // dzielenie wolniejsze.
  return TVec4<T>(x * inverse, y * inverse, z * inverse, w * inverse);
}

template <class T> inline void TVec4<T>::set(T _x, T _y, T _z, T _w) {
  x = _x;
  y = _y;
  z = _z;
  w = _w;
}

template <class T> inline bool TVec4<T>::operator==(const TVec4 &v) const {
  return x == v.x && y == v.y && z == v.z && w == v.w;
}

template <class T> inline bool TVec4<T>::operator!=(const TVec4 &v) const {
  return x != v.x || y != v.y || z != v.z || w != v.w;
}

template <class T> inline T TVec4<T>::length() const {
  return Math::Sqrt(x * x + y * y + z * z + w * w);
}

template <class T> void TVec4<T>::normalize() {
  T inv_len = 1 / length();
  x *= inv_len;
  y *= inv_len;
  z *= inv_len;
  w *= inv_len;
}

template <class T> std::ostream &operator<<(std::ostream &os, TVec4<T> &v) {
  os << "Vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
  return os;
}

typedef TVec4<float> Vec4;

}; // namespace Base
