/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "config_parser.h"
#include "log.h"
#include "base/core/str.h"

namespace Link {

bool ConfigParser::FromData(const char* data, streamsize nbytes) {
	tinyxml2::XMLError error = m_doc.Parse(data, nbytes);

	printf("parsing %s\n", data);
	if(error != tinyxml2::XML_SUCCESS) {
		LINK_CRITICAL("failed to parse config xml %s (%s). %s", m_doc.ErrorName(), m_doc.GetErrorStr1(), m_doc.GetErrorStr2());
		return false;
	}

	const tinyxml2::XMLElement* plugins_node = m_doc.FirstChildElement("link");

	if(!plugins_node) {
		LINK_CRITICAL("no 'link' node for parsing");
		return false;
	}

	m_runtime = plugins_node->IntAttribute("runtime");
	const char* hostname = plugins_node->Attribute("hostname");
	if(hostname) {
		m_hostname = hostname;
	}
	
	const tinyxml2::XMLElement* plugin_node = plugins_node->FirstChildElement("plugin");

	while(plugin_node) {

		bool has_path = plugin_node->Attribute("path") != nullptr;

		if(!has_path) { 
			m_plugin_count = 0;
			return false;
		}

		m_plugin_count++;
		plugin_node = plugin_node->NextSiblingElement("plugin");
	}

	return m_plugin_count > 0;
}

const tinyxml2::XMLElement* GetPluginNodeAt(const tinyxml2::XMLDocument& doc, int index) {
	const tinyxml2::XMLElement* plugins_node = doc.FirstChildElement("link");
	const tinyxml2::XMLElement* plugin = plugins_node->FirstChildElement("plugin");

	int counter = 0;

	while(counter++ < index) {
		plugin = plugin->NextSiblingElement("plugin");
	}
	return plugin;
}

const char* ConfigParser::GetPluginPath(int index) const {
	if(index < m_plugin_count) {
		const tinyxml2::XMLElement* plugin = GetPluginNodeAt(m_doc, index);
		return plugin->Attribute("path");
	}
	return nullptr;
}

streamsize ConfigParser::GetConfig(int index, void* buffer, streamsize nbytes) const {
	if(index < m_plugin_count) {
		const tinyxml2::XMLElement* plugin = GetPluginNodeAt(m_doc, index);

		tinyxml2::XMLPrinter printer;

		if(plugin->FirstChildElement("config") && plugin->FirstChildElement("config")->FirstChild()) {
			plugin->FirstChildElement("config")->FirstChild()->Accept(&printer);
			const char* result = printer.CStr();
			streamsize config_length = Base::String::strlen(result);

			if(buffer == nullptr) {
				return config_length;
			} else {
				streamsize copy_bytes = nbytes < config_length ? nbytes : config_length;
				memcpy(buffer, result, copy_bytes);
				return copy_bytes;
			}
		}
	}
	return 0;
}

} // namespace Link
