/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "gtest/gtest.h"

#include "base/core/macro.h"

#include "plugin_directory.h"
#include "plugin_manager.h"

#include <unistd.h>

TEST(Plugin, Loading) {
	Link::PluginDirectory directory;
	Link::PluginManager plugin_manager(directory);

	char cwd[128];
	getcwd(cwd, sizeof(cwd));
	BASE_LOG("cwd: %s\n", cwd);

	//const char* kPluginPath = "../../../lib/plugin/libFramework.dylib";
	//const char* kPluginPath = "../../../lib/plugin/libRawPlugin.dylib";

	//bool loaded = plugin_manager.Load( kPluginPath );
	//ASSERT_TRUE(loaded);

	LinkConfiguration config;

	plugin_manager.StartAll(config);
	plugin_manager.StopAll();

	plugin_manager.StartAll(config);
	plugin_manager.StopAll();

	plugin_manager.UnloadAll();
}
