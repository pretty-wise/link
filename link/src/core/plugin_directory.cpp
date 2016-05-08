/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin_directory.h"
#include "log.h"

#include "base/math/crc.h"
#include "base/core/str.h"
#include "base/core/assert.h"

#include <string.h>

namespace Link {

PluginDirectory::PluginDirectory() : m_listeners_head(nullptr) {
  m_mutex.Initialize();
}

PluginDirectory::~PluginDirectory() { m_mutex.Terminate(); }

bool PluginDirectory::Register(PluginHandle handle, const PluginInfo &info) {
  Base::MutexAutoLock lock(m_mutex);
  PluginList::iterator it = m_registered.find(handle);
  if(it != m_registered.end()) {
    // duplicate
    LINK_INFO("could not register a plugin %s (%s) - duplicate.", info.name,
              info.version);
    return false;
  }
  m_registered.insert(std::make_pair(handle, info));

  LINK_INFO("plugin registration succeeded. name: %s, version: %s, host: "
            "%s:%d, pid: %d, handle: %p",
            info.name, info.version, info.hostname, info.port, info.pid,
            handle);
  NotifyAvailable(handle, info);
  return true;
}

void PluginDirectory::Unregister(PluginHandle handle) {
  Base::MutexAutoLock lock(m_mutex);

  PluginList::iterator it = m_registered.find(handle);
  if(it != m_registered.end()) {
    PluginInfo info;
    bool res = GetInfo(handle, info);

    if(!res) {
      LINK_CRITICAL("failed unregistering plugin - info not found.");
    } else {
      m_registered.erase(it);
      LINK_INFO("plugin unregistered. name: %s, version: %s, host: %s:%d, pid: "
                "%d, handle %p",
                info.name, info.version, info.hostname, info.port, info.pid,
                handle);
      NotifyUnavailable(handle, info);
    }
  }
}

bool PluginDirectory::GetInfo(PluginHandle handle, PluginInfo &info) {
  Base::MutexAutoLock lock(m_mutex);
  PluginList::iterator it = m_registered.find(handle);

  if(it == m_registered.end()) {
    return false;
  }
  strncpy((char *)info.name, it->second.name, kPluginNameMax);
  strncpy((char *)info.version, it->second.version, kPluginVersionMax);
  info.pid = it->second.pid;
  info.port = it->second.port;
  strncpy((char *)info.hostname, it->second.hostname, kPluginHostnameMax);
  return true;
}

void PluginDirectory::AddListener(PluginDirectoryListener *listener) {
  Base::MutexAutoLock lock(m_mutex);
  listener->m_prev = nullptr;

  if(!m_listeners_head) {
    m_listeners_head = listener;
    listener->m_next = nullptr;
  } else {
    // insert front.
    listener->m_next = m_listeners_head;
    m_listeners_head->m_prev = listener;
    m_listeners_head = listener;
  }

  // notify about already registered plugins.
  for(PluginList::iterator it = m_registered.begin(); it != m_registered.end();
      ++it) {
    listener->OnPluginAvailable(it->first, it->second);
  }
}

void PluginDirectory::RemoveListener(PluginDirectoryListener *listener) {
  Base::MutexAutoLock lock(m_mutex);
  if(listener->m_prev) {
    listener->m_prev->m_next = listener->m_next;
  }

  if(listener->m_next) {
    listener->m_next = listener->m_prev;
  }

  if(m_listeners_head == listener) {
    m_listeners_head = listener->m_next;
  }

  listener->m_prev = nullptr;
  listener->m_next = nullptr;
}

PluginHandle PluginDirectory::GenerateHandle(const PluginInfo &info) {
  BASE_ASSERT(info.name[0] != 0 &&
                  Base::String::strlen(info.name) < kPluginNameMax,
              "name not specified");
  BASE_ASSERT(info.version[0] != 0 &&
                  Base::String::strlen(info.version) < kPluginVersionMax,
              "version not specified");

  // Base::MutexAutoLock lock(m_mutex);
  int handle = Base::Math::crc(info.name, Base::String::strlen(info.name));
  handle += Base::Math::crc(info.version, Base::String::strlen(info.version));
  handle += info.pid;
  return reinterpret_cast<PluginHandle>(handle);
}

void PluginDirectory::ForPlugin(
    const char *name_filter, const char *version_filter,
    const char *hostname_filter,
    std::function<void(PluginHandle handle, const PluginInfo &info)> func) {
  for(PluginList::iterator it = m_registered.begin(); it != m_registered.end();
      ++it) {
    if(PluginDirectory::Match(it->second.name, name_filter) &&
       PluginDirectory::Match(it->second.version, version_filter) &&
       PluginDirectory::Match(it->second.hostname, hostname_filter)) {
      func(it->first, it->second);
    }
  }
}

void PluginDirectory::NotifyAvailable(PluginHandle plugin,
                                      const PluginInfo &info) {
  PluginDirectoryListener *listener = m_listeners_head;
  while(listener) {
    listener->OnPluginAvailable(plugin, info);
    listener = listener->m_next;
  }
}

void PluginDirectory::NotifyUnavailable(PluginHandle plugin,
                                        const PluginInfo &info) {
  PluginDirectoryListener *listener = m_listeners_head;
  while(listener) {
    listener->OnPluginUnavailable(plugin, info);
    listener = listener->m_next;
  }
}

bool PluginDirectory::Initialize(PluginInfo &info, const char *name,
                                 const char *version, const char *hostname,
                                 u16 port, int pid) {
  BASE_ASSERT(name);
  BASE_ASSERT(version);
  BASE_ASSERT(hostname);

  if(!name || !version || !hostname)
    return false;

  if(strlen(name) > kPluginNameMax || strlen(version) > kPluginVersionMax ||
     strlen(hostname) > kPluginHostnameMax)
    return false;

  strncpy(info.name, name, kPluginNameMax);
  strncpy(info.version, version, kPluginVersionMax);
  strncpy(info.hostname, hostname, kPluginHostnameMax);
  info.port = port;
  info.pid = pid;

  return true;
}

void PluginDirectory::Copy(PluginInfo &dest, const PluginInfo &src) {
  strncpy(dest.name, src.name, kPluginNameMax);
  strncpy(dest.version, src.version, kPluginVersionMax);
  strncpy(dest.hostname, src.hostname, kPluginHostnameMax);

  dest.port = src.port;
  dest.pid = src.pid;
}

bool PluginDirectory::Equal(const PluginInfo &a, const PluginInfo &b) {
  return strncmp(a.name, b.name, kPluginNameMax) == 0 &&
         strncmp(a.version, b.version, kPluginVersionMax) == 0 &&
         strncmp(a.hostname, b.hostname, kPluginHostnameMax) == 0 &&
         a.port == b.port && a.pid == b.pid;
}

bool PluginDirectory::Match(const char *str, const char *filter) {
  int i, star;

new_segment:

  star = 0;
  if(*filter == '*') {
    star = 1;
    do {
      filter++;
    } while(*filter == '*'); /* enddo */
  }                          /* endif */

test_match:

  for(i = 0; filter[i] && (filter[i] != '*'); i++) {
    if(str[i] != filter[i]) {
      if(!str[i])
        return false;
      if((filter[i] == '?') && (str[i] != '.'))
        continue;
      if(!star)
        return false;
      str++;
      goto test_match;
    }
  }
  if(filter[i] == '*') {
    str += i;
    filter += i;
    goto new_segment;
  }
  if(!str[i])
    return true;
  if(i && filter[i - 1] == '*')
    return true;
  if(!star)
    return false;
  str++;
  goto test_match;
}

} // namespace Link
