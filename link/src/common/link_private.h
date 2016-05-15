/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "link/link.h"

#include <stdarg.h>

struct LinkInterface;

// Version of the link interface that allows
// to detect interface compatibility.
#define LINK_INTERFACE_VERSION 1

// Link exposed functions.
typedef int (*GetNotificationPtr)(void *context, struct Notification *notif,
                                  int timeout_ms);
typedef WatchHandle (*CreatePluginWatchPtr)(void *context,
                                            const char *name_filter,
                                            const char *version_filter,
                                            const char *hostname_filter);
typedef ConnectionHandle (*ConnectPtr)(void *context, PluginHandle handle);
typedef int (*SendPtr)(void *context, ConnectionHandle connection,
                       const void *bytes, streamsize count);
typedef int (*GetInfoPtr)(void *context, PluginHandle handle,
                          struct PluginInfo *info);
typedef void (*LogWritePtr)(void *context, const char *file, int line,
                            int category, const char *format, va_list args);
typedef unsigned int (*GetTotalRecvAvailablePtr)(void *context,
                                                 ConnectionHandle conn);
typedef unsigned int (*GetRecvAvailablePtr)(void *context,
                                            ConnectionHandle conn);
typedef int (*RecvPtr)(void *context, ConnectionHandle handle, void *buffer,
                       unsigned int nbytes);
typedef void (*CloseConnectionPtr)(void *context, ConnectionHandle conn);
typedef void (*CloseWatchPtr)(void *context, WatchHandle watch);
typedef PluginHandle (*RegisterPluginPtr)(void *context,
                                          struct PluginInfo *info);
typedef void (*UnregisterPluginPtr)(void *context, PluginHandle handle);
typedef struct LinkConfiguration (*GetConfigurationPtr)(void *context);

// Interface that exposes link functions to a plugin.
struct LinkInterface {
  GetNotificationPtr GetNotification;
  CreatePluginWatchPtr CreatePluginWatch;
  ConnectPtr Connect;
  SendPtr Send;
  GetInfoPtr GetInfo;
  LogWritePtr LogWrite;
  GetTotalRecvAvailablePtr GetTotalRecvAvailable;
  GetRecvAvailablePtr GetRecvAvailable;
  RecvPtr Recv;
  CloseConnectionPtr CloseConnection;
  CloseWatchPtr CloseWatch;
  RegisterPluginPtr RegisterPlugin;
  UnregisterPluginPtr UnregisterPlugin;
  GetConfigurationPtr GetConfiguration;

  void *context;
};
