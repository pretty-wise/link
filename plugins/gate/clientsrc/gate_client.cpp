/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin/gate/gate_client.h"
#include "base/core/assert.h"
#include "base/network/socket.h"
#include "common/message_stream.h"

#include "common/protobuf_stream.h"
#include "protocol/gate.pb.h"

namespace Link {
namespace Gate {

static const Base::LogChannel kGateClientLog("gate_client");
static const streamsize kReadBufferSize = 1024;

enum state_t { kDisconnected, kConnecting, kConnected };

struct user_t {
  char username[kUsernameMax];
  enum { kLoggingIn, kLoggedIn, kLoggingOut, kInvalid } state;
  UserId id;
  user_t() {
    memset(username, 0, kUsernameMax);
    state = kInvalid;
    id = kInvalidUserId;
  }
  bool IsLoggedIn() const { return state == kLoggedIn; }
};

struct context_t {
  callbacks_t callbacks;
  state_t state;
  Base::Socket::Handle socket;
  Link::MessageOutStream *out_stream;
  Link::MessageInStream *in_stream;
  user_t *user;
  u32 user_count;
  s8 *m_read_buffer;
  streamsize m_read_buffer_size;
  void *udata;
};

namespace {
bool IsValid(const callbacks_t &cbs) {
  if(cbs.on_connected == nullptr || cbs.on_disconnected == nullptr ||
     cbs.on_login == nullptr || cbs.on_logout == nullptr) {
    return false;
  }
  return true;
}
bool IsValid(const config_t &config) {
  if(config.timeout > kMaxConnectTimeoutMs) {
    return false;
  }
  return true;
}
void InternalLogout(struct context_t *ctx, u32 user_index, int reason) {
  ctx->user[user_index].state = user_t::kLoggingOut;
  ctx->callbacks.on_logout(ctx, &ctx->user[user_index], ctx->udata);
}
void InternalDisconnect(struct context_t *ctx, int reason) {
  if(ctx->state == kDisconnected) {
    return; // already disconnected
  }

  for(u32 user_index = 0; user_index < ctx->user_count; ++user_index) {
    if(ctx->user[user_index].IsLoggedIn()) {
      InternalLogout(ctx, user_index, reason);
    }
  }

  ctx->callbacks.on_disconnected(ctx, reason, ctx->udata);
  Base::Socket::Close(ctx->socket);

  ctx->state = kDisconnected;
  BASE_INFO(kGateClientLog, "disconnected. reason: %s",
            Reason::ToString(reason));
}
bool OnMessage(struct context_t *ctx, void *data, streamsize nbytes) {
  ProtobufStream stream(data, nbytes);
  gate::ServerMessage msg;
  if(!msg.ParseFromIstream(&stream)) {
    BASE_ERROR(kGateClientLog, "protocol error");
    InternalDisconnect(ctx, Reason::kStreamParseError);
    return false;
  }
  gate::ServerMessage::Type type = msg.type();
  const char *protoc = msg.protocol_version().c_str();
  BASE_INFO(kGateClientLog, "message type: %d, protocol version: %s", type,
            protoc);
  if(strcmp(protoc, kProtocolVersion) != 0) {
    BASE_ERROR(kGateClientLog, "protocol mismatch");
    InternalDisconnect(ctx, Reason::kProtocolMismatch);
    return false;
  }

  switch(type) {
  case gate::ServerMessage::kLoggedIn: {
    u32 index = msg.logged_in().index();
    UserId user_id = msg.logged_in().user_id();
    BASE_INFO(kGateClientLog, "message logged in on local index %d with id %d",
              index, user_id);
    // todo(kstasik): race condition (index can be invalid)
    user_t *user = &ctx->user[index];
    user->state = user_t::kLoggedIn;
    user->id = user_id;
    ctx->callbacks.on_login(ctx, user, ctx->udata);
    break;
  }
  case gate::ServerMessage::kLoggedOut: {
    UserId id = msg.logged_out().user_id();
    BASE_INFO(kGateClientLog, "message logged out id %d", id);
    // todo(kstasik): race condition?
    user_t *user = nullptr;
    for(u32 i = 0; i < ctx->user_count; ++i) {
      if(ctx->user[i].id == id) {
        user = &ctx->user[i];
        break;
      }
    }
    if(user) {
      user->state = user_t::kInvalid;
      ctx->callbacks.on_logout(ctx, user, ctx->udata);
    } else {
      BASE_WARN(kGateClientLog, "user not found %d", id);
    }
    break;
  }
  default:
    break;
  }
  return true;
}
} // unnamed namespace

namespace Reason {
const char *ToString(int reason) {
  switch(reason) {
  case kShutdown:
    return "kShutdown";
  case kDisconnect:
    return "kDisconnect";
  case kReadError:
    return "kReadError";
  case kEndOfFile:
    return "kEndOfFile";
  case kWriteError:
    return "kWriteError";
  case kLogoutWriteError:
    return "kLogoutWriteError";
  case kStreamParseError:
    return "kStreamParseError";
  case kProtocolMismatch:
    return "kProtocolMismatch";
  }
  return "kUnknown";
}
} // namespace Reason

struct context_t *Create(callbacks_t cbs, const config_t &config, void *udata) {
  BASE_DEBUG(kGateClientLog, "create context");
  if(!IsValid(cbs)) {
    BASE_ERROR(kGateClientLog, "no callbacks provided");
    return nullptr;
  }
  if(!IsValid(config)) {
    BASE_ERROR(kGateClientLog, "config invalid");
    return nullptr;
  }
  context_t *context = new context_t;
  context->callbacks = cbs;
  context->state = kDisconnected;
  context->socket = Base::Socket::Tcp::Open();
  context->udata = udata;
  context->user = new user_t[kMaxUsersPerConnection];
  context->user_count = 0;
  context->m_read_buffer_size = kReadBufferSize;
  context->m_read_buffer = new s8[context->m_read_buffer_size];

  if(context->socket == Base::Socket::InvalidHandle) {
    delete context;
    context = nullptr;
    BASE_ERROR(kGateClientLog, "problem creating socket");
    return nullptr;
  }
  const streamsize kStreamBytes = 1024;
  context->out_stream = new Link::MessageOutStream(kStreamBytes);
  context->in_stream = new Link::MessageInStream(kStreamBytes);

  BASE_INFO(kGateClientLog, "context created");
  return context;
}

void Destroy(struct context_t *ctx) {
  BASE_DEBUG(kGateClientLog, "destroying context");
  if(ctx->state != kDisconnected) {
    InternalDisconnect(ctx, Reason::kShutdown);
  }
  delete ctx->in_stream;
  delete ctx->out_stream;
  delete[] ctx->user;
  delete[] ctx->m_read_buffer;
  delete ctx;
  BASE_INFO(kGateClientLog, "context destroyed");
}

static bool InternalSend(struct context_t *ctx, const void *data,
                         streamsize nbytes) {
  BASE_ASSERT(ctx->state == kConnected);
  return ctx->out_stream->Write(data, nbytes);
}

bool Connect(struct context_t *ctx, const Base::Url &address) {
  BASE_INFO(kGateClientLog, "connecting to %d.%d.%d.%d:%d",
            PRINTF_URL(address));
  if(!ctx) {
    BASE_ERROR(kGateClientLog, "invalid context");
    return false;
  }

  if(ctx->state != kDisconnected) {
    BASE_ERROR(kGateClientLog, "already connected or connecting");
    return false;
  }

  ctx->socket = Base::Socket::Tcp::Open();
  if(ctx->socket == Base::Socket::InvalidHandle) {
    BASE_ERROR(kGateClientLog, "problem opening socket");
    return false;
  }
  ctx->state = kConnecting;

  // todo(kstasik): make it async
  int res = Base::Socket::Tcp::Connect(ctx->socket, address);
  if(res == Base::Socket::Tcp::kFailed) {
    BASE_ERROR(kGateClientLog, "problem connecting");
    return false;
  }

  if(res == Base::Socket::Tcp::kConnected) {
    BASE_INFO(kGateClientLog, "connected");
    ctx->state = kConnected;
    ctx->callbacks.on_connected(ctx, ctx->udata);
  }
  return true;
}

void Disconnect(struct context_t *ctx) {
  InternalDisconnect(ctx, Reason::kDisconnect);
}

bool IsConnected(struct context_t *ctx) { return ctx->state == kConnected; }

int Update(struct context_t *ctx) {
  if(!ctx) {
    return -1;
  }

  if(ctx->state == kConnecting) {
    int result = Base::Socket::Tcp::IsConnected(ctx->socket);
    if(result == Base::Socket::Tcp::kFailed) {
      BASE_INFO(kGateClientLog, "failed to connect");
      ctx->state = kDisconnected;
      ctx->callbacks.on_disconnected(ctx, Reason::kTimeout, ctx->udata);
    } else if(result == Base::Socket::Tcp::kConnected) {
      BASE_INFO(kGateClientLog, "connected");
      ctx->state = kConnected;
      ctx->callbacks.on_connected(ctx, ctx->udata);
    }
  }

  if(ctx->state != kConnected) {
    return 0;
  }

  // process incoming messages.
  MessageInStream::ProcessResult res = ctx->in_stream->Process(ctx->socket);
  if(res == MessageInStream::RS_ERROR) {
    BASE_ERROR(kGateClientLog, "error processing incoming data");
    InternalDisconnect(ctx, Reason::kReadError);
    return -1;
  } else if(res == MessageInStream::RS_EOF) {
    BASE_INFO(kGateClientLog, "end of connection");
    InternalDisconnect(ctx, Reason::kEndOfFile);
    return 0;
  }

  while(!ctx->in_stream->Empty()) {
    streamsize read;
    Result res = ctx->in_stream->Read(ctx->m_read_buffer,
                                      ctx->m_read_buffer_size, &read);
    if(res == RS_BUFFER_TOO_SMALL) {
      InternalDisconnect(
          ctx,
          Reason::kReadError); // todo(kstasik): detailed reason (duplicated)
      return -1;
    }
    if(!OnMessage(ctx, ctx->m_read_buffer, read)) {
      return -1;
    }
  }

  // flush outgoing messages.
  if(!ctx->out_stream->Flush(ctx->socket)) {
    InternalDisconnect(ctx, Reason::kWriteError);
    return -1;
  }

  return 0;
}

struct user_t *Login(struct context_t *ctx, const char *name) {
  if(!IsConnected(ctx)) {
    BASE_ERROR(kGateClientLog, "not connected");
    return nullptr;
  }
  if(strlen(name) >= kUsernameMax) {
    BASE_ERROR(kGateClientLog, "username too long %d", strlen(name));
    return nullptr;
  }
  if(ctx->user_count >= kMaxUsersPerConnection) {
    BASE_ERROR(kGateClientLog, "user limit reached");
    return nullptr;
  }

  u32 index = ctx->user_count;
  user_t &user = ctx->user[index];
  ++ctx->user_count;

  user.state = user_t::kInvalid;
  strcpy(user.username, name);

  gate::ClientMessage msg;
  msg.set_type(gate::ClientMessage::kLogin);
  msg.set_protocol_version(kProtocolVersion);
  gate::Login &login = *msg.mutable_login();
  login.set_username(name);
  login.set_index(index);
  std::string serialized;
  msg.SerializeToString(&serialized);

  if(!InternalSend(ctx, static_cast<const void *>(serialized.data()),
                   serialized.length())) {
    --ctx->user_count;
    memset(user.username, 0, kUsernameMax);
    return nullptr;
  }
  BASE_DEBUG(kGateClientLog, "logging in %p as %d", &user, index);
  return &user;
}

bool Logout(struct context_t *ctx, struct user_t *user) {
  BASE_DEBUG(kGateClientLog, "logging out %p", user);
  if(!IsConnected(ctx)) {
    BASE_ERROR(kGateClientLog, "not connected");
    return false;
  }

  struct user_t *user_data = nullptr;
  u32 user_index = 0;
  {
    for(user_index = 0; user_index < ctx->user_count; ++user_index) {
      if(&ctx->user[user_index] == user) {
        user_data = &ctx->user[user_index];
        break;
      }
    }
  }

  if(user_data == nullptr) {
    BASE_ERROR(kGateClientLog, "cannot find user");
    return false;
  }

  if(!user_data->IsLoggedIn()) {
    BASE_ERROR(kGateClientLog, "not logged in");
    return false;
  }

  gate::ClientMessage msg;
  msg.set_type(gate::ClientMessage::kLogout);
  msg.set_protocol_version(
      kProtocolVersion); // todo(kstasik): refactor protocol version.
  gate::Logout &logout = *msg.mutable_logout();
  logout.set_user_id(user_data->id);
  std::string serialized;
  msg.SerializeToString(&serialized);
  if(!InternalSend(ctx, static_cast<const void *>(serialized.data()),
                   serialized.length())) {
    BASE_ERROR(kGateClientLog, "problem sending logout message");
    InternalDisconnect(ctx, Reason::kLogoutWriteError);
    return false;
  }
  BASE_INFO(kGateClientLog, "sending logout with index %d", user);
  return true;
}

} // namespace Gate
} // namespace Link
