#include "link/link.h"
#include "link_private.h"

static struct LinkInterface s_interface;

/// If any of those symbols cannot be found by core, that means you don't
/// call any of those functions from your plugin, which makes compiler
/// strip those from shared object.

int GetInterfaceVersion() { return LINK_INTERFACE_VERSION; }

int StartupPlugin(struct LinkInterface *iface, const char *config,
                  streamsize nbytes) {
  s_interface = *iface;
  return Startup(config, nbytes);
}

struct LinkConfiguration GetConfiguration() {
  return s_interface.GetConfiguration(s_interface.context);
}

int GetNotification(struct Notification *notif, int timeout_ms) {
  return s_interface.GetNotification(s_interface.context, notif, timeout_ms);
}

WatchHandle CreateWatch(const char *name_match, const char *version_match,
                        const char *hostname_match) {
  return s_interface.CreatePluginWatch(
      s_interface.context, name_match ? name_match : "",
      version_match ? version_match : "", hostname_match ? hostname_match : "");
}

int GetPluginInfo(PluginHandle plugin, struct PluginInfo *info) {
  return s_interface.GetInfo(s_interface.context, plugin, info);
}

ConnectionHandle Connect(PluginHandle handle) {
  return s_interface.Connect(s_interface.context, handle);
}

int Send(ConnectionHandle conn, const void *bytes, streamsize count) {
  return s_interface.Send(s_interface.context, conn, bytes, count);
}

unsigned int GetTotalRecvAvailable(ConnectionHandle conn) {
  return s_interface.GetTotalRecvAvailable(s_interface.context, conn);
}

unsigned int GetRecvAvailable(ConnectionHandle conn) {
  return s_interface.GetRecvAvailable(s_interface.context, conn);
}

int Recv(ConnectionHandle handle, void *buffer, unsigned int nbytes) {
  return s_interface.Recv(s_interface.context, handle, buffer, nbytes);
}

void CloseConnection(ConnectionHandle conn) {
  s_interface.CloseConnection(s_interface.context, conn);
}

void CloseWatch(WatchHandle watch) {
  s_interface.CloseWatch(s_interface.context, watch);
}

PluginHandle RegisterPlugin(struct PluginInfo *info) {
  return s_interface.RegisterPlugin(s_interface.context, info);
}

void UnregisterPlugin(PluginHandle handle) {
  s_interface.UnregisterPlugin(s_interface.context, handle);
}

void LogWritePlugin(const char *file, int line, int category,
                    const char *format, ...) {
  va_list args;
  va_start(args, format);
  s_interface.LogWrite(s_interface.context, file, line, category, format, args);
  va_end(args);
}
