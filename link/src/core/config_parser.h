/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "base/core/types.h"
#include "tinyxml2.h"
#include <string>

namespace Link {

class ConfigParser {
public:
  ConfigParser() : m_runtime(0), m_plugin_count(0) {}
  bool FromData(const char *data, streamsize nbytes);
  int GetRuntime() const { return m_runtime; }
  int GetPluginCount() const { return m_plugin_count; }
  const char *GetHostname() const {
    return m_hostname.empty() ? NULL : m_hostname.c_str();
  }
  const char *GetPluginPath(int index) const;

  // @return If buffer is null, the function returns bytes needed
  // to store the content of configuration. Otherwise the number of bytes
  // written.
  streamsize GetConfig(int index, void *buffer, streamsize buffer_size) const;

private:
  tinyxml2::XMLDocument m_doc;
  int m_runtime;
  int m_plugin_count;
  std::string m_hostname;
};

} // namespace Link
