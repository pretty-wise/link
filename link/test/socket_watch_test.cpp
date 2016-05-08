/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gtest/gtest.h"

#include "base/core/macro.h"
#include "base/threading/thread.h"

#include "SocketWatch.h"
#include "TcpSocket.h"
#include "plugin_directory.h"
#include "connection_manager.h"
#include "Connection.h"
#include "ConnectionListener.h"

u16 listen_port;

int connect_thread(void* arg) {
	Base::Thread::Sleep(100);
	//Link::TcpSocket* socket = static_cast<Link::TcpSocket*>(arg);

	Link::TcpSocket new_socket;
	new_socket.Open();
	Base::Url url(Base::AddressIPv4(127, 0, 0, 1), listen_port);
	bool res = new_socket.Open();
	EXPECT_TRUE(res);
	res = new_socket.Connect(url);
	EXPECT_TRUE(res);
	return 0;
}

TEST(SocketWatch, ConnectAndAccept) {
	Link::SocketWatch watch(20);
	Link::TcpSocket socket;
	bool event_processed = false;

	bool res = socket.Open();
	EXPECT_TRUE(res);
	listen_port = 0;
	res = socket.Listen(listen_port);
	EXPECT_TRUE(res);

	bool registered = watch.Register(socket, 0);
	EXPECT_TRUE(registered);

	Base::Thread async;
	res = async.Initialize(&connect_thread, &socket);
	EXPECT_TRUE(res);

	auto process_func = [&](ConnectionHandle conn, bool read, bool write, bool error) {
		//BASE_LOG("epoll event for %p (R:%s/W:%s/E:%s)\n", conn, read ? "Y" : "N", write ? "Y" : "N", error ? "Y" : "N");
		event_processed = true;

		if(read) {
			Link::TcpSocket new_connection;
			Base::Url new_address;
			bool accepted = socket.Accept(new_connection, new_address);
			EXPECT_TRUE(accepted);
		}
	};

	watch.Wait(1000, process_func);
	EXPECT_TRUE(event_processed);

	async.Join();
}

class ConnectionTracker : public Link::ConnectionListener {
public:
	ConnectionTracker() : m_curr(Link::Connection::kError) {}
	virtual void OnStateChange(const Link::Connection& connection, Link::Connection::State old_state, Link::Connection::State new_state) {
		m_curr = new_state;
	}
	Link::Connection::State Current() const { return m_curr; }
private:
	Link::Connection::State m_curr;
};

TEST(SocketWatch, Test2) {
	Link::PluginDirectory plugin_directory;
	Link::ConnectionManager conn_manager(plugin_directory, 20);

	listen_port = 0;
	bool res = conn_manager.CreateListenSocket(listen_port);
	ASSERT_TRUE(res);


	PluginHandle plugin_a = (PluginHandle)0xAA;
	PluginHandle plugin_b = (PluginHandle)0xBB;

	PluginInfo info_a, info_b;
	Link::PluginDirectory::Initialize(info_a, "plugin_a", "1.00", "127.0.0.1", listen_port, 0);
	Link::PluginDirectory::Initialize(info_b, "plugin_b", "1.00", "127.0.0.1", listen_port, 0);

	ASSERT_TRUE( plugin_directory.Register(plugin_a, info_a) );
	ASSERT_TRUE( plugin_directory.Register(plugin_b, info_b) );

	ConnectionTracker tracker_a;
	ConnectionHandle conn_a = conn_manager.Open(plugin_a, plugin_b, &tracker_a);
	BASE_LOG("connection created: %p\n", conn_a);
	EXPECT_TRUE(conn_a != 0);
	EXPECT_TRUE(tracker_a.Current() == Link::Connection::kConnecting);

	conn_manager.HandleIO(1000);

	// connection is not established until first send.
	// todo: check if it's the expected behaviour for local sockets.
	EXPECT_TRUE(tracker_a.Current() == Link::Connection::kConnecting);

	const char message[] = "message";
	ConnectionHandle conn_b = (ConnectionHandle)((size_t)conn_a + 1);
	res	= conn_manager.Process(conn_b, [&](Link::Connection* connection){
		connection->Send(message, sizeof(message));
	});
	EXPECT_TRUE(res);

	conn_manager.HandleIO(1000);

	// first send was processed so the connection is established
	EXPECT_TRUE(tracker_a.Current() == Link::Connection::kEstablished);

	conn_manager.Close(conn_b);

	// this might only catch data available for reading,
	// if so we need to catch the disconnection after.
	conn_manager.HandleIO(1000);

	if(tracker_a.Current() == Link::Connection::kEstablished) {
		conn_manager.HandleIO(1000);
	}

	// the other endpoint of the connection has been closed.
	EXPECT_TRUE(tracker_a.Current() == Link::Connection::kDisconnected);
}
