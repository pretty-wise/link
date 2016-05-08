/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "plugin_interface.h"
#include "plugin/rest/rest_client.h"
#include "plugin/rest/rest_command.h"
#include "link/plugin_log.h"

const char* SimplePlugin::Name = "rest_client_plugin_test";
const char* SimplePlugin::Version = "1.0";

class RestCommandTest : public Link::RestCommand {
public:
	const char* Name() const { return "rest-command-test"; }

	virtual bool OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data) {
													 *response_data = "restponse";
													 return true;
												 }
};

class RestClientPluginTest : public SimplePlugin, Link::RestClient {
public:
	enum CONSTANT {
		kUpdateDeltaMs = 100
	};
	RestClientPluginTest() : SimplePlugin(kUpdateDeltaMs){}
	bool OnStartup(const char* config, streamsize nbytes);
	void OnShutdown();
	void OnNotification(const Notification& notification);
private:
	RestCommandTest m_command;
};


bool RestClientPluginTest::OnStartup(const char* config, streamsize nbytes) {
	bool result = RegisterCommand(&m_command);
	if(!result) {
		PLUGIN_INFO("failed to register %s", m_command.Name());
	}
	return result;
}

void RestClientPluginTest::OnShutdown() {
	UnregisterCommand(&m_command);
}

void RestClientPluginTest::OnNotification(const Notification& notification) {
	Link::RestClient::ProcessNotification(notification);
}

//
// Framework binding.
//

SimplePlugin* SimplePlugin::CreatePlugin() {
	return new RestClientPluginTest();
}

void SimplePlugin::DestroyPlugin(SimplePlugin* plugin) {
	delete plugin;
}
