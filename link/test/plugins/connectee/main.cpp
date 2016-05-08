/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "connectee.h"

const char* SimplePlugin::Name = "connectee";
const char* SimplePlugin::Version = "1.0";

SimplePlugin* SimplePlugin::CreatePlugin() {
	return new Connectee();
}

void SimplePlugin::DestroyPlugin(SimplePlugin* plugin) {
	delete plugin;
}
