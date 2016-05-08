/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#pragma once
#include "base/core/types.h"
#include "link/link.h"

#include <vector>
#include <functional>

namespace Link {

class Plugin;
class PluginDirectory;

// Holds all the plugins related to application.
// TODO: locking? (Find, Reload)
class PluginManager
{
public:
		PluginManager(PluginDirectory& directory);

		bool Load(const char* module_path, const char* config, streamsize nbytes);
		void UnloadAll();
		void ReloadAll();

		void StartAll(const LinkConfiguration& config);
		void StopAll();

	Plugin* Find(PluginHandle handle);

private:
		typedef std::vector<Plugin*> PluginList;

		PluginList m_plugins;
		PluginDirectory& m_plugin_directory;
};

} // namespaca Link
