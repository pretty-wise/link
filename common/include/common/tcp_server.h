/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/types.h"
#include "base/network/socket.h"
#include "base/network/epoll.h"
#include "base/threading/mutex.h"

#define PRINTF_URL(url)                                                        \
  url.GetAddress().GetA(), url.GetAddress().GetB(), url.GetAddress().GetC(),   \
      url.GetAddress().GetD(), url.GetPort()

namespace Link {

class TCPServer {
public:
  enum class CloseReason {
    kShutdown,      // the process has shutdown.
    kEnded,         // ended gracefully by other peer.
    kReadFailure,   // reading message from socket failed.
    kWriteFailure,  // write on socket failed
    kProtocolError, // failed to read incoming data.
    kRESTfulAPI     // rest api triggered.
  };
  typedef u32 Handle;
  static const Handle kInvalidHandle = (Handle)-1;
  typedef void (*OnConnected)(Handle connection, const Base::Url &url,
                              void *udata);
  typedef void (*OnDisconnected)(Handle connection, CloseReason reason,
                                 const Base::Url &url, void *udata);
  typedef void (*OnMessage)(Handle from, void *buffer, u32 nbytes, void *udata);

  static const char *ToString(CloseReason r);

  struct Callbacks {
    OnConnected on_connected;
    OnDisconnected on_disconnected;
    OnMessage on_message;
  };

private:
  u32 m_capacity;
  u32 m_next;
  struct Connection *m_data;
  Base::Socket::Handle m_listen_socket; //! TCP conneciton listen socket.
  Base::EpollHandle m_epoll;            //! TCP connection event handling.
  Callbacks m_cbs;
  void *m_context;
  s8 *m_buffer; // intermediate message buffer.
public:
  /// @param max_num_connections Maximum opened connections.
  TCPServer(u32 max_num_connections, Callbacks cbs, void *udata);
  ~TCPServer();

  /// Opens a listen socket on a free port.
  /// @param port Port the manager listens on.
  /// @return True if socket opened successfully, false otherwise.
  bool CreateListenSocket(u16 &port);

  bool Send(Handle handle, const void *data, streamsize nbytes);

  void Close(Handle handle, CloseReason reason);
  void CloseAll(CloseReason reason);

  void HandleIO(time_ms timeout);

private:
  void AcceptConnection();
  Handle GenerateHandle();
};

} // namespace Link2
