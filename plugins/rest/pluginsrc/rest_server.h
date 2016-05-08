/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "civetweb.h"

#include "link/link.h"

#include "base/threading/mutex.h"

#include <string>
#include <unordered_map>
#include <queue>

namespace Link {

class JsonWriter;

class RestServer {
public:
	/// Timeout for a HTTP request in milliseconds.
	static const unsigned int kRequestTimeout = 5000;

	RestServer();
	~RestServer();

	/// Starts HTTP server.
	/// @param options Holds HTTP server options.
	/// For more info go to: https://github.com/sunsetbrew/civetweb/blob/master/docs/UserManual.md.
	bool Start(const char** options);

	/// Shuts down HTTP server with all ongoing connections.
	void Stop();

	/// Registers a command for a plugin.
	/// @param cmd_uri Command URI.
	/// @param connection_handle Connection to a plugin that owns the command.
	/// @return true if registration succeeded, false otherwise.
	/// Command registration may fail because of invalid or duplicate name.
	bool Register(const char* cmd_uri, ConnectionHandle connection);

	/// Unregisters a command by URI.
	/// @param cmd_uri Command URI to unregister.
	void Unregister(const char* cmd_uri);

	/// Unregisters command by id.
	/// @param command_id Id of a command to unregister.
	void Unregister(int command_id);

	/// Unregisters all commands registered for a given connection.
	/// @param connection Connection handle which commands should be unregistered.
	void UnregisterAll(ConnectionHandle connection);

	/// Sends a response to a given request.
	/// @param connection_id Connection handle to send the response to.
	/// @param request_id Id of the request the response belongs to.
	/// @param bytes Response data.
	/// @param length Length of response data.
	/// @return true if response sent, false otherwise.
	bool SendResponse(ConnectionHandle connection_handle, int request_id, const char* bytes, int length);

	/// Writes registered with specified json writer.
	void WriteAvailableCommands(JsonWriter* writer) const;
private:

	/// Describes REST command.
	struct Command {
		std::string uri; //< Registered uri.
		ConnectionHandle handle; //< Connection handle to command owner.
	};

	/// Request timeout data.
	struct Timeout {
		unsigned int expiration_time; // Expiration time in milliseconds.
		int request_id; // Request to expire.
	};

	friend bool operator< (const Timeout& a, const Timeout& b) {
		return a.expiration_time >= b.expiration_time;
	}

	/// Ongoing connection information held in ConnectionList,
	/// where request id is a key.
	struct Connection{
		ConnectionHandle handle; // Plugin connection.
		struct mg_connection* connection; // HTTP connection
	};

	typedef std::unordered_map<int, Command> CommandList; //< The key is a command id.
	typedef std::unordered_map<int, Connection> ConnectionList; //< The key is a request id.
	typedef std::priority_queue<Timeout> TimeoutQueue; //< Ordered by expiration time.

	/// Finds a command data by id.
	/// @param command_id Id of command.
	/// @param cmd Retrieved command. Valid only if true was returned.
	/// Command returned by value.
	/// @return true if command found, false otherwise.
	bool Find(int command_id, Command* cmd);

	/// Forwards HTTP request to the plugin for processing
	/// @param plugin_conn_handle Connection handle to a plugin that owns received REST command.
	/// @param message REST command received.
	/// @param length Message length.
	/// @param request_id Generated id for this request.
	/// @param conn HTTP connection associated with this request.
	/// @param timeout Request timeout in milliseconds.
	/// @return true if message was successfully sent to the plugin, false otherwise.
	bool ForwardToPlugin(ConnectionHandle plugin_conn_handle, const char* message, unsigned int length, int request_id, struct mg_connection* conn, unsigned int timeout);
private:
	/// HTTP server context.
	struct mg_context* m_context;
	CommandList m_commands;
	mutable Base::Mutex m_commands_lock;
	ConnectionList m_connections;
	Base::Mutex m_connections_lock;
	TimeoutQueue m_timeout; //< Timeout queue.
	Base::Mutex m_timeout_lock;

	/// HTTP request callbacks.
	/// @param conn Established connection.
	/// @param cbdata Command id.
	/// @return 0 if request not handled, 1 if connection must be kept
	/// @return 2 if connection handled and must be closed
	static int RequestHandler(struct mg_connection *conn, void *cbdata);

private:
	RestServer(const RestServer&); // no copy
	RestServer& operator=(const RestServer&); // no copy
};

} // namespace Link
