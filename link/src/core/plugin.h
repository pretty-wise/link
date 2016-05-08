/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once

#include "base/core/shared_object.h"
#include "link/link.h"
#include "link_private.h"
#include "notification_queue.h"
#include "connection_manager.h"
#include "watch_manager.h"
#include "plugin_directory.h"

#include <string>

namespace Link {

class PluginManager;

// Entry point for everyhing related to local plugin.
class Plugin : public PluginDirectoryListener, public ConnectionListener {
public:
  Plugin(PluginManager &owner, PluginDirectory &directory);
  virtual ~Plugin();

  /// Returns the plugin name.
  /// @return The plugin name as a null terminated string.
  const char *Name() const;

  /// Returns the plugin version.
  /// @return The plugin version as a null terminated string.
  const char *Version() const;

  bool Load(const char *path, const char *config, streamsize nbytes);
  void Unload();

  bool Reload();

  bool Start(const LinkConfiguration &config);
  void Stop();

  bool IsCompatible() const;

  PluginHandle GetHandle() const { return m_handle; }

  static bool IsValid(const LinkConfiguration &configuration);

private:
  void OnStateChange(const Connection &conn, ConnectionState old_state,
                     ConnectionState new_state);
  ;

private:
  // symbols exposed by all plugins.
  typedef int (*GetInterfaceVersionPtr)();
  typedef const char *(*GetNamePtr)();
  typedef const char *(*GetVersionPtr)();
  typedef int (*StartupPtr)(struct LinkInterface *iface, const char *config,
                            streamsize nbytes);
  typedef void (*ShutdownPtr)();

private:
  void NotifyShutdown(ShutdownNotification::Reason reason);
  void NotifyAvailable(PluginHandle plugin, WatchHandle watch);
  void NotifyUnavailable(PluginHandle plugin, WatchHandle watch);

private:
  // Entry points for plugin side calls. See Link.h for documentation.
  static int GetNotification(void *context, Notification *notif,
                             int timeout_ms);
  static WatchHandle CreatePluginWatch(void *context, const char *name_filter,
                                       const char *version_filter,
                                       const char *hostname_filter);
  static ConnectionHandle Connect(void *context, PluginHandle plugin);
  static int GetInfo(void *context, PluginHandle plugin, PluginInfo *info);
  static int Send(void *context, ConnectionHandle conn, const void *data,
                  streamsize bytes);
  static unsigned int GetTotalRecvAvailable(void *context,
                                            ConnectionHandle conn);
  static unsigned int GetRecvAvailable(void *context, ConnectionHandle conn);
  static int Recv(void *context, ConnectionHandle handle, void *buffer,
                  unsigned int nbytes);
  static void CloseConnection(void *context, ConnectionHandle conn);
  static void CloseWatch(void *context, WatchHandle watch);
  static PluginHandle RegisterPlugin(void *context, PluginInfo *info);
  static void UnregisterPlugin(void *context, PluginHandle handle);
  static LinkConfiguration GetConfiguration(void *context);

  static void LogWrite(void *context, const char *file, int line, int category,
                       const char *format, va_list args);

private:
  void OnPluginAvailable(PluginHandle handle, const PluginInfo &info);
  void OnPluginUnavailable(PluginHandle handle, const PluginInfo &info);

private:
  PluginManager &m_owner;

  std::string m_path;
  Base::SharedObject m_shared_object;
  NotificationQueue m_notifications;
  PluginDirectory &m_plugin_directory;
  ConnectionManager m_connections;
  WatchManager m_watches;
  PluginHandle m_handle;
  struct {
    GetInterfaceVersionPtr GetInterfaceVersion;
    GetNamePtr GetName;
    GetVersionPtr GetVersion;
    StartupPtr Startup;
    ShutdownPtr Shutdown;
  } m_remote_call;

  bool m_started; // true if plugin has been started.

  const char *m_config_data;
  streamsize m_config_data_size;

  LinkConfiguration m_configuration;
};

} // namespace Link
