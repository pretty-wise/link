/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "base/core/types.h"
#include "base/threading/mutex.h"
#include "link/link.h"

#include <map>
#include <functional>

namespace Link {

class PluginDirectoryListener {
public:
  virtual void OnPluginAvailable(PluginHandle plugin,
                                 const PluginInfo &info) = 0;
  virtual void OnPluginUnavailable(PluginHandle plugin,
                                   const PluginInfo &info) = 0;

  friend class PluginDirectory; // dlinked list.
private:
  PluginDirectoryListener *m_next;
  PluginDirectoryListener *m_prev;
};

// Holds information about all known plugins (local & remote).
class PluginDirectory {
public:
  PluginDirectory();
  ~PluginDirectory();

  bool Register(PluginHandle handle, const PluginInfo &info);
  void Unregister(PluginHandle handle);
  bool GetInfo(PluginHandle handle, PluginInfo &info);

  void AddListener(PluginDirectoryListener *listener);
  void RemoveListener(PluginDirectoryListener *listener);

  void ForPlugin(
      const char *name_filter, const char *version_filter,
      const char *hostname_filter,
      std::function<void(PluginHandle handle, const PluginInfo &info)> func);

  static PluginHandle GenerateHandle(const PluginInfo &info);
  Base::Mutex &AccessLock() { return m_mutex; }

  static bool Match(const char *value, const char *filter);

  static bool Initialize(PluginInfo &info, const char *name,
                         const char *version, const char *hostname, u16 port,
                         int pid);
  static void Copy(PluginInfo &dest, const PluginInfo &src);
  static bool Equal(const PluginInfo &a, const PluginInfo &b);

private:
  void NotifyAvailable(PluginHandle plugin, const PluginInfo &info);
  void NotifyUnavailable(PluginHandle plugin, const PluginInfo &info);

  typedef std::map<PluginHandle, PluginInfo> PluginList;

  PluginDirectoryListener *m_listeners_head;

  PluginList m_registered;
  Base::Mutex m_mutex;
};

} // namespace Link
