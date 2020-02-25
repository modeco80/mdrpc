#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include "PropertyHelpers.hpp"

namespace mdrpc LOCAL_SYM {

	/**
	 * Interface for plugins to follow
	 */
    struct IMpvPlugin {
		/**
		 * Safe handle to MPV.
		 */
		SafeMpvHandle mpvHandle;

		IMpvPlugin(mpv_handle* handle) {
			mpvHandle = SafeMpvHandle(handle);
		}

		/**
		 * Processes events as they are recieved.
		 * 
		 * \param[in] ev Native MPV event.
		 */
		virtual void ProcessEvent(mpv_event* ev) {
			if(!ev)
				return;

			std::cout << "IMpvPlugin::ProcessEvent base Event ID " << ev->event_id << '\n';
		}
		
    };

}
