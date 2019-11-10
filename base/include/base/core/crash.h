/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

namespace Base {

// Spawns a daemon process that listens for crash data and processes it.
bool RegisterCrashHandler();

void Crash();

} // namespace Base
