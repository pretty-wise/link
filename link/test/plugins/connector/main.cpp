/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "connector.h"

const char* SimplePlugin::Name = "connector";
const char* SimplePlugin::Version = "1.0";

SimplePlugin* SimplePlugin::CreatePlugin() {
	return new Connector();
}

void SimplePlugin::DestroyPlugin(SimplePlugin* plugin) {
	delete plugin;
}
