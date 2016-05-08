/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"

namespace Link {
namespace Gate {

typedef u32 UserId;
static const UserId kInvalidUserId = (UserId)-1;

// Maximum length of a username.
static const u32 kUsernameMax = 128;

// Default number of user logins per connection.
static const u32 kMaxUsersPerConnection = 4;

// Version of the communication protocol.
static const char* kProtocolVersion = "0.1";

} // namespace Gate
} // namespace Link
