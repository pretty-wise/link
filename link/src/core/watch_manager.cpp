/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "watch_manager.h"

namespace Link {

WatchManager::WatchManager() {
		m_mutex.Initialize();
}

WatchManager::~WatchManager() {
		m_mutex.Terminate();
}

WatchHandle WatchManager::Create(const char* name, const char* version, const char* hostname) {
		WatchHandle handle = Generate();

		WatchData data;
		data.name_filter = name;
		data.version_filter = version;
		data.hostname_filter = hostname;
		data.handle = handle;

		m_watches.push_back(data);

		return handle;
}

WatchHandle WatchManager::Generate(){
		static int handle = 0;
		return reinterpret_cast<WatchHandle>(++handle);
}

void WatchManager::Remove(WatchHandle handle) {
		Base::MutexAutoLock lock(m_mutex);

		for(WatchList::iterator it = m_watches.begin(); it != m_watches.end(); ) {
				if((*it).handle == handle) {
						it = m_watches.erase(it);
				} else {
						++it;
				}
		}
}

void WatchManager::Filter(const char* plugin_name, const char* plugin_version, const char* plugin_hostname, std::vector<WatchHandle>& handles) const {
	handles.clear();
	Base::MutexAutoLock lock(m_mutex);
	for(WatchList::const_iterator it = m_watches.begin(); it != m_watches.end(); ++it) {
		if(PluginDirectory::Match(plugin_name, (*it).name_filter.c_str()) 
			&& PluginDirectory::Match(plugin_version, (*it).version_filter.c_str())
			&& PluginDirectory::Match(plugin_hostname, (*it).hostname_filter.c_str())){
				handles.push_back((*it).handle);
		}
	}
}

} // namespace Link
