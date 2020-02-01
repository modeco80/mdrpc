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
					std::cout << GetFormattedSong() << '\n';
				}
			}
		}

		/**
		 * Returns formatted artist/title/album
		 * If fetching from our cache fails, we go back to the filename
		 */
		std::string GetFormattedSong() {
			const std::array<const char*, 2> artist_keys = {{
				"artist",
				"ARTIST"
			}};

			const std::array<const char*, 3> title_keys = {{
				"title",
				"TITLE",
				"icy-title"
			}};

			const std::array<const char*, 2> album_keys = {{
				"album",
				"ALBUM"
			}};

			std::string artist;
			std::string title;
			std::string album;

			for(const char* key : artist_keys) {
				if(cached_metadata.find(key) == cached_metadata.end())
					continue;

				std::string artist_ = property::convert_node_string(cached_metadata[key]);

				if(artist_.empty())
					continue;

				artist = artist_;
			}

			for(const char* key : title_keys) {
				if(cached_metadata.find(key) == cached_metadata.end())
					continue;

				std::string title_ = property::convert_node_string(cached_metadata[key]);

				if(title_.empty())
					continue;

				title = title_;
			}

			for(const char* key : album_keys) {
				if(cached_metadata.find(key) == cached_metadata.end())
					continue;

				std::string album_ = property::convert_node_string(cached_metadata[key]);

				if(album_.empty())
					continue;

				//std::cout << '\n' << album_ << '\n';
				album = album_;
			}

			if(artist.empty() && title.empty() && album.empty())
				return property::get_osd_string_converted(mpvHandle, "filename");
			else if(artist.empty() && title.empty())
				return album;
			else if(artist.empty() && album.empty())
				return title;
			else if(artist.empty())
				return title + " on " + album;
			else
				return artist + " - " + title + " on " + album;
		}

		/**
		 * Last processed event, used for debounce purposes
		 */
		mpv_event last_processed_ev;
 
		/**
		 * Cached file metadata for the current playing file
		 */ 
		std::map<std::string, mpv_node> cached_metadata;

		std::thread discord_thread;
		
	};

}
