/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "link/link.h"

#include <functional>

class SimplePlugin {
public:
	static const char* Name;
	static const char* Version;

	static SimplePlugin* CreatePlugin();
	static void DestroyPlugin(SimplePlugin* plugin);

public:
	SimplePlugin(unsigned int update_dt) : m_dt_ms(update_dt){}
	virtual ~SimplePlugin(){}
	virtual bool OnStartup(const char* config, streamsize nbytes) = 0;
	virtual void OnShutdown() = 0;
	virtual void OnThreadEntry() {}
	virtual void OnThreadExit() {}
	virtual void OnUpdate(unsigned int dt) { (void)dt; }
	virtual void OnNotification(const Notification& notification ) { (void)notification; }
	virtual void OnWatchMatch(const WatchNotification& notification) { (void)notification; }
	virtual void OnShutdown(const ShutdownNotification& notification) { (void)notification; }
	virtual void OnConfigChange(const ConfigNotification& notification) { (void)notification; }
	virtual void OnPluginConnected(const ConnectionNotification& notification) { (void)notification; }
	virtual void OnConnected(const ConnectionNotification& notification) { (void)notification; }
	virtual void OnDisconnected(const ConnectionNotification& notification) { (void)notification; }
	virtual void OnRecvReady(const ConnectionNotification& notification) { (void)notification; }
public:
	unsigned int IdleDt() const { return m_dt_ms; }

	static void Recv(ConnectionHandle conn, void* buffer, unsigned int nbytes, std::function<void (void*, unsigned int)> func);

protected:
	unsigned int m_dt_ms;
};
