/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
#include "link/link.h"
#include "link/plugin_log.h"
#include "base/threading/thread.h"
#include "base/core/macro.h"

Base::Thread thread;

bool running = true;

int PluginLoop(void*){

		WatchHandle watch = CreateWatch("*", "*", "*");

		while(running){

				Notification notif;
				if(GetNotification(&notif, 0)){
						PLUGIN_INFO("notification received: %d.\n", notif.type);
						int res = 0;

						switch (notif.type) {
								case kShutdown:
										running = false;
										break;
								case kWatch:
										PluginInfo info;
										res = GetPluginInfo(notif.content.watch.plugin, &info);
										PLUGIN_INFO("plugin %s(%s) available\n", info.name, info.version);
										if(watch == notif.content.watch.handle && notif.content.watch.plugin_state == kPluginAvailable) {
												Connect(notif.content.watch.plugin);
										}
										break;
								default:
										break;
						}
						// process notification
				}
		}
		return 0;
}

const char* GetName()
{
		return "dummy plugin";
}

const char* GetVersion()
{
		return "1.00";
}

int Startup(const char* config, streamsize nbytes) {
		thread.Initialize(&PluginLoop, nullptr);
		return 0;
}

void Shutdown() {
		thread.Join();
}
