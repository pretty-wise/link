/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "base/core/types.h"
#include "link/link.h"
#include "base/network/socket.h"
#include "base/network/epoll.h"
#include "base/threading/mutex.h"

namespace Link {

enum ConnectionState {
	kDisconnected,
	kConnecting,
	kHandshake,
	kEstablished,
	kError
};

const char* ToString(ConnectionState s);

class ConnectionListener {
public:
	struct Connection {
		ConnectionHandle handle;
		PluginHandle endpoint;
		bool outgoing;
	};
	virtual void OnStateChange(const struct Connection& connection, ConnectionState old, ConnectionState new_state) = 0;
private:
};

enum CloseReason {
	CR_UserRequest,
	CR_CloseAll,
	CR_ReadError,
	CR_WriteError,
	CR_EndOfFile,
	CR_HandshakeReadError,
	CR_HandshakeParseError,
	CR_HandshakeInfoMismatch,
	CR_HandshakeRefused,
	CR_HandshakeSynSendError,
	CR_HandshakeAckSendError
	// must stay in sync with ToString.
};

const char* ToString(CloseReason reason);

bool operator==(const ConnectionListener::Connection& rhs, const ConnectionListener::Connection& lhs);

class PluginDirectory;
class ConnectionManager;
class MessageQueue;
class MessageInStream;
class MessageOutStream;

struct Connection {
	enum Enum {
		kLocal = 0x1, // local or remote connection.
		kOutgoing = 0x2, // outgoing or incoming connection.
		kWritable = 0x4, // connection writable.
		kSynReceived = 0x8, // received syn packet.
		kSynSent = 0x10, // sent plugin data.
		kAckSent = 0x20, // sent ack.
		kAckReceived = 0x40,
		kClosed = 0x80
	};

	struct ConnectionVfptr* vfptr;
	ConnectionHandle handle; // Connection id.
	ConnectionState state; // Current connection state.
	PluginHandle endpoint; // Destination plugin id.
	ConnectionListener* listener; // Connection state listener.
	u16 detail_index; // Index of detail data (local or remote).
	u16 flags; // see enum ConnectionFlag.
	u32 time; // connection timer.

	void Init(ConnectionHandle _handle, PluginHandle _source, PluginHandle _dest, ConnectionListener* _listener, u16 _detail_index, u16 _flags);
	void SetState(ConnectionState _state);
};

struct LocalConnection {
	ConnectionHandle remote_handle; // Other endpoint id.
	ConnectionManager* remote_manager; // Manager of the other endpoint.
	MessageQueue* in_messages; // Incoming message buffer of this connection.
	MessageQueue* out_messages; // ? Other endpoint's incoming message buffer.

	void Init(ConnectionManager* _remote, ConnectionHandle _remote_handle, streamsize _buffer_size);
};

struct RemoteConnection {
	Base::Socket::Handle socket;
	MessageInStream* in_messages;
	MessageOutStream* out_messages;
};

struct TCPConnectionData {
	struct RemoteConnection* connection;
	u16 count;
	u16 capacity;	
	Base::Socket::Handle listen_socket; //! TCP conneciton listen socket.
	Base::EpollHandle epoll; //! TCP connection event handling.
};

struct InProcConnectionData {
	struct LocalConnection* connection;
	u16 count;
	u16 capacity;
};

// Holds connections related to a single plugin.
class ConnectionManager {
public:
	/// @param max_num_connections Maximum opened connections.
	ConnectionManager(PluginDirectory& plugins, u32 max_num_connections = 20);
	~ConnectionManager();

	void Initialize(PluginHandle owner);

	/// Opens a listen socket on a free port.
	/// @param port Port the manager listens on.
	/// @return True if socket opened successfully, false otherwise.
	bool CreateListenSocket(u16& port);

	/// Opens a local connection to given plugin.
	/// @param destination Destination plugin.
	/// @param listener Listens for connection status changes.
	/// @return Connection id. Zero if creation failed.
	ConnectionHandle OpenLocal(ConnectionManager& endpoint_connections,
															ConnectionListener* listener_local,
															ConnectionListener* listener_remote);

	/// Opens a remote connection to a given plugin.
	/// @return Connection id. Zero if creation failed.
	ConnectionHandle OpenRemote(ConnectionListener* listener_local, 
																PluginHandle local_id, 
																PluginHandle remote_id);

	/// Closes given connection.
	/// @param handle Connection to close.
	void Close(ConnectionHandle handle);

	/// Closes all connections.
	void CloseAll();

	/// Returns a single notification event, if any exist, for the managed
	/// connections.
	/// @param notification A pointer to the notification to be populated.
	/// @return true if notification was populated. If false is returned then
	/// the contents of notification is undefined.
	bool GetNotification(Notification* notification);

	bool Send(ConnectionHandle handle, const void* data, streamsize nbytes);
	unsigned int TotalRecvAvailable(ConnectionHandle handle) const;
	unsigned int NextRecvAvailable(ConnectionHandle handle) const;
	Result Recv(ConnectionHandle handle, void* bytes, streamsize count, streamsize* received);

	/// Processes all connection with a given timeout.
	void HandleIO(ConnectionListener* listener, time_ms timeout);

public:
	void Close(ConnectionHandle handle, CloseReason reason);
	void TryClose(u16 index);
	void DestroyRemoteConnection(u16 index);
	void DestroyLocalConnection(u16 index);

	void FlushOutgoing();
	void AcceptConnections(ConnectionListener* listener);

private:
	void UpdateHandshake();
	bool SendHandshakeSyn(struct RemoteConnection& conn);
	bool SendHandshakeAck(struct RemoteConnection& conn);

private:
	/// Generates new connection handle.
	/// @return Generated connection handle, always valid.
	ConnectionHandle GenerateHandle();

	PluginDirectory& m_plugin_dir; //! Directory of local and remote plugins.
	PluginHandle m_owner; //! Id of the parent plugin.
	s64 m_handle_generator; //! Connection handle generator.
	mutable Base::Mutex m_mutex; //! Connection manager lock. Protects all data.
	const u16 m_max_connections; //! Maximum connections supported.
	u16 m_connection_count; //! Current number of connections.
	ConnectionHandle* m_connection_list; //! List of connection handles. Index relates to m_data array.
	struct Connection* m_data; //! Common connection data.

public:
	TCPConnectionData m_tcp;
	InProcConnectionData m_inproc;
private:
	u16 AddConnection(ConnectionHandle handle);
	void RemoveConnection(u16 remove_index);
	u16 FindIndex(ConnectionHandle handle) const;
};

} // namespace Link
