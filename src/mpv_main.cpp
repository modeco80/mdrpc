#include "mpv_plugin.hpp"
#include "mpv_discord.hpp"

/**
 * Static heap allocated plugin
 */
static mdrpc::IMpvPlugin* plugin_iface = nullptr;

extern "C" {

	int mpv_open_cplugin(mpv_handle* handle) {
		plugin_iface = new mdrpc::DiscordPlugin(handle);

		while(true) {
			mpv_event* event = mpv_wait_event(plugin_iface->mpvHandle.get(), -1);
			if(event->event_id == MPV_EVENT_SHUTDOWN) {
				plugin_iface->ProcessEvent(event);
				break;
			}
			plugin_iface->ProcessEvent(event);
		}

		// plugin EOL
		delete plugin_iface;
		return 0;
	}

}
