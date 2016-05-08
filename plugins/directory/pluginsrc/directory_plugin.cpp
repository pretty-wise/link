/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "directory_plugin.h"

#include "link/plugin_log.h"
#include "base/core/str.h"
#include "base/core/memory.h"
#include "base/core/assert.h"

#include "common/protobuf_stream.h"
#include "protocol/directory.pb.h"
#include "tinyxml2.h"

#include <sys/types.h> // getpid
#include <unistd.h>    // getpid
#include <algorithm>   // std::find

namespace Link {

bool ListPluginsCmd::OnCommand(const std::string &query_string,
                               const std::string &post_data,
                               std::string *response_data) {
  (void)query_string;
  (void)post_data;

  m_directory.WritePluginList(response_data);
  return true;
}

DirectoryPlugin::DirectoryPlugin()
    : SimplePlugin(kUpdateDeltaMs), m_is_master(true), m_master_handle(0),
      m_master_connection(0), m_plugin_list_cmd(*this) {
  m_recv_buffer = malloc(kRecvBufferSize);
}

DirectoryPlugin::~DirectoryPlugin() { free(m_recv_buffer); }

bool DirectoryPlugin::OnStartup(const char *config, streamsize nbytes) {

  if(!RegisterCommand(&m_plugin_list_cmd)) {
    PLUGIN_ERROR("problem registering plugin list command");
    return false;
  }

  m_is_master = true;
  if(config && nbytes > 0) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.Parse(config, nbytes);

    if(err == tinyxml2::XML_SUCCESS) {
      tinyxml2::XMLElement *master_node = doc.FirstChildElement("master");
      if(master_node) {
        const char *name = master_node->Attribute("name");
        const char *version = master_node->Attribute("version");
        const char *hostname = master_node->Attribute("hostname");
        u16 port = master_node->IntAttribute("port");
        int pid = master_node->IntAttribute("pid");

        if(RegisterMasterPlugin(name, version, hostname, port, pid)) {
          PLUGIN_INFO("slave mode. master plugin is: %s(%s) %s:%d, %d", name,
                      version, hostname, port, pid);
          m_is_master = false;
        } else {
          PLUGIN_ERROR("invalid plugin data");
          return false;
        }
      } else {
        PLUGIN_ERROR("no master node in config.");
        return false;
      }
    } else {
      PLUGIN_ERROR("problem parsing config: %s(%d)", doc.ErrorName(), err);
      return false;
    }
  }

  PLUGIN_INFO("directory started in %s mode", m_is_master ? "master" : "slave");

  // todo(kstasik): need to limit this to only local process plugins.
  // now we filter when there is a watch match. remove the filtering if this is
  // improved.
  m_watch = CreateWatch("*", "*", GetConfiguration().hostname);
  return true;
}

void DirectoryPlugin::OnShutdown() {
  UnregisterCommand(&m_plugin_list_cmd);
  UnregisterPlugin(m_master_handle);
  CloseWatch(m_watch);
}

void DirectoryPlugin::OnWatchMatch(const WatchNotification &notification) {
  // only local plugins will be notified here.

  if(notification.handle != m_watch) {
    return; // self plugin ignored.
  }

  switch(notification.plugin_state) {
  case PluginState::kPluginAvailable: {
    PluginInfo info;
    if(0 != GetPluginInfo(notification.plugin, &info)) {
      PLUGIN_ERROR("problem obtaining plugin info");
      return;
    }

    // we have to filter by process id here.
    if(info.pid == getpid()) {
      PLUGIN_INFO("local plugin available plugin(%p) - %s(%s) pid(%d) %d",
                  notification.plugin, info.name, info.version, info.pid,
                  getpid());
      AddPluginToRegistry(0, notification.plugin, info);
      if(m_is_master) {
        AnnounceToAll(notification.plugin, info);
      } else {
        if(m_master_connection != 0) {
          Publish(notification.plugin, info);
        }
      }
    }
  } break;
  case PluginState::kPluginUnavailable: {
    // check if the plugin is in local plugin list.
    PluginList::iterator it =
        std::find(m_connection_to_plugin_map[0].begin(),
                  m_connection_to_plugin_map[0].end(), notification.plugin);
    if(it != m_connection_to_plugin_map[0].end()) {
      PLUGIN_INFO("local plugin unavailable plugin(%p)", notification.plugin);
      m_connection_to_plugin_map[0].erase(it);
      if(m_is_master) {
        RevokeToAll(notification.plugin);
      } else {
        Suppress(notification.plugin);
      }
    }
  } break;
  }
}

void DirectoryPlugin::OnRecvReady(const ConnectionNotification &notif) {
  SimplePlugin::Recv(
      notif.handle, m_recv_buffer, kRecvBufferSize,
      [&](void *buffer, unsigned int nbytes) {
        ProtobufStream stream(buffer, nbytes);
        directory::Update msg;
        if(!msg.ParseFromIstream(&stream)) {
          PLUGIN_ERROR("failed parsing update message");
          return;
        }

        for(int info_index = 0; info_index < msg.info_size(); ++info_index) {
          const directory::PluginInfo &msg_info = msg.info(info_index);
          PluginInfo info;
          Base::String::strncpy(info.name, msg_info.name().c_str(),
                                kPluginNameMax);
          Base::String::strncpy(info.version, msg_info.version().c_str(),
                                kPluginVersionMax);
          Base::String::strncpy(info.hostname, msg_info.host().c_str(),
                                kPluginHostnameMax);
          info.port = static_cast<u16>(msg_info.port());
          info.pid = msg_info.pid();

          PluginHandle received_handle =
              reinterpret_cast<PluginHandle>(msg_info.handle());
          BASE_ASSERT(received_handle != 0);

          switch(msg.type()) {
          case directory::Update::ANNOUNCE: {
            if(m_is_master) {
              PLUGIN_ERROR("ANNOUNCE message received by master");
              continue;
            }

            PLUGIN_INFO("plugin ctrl ANNOUNCE received. registering locally "
                        "plugin(%p) %s(%s).",
                        received_handle, info.name, info.version);
            PluginHandle registered_handle = RegisterPlugin(&info);
            if(registered_handle != 0) {
              // todo(kstasik): what if it's a duplicate?
              BASE_ASSERT(received_handle == registered_handle);
            }
            AddPluginToRegistry(notif.handle, received_handle, info);
          } break;
          case directory::Update::REVOKE: {
            if(m_is_master) {
              PLUGIN_ERROR("REVOKE message received by master");
              continue;
            }

            PLUGIN_INFO(
                "plugin ctrl REVOKE received. unregistering locally %s(%s)",
                info.name, info.version);
            UnregisterPlugin(received_handle);
          } break;
          case directory::Update::PUBLISH: {
            if(!m_is_master) {
              PLUGIN_ERROR("PUBLISH message received by slave");
              continue;
            }

            PLUGIN_INFO("plugin ctrl PUBLISH received. registering locally "
                        "plugin(%p) %s(%s), broadcasting ANNOUNCE",
                        received_handle, info.name, info.version);
            PluginHandle registered_handle = RegisterPlugin(&info);
            if(registered_handle != 0) {
              // todo(kstasik): what if it's a duplicate?
              BASE_ASSERT(registered_handle == received_handle);
            }
            AddPluginToRegistry(notif.handle, received_handle, info);
            AnnounceToAll(received_handle, info);
          } break;
          case directory::Update::SUPPRESS: {
            if(!m_is_master) {
              PLUGIN_ERROR("SUPPRESS message received by slave");
              continue;
            }

            PLUGIN_INFO("plugin ctrl SUPPRESS received. unregistering locally "
                        "plugin(%p), broadcasting REVOKE",
                        received_handle);
            UnregisterPlugin(received_handle);
            RevokeToAll(received_handle);
          } break;
          }
        }

      });
}

void DirectoryPlugin::OnPluginConnected(
    const ConnectionNotification &notification) {
  // master has other plugins connect.
  // slave does not accept connections.

  if(GetRestConnection() == notification.handle) {
    return; // ignore rest plugin connection.
  }

  if(m_is_master) {
    // slave connected to master.
    PLUGIN_INFO("slave plugin %p connected via %p - announcing all plugins",
                notification.endpoint, notification.handle);
    // todo: add <connection, plugin>
    AnnounceAll(notification.handle);
  } else {
    PLUGIN_WARN("should not have any plugins connect %p via %p",
                notification.endpoint, notification.handle);
    CloseConnection(notification.handle);
  }
}

void DirectoryPlugin::OnConnected(const ConnectionNotification &notification) {
  // master does not connect to anyone
  // slave connects to master.

  if(GetRestConnection() == notification.handle) {
    return; // ignore rest plugin connection.
  }

  if(!m_is_master) {
    if(notification.handle == m_master_connection) {
      // slave connected to master. now, add all local plugins to master
      // dictionary.
      PLUGIN_INFO("slave connected to master. announcing local plugins");
      PublishLocalPlugins();
    } else {
      PLUGIN_WARN("unknown connection %p to %p", notification.handle,
                  notification.endpoint);
      CloseConnection(notification.handle);
    }
  }
}

void DirectoryPlugin::OnDisconnected(
    const ConnectionNotification &notification) {
  if(GetRestConnection() == notification.handle) {
    return; // ignore rest plugin connection.
  }

  if(m_is_master) {
    PLUGIN_INFO("slave disconnected");
    ConnectionToPlugins::const_iterator disconnected =
        m_connection_to_plugin_map.find(notification.handle);
    if(disconnected != m_connection_to_plugin_map.end()) {
      for(ConnectionToPlugins::const_iterator it =
              m_connection_to_plugin_map.begin();
          it != m_connection_to_plugin_map.end(); ++it) {
        // send revoke message to all except the closed connection.
        if(it->first != 0 && it->first != notification.handle) {
          Revoke(it->first, disconnected->second);
        }
      }

      // unregister all plugins related to closed connection.
      for(PluginList::const_iterator pit = disconnected->second.begin();
          pit != disconnected->second.end(); ++pit) {
        UnregisterPlugin(*pit);
      }
      // remove connection info with plugin data.
      m_connection_to_plugin_map.erase(disconnected);
    }
  } else {
    PLUGIN_INFO("master plugin gone");
  }
}

void DirectoryPlugin::OnNotification(const Notification &notif) {
  RestClient::ProcessNotification(notif);
}

bool DirectoryPlugin::RegisterMasterPlugin(const char *name,
                                           const char *version,
                                           const char *hostname, u16 port,
                                           int pid) {
  if(!name || name[0] == 0 || Base::String::strlen(name) >= kPluginNameMax) {
    PLUGIN_ERROR("name not specified or too long");
    return false;
  }
  if(!version || version[0] == 0 ||
     Base::String::strlen(version) >= kPluginVersionMax) {
    PLUGIN_ERROR("version not specified or too long");
    return false;
  }
  if(!hostname || hostname[0] == 0 ||
     Base::String::strlen(hostname) >= kPluginHostnameMax) {
    PLUGIN_ERROR("hostname not specified or too long");
    return false;
  }
  if(port == 0) {
    PLUGIN_ERROR("invalid port %d", port);
    return false;
  }
  if(pid == 0) {
    PLUGIN_ERROR("ivalid pid %d", pid);
    return false;
  }

  memset(&m_master_info, 0, sizeof(m_master_info));
  memcpy(m_master_info.hostname, hostname, kPluginHostnameMax);
  memcpy(m_master_info.name, name, kPluginNameMax);
  memcpy(m_master_info.version, version, kPluginVersionMax);
  m_master_info.pid = pid;
  m_master_info.port = port;

  m_master_handle = RegisterPlugin(&m_master_info);
  if(m_master_handle == 0) {
    PLUGIN_WARN("problem registering master info");
  }

  PLUGIN_INFO("master plugin registered as %p", m_master_handle);

  return m_master_handle != 0;
}

void DirectoryPlugin::OnUpdate(unsigned int dt) {
  if(!m_is_master) {
    if(m_master_connection == 0) {

      PluginInfo info;
      if(0 == GetPluginInfo(m_master_handle, &info)) {
        m_master_connection = Connect(m_master_handle);
        PLUGIN_INFO("slave trying to connect to master with: %p",
                    m_master_connection);
      } else {
        PLUGIN_ERROR("failed to get master plugin info from %p",
                     m_master_handle);
      }
      PLUGIN_INFO("info %s %s", info.name, info.version);
    }
  }
}

void DirectoryPlugin::AnnounceAll(ConnectionHandle conn) {
  // send info about all known plugins. used by master to update connecting
  // slave.
  for(ConnectionToPlugins::const_iterator conn_it =
          m_connection_to_plugin_map.begin();
      conn_it != m_connection_to_plugin_map.end(); ++conn_it) {
    // ConnectionHandle conn_handle = conn_it->first;
    for(PluginList::const_iterator plugin_it = conn_it->second.begin();
        plugin_it != conn_it->second.end(); ++plugin_it) {
      PluginHandle plug_handle = *plugin_it;
      Announce(plug_handle, conn);
    }
  }
}

void DirectoryPlugin::AnnounceToAll(PluginHandle plugin,
                                    const PluginInfo &info) {
  for(ConnectionToPlugins::const_iterator conn_it =
          m_connection_to_plugin_map.begin();
      conn_it != m_connection_to_plugin_map.end(); ++conn_it) {
    ConnectionHandle conn_handle = conn_it->first;
    if(conn_handle != 0) {
      Announce(plugin, info, conn_handle);
    }
  }
}

void DirectoryPlugin::PublishLocalPlugins() {
  ConnectionToPlugins::const_iterator locals =
      m_connection_to_plugin_map.find(0);
  if(locals != m_connection_to_plugin_map.end()) {
    for(PluginList::const_iterator pit = locals->second.begin();
        pit != locals->second.end(); ++pit) {
      Publish(*pit);
    }
  }
}

void DirectoryPlugin::RevokeToAll(PluginHandle plugin) {
  for(ConnectionToPlugins::const_iterator conn_it =
          m_connection_to_plugin_map.begin();
      conn_it != m_connection_to_plugin_map.end(); ++conn_it) {
    ConnectionHandle conn_handle = conn_it->first;
    if(conn_handle != 0) {
      Revoke(plugin, conn_handle);
    }
  }
}

void DirectoryPlugin::Revoke(ConnectionHandle conn, const PluginList &plugins) {
  for(PluginList::const_iterator it = plugins.begin(); it != plugins.end();
      ++it) {

    Revoke(*it, conn);
  }
}

void DirectoryPlugin::Announce(PluginHandle plugin, ConnectionHandle conn) {
  BASE_ASSERT(m_is_master, "only master can announce plugins");
  PluginInfo info;
  if(0 != GetPluginInfo(plugin, &info)) {
    PLUGIN_WARN("problem announcing plugin(%p) to conn(%p) - no info", plugin,
                conn);
    return;
  }

  Announce(plugin, info, conn);
}

void DirectoryPlugin::Announce(PluginHandle plugin, const PluginInfo &info,
                               ConnectionHandle conn) {
  PLUGIN_INFO("plugin ctrl ANNOUNCE sending for plugin(%p) %s(%s)", plugin,
              info.name, info.version);

  directory::Update msg;
  msg.set_type(directory::Update::ANNOUNCE);

  directory::PluginInfo *msg_info = msg.add_info();
  msg_info->set_name(info.name);
  msg_info->set_version(info.version);
  msg_info->set_host(info.hostname);
  msg_info->set_port(info.port);
  msg_info->set_pid(info.pid);
  msg_info->set_handle(reinterpret_cast<s64>(plugin));
  std::string serialized;
  msg.SerializeToString(&serialized);

  int ret = Send(conn, serialized.data(),
                 static_cast<unsigned int>(serialized.length()));

  if(0 != ret) {
    PLUGIN_WARN("problem announcing plugin(%p) to conn(%p) - can't send",
                plugin, conn);
  }
}

void DirectoryPlugin::Publish(PluginHandle plugin) {
  BASE_ASSERT(!m_is_master, "only slave can publish plugins");
  PluginInfo info;
  if(0 != GetPluginInfo(plugin, &info)) {
    PLUGIN_WARN("problem publishing plugin(%p) - no info", plugin);
    return;
  }

  Publish(plugin, info);
}

void DirectoryPlugin::Publish(PluginHandle plugin, const PluginInfo &info) {
  PLUGIN_INFO("plugin ctrl PUBLISH sending for plugin(%p) %s(%s)", plugin,
              info.name, info.version);

  directory::Update msg;
  msg.set_type(directory::Update::PUBLISH);

  directory::PluginInfo *msg_info = msg.add_info();
  msg_info->set_name(info.name);
  msg_info->set_version(info.version);
  msg_info->set_host(info.hostname);
  msg_info->set_port(info.port);
  msg_info->set_pid(info.pid);
  msg_info->set_handle(reinterpret_cast<s64>(plugin));
  std::string serialized;
  msg.SerializeToString(&serialized);

  int ret = Send(m_master_connection, serialized.data(),
                 static_cast<unsigned int>(serialized.length()));

  if(0 != ret) {
    PLUGIN_WARN("problem publishing plugin(%p) to conn(%p) - can't send",
                plugin, m_master_connection);
  }
}

void DirectoryPlugin::Revoke(PluginHandle plugin, ConnectionHandle conn) {
  PLUGIN_INFO("plugin ctrl REVOKE sending for plugin(%p)", plugin);

  directory::Update msg;
  msg.set_type(directory::Update::REVOKE);

  directory::PluginInfo *msg_info = msg.add_info();
  msg_info->set_handle(reinterpret_cast<s64>(plugin));
  std::string serialized;
  msg.SerializeToString(&serialized);

  int ret = Send(conn, serialized.data(),
                 static_cast<unsigned int>(serialized.length()));

  if(0 != ret) {
    PLUGIN_WARN("problem revoking plugin(%p) to conn(%p) - can't send", plugin,
                conn);
  }
}

void DirectoryPlugin::Suppress(PluginHandle plugin) {
  PLUGIN_INFO("plugin ctrl SUPPRESS sending for plugin(%p)", plugin);

  directory::Update msg;
  msg.set_type(directory::Update::SUPPRESS);

  directory::PluginInfo *msg_info = msg.add_info();
  msg_info->set_handle(reinterpret_cast<s64>(plugin));
  std::string serialized;
  msg.SerializeToString(&serialized);

  int ret = Send(m_master_connection, serialized.data(),
                 static_cast<unsigned int>(serialized.length()));

  if(0 != ret) {
    PLUGIN_WARN("problem suppressing plugin(%p) to conn(%p) - can't send",
                plugin, m_master_connection);
  }
}

void DirectoryPlugin::AddPluginToRegistry(ConnectionHandle conn,
                                          PluginHandle plugin,
                                          const PluginInfo &info) {
  // for some reason local plugins were added to local list and some connection.
  // need to investigate why and remove this 'getpid' check.
  if(info.pid == getpid()) {
    conn = 0; // adding a local plugin.
  }
  // todo(kstasik): fix this.
  if(conn == 0 && info.pid != getpid()) {
    return;
  }

  // all entries need to be unique.
  PluginList::iterator it =
      std::find(m_connection_to_plugin_map[conn].begin(),
                m_connection_to_plugin_map[conn].end(), plugin);
  if(it == m_connection_to_plugin_map[conn].end()) {
    m_connection_to_plugin_map[conn].push_back(plugin);
  }
}

void DirectoryPlugin::WritePluginList(std::string *data) {

  JsonWriter writer(*data);

  writer.Write("master", m_is_master);

  std::vector<PluginInfo> plugins;

  for(ConnectionToPlugins::const_iterator conn_it =
          m_connection_to_plugin_map.begin();
      conn_it != m_connection_to_plugin_map.end(); ++conn_it) {
    // ConnectionHandle conn_handle = conn_it->first;
    for(PluginList::const_iterator plugin_it = conn_it->second.begin();
        plugin_it != conn_it->second.end(); ++plugin_it) {
      PluginHandle plug_handle = *plugin_it;

      PluginInfo info;
      if(0 == GetPluginInfo(plug_handle, &info)) {
        PLUGIN_INFO("%p plugin of conn(%p) plugin(%p) is %s %s %d %d", this,
                    conn_it->first, plug_handle, info.name, info.version,
                    info.port, info.pid);
        plugins.push_back(info);
      } else {
        PLUGIN_ERROR("failed to get master plugin info from %p",
                     m_master_handle);
      }
    }
  }

  writer.Write("plugins", plugins);
}

} // namespace Link
