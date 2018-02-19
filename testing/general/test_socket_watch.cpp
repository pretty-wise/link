/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gtest/gtest.h"

#include "base/core/macro.h"
#include "base/threading/thread.h"

#include "connection_manager.h"
#include "plugin_directory.h"
#include "socket_watch.h"

/* SocketWatch is not implemented

u16 listen_port;

int connect_thread(void *arg) {
  Base::Thread::Sleep(100);

  int error = 0;
  Base::Socket::Handle new_socket = Base::Socket::Tcp::Open(&error);
  if(new_socket == Base::Socket::InvalidHandle) {
    // todo(kstasik): make this func void - FAIL here
    return 0;
  }
  Base::Url url("127.0.0.1", listen_port);
  Base::Socket::Address address;
  bool address_created = Base::Socket::Address::CreateTCP(url, &address);
  if(!address_created) {
    return -1;
  }
  int res = Base::Socket::Tcp::Connect(new_socket, address);
  if(res < 0) {
    return -1;
  }
  return 0;
}

TEST(SocketWatch, ConnectAndAccept) {
  Link::SocketWatch watch(20);
  bool event_processed = false;

  int error = 0;
  auto socket = Base::Socket::Tcp::Open(&error);
  EXPECT_FALSE(socket == Base::Socket::InvalidHandle);

  // u16 listen_port = 0;
  bool listens = Base::Socket::Tcp::Listen(socket, &listen_port);
  EXPECT_TRUE(listens);

  bool registered = watch.Register(socket, 0);
  EXPECT_TRUE(registered);

  Base::Thread async;
  bool res = async.Initialize(&connect_thread, &socket);
  EXPECT_TRUE(res);

  auto process_func = [&](ConnectionHandle conn, bool read, bool write,
                          bool error) {
    // BASE_LOG("epoll event for %p (R:%s/W:%s/E:%s)\n", conn, read ? "Y" : "N",
    // write ? "Y" : "N", error ? "Y" : "N");
    event_processed = true;

    if(read) {
      Base::Socket::Handle new_connection = Base::Socket::InvalidHandle;
      Base::Socket::Address new_address;
      bool accepted =
          Base::Socket::Tcp::Accept(socket, &new_connection, &new_address);
      EXPECT_TRUE(accepted);
    }
  };

  watch.Wait(1000, process_func);
  EXPECT_TRUE(event_processed);

  async.Join();
}

class ConnectionTracker : public Link::ConnectionListener {
public:
  ConnectionTracker() : m_curr(Link::kError) {}
  virtual void OnStateChange(const struct Connection &connection,
                             Link::ConnectionState old_state,
                             Link::ConnectionState new_state) override {
    m_curr = new_state;
  }
  Link::ConnectionState Current() const { return m_curr; }

private:
  Link::ConnectionState m_curr;
};

TEST(SocketWatch, Test2) {
  Link::PluginDirectory plugin_directory;
  const u32 max_connections = 20;
  Link::ConnectionManager conn_manager(plugin_directory, max_connections);

  listen_port = 0;
  bool res = conn_manager.CreateListenSocket(listen_port);
  ASSERT_TRUE(res);

  PluginHandle plugin_a = (PluginHandle)0xAA;
  PluginHandle plugin_b = (PluginHandle)0xBB;

  PluginInfo info_a, info_b;
  Link::PluginDirectory::Initialize(info_a, "plugin_a", "1.00", "127.0.0.1",
                                    listen_port, 0);
  Link::PluginDirectory::Initialize(info_b, "plugin_b", "1.00", "127.0.0.1",
                                    listen_port, 0);

  ASSERT_TRUE(plugin_directory.Register(plugin_a, info_a));
  ASSERT_TRUE(plugin_directory.Register(plugin_b, info_b));

  ConnectionTracker tracker_a;
  ConnectionHandle conn_a =
      conn_manager.OpenRemote(&tracker_a, plugin_a, plugin_b);
  BASE_LOG("connection created: %p\n", conn_a);
  EXPECT_TRUE(conn_a != 0);
  EXPECT_TRUE(tracker_a.Current() == Link::kConnecting);

  conn_manager.HandleIO(&tracker_a, 1000);

  // connection is not established until first send.
  // todo: check if it's the expected behaviour for local sockets.
  EXPECT_TRUE(tracker_a.Current() == Link::kConnecting);

  const char message[] = "message";
  ConnectionHandle conn_b = (ConnectionHandle)((size_t)conn_a + 1);
  todo(kstasik)
      : what is Process
        ? res = conn_manager.Process(conn_b, [&](Link::Connection *connection) {
            connection->Send(message, sizeof(message));
          });
  EXPECT_TRUE(res);

  conn_manager.HandleIO(&tracker_a, 1000);

  // first send was processed so the connection is established
  EXPECT_TRUE(tracker_a.Current() == Link::kEstablished);

  conn_manager.Close(conn_b);

  // this might only catch data available for reading,
  // if so we need to catch the disconnection after.
  conn_manager.HandleIO(&tracker_a, 1000);

  if(tracker_a.Current() == Link::kEstablished) {
    conn_manager.HandleIO(&tracker_a, 1000);
  }

  // the other endpoint of the connection has been closed.
  EXPECT_TRUE(tracker_a.Current() == Link::kDisconnected);
}
*/
