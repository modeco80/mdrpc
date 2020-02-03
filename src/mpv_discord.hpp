#include <discord_rpc.h>
#include "PerIntervalRunner.hpp"

namespace mdrpc {

	/**
	 * Discord state enumeration
	 */
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

	/**
	 * Discord application ID
	 */
	constexpr static char discord_appid[] = "";

	/**
	 * Large image key
	 */
	constexpr static char discord_large[] = "mpv-logo";

	/**
	 * Small image key conversions of DiscordState
	 */ 
	constexpr std::array<const char*, DiscordState::Count_> discord_keys = {{
		"mpv-idle",
		"mpv-paused",
		"mpv-playing",
		"mpv-fetching",
		"mpv-youtube"
	}};

	/**
	 * Human-readable conversions of DiscordState
	 */ 
	constexpr std::array<const char*, DiscordState::Count_> discord_states = {{
		"Idle",
		"Paused",
		"Playing",
		"Fetching Remote Content",
		"Fetching using Youtube-DL"
	}};

	struct DiscordPlugin : public IMpvPlugin {

		DiscordPlugin(mpv_handle* handle) 
			: IMpvPlugin(handle)  {

			discord_runner.Start(15000, [&]() {
				DiscordInterval();
			}, [&]() {
				DiscordInit();
			});
		}

		~DiscordPlugin() {
			if(!cached_metadata.empty())
				cached_metadata.clear();

			discord_runner.Stop();
		}


		void DiscordInit() {
			using namespace std::placeholders;
			
			DiscordEventHandlers handlers;
			handlers.ready = std::bind(&DiscordPlugin::DiscordReady, this, _1);
			handlers.disconnected = std::bind(&DiscordPlugin::DiscordDisconnect, this, _1, _2);
			handlers.errored = std::bind(&DiscordPlugin::DiscordError, this, _1, _2);

			std::cout << "MDRPC2: Initalizing Discord\n\n\n";
			Discord_Initialize(discord_appid, &handlers, 1, NULL);
		}

		void DiscordInterval() {
			// Update discord stuff
		}

		// Discord Handlers

		void DiscordReady(const DiscordUser* user) {
			if(!user)
				return; // ?
			
			std::cout << "MDRPC2: Got discord for " << user->username << "#" << user->discriminator << "\n\n";
		}

		void DiscordDisconnect(int error, const char* reason) {
			std::cout << "MDRPC2: Disconnected (" << error << " \"" << reason << "\"\n\n\n";
		}

		void DiscordError(int error, const char* reason) {
			std::cout << "MDRPC2: Error (" << error << " \"" << reason << "\"\n\n";
		}

		/**
		 * Processes events as they are recieved from MPV.
		 * 
		 * \param[in] ev Native MPV event.
		 */
		void ProcessEvent(mpv_event* ev) {
			if(!ev)
				return;
			
			switch(ev->event_id) {
				default:
					break;

				// TODO: Implement more

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
			constexpr std::array<const char*, 2> artist_keys = {{
				"artist",
				"ARTIST"
			}};

			constexpr std::array<const char*, 3> title_keys = {{
				"title",
				"TITLE",
				"icy-title"
			}};

			constexpr std::array<const char*, 2> album_keys = {{
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
		
		mdrpc::PerIntervalRunner discord_runner;
	};

}
