#include <discord_rpc.h>


namespace mdrpc {

	struct DiscordPlugin : public IMpvPlugin {

		DiscordPlugin(mpv_handle* handle) 
			: IMpvPlugin(handle)  {
			// initalize discord stuff&things
		}

		~DiscordPlugin() {
			if(!cached_metadata.empty())
				cached_metadata.clear();
		}

		/**
		 * Processes events as they are recieved.
		 * 
		 * \param[in] ev Native MPV event.
		 */
		void ProcessEvent(mpv_event* ev) {
			if(!ev)
				return;

			// basic debounce (a memcmp() would be more accurate, but may cause some ouch)
			if(last_processed_ev.event_id == ev->event_id)
				return;

			memcpy(&last_processed_ev, &ev, sizeof(mpv_event));
			
			switch(ev->event_id) {
				default:
					break;

				case MPV_EVENT_FILE_LOADED: {
					cached_metadata.clear();
					cached_metadata = property::get_node_map_converted(mpvHandle, "metadata");

					// for debugging purposes only
					for(auto&& entry : cached_metadata)
						std::cout << entry.first << ": " << entry.second.u.string << '\n';
				}
			}
		}

		/**
		 * Last processed event, used for debounce purposes
		 */
		mpv_event last_processed_ev;
 
		// reset/written to on file load
		std::map<std::string, mpv_node> cached_metadata;

		std::thread discord_thread;
		
	};

}
