/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "link/link.h"
#include "common/json/json_writer.h"

namespace Link {

template<>
inline void JsonWriter::AppendValue(const PluginInfo& info) {
	m_write_count = 0;
	m_destination += "{";
	
	Write("name", info.name);
	Write("version", info.version);
	Write("hostname", info.hostname);
	Write("port", info.port);
	Write("pid", info.pid);

	m_write_count = 0;
	m_destination += "}";
};

} // namespace Link
