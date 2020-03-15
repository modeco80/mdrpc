#include "SymHide.hpp"
#include "DiscordPlugin.hpp"
#include "Singleton.hpp"
#include "Version.hpp"

#ifdef _WIN32
// We do not use Windows APIs in the plugin
// but we need a few things for the DllMain
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

extern "C" {

	EXPORT_SYM int mpv_open_cplugin(mpv_handle* handle) {
		Utils::Singleton<mdrpc::DiscordPlugin> plugin_singleton;

		auto plugin = plugin_singleton.Get(handle);

		std::cout << "mdrpc version " << mdrpc::Version::tag << "!!\n";
		while(true) {
			auto handle = plugin.mpvHandle.get();
			mpv_event* event = mpv_wait_event(handle, -1);

			if(event->event_id == MPV_EVENT_SHUTDOWN) {
				// allow processing shutdown events so we can (cleanly)
				// stop what we're doing
				plugin.ProcessEvent(event);
				break;
			}
			plugin.ProcessEvent(event);
		}

		// plugin EOL
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
