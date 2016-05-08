/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin_manager.h"
#include "plugin.h"

#include "log.h"

namespace Link {

PluginManager::PluginManager(PluginDirectory &directory)
    : m_plugin_directory(directory) {}

bool PluginManager::Load(const char *plugin_path, const char *config,
                         streamsize nbytes) {
  Plugin *plugin = new Plugin(*this, m_plugin_directory);

  if(!plugin->Load(plugin_path, config, nbytes)) {
    plugin->Unload();
    delete plugin;

    LINK_WARN("failed loading %s!", plugin_path);
    return false;
  }

  LINK_INFO("plugin %s(%s) loaded.", plugin->Name(), plugin->Version());

  if(!plugin->IsCompatible()) {
    LINK_ERROR("plugin %s is incompatible. Expected version %d", plugin_path,
               LINK_INTERFACE_VERSION);
    plugin->Unload();
    delete plugin;
    return false;
  }

  m_plugins.push_back(plugin);

  return true;
}

void PluginManager::UnloadAll() {
  for(PluginList::iterator it = m_plugins.begin(); it != m_plugins.end();
      ++it) {
    std::string plugin_name = (*it)->Name();
    std::string plugin_version = (*it)->Version();
    (*it)->Unload();
    LINK_INFO("plugin %s(%s) unloaded.", plugin_name.c_str(),
              plugin_version.c_str());
    delete *it;
  }
  m_plugins.clear();
}

void PluginManager::ReloadAll() {
  for(PluginList::iterator it = m_plugins.begin(); it != m_plugins.end();
      ++it) {
  }
}

void PluginManager::StartAll(const LinkConfiguration &config) {
  for(PluginList::iterator it = m_plugins.begin(); it != m_plugins.end();
      ++it) {
    LINK_INFO("starting %s(%s).", (*it)->Name(), (*it)->Version());
    bool started = (*it)->Start(config);
    LINK_INFO("plugin %s(%s) %s.", (*it)->Name(), (*it)->Version(),
              started ? "started" : "failed to start");
  }
}

void PluginManager::StopAll() {
  for(PluginList::iterator it = m_plugins.begin(); it != m_plugins.end();
      ++it) {
    LINK_INFO("stopping %s(%s).", (*it)->Name(), (*it)->Version());
    (*it)->Stop();
    LINK_INFO("stopped %s(%s).", (*it)->Name(), (*it)->Version());
  }
}

Plugin *PluginManager::Find(PluginHandle handle) {
  for(PluginList::iterator it = m_plugins.begin(); it != m_plugins.end();
      ++it) {
    if((*it)->GetHandle() == handle) {
      return *it;
    }
  }

  return nullptr;
}

} // namespace Link
