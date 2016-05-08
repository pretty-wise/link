/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */

#include "link/link.h"
#include "plugin_interface.h"
#include "base/threading/thread.h"
#include "link/plugin_log.h"
#include "base/core/assert.h"
#include "base/core/time_utils.h"

#include <stdlib.h>

SimplePlugin *g_plugin = 0;

Base::Thread g_thread;

bool g_running = true;

const char *GetName() { return SimplePlugin::Name; }
const char *GetVersion() { return SimplePlugin::Version; }

int PluginLoop(void *) {
  PLUGIN_INFO("thread started");
  g_plugin->OnThreadEntry();

  unsigned int last_time = Base::Time::GetTimeMs();
  while(g_running) {
    Notification notif;
    if(GetNotification(&notif, g_plugin->IdleDt())) {
      // PLUGIN_INFO("notification received: %d.", notif.type);
      g_plugin->OnNotification(notif);
      switch(notif.type) {
      case kShutdown:
        g_plugin->OnShutdown(notif.content.shutdown);
        g_running = false;
        break;
      case kWatch: {
        PluginInfo info;
        if(0 == GetPluginInfo(notif.content.watch.plugin, &info)) {
          PLUGIN_INFO("watch plugin match %s(%s) for %p", info.name,
                      info.version, notif.content.watch.handle);
        } else {
          PLUGIN_WARN("unknown plugin match for %p",
                      notif.content.connection.endpoint);
        }
        g_plugin->OnWatchMatch(notif.content.watch);
        break;
      }
      case kConfigChanged:
        g_plugin->OnConfigChange(notif.content.config);
        break;
      case kEstablished: {
        PluginInfo info;
        if(0 == GetPluginInfo(notif.content.connection.endpoint, &info)) {
          PLUGIN_INFO("connected to %s(%s)", info.name, info.version);
        } else {
          PLUGIN_WARN("connected to unknown plugin %p",
                      notif.content.connection.endpoint);
        }
        g_plugin->OnConnected(notif.content.connection);
        break;
      }
      case kConnected: {
        PluginInfo info;
        if(0 == GetPluginInfo(notif.content.connection.endpoint, &info)) {
          PLUGIN_INFO("plugin connected %s(%s)", info.name, info.version);
        } else {
          PLUGIN_WARN("unknown plugin connected %p",
                      notif.content.connection.endpoint);
        }
        g_plugin->OnPluginConnected(notif.content.connection);
        break;
      }
      case kDisconnected:
        PluginInfo info;
        if(0 == GetPluginInfo(notif.content.connection.endpoint, &info)) {
          PLUGIN_INFO("plugin disconnected %s(%s)", info.name, info.version);
        } else {
          PLUGIN_WARN("unknown plugin disconnected %p",
                      notif.content.connection.endpoint);
        }
        g_plugin->OnDisconnected(notif.content.connection);
        break;
      case kRecvReady:
        g_plugin->OnRecvReady(notif.content.connection);
      default:
        break;
      }
    }

    unsigned int now = Base::Time::GetTimeMs();
    unsigned int dt = now - last_time;
    g_plugin->OnUpdate(dt);
    last_time = now;
  }

  g_plugin->OnThreadExit();
  PLUGIN_INFO("thread exiting");
  return 0;
}

int Startup(const char *config, streamsize nbytes) {
  PLUGIN_INFO("startup");
  g_plugin = SimplePlugin::CreatePlugin();

  if(!g_plugin) {
    PLUGIN_INFO("creation failed");
    return -1;
  }

  if(!g_plugin->OnStartup(config, nbytes)) {
    PLUGIN_ERROR("startup failed");
    SimplePlugin::DestroyPlugin(g_plugin);
    g_plugin = nullptr;
    return -2;
  }

  g_running = true;
  g_running = g_thread.Initialize(&PluginLoop, nullptr);

  if(!g_running) {
    PLUGIN_ERROR("thread start failed");
  }

  return g_running ? 0 : -1;
}

void Shutdown() {
  g_thread.Join();
  g_plugin->OnShutdown();
  SimplePlugin::DestroyPlugin(g_plugin);
}

void SimplePlugin::Recv(ConnectionHandle conn, void *buffer,
                        unsigned int nbytes,
                        std::function<void(void *, unsigned int)> func) {
  BASE_ASSERT(conn != 0);
  int nrecv = 0;
  int data_received = 0;
  void *work_buffer = buffer;
  unsigned int work_buffer_size = nbytes;

  int total_available = GetTotalRecvAvailable(conn);

  static const unsigned int kBufferTooSmall = -2;

  // only process limited amount of data in this call
  while(data_received < total_available) {
    do {
      if(nrecv == kBufferTooSmall) {
        // previous recv call failed because buffer was too small. allocating
        // bigger buffer.
        work_buffer_size = GetRecvAvailable(conn);
        work_buffer = malloc(work_buffer_size);
        PLUGIN_INFO("recv failed - buffer too small. %d expecting %d.",
                    work_buffer_size, work_buffer_size);
      }

      //			((char*)work_buffer)[work_buffer_size-1] = 0;

      nrecv = ::Recv(conn, work_buffer, work_buffer_size /*-1*/);

      if(0 < nrecv) {
        func(work_buffer, nrecv);
        data_received += nrecv;
      }

      if(work_buffer != buffer) {
        // free temp buffer and switch back to input buffer.
        free(work_buffer);
        work_buffer = buffer;
        work_buffer_size = nbytes;
      }

    } while(nrecv == kBufferTooSmall); // if buffer was too small try receiving
                                       // with bigger buffer.
  }
}
