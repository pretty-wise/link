/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "engine/core/Types.h"
#include "engine/math/Vec3.h"

namespace Base {

template <class T> class TRay {
public:
  TRay(const TVec3<T> &origin, const TVec3<T> &direction);

  inline void SetOrigin(const TVec3<T> &origin);
  inline const TVec3<T> &GetOrigin() const;

  inline void SetDirection(const TVec3<T> &dir);
  inline const TVec3<T> &GetDirection() const;

  inline TVec3<T> GetPoint(T distance) const;

  //	inline bool						intersects(const AABoundingBox& box,
  //T& distance) const;

private:
  TVec3<T> m_origin;
  TVec3<T> m_direction;
};

typedef TRay<float> Ray;

template <class T>
TRay<T>::TRay(const TVec3<T> &origin, const TVec3<T> &direction)
    : m_origin(origin), m_direction(direction) {}

template <class T> inline void TRay<T>::SetOrigin(const TVec3<T> &origin) {
  m_origin = origin;
}

template <class T> inline const TVec3<T> &TRay<T>::GetOrigin() const {
  return m_origin;
}

template <class T> inline void TRay<T>::SetDirection(const TVec3<T> &dir) {
  m_direction = dir;
}

template <class T> inline const TVec3<T> &TRay<T>::GetDirection() const {
  return m_direction;
}

template <class T> inline TVec3<T> TRay<T>::GetPoint(T distance) const {
  return m_origin + m_direction * distance;
}

template <class T>
inline bool TRay<T>::intersects(const AABoundingBox &box, T &distance) const {
  // check if not inside the box.
  if(box.contains(m_origin)) {
    distance = 0.f;
    return true;
  }

  float t;
  distance = 0.f;
  Vec3 hitpoint;
  bool hit = false;

  // Check each face in turn, only check closest 3
  // box.getMinimum() x
  if(m_origin.x <= box.getMinimum().x && m_direction.x > 0) {
    t = (box.getMinimum().x - m_origin.x) / m_direction.x;
    if(t >= 0) {
      // Substitute t back into ray and check bounds and dist
      hitpoint = m_origin + m_direction * t;
      if(hitpoint.y >= box.getMinimum().y && hitpoint.y <= box.getMaximum().y &&
         hitpoint.z >= box.getMinimum().z && hitpoint.z <= box.getMaximum().z &&
         (!hit || t < distance)) {
        hit = true;
        distance = t;
      }
    }
  }
  // box.getMaximum() x
  if(m_origin.x >= box.getMaximum().x && m_direction.x < 0) {
    t = (box.getMaximum().x - m_origin.x) / m_direction.x;
    if(t >= 0) {
      // Substitute t back into ray and check bounds and dist
      hitpoint = m_origin + m_direction * t;
      if(hitpoint.y >= box.getMinimum().y && hitpoint.y <= box.getMaximum().y &&
         hitpoint.z >= box.getMinimum().z && hitpoint.z <= box.getMaximum().z &&
         (!hit || t < distance)) {
        hit = true;
        distance = t;
      }
    }
  }
  // box.getMinimum() y
  if(m_origin.y <= box.getMinimum().y && m_direction.y > 0) {
    t = (box.getMinimum().y - m_origin.y) / m_direction.y;
    if(t >= 0) {
      // Substitute t back into ray and check bounds and dist
      hitpoint = m_origin + m_direction * t;
      if(hitpoint.x >= box.getMinimum().x && hitpoint.x <= box.getMaximum().x &&
         hitpoint.z >= box.getMinimum().z && hitpoint.z <= box.getMaximum().z &&
         (!hit || t < distance)) {
        hit = true;
        distance = t;
      }
    }
  }
  // box.getMaximum() y
  if(m_origin.y >= box.getMaximum().y && m_direction.y < 0) {
    t = (box.getMaximum().y - m_origin.y) / m_direction.y;
    if(t >= 0) {
      // Substitute t back into ray and check bounds and dist
      hitpoint = m_origin + m_direction * t;
      if(hitpoint.x >= box.getMinimum().x && hitpoint.x <= box.getMaximum().x &&
         hitpoint.z >= box.getMinimum().z && hitpoint.z <= box.getMaximum().z &&
         (!hit || t < distance)) {
        hit = true;
        distance = t;
      }
    }
  }
  // box.getMinimum() z
  if(m_origin.z <= box.getMinimum().z && m_direction.z > 0) {
    t = (box.getMinimum().z - m_origin.z) / m_direction.z;
    if(t >= 0) {
      // Substitute t back into ray and check bounds and dist
      hitpoint = m_origin + m_direction * t;
      if(hitpoint.x >= box.getMinimum().x && hitpoint.x <= box.getMaximum().x &&
         hitpoint.y >= box.getMinimum().y && hitpoint.y <= box.getMaximum().y &&
         (!hit || t < distance)) {
        hit = true;
        distance = t;
      }
    }
  }
  // box.getMaximum() z
  if(m_origin.z >= box.getMaximum().z && m_direction.z < 0) {
    t = (box.getMaximum().z - m_origin.z) / m_direction.z;
    if(t >= 0) {
      // Substitute t back into ray and check bounds and dist
      hitpoint = m_origin + m_direction * t;
      if(hitpoint.x >= box.getMinimum().x && hitpoint.x <= box.getMaximum().x &&
         hitpoint.y >= box.getMinimum().y && hitpoint.y <= box.getMaximum().y &&
         (!hit || t < distance)) {
        hit = true;
        distance = t;
      }
    }
  }

  return hit;
}
* /

} // namespace Base
