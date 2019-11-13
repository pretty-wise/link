/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "link/link.h"

#include <unordered_map>

namespace Link{

class RestCommand;
class JsonReader;

/// Interface allowing plugins to register rest commands and communicate
/// with the rest plugin.
class RestClient {
public:
	RestClient();
	~RestClient();
  
	/// Registers a command.
	/// @param command Command to register.
	/// @return true if command has been registered, false otherwise.
	bool RegisterCommand(RestCommand* command);

	/// Unregisters a command.
	/// @param command Command to unregister.
	void UnregisterCommand(RestCommand* command);

	void ProcessNotification(const Notification& notification);

	enum CONSTANT {
		kRecvBufferSize = 1024
	};

protected:
	// todo(kstasik): rewrite rest plugin to not be a rest client.
	// this way we will avoid self-connection related notifications
	// and we can remove this connection handle check.
	ConnectionHandle GetRestConnection() const { return m_rest_connection; }
private:
	typedef std::unordered_map<int, RestCommand*> CommandMap;

	void SendRegisterRequest(RestCommand* command);
	void SendUnregisterRequest(RestCommand* command);
	void RetryRegistrations();
	void ProcessRecv(const ConnectionNotification& notif);
	void ProcessJson(void* data, unsigned int nbytes);
	void ProcessRegisterCommandResponse(JsonReader& reader);
	void ProcessRequest(JsonReader& reader);

	// Watch listening for rest plugin.
	WatchHandle m_rest_watch;

	// Connection to rest plugin.
	ConnectionHandle m_rest_connection;

	// Linked list of commands to be registered with rest plugin.
	RestCommand* m_registered;

	// Commands already registered with rest plugin.
	CommandMap m_commands;

	void* m_recv_buffer;
};

} // namespace Link
