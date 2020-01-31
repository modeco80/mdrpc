#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "mpv_helpers.hpp"

// TODO: include discord rpc

namespace mdrpc {


    struct IMpvPlugin {
		SafeMpvHandle mpvHandle;

		IMpvPlugin(mpv_handle* handle) {
			mpvHandle = SafeMpvHandle(handle);
		}

		virtual void ProcessEvent(mpv_event* ev) {
			if(!ev)
				return;

			std::cout << "Someone forgot to implement IMpvPlugin::ProcessEvent! Event ID " << ev->event_id << '\n';
		}
		
    };


}
