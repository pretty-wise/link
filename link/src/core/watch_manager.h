/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "base/core/types.h"
#include "base/threading/mutex.h"
#include "link/link.h"
#include "plugin_directory.h"

#include <string>
#include <vector>

namespace Link {

class WatchManager {
public:
  WatchManager();
  ~WatchManager();

  // Creates a watch for a given name & version filter.
  WatchHandle Create(const char *name_filter, const char *version_filter,
                     const char *hostname_filter);

  // Removes watch by handle.
  void Remove(WatchHandle handle);

  // Returns a list of watches matching given name & version.
  void Filter(const char *name, const char *version, const char *hostname,
              std::vector<WatchHandle> &handles) const;

  Base::Mutex &AccessLock() { return m_mutex; }

private:
  WatchHandle Generate();

  struct WatchData {
    std::string name_filter;
    std::string version_filter;
    std::string hostname_filter;
    WatchHandle handle;
  };

  typedef std::vector<WatchData> WatchList;

  WatchList m_watches;
  mutable Base::Mutex m_mutex;
};

} // namespace Link
