//
// main source file for mdrpc2
//

#include "mpv_plugin.hpp"

// discord plugin
#include "mpv_discord.hpp"

// Static heap allocated plugin
static mdrpc::IMpvPlugin* plugin_iface = nullptr;

extern "C" {

	int mpv_open_cplugin(mpv_handle* handle) {
		// would it be cleaner to handle this in a constructor function?
		// (to be fair, this can be treated as one basically, so I don't think it MATTERS
		// buuut idk)
		plugin_iface = new mdrpc::DiscordPlugin(handle);

		while(true) {
			mpv_event* event = mpv_wait_event(plugin_iface->mpvHandle.get(), -1);
			if(event->event_id == MPV_EVENT_SHUTDOWN)
				break;
			plugin_iface->ProcessEvent(event);
		}

		// plugin EOL
		delete plugin_iface;
		return 0;
	}

}
