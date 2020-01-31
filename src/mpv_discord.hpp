#include <discord_rpc.h>

namespace mdrpc {

	struct DiscordPlugin : public IMpvPlugin {

		DiscordPlugin(mpv_handle* handle) 
			: IMpvPlugin(handle)  {
			// initalize discord stuff&things
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



			property::get_node_map(mpvHandle, "metadata", [](mpv_node node) {
					for(int i = 0; i < node.u.list->num; ++i) {
						std::cout << node.u.list->keys[i] << ": "; property::print_node(node.u.list->values[i]);
					}
			});
		}

		/**
		 * Last processed event, used for debounce purposes
		 */
		mpv_event last_processed_ev;
 
	};

}
