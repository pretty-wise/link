/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "base/core/types.h"

typedef struct plugin_t* PluginHandle;
typedef struct connection_t* ConnectionHandle;
typedef struct watch_t* WatchHandle;

enum CONSTANTS{
		kPluginNameMax = 128,
		kPluginVersionMax = 16,
		kPluginHostnameMax = 64,
		kLinkHostnameMax = 128
};

struct PluginInfo {
	int pid;
	u16 port;
	char hostname[kPluginHostnameMax];
	char name[kPluginNameMax];
	char version[kPluginVersionMax];
};

enum NotificationType{
	/// Plugin configuration changed.
	/// New configuration is exposed through notificaiton.
	kConfigChanged = 0,
	/// A plugin that matches a specified watch has become available or unavailable.
	/// More info about the plugin can be obtained using GetPluginInfo call.
	kWatch,
	/// A connection created by Connect call has been established.
	kEstablished,
	/// Remote plugin connected. Store the connection handle to
	/// process with Send and Recv calls.
	kConnected,
	/// Denotes that a connection has been closed on the other end.
	/// Call CloseConnection to cleanup the handle.
	kDisconnected,
	/// Denotes that a connection has data ready for reading.
	/// Fetch the data using Recv.
	kRecvReady,
	/// Plugin is being destroyed. Cleanup before plugin shutdown.
	kShutdown
};

enum PluginState {
	kPluginAvailable,
	kPluginUnavailable
};

struct ConfigNotification {
	const void* buffer;
	unsigned int nbytes;
};

struct ShutdownNotification {
	enum Reason {
		kUnload, // todo: support.
		kReload,
		kStop
	} reason;
};

struct WatchNotification {
		WatchHandle handle;
		PluginHandle plugin;
		int plugin_state; //< PluginState value.
};

struct ConnectionNotification {
		ConnectionHandle handle;
		PluginHandle endpoint;
};

struct Notification{
	enum NotificationType type;
	union {
		struct ConfigNotification config;
		struct ShutdownNotification shutdown;
		struct WatchNotification watch;
		struct ConnectionNotification connection;
	} content;
};

struct LinkConfiguration {
	char hostname[kLinkHostnameMax];
	struct PluginInfo info;
};


#if defined __cplusplus
extern "C"{
#endif

//--------------------------------------------------------------------------------

/// Retrieves link core configuration.
/// @return link configuration.
struct LinkConfiguration GetConfiguration();

/// Retrieves plugin notification from the core.
/// Blocks for timeout_ms mulliseconds if thre are no pending notifications.
/// @param notif Retrieved notification. See Notification for more details.
/// @param timeout_ms Time in milliseconds to wait for notifications.
/// @return Zero if notificaiton has been retrieved,
///	 -1 if timeout occured.
///	 -2 if there was an error retrieving notification.
int GetNotification(struct Notification* notif, int timeout_ms);

/// Creates a watch that monitors for plugin availability.
/// WatchNotification will be emitted to notify about plugin availability.
/// @param name_filter Name filter to match.
/// @param version_filter Version to match.
/// @param hostname_filter Hostname to match.
/// @return Watch handle.
WatchHandle CreateWatch(const char* name_filter, const char* version_filter, const char* hostname_filter);

/// Initiates connection to a given plugin.
/// @param handle Plugin to initiate connection to.
/// @return A non-zero connection handle, or 0 if the attempt failed.
ConnectionHandle Connect(PluginHandle handle);

/// Queues a packet of data for sending
/// @param conn Connection handle to push the packet through.
/// @param bytes Data to send.
/// @param count Number of bytes to send.
/// @return Zero if successful, negative value otherwise.
int Send(ConnectionHandle conn, const void* data, streamsize bytes);

/// Retrieves info about specified plugin handle.
/// @param plugin Plugin to retrieve info about.
/// @param info Retrieved plugin info.
/// @return Zero if successful, negative value otherwise.
int GetPluginInfo(PluginHandle plugin, struct PluginInfo* info);

/// Retrieves total number of bytes available on given connection.
/// @param conn Connection to check availability on.
/// @return Number of bytes available for reading,
///	 zero if there is nothing to read.
unsigned int GetTotalRecvAvailable(ConnectionHandle conn);

/// Number of bytes available for next Recv operation.
/// When multiple threads read from one connection the value might change before
/// calling Recv on connection!
/// @param conn Connection to read from.
/// @return Number of bytes available for reading,
///	 zero if there is no data to read.
unsigned int GetRecvAvailable(ConnectionHandle conn);

/// Retrieves next packet data from a connection.
/// @param handle Connection to retrieve data from.
/// @param buffer Buffer to store data in.
/// @param nbytes Capacity of the buffer.
/// @return Zero if there is no data to receive.
///	-1 if handle is invalid.
///	-2 if supplied buffer is too small.
int Recv(ConnectionHandle handle, void* buffer, unsigned int nbytes);

/// Closes the connection.
/// @param conn Connection to close.
void CloseConnection(ConnectionHandle conn);

/// Closes the watch.
/// No more notifications will be received for this watch.
/// @param watch Watch handle to close.
void CloseWatch(WatchHandle watch);

/// Registers a plugin handle to be recognised in this process.
/// @param info Plugin description.
/// @return Handle of registered plugin, zero if failed.
PluginHandle RegisterPlugin(struct PluginInfo* info);

/// Unregisters a plugin handle.
/// @param handle Plugin handle to unregister.
void UnregisterPlugin(PluginHandle handle);

//--------------------------------------------------------------------------------

/// Retrieves plugin name.
/// @return Name of the plugin.
const char* GetName();

/// Retrieves plugin version.
/// @return Version of the plugin.
const char* GetVersion();

/// Startup function implemented by the plugin.
/// Usually plugin needs to start it's own thread(s) to operate.
/// @param config Plugin configuration data.
/// @param nbytes Length of config data, in bytes.
/// @return 0 upon successful startup, negative value otherwise.
int Startup(const char* config, streamsize nbytes);

/// Plugin shutdown call. Do any cleanup here.
/// Before this call is triggered a Shutdown notification will be emitted
/// to the plugin. A good practice is to call join on any plugin thread here
/// and shutdown/cleanup the threads upon receiving the notification.
void Shutdown();

//--------------------------------------------------------------------------------

#if defined __cplusplus
}
#endif
