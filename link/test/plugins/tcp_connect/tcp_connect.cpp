/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "tcp_connect.h"
#include "link/link.h"
#include "link/plugin_log.h"

#include "base/core/time_utils.h"
#include "common/json/json_reader.h"
#include "common/json/json_writer.h"

bool RegisterCmd::OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data) {
	PLUGIN_INFO("register command: %s (%s).", query_string.c_str(), post_data.c_str());

	Link::JsonReader reader;
	if(!reader.Parse(post_data.c_str(), post_data.length())) {
		*response_data = "\"not a valid json\"";
		return true;
	}

	PluginInfo info;
	std::string hostname, plugin_name, plugin_version;

	// read values from json.
	if(!reader.Read("hostname", &hostname)) {
		*response_data = "\"hostname not found\"";
		return true;
	}

	if(!reader.Read("port", &info.port)) {
		*response_data = "\"port not found\"";
		return true;
	}

	if(!reader.Read("name", &plugin_name)) {
		*response_data = "\"name not found\"";
		return true;
	}

	if(!reader.Read("version", &plugin_version)) {
		*response_data = "\"version not found\"";
		return true;
	}

	if(!reader.Read("pid", &info.pid)) {
		*response_data = "\"pid not found\"";
		return true;
	}

	// copy obtained values.
	Base::String::strncpy(info.hostname, hostname.c_str(), kPluginHostnameMax);
	Base::String::strncpy(info.name, plugin_name.c_str(), kPluginNameMax);
	Base::String::strncpy(info.version, plugin_version.c_str(), kPluginVersionMax);

	// register plugin
	PluginHandle handle = RegisterPlugin(&info);

	Link::JsonWriter writer(*response_data);

	writer.Write("handle", (s64)handle);
	writer.Finalize();

	return true;
}

bool ConnectCmd::OnCommand(const std::string& query_string,
												 const std::string& post_data,
												 std::string* response_data) {
	Link::JsonReader reader;
	if(!reader.Parse(post_data.c_str(), post_data.length())) {
		*response_data = "\"not a valid json\"";
		return true;
	}

	s64 number = 0;
	if(!reader.Read<s64>("handle", &number)) {
		*response_data = "\"handle not found\"";
		return true;
	}

	PluginHandle plugin_handle = reinterpret_cast<PluginHandle>(number);

	ConnectionHandle connection = Connect(plugin_handle);

	if(connection == 0) {
		*response_data = "\"connection attempt failed\"";
		return true;
	}

	m_connection = connection;
	m_established = false;

	*response_data = "{\"success\"}";

	return true;
}
	
TcpConnect::TcpConnect()
 : SimplePlugin(kUpdateDeltaMs)
 , m_connect_cmd(m_connection_send, m_established) {

}

bool TcpConnect::OnStartup(const char* config, streamsize nbytes) {
	bool result = RegisterCommand(&m_register_cmd);
	result &= RegisterCommand(&m_connect_cmd);

	return result;
}

void TcpConnect::OnShutdown() {
	UnregisterCommand(&m_register_cmd);
	UnregisterCommand(&m_connect_cmd);
}

static const int num_chunks = 20;

void TcpConnect::OnConnected(const ConnectionNotification& notification) {
	m_num_chunk_send = 0;
	if(m_connection_send == notification.handle) {
		PLUGIN_INFO("sending chunks to %p", notification.handle);
		m_established = true;
	}
}

void TcpConnect::OnPluginConnected(const ConnectionNotification& notification) {
	PLUGIN_INFO("connected %p", notification.handle);
	m_connection_recv = notification.handle;
}

void TcpConnect::OnDisconnected(const ConnectionNotification& notification) {
	if(m_connection_send == notification.handle) {
		PLUGIN_INFO("connection closed %p send", notification.handle);
		m_connection_send = 0;
		m_num_chunk_send = 0;
		m_established = false;
	} else if(m_connection_recv == notification.handle) {
		PLUGIN_INFO("connection closed %p recv", notification.handle);
		m_connection_recv = 0;
		m_num_chunk_recv = 0;
	}
}

static const char chunk[] = "message_chunk";

void TcpConnect::OnRecvReady(const ConnectionNotification& notification) {
	char chunk_buffer[sizeof(chunk)];
//	PLUGIN_INFO("sth received on %p expecting %p %d %d", notification.handle, m_connection_recv, GetRecvAvailable(m_connection_recv), sizeof(chunk));
	if(notification.handle == m_connection_recv) {
		SimplePlugin::Recv(notification.handle, chunk_buffer, sizeof(chunk), [&](void* buffer, unsigned int nbytes){
			m_num_chunk_recv++;
			PLUGIN_INFO("chunk %d received %s", m_num_chunk_recv, chunk_buffer);
			if(m_num_chunk_recv == num_chunks) {
				PLUGIN_INFO("all data received, closing connection");
			}
		});
	}
}

void TcpConnect::OnUpdate(unsigned int dt) {
	if(m_connection_send != 0 && m_established && m_num_chunk_send++ < num_chunks) {
		PLUGIN_INFO("sending chunk %d", m_num_chunk_send);
		bool sent = 0 == Send(m_connection_send, chunk, sizeof(chunk));
		if(!sent) {
			PLUGIN_INFO("can't send more, closing connection");
			CloseConnection(m_connection_send);
			m_connection_send = 0;
		} else if(m_num_chunk_send == num_chunks) {
			PLUGIN_INFO("sent all data, closing connection");
			CloseConnection(m_connection_send);
			m_connection_send = 0;
		}
	}
}

void TcpConnect::OnNotification(const Notification& notification) {
	Link::RestClient::ProcessNotification(notification);
}
