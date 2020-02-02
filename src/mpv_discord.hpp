#include <discord_rpc.h>
#include "PerIntervalRunner.hpp"

namespace mdrpc {

	constexpr static char discord_appid[] = "";
	constexpr static char discord_large[] = "mpv";

	enum DiscordState : std::uint8_t {
		Idle,
		Paused,
		Playing,
		Fetching,
		// TODO: Is there a way to determine if
		// the user is trying to fetch using youtube-dl?
		FetchingYoutube,
		Count_
	};

	constexpr std::array<const char*, DiscordState::Count_> discord_keys = {{
		"mpv-idle",
		"mpv-paused",
		"mpv-playing",
		"mpv-fetching",
		"mpv-youtube"
	}};


	struct DiscordPlugin : public IMpvPlugin {

		DiscordPlugin(mpv_handle* handle) 
			: IMpvPlugin(handle)  {

			int local_i = 0;
			runner_test.Start(1000, [&, &local_i](){
				std::cout << "test, every 1000ms!\n";

				if(local_i == 10) {
					std::cout << "exiting thread\n";
					runner_test.Stop();
				}

				local_i++;
			});

			runner_test_args.Start(1000, [&](int arg){
				std::cout << "my arg is " << arg << '\n'; 
			}, 100);
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

				album = album_;
			}

			// god no
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
		 * Cached file metadata for the file that is currently playing
		 */ 
		std::map<std::string, mpv_node> cached_metadata;

		std::thread discord_thread;
		
		mdrpc::PerIntervalRunner runner_test;

		mdrpc::PerIntervalRunner runner_test_args;

	};

}
