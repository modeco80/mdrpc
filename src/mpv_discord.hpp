
namespace mdrpc {

	struct DiscordPlugin : public IMpvPlugin {

		DiscordPlugin(mpv_handle* handle) 
			: IMpvPlugin(handle)  {
			// initalize discord stuff&things
		}
		
		void ProcessEvent(mpv_event* ev) {
			if(!ev)
				return;

			// basic debounce (a memcmp() would be more accurate, but may cause some ouch)
			if(last_processed_ev.event_id == ev->event_id)
				return;

			memcpy(&last_processed_ev, &ev, sizeof(mpv_event));



			property::get_string_osd(mpvHandle, "time-pos", [](char* str) {
				std::cout << "time is " << str << '\n';	
			});
		}

		// holds a copy of the last processed event for debouncing purposes
		// (so that we don't process the same event multiple times, 
		// for responsiveness reasons & not spamming the discord thread with messages mostly)
		mpv_event last_processed_ev;
 
	};

}
