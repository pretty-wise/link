/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */

#include "plugin_interface.h"
#include "link/link.h"

const char* SimplePlugin::Name = "MyPlugin";
const char* SimplePlugin::Version = "1.0";

class MyPlugin : public SimplePlugin {
public:
	MyPlugin() : SimplePlugin(1000){}
	bool OnStartup(const char* config, streamsize nbytes) { return true; }
	void OnShutdown(){}
private:
};


SimplePlugin* SimplePlugin::CreatePlugin() {
	return new MyPlugin();
}

void SimplePlugin::DestroyPlugin(SimplePlugin* plugin) {
	delete plugin;
}
