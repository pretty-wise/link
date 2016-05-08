/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "tcp_connect.h"

const char* SimplePlugin::Name = "tcp_connect";
const char* SimplePlugin::Version = "1.0";

SimplePlugin* SimplePlugin::CreatePlugin() {
	return new TcpConnect();
}

void SimplePlugin::DestroyPlugin(SimplePlugin* plugin) {
	delete plugin;
}
