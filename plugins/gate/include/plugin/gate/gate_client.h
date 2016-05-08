/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/network/url.h"
#include "plugin/gate/gate_defs.h"

namespace Link {
namespace Gate {

namespace Reason {
	const int kShutdown         = 0x80000001; // gate shutdown
	const int kDisconnect       = 0x80000002; // client called disconnec
	const int kReadError        = 0x80000003; // error reading socket
	const int kEndOfFile        = 0x80000004; // connection closed
	const int kWriteError       = 0x80000005;
	const int kLogoutWriteError = 0x80000006; // cannot send logout message.
	const int kStreamParseError = 0x80000007; // failed reading server message.
	const int kProtocolMismatch = 0x80000008; // client-server protocol mismatch
	const int kTimeout 					= 0x80000009; // connection timed out.

	const char* ToString(int reason);
} // namespace Reason

typedef void (*OnConnected)(struct context_t* ctx, void* udata);
typedef void (*OnDisconnected)(struct context_t* ctx, int reason, void* udata);
typedef void (*OnLogin)(struct context_t* ctx, struct user_t* user, void* udata);
typedef void (*OnLogout)(struct context_t* ctx, struct user_t* user, void* udata);

struct callbacks_t { 
	OnConnected on_connected;
	OnDisconnected on_disconnected;
	OnLogin on_login;
	OnLogout on_logout;
};

static const u16 kDefaultConnectTimeoutMs = 5*1000;
static const u16 kMaxConnectTimeoutMs = 30*1000;

struct config_t {
	u16 timeout;
};

// Create a client-side serivice.
struct context_t* Create(callbacks_t cbs, const config_t& config, void* udata);

// Destroy the service.
void Destroy(struct context_t* ctx);

// Connect to a remote gate server.
bool Connect(struct context_t* ctx, const Base::Url& address);

bool IsConnected(struct context_t* ctx);

// Disconnect from a remote gate server.
void Disconnect(struct context_t* ctx);

// Login to the gate server after connecting. Can log in multiple users.
struct user_t* Login(struct context_t* ctx, const char* name);

// 
bool Logout(struct context_t* ctx, struct user_t* user);

// Sends and receives data on socket.
int Update(struct context_t* ctx);

} // namespace Gate 
} // namespace Link
