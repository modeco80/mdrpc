#include "SymHide.hpp"
#include "DiscordPlugin.hpp"

#include "Version.hpp"

#ifdef _WIN32
// We do not use Windows APIs in the plugin
// but we need a few things for the DllMain
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/**
 * Static heap allocated plugin
 */
static mdrpc::IMpvPlugin* plugin_iface = nullptr;

extern "C" {

	EXPORT_SYM int mpv_open_cplugin(mpv_handle* handle) {
		std::cout << "mdrpc version " << mdrpc::Version::tag << " startup\n";
		plugin_iface = new mdrpc::DiscordPlugin(handle);

		while(true) {
			mpv_event* event = mpv_wait_event(plugin_iface->mpvHandle.get(), -1);
			if(event->event_id == MPV_EVENT_SHUTDOWN) {
				// allow processing shutdown events so we can (cleanly)
				// stop what we're doing
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

#ifdef _WIN32
// Null DllMain so linking doesn't fall apart on Windows
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hinst);
		break;

    	default:
		break;
	}
	return TRUE;
}
#endif
