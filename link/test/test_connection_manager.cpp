/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gtest/gtest.h"

#include "link/link.h"
#include "plugin_directory.h"
#include "connection_manager.h"

class StateListener : public Link::ConnectionListener {
public:
	StateListener() : state(Link::ConnectionState::kDisconnected){
		connection.handle = 0;
		connection.endpoint = 0;
	}
	void OnStateChange(const Connection& data,
			Link::ConnectionState old,
			Link::ConnectionState new_state) {
		connection = data;
		state = new_state;
	}

	Connection connection;
	Link::ConnectionState state;
};


class ConnectionValidator : public Link::ConnectionListener {
public:
	ConnectionValidator() {
		m_endpoint_A.handle = 0;
		m_endpoint_B.handle = 0;
	}
	void OnStateChange(const Connection& data,
			Link::ConnectionState old,
			Link::ConnectionState new_state) {
		if(m_endpoint_A.handle == 0) {
			m_endpoint_A = data;
		} else if(m_endpoint_B.handle == 0) {
			m_endpoint_B = data;
		}

		if(data == m_endpoint_A) {
			m_state_A = new_state;
		}
		if(data == m_endpoint_B) {
			m_state_B = new_state;
		}

		if(new_state == Link::ConnectionState::kDisconnected) {
			if(m_endpoint_A.handle == data.handle) {
				m_endpoint_A.handle = 0;
			}
			if(m_endpoint_B.handle == data.handle) {
				m_endpoint_B.handle = 0;
			}
		}
	}

	ConnectionHandle GetHandleA() const { return m_endpoint_A.handle; }

	bool IsEstablished() const { return m_endpoint_A.handle != 0 && m_endpoint_B.handle != 0
		&& m_state_A == Link::ConnectionState::kEstablished 
		&& m_state_B == Link::ConnectionState::kEstablished; }

	bool IsFunctional(Link::ConnectionManager& manager, Link::ConnectionManager& other) {
		if(!IsEstablished()) {
			return false;
		}
	
		if(manager.NextRecvAvailable(m_endpoint_A.handle) != 0 
			|| manager.TotalRecvAvailable(m_endpoint_A.handle) != 0) {
			return false;
		}

		if(other.NextRecvAvailable(m_endpoint_B.handle) != 0 
			|| other.TotalRecvAvailable(m_endpoint_B.handle) != 0) {
			return false;
		}

		const char message[] = "message";
		const streamsize length = sizeof(message);

		bool sent = manager.Send(m_endpoint_A.handle, message, length);
		if(!sent) {
			return false;
		}

		if(other.NextRecvAvailable(m_endpoint_B.handle) != length 
			|| other.TotalRecvAvailable(m_endpoint_B.handle) != length) {
			return false;
		}

		streamsize received = 0;
		const int recv_buffer_size = length;
		char recv_buffer[recv_buffer_size];
		Result result = other.Recv(m_endpoint_B.handle, recv_buffer, recv_buffer_size, &received);
		if(result != RS_SUCCESS) {
			return false;
		}
		if(received != recv_buffer_size) {
			return false;
		}

		if(manager.NextRecvAvailable(m_endpoint_A.handle) != 0 
			|| manager.TotalRecvAvailable(m_endpoint_A.handle) != 0) {
			return false;
		}

		if(other.NextRecvAvailable(m_endpoint_B.handle) != 0 
			|| other.TotalRecvAvailable(m_endpoint_B.handle) != 0) {
			return false;
		}
		return true;
	}
private:
	Connection m_endpoint_A, m_endpoint_B;
	Link::ConnectionState m_state_A, m_state_B;
};

TEST(ConnectionManager, ConnectionStatus) {
	bool registered = false;

	Link::PluginDirectory dir;
	Link::ConnectionManager central(dir), other(dir);

	PluginInfo info_A, info_B;
	Link::PluginDirectory::Initialize(info_A, "info_A", "1.0", "localhost", 0, 0);
	Link::PluginDirectory::Initialize(info_B, "info_B", "1.0", "localhost", 0, 0);

	PluginHandle plugin_A = dir.GenerateHandle(info_A);
	PluginHandle plugin_B = dir.GenerateHandle(info_B);

	central.Initialize(plugin_A);
	other.Initialize(plugin_B);

	registered = dir.Register(plugin_A, info_A);
	ASSERT_TRUE(registered);

	registered = dir.Register(plugin_B, info_B);
	ASSERT_TRUE(registered);

	StateListener listener_out, listener_in;

	ConnectionHandle conn_A = central.OpenLocal(other, &listener_out, &listener_in);
	ASSERT_TRUE(conn_A);

	ASSERT_TRUE( listener_out.state == Link::ConnectionState::kEstablished );
	ASSERT_TRUE( listener_in.state == Link::ConnectionState::kEstablished );

	central.Close(conn_A);

	ASSERT_TRUE( listener_in.state == Link::ConnectionState::kDisconnected );
	ASSERT_TRUE( listener_out.state == Link::ConnectionState::kDisconnected );
}

TEST(ConnectionManager, Reuse) {
	bool registered = false;

	u16 local_connection_limit = 1;
	Link::PluginDirectory dir;
	Link::ConnectionManager manager(dir, local_connection_limit), other(dir, 2 * local_connection_limit);

	PluginInfo info_A, info_B;
	Link::PluginDirectory::Initialize(info_A, "info_A", "1.0", "localhost", 0, 0);
	Link::PluginDirectory::Initialize(info_B, "info_B", "1.0", "localhost", 0, 0);

	PluginHandle plugin_A = dir.GenerateHandle(info_A);
	PluginHandle plugin_B = dir.GenerateHandle(info_B);
	
	manager.Initialize(plugin_A);
	other.Initialize(plugin_B);

	registered = dir.Register(plugin_A, info_A);
	ASSERT_TRUE(registered);

	registered = dir.Register(plugin_B, info_B);
	ASSERT_TRUE(registered);

	ConnectionValidator validator;

	ConnectionHandle handle = manager.OpenLocal(other, &validator, &validator);
	ASSERT_TRUE(handle != 0);

	ASSERT_TRUE(validator.IsEstablished());
	ASSERT_TRUE(validator.IsFunctional(manager, other));

	manager.Close(handle);

	handle = manager.OpenLocal(other, &validator, &validator);
	ASSERT_TRUE(handle != 0);

	ASSERT_TRUE(validator.IsEstablished());
	ASSERT_TRUE(validator.IsFunctional(manager, other));

	manager.Close(handle);
}


TEST(ConnectionManager, LocalConnectionLimit) {
	bool registered = false;

	u16 local_connection_limit = 10;
	Link::PluginDirectory dir;
	Link::ConnectionManager manager(dir, local_connection_limit), other(dir, 2 * local_connection_limit);

	PluginInfo info_A, info_B;
	Link::PluginDirectory::Initialize(info_A, "info_A", "1.0", "localhost", 0, 0);
	Link::PluginDirectory::Initialize(info_B, "info_B", "1.0", "localhost", 0, 0);

	PluginHandle plugin_A = dir.GenerateHandle(info_A);
	PluginHandle plugin_B = dir.GenerateHandle(info_B);
	
	manager.Initialize(plugin_A);
	other.Initialize(plugin_B);

	registered = dir.Register(plugin_A, info_A);
	ASSERT_TRUE(registered);

	registered = dir.Register(plugin_B, info_B);
	ASSERT_TRUE(registered);

	// create all possible connections.
	ConnectionHandle handle;
	for(int i = 0; i < local_connection_limit; ++i) {
		handle = manager.OpenLocal(other, nullptr, nullptr);
		ASSERT_TRUE(handle != 0);
	}

	// add should fail.
	ConnectionHandle failed = manager.OpenLocal(other, nullptr, nullptr);
	ASSERT_TRUE(failed == 0);

	// closing last connection.
	manager.Close(handle);

	// should be able to add on free slot.
	handle = manager.OpenLocal(other, nullptr, nullptr);
	ASSERT_TRUE(handle != 0);

	// extra add should fail.
	handle = manager.OpenLocal(other, nullptr, nullptr);
	ASSERT_TRUE(handle == 0);

	manager.CloseAll();
}

TEST(ConnectionManager, RemoteConnectionLimit) {
	bool registered = false;

	u16 remote_connection_limit = 10;
	Link::PluginDirectory dir;
	Link::ConnectionManager manager(dir, remote_connection_limit + 1), other(dir, remote_connection_limit);

	PluginInfo info_A, info_B;
	Link::PluginDirectory::Initialize(info_A, "info_A", "1.0", "localhost", 0, 0);
	Link::PluginDirectory::Initialize(info_B, "info_B", "1.0", "localhost", 0, 0);

	PluginHandle plugin_A = dir.GenerateHandle(info_A);
	PluginHandle plugin_B = dir.GenerateHandle(info_B);

	manager.Initialize(plugin_A);
	other.Initialize(plugin_B);

	registered = dir.Register(plugin_A, info_A);
	ASSERT_TRUE(registered);

	registered = dir.Register(plugin_B, info_B);
	ASSERT_TRUE(registered);

	// create all possible connections.
	ConnectionHandle handle;
	for(int i = 0; i < remote_connection_limit; ++i) {
		handle = manager.OpenLocal(other, nullptr, nullptr);
		ASSERT_TRUE(handle != 0);
	}

	// add should fail.
	ConnectionHandle failed = manager.OpenLocal(other, nullptr, nullptr);
	ASSERT_TRUE(failed == 0);

	// closing last connection.
	manager.Close(handle);

	// add should still fail since we don't remove remote endpoints.
	handle = manager.OpenLocal(other, nullptr, nullptr);
	ASSERT_TRUE(handle == 0);

	manager.CloseAll();
}

TEST(ConnectionManager, SendAndReceive) {
	u16 connection_limit = 10;
	Link::PluginDirectory dir;
	Link::ConnectionManager manager(dir, connection_limit), other(dir, connection_limit);

	PluginInfo info_A, info_B;
	Link::PluginDirectory::Initialize(info_A, "info_A", "1.0", "localhost", 0, 0);
	Link::PluginDirectory::Initialize(info_B, "info_B", "1.0", "localhost", 0, 0);

	PluginHandle plugin_A = dir.GenerateHandle(info_A);
	PluginHandle plugin_B = dir.GenerateHandle(info_B);

	manager.Initialize(plugin_A);
	other.Initialize(plugin_B);

	dir.Register(plugin_A, info_A);
	dir.Register(plugin_B, info_B);

	StateListener endpoint_A, endpoint_B;

	ConnectionHandle handle = manager.OpenLocal(other, &endpoint_A, &endpoint_B);
	ASSERT_TRUE(handle != 0);

	ASSERT_TRUE(endpoint_A.connection.handle == handle);
	ASSERT_TRUE(endpoint_B.connection.handle != 0 && endpoint_B.connection.handle);

	ASSERT_TRUE(manager.NextRecvAvailable(endpoint_A.connection.handle) == 0 
		&& manager.TotalRecvAvailable(endpoint_A.connection.handle) == 0);

	ASSERT_TRUE(other.NextRecvAvailable(endpoint_B.connection.handle) == 0 
		&& other.TotalRecvAvailable(endpoint_B.connection.handle) == 0);

	ASSERT_TRUE( endpoint_A.state == Link::ConnectionState::kEstablished );
	ASSERT_TRUE( endpoint_B.state == Link::ConnectionState::kEstablished );

	const char message[] = "message";
	const streamsize length = sizeof(message);

	bool sent = manager.Send(handle, message, length);
	ASSERT_TRUE(sent);

	ASSERT_TRUE(other.NextRecvAvailable(endpoint_B.connection.handle) == length 
		&& other.TotalRecvAvailable(endpoint_B.connection.handle) == length);

	streamsize received = 0;
	const int recv_buffer_size = length;
	char recv_buffer[recv_buffer_size];
	Result result = other.Recv(endpoint_B.connection.handle, recv_buffer, recv_buffer_size, &received);
	ASSERT_TRUE(result == RS_SUCCESS);
	ASSERT_TRUE(received == recv_buffer_size);

	ASSERT_TRUE(manager.NextRecvAvailable(endpoint_A.connection.handle) == 0 
		&& manager.TotalRecvAvailable(endpoint_A.connection.handle) == 0);

	ASSERT_TRUE(other.NextRecvAvailable(endpoint_B.connection.handle) == 0 
		&& other.TotalRecvAvailable(endpoint_B.connection.handle) == 0);

	manager.CloseAll();
}

TEST(ConnectionManager, AddingRemovingConnections) {
	u16 connection_limit = 10;
	Link::PluginDirectory dir;
	Link::ConnectionManager manager(dir, connection_limit), other(dir, connection_limit);

	PluginInfo info_A, info_B;
	Link::PluginDirectory::Initialize(info_A, "info_A", "1.0", "localhost", 0, 0);
	Link::PluginDirectory::Initialize(info_B, "info_B", "1.0", "localhost", 0, 0);

	PluginHandle plugin_A = dir.GenerateHandle(info_A);
	PluginHandle plugin_B = dir.GenerateHandle(info_B);

	manager.Initialize(plugin_A);
	other.Initialize(plugin_B);

	dir.Register(plugin_A, info_A);
	dir.Register(plugin_B, info_B);

	const int connection_count = 10;

	ConnectionValidator validator[connection_count];

	// create connections.
	for(int i = 0; i < connection_count; ++i) {
		ConnectionHandle handle = manager.OpenLocal(other, &validator[i], &validator[i]);
		ASSERT_TRUE(handle != 0);

		// validate connections created so far <v,i).
		for(int v = 0; v < i; ++v) {
			ASSERT_TRUE(validator[v].IsEstablished());
			ASSERT_TRUE(validator[v].IsFunctional(manager, other));
		}
	}

	// close connections from 0 to connection_count.
	for(int i = 0; i < connection_count; ++i) {
		manager.Close(validator[i].GetHandleA());

		// validate connections that are still opened.
		for(int v = i+1; v < connection_count; ++v) {
			ASSERT_TRUE(validator[v].IsEstablished());
			ASSERT_TRUE(validator[v].IsFunctional(manager, other));
		}
	}

	/*
	other.CloseAll();

	// open connections again.
	for(int i = 0; i < connection_count; ++i) {
		ConnectionHandle handle = manager.OpenLocal(other, &validator[i], &validator[i]);
		ASSERT_TRUE(handle != 0);

		// validate connections created so far <v,i)
		for(int v = 0; v < i; ++v) {
			ASSERT_TRUE(validator[v].IsEstablished());
			ASSERT_TRUE(validator[v].IsFunctional(manager, other));
		}
	}

	// close connections from connection_count to 0.
	for(int i = connection_count-1; i >= 0; --i) {
		manager.Close(validator[i].GetHandleA());

		// validate connections that are still opened.
		for(int v = i-1; v >= 0; --v) {
			ASSERT_TRUE(validator[v].IsEstablished());
			ASSERT_TRUE(validator[v].IsFunctional(manager, other));
		}
	}
	*/
}
