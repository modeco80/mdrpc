#include <discord_rpc.h>
#include <sstream>
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
	constexpr static char discord_appid[] = "673967887387590658";

	/**
	 * Large image key that mdrpc expects
	 */
	constexpr static char discord_large[] = "mpv-logo";

	/**
	 * Small image key conversions of DiscordState
	 */ 
	constexpr std::array<const char*, DiscordState::Count_> discord_keys = {{
		"mpv-idle",
		"mpv-paused",
		"mpv-playing",
		"mpv-buffering"
	}};

	/**
	 * Human-readable conversions of DiscordState
	 */ 
	constexpr std::array<const char*, DiscordState::Count_> discord_states = {{
		"Idle",
		"Paused",
		"Playing",
		"Buffering Remote Content"
	}};

	struct DiscordPlugin : public IMpvPlugin {

		DiscordPlugin(mpv_handle* handle) 
			: IMpvPlugin(handle) {
		}

		~DiscordPlugin() {
			if(!cached_metadata.empty())
				cached_metadata.clear();
		}


		void DiscordInit() {
			using namespace std::placeholders;
			
			DiscordEventHandlers handlers;
			memset(&handlers, 0, sizeof(DiscordEventHandlers));
			handlers.ready = std::bind(&DiscordPlugin::DiscordReady, this, _1);
			handlers.disconnected = std::bind(&DiscordPlugin::DiscordDisconnect, this, _1, _2);
			handlers.errored = std::bind(&DiscordPlugin::DiscordError, this, _1, _2);

			Discord_Initialize(discord_appid, &handlers, 1, NULL);
#ifdef DISCORD_DISABLE_IO_THREAD
			Discord_UpdateConnection();
#endif
		}

		void DiscordUpdate() {
			static DiscordRichPresence rpc;
			rpc.largeImageKey = discord_large;
			rpc.largeImageText = "mpv";

			rpc.details = discord_states[discord_state];

			std::string details = GetFormat();

			std::vector<char> conv;
			conv.resize(details.size());

			std::remove_copy_if(details.begin(), details.end(), conv.begin(), [](char c) {
				return c == '\0';
			});

			conv.push_back('\0');


			rpc.state = conv.data();

			Discord_UpdatePresence(&rpc);
			Discord_RunCallbacks();
#ifdef DISCORD_DISABLE_IO_THREAD
			Discord_UpdateConnection();
#endif
		}

		void DiscordReady(const DiscordUser* user) {
			std::cout << "mdrpc2: Discord connected (" << user->username << "#" << user->discriminator << ")\n";
		}

		void DiscordDisconnect(int error, const char* reason) {
			std::cout << "mdrpc2: Discord disconnected (" << error << " \"" << reason << "\"\n";
		}

		void DiscordError(int error, const char* reason) {
			std::cout << "mdrpc2: Discord error (" << error << " \"" << reason << "\"\n";
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

				case MPV_EVENT_FILE_LOADED: {
					discord_state = DiscordState::Playing;
					cached_metadata.clear();
					cached_metadata = property::get_node_map_converted(mpvHandle, "metadata");
					filename = property::get_osd_string_converted(mpvHandle, "filename");

					if(discord_runner.Running())
						discord_runner.Stop();

					discord_runner.Start(5000, [&]() {
						DiscordUpdate();
					}, [&]() {
						// Initalize discord
						DiscordInit();
					});
				} break;
				
				case MPV_EVENT_IDLE: {
					discord_state = DiscordState::Idle;
				} break;

				case MPV_EVENT_SHUTDOWN: {
					if(discord_runner.Running())
						discord_runner.Stop();
				}
			}
		}


		std::string GetFormat() {
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

			for(const char* key : artist_keys) {
				if(cached_metadata.find(key) == cached_metadata.end())
					continue;

				auto artist_ = property::convert_node_string(cached_metadata[key]);

				if(artist_.empty())
					continue;

				artist = std::string(artist_.begin(), artist_.end());
			}

			for(const char* key : title_keys) {
				if(cached_metadata.find(key) == cached_metadata.end())
					continue;

				auto title_ = property::convert_node_string(cached_metadata[key]);

				if(title_.empty())
					continue;

				title = std::string(title_.begin(), title_.end());
			}

			std::stringstream ss;

			if(artist.empty() && title.empty())
				ss << filename;
			else if(artist.empty())
				ss << title;
			else
				ss << artist << " - " << title;

			// TODO: add play line to stringstream

			return ss.str();
		}


		/**
		 * Cached file metadata for the file that is currently playing
		 */ 
		std::map<std::string, mpv_node> cached_metadata;

		std::string filename;
		
		mdrpc::PerIntervalRunner discord_runner;

		DiscordState discord_state;
	};

}
