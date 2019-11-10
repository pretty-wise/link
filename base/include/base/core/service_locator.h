/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/macro.h"

template<class T>
class Locator {
public:
		static void Expose(T* service) { s_Service = service; }
protected:
		static T& Service() { BASE_ASSERT(s_Service); return *s_Service; }
private:
		static T* s_Service;
};

template < class T >
T * Locator<T>::s_Service = nullptr;
