/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin.h"
#include "link_private.h"
#include "plugin_manager.h"
#include "log.h"
#include "base/network/address.h"

#include "base/core/memory.h"
#include "base/core/str.h"
#include "base/core/assert.h"

namespace Link {

Plugin::Plugin(PluginManager &owner, PluginDirectory &directory)
    : m_owner(owner), m_plugin_directory(directory), m_connections(directory),
      m_started(false), m_config_data(nullptr), m_config_data_size(0) {
  m_plugin_directory.AddListener(this);
}

Plugin::~Plugin() {
  m_plugin_directory.RemoveListener(this);
  delete[] m_config_data;
}

bool Plugin::Load(const char *path, const char *config, streamsize nbytes) {
  if(!m_shared_object.Open(path)) {
    LINK_WARN("failed opening so %s.", path);
    return false;
  }

  // store config data.
  delete[] m_config_data;
  m_config_data = new char[nbytes];
  m_config_data_size = nbytes;
  memcpy((void *)m_config_data, config, nbytes);

  m_path = path;

  const char *symbols[] = {"GetInterfaceVersion", "GetName",  "GetVersion",
                           "StartupPlugin",       "Shutdown", NULL};

  int i = 0;
  while(const char *symbol_name = symbols[i]) {
    void *sym = m_shared_object.GetSymbol(symbol_name);

    if(!sym) {
      LINK_INFO("Symbol not found %s", symbol_name);
      return false;
    }

    switch(i) {
    case 0:
      m_remote_call.GetInterfaceVersion = (GetInterfaceVersionPtr)sym;
      break;
    case 1:
      m_remote_call.GetName = (GetNamePtr)sym;
      break;
    case 2:
      m_remote_call.GetVersion = (GetVersionPtr)sym;
      break;
    case 3:
      m_remote_call.Startup = (StartupPtr)sym;
      break;
    case 4:
      m_remote_call.Shutdown = (ShutdownPtr)sym;
      break;
    default:
      break;
    }
    i++;
  }

  return true;
}

bool Plugin::IsCompatible() const {
  return m_remote_call.GetInterfaceVersion() == LINK_INTERFACE_VERSION;
}

void Plugin::Unload() {
  if(m_shared_object.IsLoaded())
    m_shared_object.Close();
}

bool Plugin::Reload() {
  Unload();

  if(m_path.empty())
    return false;
  return Load(m_path.c_str(), m_config_data, m_config_data_size);
}

bool Plugin::Start(const LinkConfiguration &configuration) {
  if(!IsValid(configuration)) {
    LINK_ERROR("link configuration is invalid");
    return false;
  }

  if(m_remote_call.Startup < 0)
    return false;

  if(m_started)
    return false;

  LinkInterface iface;
  iface.GetNotification = GetNotification;
  iface.CreatePluginWatch = CreatePluginWatch;
  iface.GetInfo = GetInfo;
  iface.Connect = Connect;
  iface.Send = Send;
  iface.LogWrite = LogWrite;
  iface.GetTotalRecvAvailable = GetTotalRecvAvailable;
  iface.GetRecvAvailable = GetRecvAvailable;
  iface.Recv = Recv;
  iface.CloseConnection = CloseConnection;
  iface.CloseWatch = CloseWatch;
  iface.RegisterPlugin = RegisterPlugin;
  iface.UnregisterPlugin = UnregisterPlugin;
  iface.GetConfiguration = GetConfiguration;
  iface.context = this;

  {
    Base::MutexAutoLock lock(m_plugin_directory.AccessLock());

    u16 port = 0;
    if(!m_connections.CreateListenSocket(port)) {
      LINK_ERROR("plugin %s(%s) failed to listen on port %d.", Name(),
                 Version(), port);
      return false;
    }

    LINK_INFO("plugin %s(%s) listening on port %d", Name(), Version(), port);

    PluginInfo info;
    Base::String::strncpy(info.hostname, configuration.hostname,
                          kPluginHostnameMax);
    info.port = port;
    Base::String::strncpy(info.name, m_remote_call.GetName(), kPluginNameMax);
    Base::String::strncpy(info.version, m_remote_call.GetVersion(),
                          kPluginVersionMax);
    info.pid = getpid();

    // prepare configuration before starting plugin.
    m_configuration = configuration;
    memcpy(&m_configuration.info, &info, sizeof(PluginInfo));

    LINK_INFO("** plugin %s configuration:", m_configuration.info.name);
    LINK_INFO(" * version %s", m_configuration.info.version);
    LINK_INFO(" * hostname: %s", m_configuration.hostname);
    LINK_INFO(" * port: %d", m_configuration.info.port);
    LINK_INFO(" * pid: %d", m_configuration.info.pid);

    int result =
        m_remote_call.Startup(&iface, m_config_data, m_config_data_size);
    m_started = result == 0;

    if(result == 0) {

      m_handle = m_plugin_directory.GenerateHandle(info);
      m_connections.Initialize(m_handle);

      bool registered = m_plugin_directory.Register(m_handle, info);

      LINK_INFO("plugin %s(%s) registered as %p.", Name(), Version(), m_handle);

      if(!registered) {
        NotifyShutdown(ShutdownNotification::kStop);
        m_remote_call.Shutdown();
        m_started = false;
      }

      return m_started;
    }
  }
  return false;
}

const char *Plugin::Name() const { return m_remote_call.GetName(); }

const char *Plugin::Version() const { return m_remote_call.GetVersion(); }

void Plugin::Stop() {
  // close all connections before shutdown. todo: connections shutdown instead?
  m_connections.CloseAll();

  NotifyShutdown(ShutdownNotification::kStop);

  if(!m_remote_call.Shutdown)
    return;

  if(m_started) {
    m_remote_call.Shutdown();
  }

  m_started = false;
}

void Plugin::OnStateChange(const Connection &conn, ConnectionState old_state,
                           ConnectionState new_state) {
  if(new_state == ConnectionState::kEstablished) {
    Notification notif;
    notif.type = conn.outgoing ? NotificationType::kEstablished
                               : NotificationType::kConnected;
    notif.content.connection.handle = conn.handle;
    notif.content.connection.endpoint = conn.endpoint;
    m_notifications.Push(notif);
  } else if(old_state == ConnectionState::kEstablished) {
    Notification notif;
    notif.type = NotificationType::kDisconnected;
    notif.content.connection.handle = conn.handle;
    notif.content.connection.endpoint = conn.endpoint;
    m_notifications.Push(notif);
  }
}

void Plugin::NotifyShutdown(ShutdownNotification::Reason reason) {
  Notification notif;
  notif.type = NotificationType::kShutdown;
  notif.content.shutdown.reason = reason;
  m_notifications.Push(notif);
}

void Plugin::NotifyAvailable(PluginHandle plugin, WatchHandle watch) {
  Notification notif;
  notif.type = NotificationType::kWatch;
  notif.content.watch.plugin = plugin;
  notif.content.watch.handle = watch;
  notif.content.watch.plugin_state = kPluginAvailable;
  m_notifications.Push(notif);
}

void Plugin::NotifyUnavailable(PluginHandle plugin, WatchHandle watch) {
  Notification notif;
  notif.type = NotificationType::kWatch;
  notif.content.watch.plugin = plugin;
  notif.content.watch.handle = watch;
  notif.content.watch.plugin_state = kPluginUnavailable;
  m_notifications.Push(notif);
}

int Plugin::GetNotification(void *context, Notification *notif,
                            int timeout_ms) {
  Plugin *module = (Plugin *)context;

  // TODO: timeout on io, push io notifications.
  // LINK_INFO("timeout %d", timeout_ms);
  if(timeout_ms > 0) {
    module->m_connections.HandleIO(module, timeout_ms);
  }

  // quick check if there is notification waiting.
  bool has_notification = module->m_notifications.Pop(*notif, 0);

  if(!has_notification) {
    // no notificaion, so check for connection notifs.
    has_notification = module->m_connections.GetNotification(notif);
    if(!has_notification) {
      // no connection notification, re-check and block with timeout.
      has_notification = module->m_notifications.Pop(*notif, timeout_ms);
    }
  }

  return has_notification ? 1 : 0;
}

WatchHandle Plugin::CreatePluginWatch(void *context, const char *name_filter,
                                      const char *version_filter,
                                      const char *hostname_filter) {
  Plugin *module = (Plugin *)context;
  Base::MutexAutoLock dir_lock(module->m_plugin_directory.AccessLock());
  Base::MutexAutoLock lock(module->m_watches.AccessLock());
  WatchHandle handle =
      module->m_watches.Create(name_filter, version_filter, hostname_filter);
  if(handle != 0) {
    // check if plugin is already available and notify.
    module->m_plugin_directory.ForPlugin(
        name_filter, version_filter, hostname_filter,
        [&](PluginHandle plugin_handle, const PluginInfo &info) {
          module->NotifyAvailable(plugin_handle, handle);
        });
  }

  return handle;
}

ConnectionHandle Plugin::Connect(void *context, PluginHandle plugin) {
  Plugin *module = (Plugin *)context;

  Plugin *other_plugin = module->m_owner.Find(plugin);

  /* rest plugin connects to self.
          if(module->m_handle == other_plugin->m_handle) {
                  // connecting to self is not allowed for now.
                  // it might need some assert removal in ConnectionManager.
                  LINK_ERROR("can't connect to self %p.", module->m_handle);
                  return 0;
          }
  */

  // we own the other plugin, that means it's a local connection.
  if(other_plugin) {
    return module->m_connections.OpenLocal(other_plugin->m_connections, module,
                                           other_plugin);
  }

  return module->m_connections.OpenRemote(module, module->m_handle, plugin);
}

int Plugin::GetInfo(void *context, PluginHandle plugin, PluginInfo *info) {
  Plugin *module = (Plugin *)context;

  if(module->m_plugin_directory.GetInfo(plugin, *info)) {
    return 0;
  }

  return -1;
}

int Plugin::Send(void *context, ConnectionHandle conn, const void *data,
                 streamsize bytes) {
  Plugin *module = (Plugin *)context;
  return module->m_connections.Send(conn, data, bytes) ? 0 : -1;
}

unsigned int Plugin::GetTotalRecvAvailable(void *context,
                                           ConnectionHandle conn) {
  Plugin *_this = (Plugin *)context;

  unsigned int bytes_read = _this->m_connections.TotalRecvAvailable(conn);

  return bytes_read;
}

unsigned int Plugin::GetRecvAvailable(void *context, ConnectionHandle conn) {
  Plugin *_this = (Plugin *)context;

  return _this->m_connections.NextRecvAvailable(conn);
}

int Plugin::Recv(void *context, ConnectionHandle handle, void *buffer,
                 unsigned int nbytes) {
  ///	-1 if handle is invalid.
  ///	-2 if supplied buffer is too small.

  Plugin *_this = (Plugin *)context;

  streamsize received = 0;

  Result result = _this->m_connections.Recv(handle, buffer, nbytes, &received);

  // TODO if(!connection_found) {
  //	return -1;
  //}

  if(result == RS_BUFFER_TOO_SMALL) {
    return -2;
  }

  BASE_ASSERT(result == RS_SUCCESS ||
                  (result == RS_NOTENOUGH_MEM && received == 0) ||
                  (result == RS_EMPTY && received == 0),
              "unhandeled error %d %d", result, received);

  return received;
}

void Plugin::CloseConnection(void *context, ConnectionHandle conn) {
  if(conn != 0) {
    Plugin *_this = (Plugin *)context;
    _this->m_connections.Close(conn);
  }
}

void Plugin::CloseWatch(void *context, WatchHandle watch) {
  if(watch != 0) {
    Plugin *_this = (Plugin *)context;
    _this->m_watches.Remove(watch);
  }
}

PluginHandle Plugin::RegisterPlugin(void *context, PluginInfo *info) {
  Plugin *_this = (Plugin *)context;

  if(!info) {
    LINK_ERROR("failed to register plugin - no info");
    return 0;
  }

  // TODO: check info if valid.

  PluginHandle handle = _this->m_plugin_directory.GenerateHandle(*info);

  if(handle == 0) {
    LINK_ERROR("failed to generate handle for %s", info->name);
    return 0;
  }

  if(!_this->m_plugin_directory.Register(handle, *info)) {
    LINK_WARN("failed to register plugin: %s", info->name);
    return 0;
  }

  LINK_INFO("registered plugin %s(%s) %s:%d [%d[", info->name, info->version,
            info->hostname, info->port, info->pid);

  return handle;
}

void Plugin::UnregisterPlugin(void *context, PluginHandle handle) {
  if(handle != 0) {
    Plugin *_this = static_cast<Plugin *>(context);
    _this->m_plugin_directory.Unregister(handle);
  }
}

LinkConfiguration Plugin::GetConfiguration(void *context) {
  Plugin *_this = static_cast<Plugin *>(context);
  return _this->m_configuration;
}

void Plugin::OnPluginAvailable(PluginHandle handle, const PluginInfo &info) {
  std::vector<WatchHandle> handles;
  m_watches.Filter(info.name, info.version, info.hostname, handles);
  for(auto watch = handles.begin(); watch != handles.end(); ++watch) {
    NotifyAvailable(handle, *watch);
  }
}

void Plugin::OnPluginUnavailable(PluginHandle handle, const PluginInfo &info) {
  std::vector<WatchHandle> handles;
  m_watches.Filter(info.name, info.version, info.hostname, handles);

  for(auto watch = handles.begin(); watch != handles.end(); ++watch) {
    NotifyUnavailable(handle, *watch);
  }
}

void Plugin::LogWrite(void *context, const char *file, int line, int category,
                      const char *format, va_list args) {
  Plugin *plugin = static_cast<Plugin *>(context);
  Link::Log::PluginWrite(file, line, category, plugin->Name(),
                         plugin->Version(), format, args);
}

bool Plugin::IsValid(const LinkConfiguration &c) {
  if(c.hostname[0] == 0) {
    LINK_WARN("hostname invalid");
    return false;
  }
  return true;
}

} // namespace Link
