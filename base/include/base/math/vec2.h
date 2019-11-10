/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/math/math.h"

namespace Base {

template <class T> class TVec2 {
public:
  static const TVec2 Zero;
  static const TVec2 Identity;

  inline TVec2();
  inline TVec2(const TVec2 &);
  inline TVec2(T x, T y);
  inline TVec2(const float *array);

  inline TVec2 &operator+=(const TVec2 &);
  inline TVec2 &operator-=(const TVec2 &);
  inline TVec2 &operator*=(T);
  inline TVec2 &operator/=(T);

  inline TVec2 operator+();
  inline TVec2 operator-();

  inline TVec2 operator+(const TVec2 &) const;
  inline TVec2 operator-(const TVec2 &) const;

  friend TVec2 operator*(T, const TVec2 &);

  inline TVec2 operator*(T) const;
  inline TVec2 operator/(T) const;

  inline void Set(T x, T y);

  inline bool operator==(const TVec2 &) const;
  inline bool operator!=(const TVec2 &) const;

  inline T SquaredLength() const;
  inline T Length() const;

public:
  union {
    struct {
      T x, y;
    };
    T m_coords[2];
  };
};

template <class T> inline TVec2<T>::TVec2() {}

template <class T> inline TVec2<T>::TVec2(const TVec2 &v) : x(v.x), y(v.y) {}

template <class T> inline TVec2<T>::TVec2(T _x, T _y) : x(_x), y(_y) {}

template <class T>
inline TVec2<T>::TVec2(const float *array)
    : x(array[0]), y(array[1]) {}

template <class T> inline TVec2<T> &TVec2<T>::operator+=(const TVec2 &v) {
  x += v.x;
  y += v.y;
  return *this;
}

template <class T> inline TVec2<T> &TVec2<T>::operator-=(const TVec2 &v) {
  x -= v.x;
  y -= v.y;
  return *this;
}

template <class T> inline TVec2<T> &TVec2<T>::operator*=(T a) {
  x *= a;
  y *= a;
  return *this;
}

template <class T> inline TVec2<T> &TVec2<T>::operator/=(T a) {
  x /= a;
  y /= a;
  return *this;
}

template <class T> inline TVec2<T> TVec2<T>::operator+() { return *this; }

template <class T> inline TVec2<T> TVec2<T>::operator-() {
  return TVec2(-x, -y);
}

template <class T> inline TVec2<T> TVec2<T>::operator+(const TVec2 &v) const {
  return TVec2<T>(x + v.x, y + v.y);
}

template <class T> inline TVec2<T> TVec2<T>::operator-(const TVec2 &v) const {
  return TVec2<T>(x - v.x, y - v.y);
}

template <class T> inline TVec2<T> TVec2<T>::operator*(T a) const {
  return TVec2<T>(x * a, y * a);
}

template <class T> inline TVec2<T> TVec2<T>::operator/(T a) const {
  T inverse = 1 / a; // dzielenie wolniejsze.
  return TVec2<T>(x * inverse, y * inverse);
}

template <class T> inline void TVec2<T>::Set(T _x, T _y) {
  x = _x;
  y = _y;
}

template <class T> inline bool TVec2<T>::operator==(const TVec2 &v) const {
  return x == v.x && y == v.y;
}

template <class T> inline bool TVec2<T>::operator!=(const TVec2 &v) const {
  return x != v.x || y != v.y;
}

template <class T> inline T TVec2<T>::SquaredLength() const {
  return x * x + y * y;
}

template <class T> inline T TVec2<T>::Length() const {
  return Math::Sqrt(x * x + y * y);
}

typedef TVec2<float> Vec2;
typedef TVec2<int> Vec2i;

} // namespace Base
