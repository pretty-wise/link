/*
 * Copywrite 2014-2015 Krzysztof Stasik. All rights reserved.
 */
//
//	main.cpp
//	Raw Plugin
//
//	Created by Krzysiek Stasik on 08/06/14.
//	Copyright (c) 2014 Krzysiek Stasik. All rights reserved.
//

#include "link/link.h"
#include "base/threading/thread.h"
#include "link/plugin_log.h"

Base::Thread thread;

bool running = true;

int PluginLoop(void*){

	while(running){
		Notification notif;
		if(GetNotification(&notif, 0)){
			PLUGIN_INFO("notification received: %d.", notif.type);
			switch (notif.type) {
				case kShutdown:
				running = false;
					break;
				default:
					break;
			}
		}
	}
	return 0;
}

const char* GetName()
{
	return "raw_plugin";
}

const char* GetVersion()
{
	return "1.0";
}

int Startup(const char* config, streamsize nbytes) {
	PLUGIN_INFO("startup");
	thread.Initialize(&PluginLoop, nullptr);
	return 0;
}

void Shutdown() {
	thread.Join();
	PLUGIN_INFO("shutdown");
}
