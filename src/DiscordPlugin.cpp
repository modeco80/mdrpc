#include <sstream>
#include "SymHide.hpp"
#include "DiscordPlugin.hpp"

namespace mdrpc LOCAL_SYM {

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
	constexpr std::array<const char*, PlayerState::Count_> discord_keys = {{
		"mpv-idle",
		"mpv-paused",
		"mpv-playing",
		"mpv-buffering"
	}};

	/**
	 * Human-readable conversions of DiscordState
	 */ 
	constexpr std::array<const char*, PlayerState::Count_> current_states = {{
		"Idle",
		"Paused",
		"Playing",
		"Buffering Remote Content"
	}};

	DiscordPlugin::DiscordPlugin(mpv_handle* handle) {
		mpvHandle = ModernMPV::SafeHandle(handle);
	}

	DiscordPlugin::~DiscordPlugin() {
		if(!cached_metadata.empty())
			cached_metadata.clear();
	}


	void DiscordPlugin::ProcessEvent(mpv_event* ev) {
		if(!ev)
			return;
			
		switch(ev->event_id) {
			default:
				break;

			case MPV_EVENT_FILE_LOADED: {
				current_state = PlayerState::Playing;
				cached_metadata.clear();

				// load in metadata
				cached_metadata = ModernMPV::Properties::get_node_map(mpvHandle, "metadata");
				cached_filename = ModernMPV::Properties::get_osd_string(mpvHandle, "filename");

				if(discord_runner.Running())
					discord_runner.Stop();

				if(state_runner.Running())
					state_runner.Stop();

				discord_runner.Start(1500, [&]() {
					RpcThreadInterval();
				}, [&]() {
					// Initalize discord
					RpcThreadInit();
				});

				state_runner.Start(500, [&]() {
					StateThreadInterval();
				});
			} break;
				
			case MPV_EVENT_IDLE: {
				if(state_runner.Running())
					state_runner.Stop();

				current_state = PlayerState::Idle;
			} break;

			case MPV_EVENT_SHUTDOWN: {
				if(discord_runner.Running())
					discord_runner.Stop();

				if(state_runner.Running())
					state_runner.Stop();

				Discord_Shutdown();
			}
		}
	}

	void DiscordPlugin::RpcThreadInit() {
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

	void DiscordPlugin::RpcThreadInterval() {
		static DiscordRichPresence rpc;
		rpc.largeImageKey = discord_large;
		rpc.largeImageText = "mpv";

		auto state = Utils::StringToC(GetState());
		auto song = Utils::StringToC(GetSong());

		rpc.details = state.data();
		rpc.state = song.data();

		Discord_UpdatePresence(&rpc);
		Discord_RunCallbacks();
#ifdef DISCORD_DISABLE_IO_THREAD
		Discord_UpdateConnection();
#endif
	}

	void DiscordPlugin::DiscordReady(const DiscordUser* user) {
		std::cout << "mdrpc: Discord connected (" << user->username << "#" << user->discriminator << ")\n";
	}

	void DiscordPlugin::DiscordDisconnect(int error, const char* reason) {
		std::cout << "mdrpc: Discord disconnected (" << error << " \"" << reason << "\"\n";
	}

	void DiscordPlugin::DiscordError(int error, const char* reason) {
		std::cout << "mdrpc: Discord error (" << error << " \"" << reason << "\"\n";
	}

	void DiscordPlugin::StateThreadInterval() {
		ModernMPV::Properties::get_bool(mpvHandle, "pause", [&](bool Value) {
			if(Value)
				current_state = PlayerState::Paused;
			else
				current_state = PlayerState::Playing;
		});

		ModernMPV::Properties::get_bool(mpvHandle, "paused-for-cache", [&](bool Value) {
			if(Value)
				current_state = PlayerState::Buffering;
		});
	}

	std::string DiscordPlugin::GetState() {
		std::stringstream stream;
		stream << current_states[current_state] << ' ' << '(';

		double speed;
		ModernMPV::Properties::get_double(mpvHandle, "speed", [&](double v) {
			speed = v;
		});

		stream << ModernMPV::Properties::get_osd_string(mpvHandle, "time-pos") << '/';
		stream << ModernMPV::Properties::get_osd_string(mpvHandle, "duration");

		if(speed != 1.0)
			stream << ' ';

		if(Utils::AddIf(stream, speed, [](double v) { return v == 1.0; })) {
			stream << 'x';
		}

		stream << ')';

		return stream.str();
	}

	std::string DiscordPlugin::GetSong() {
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

			auto artist_ = ModernMPV::Properties::get_node_string(cached_metadata[key]);

			if(artist_.empty())
				continue;

			artist = artist_;
		}

		for(const char* key : title_keys) {
			if(cached_metadata.find(key) == cached_metadata.end())
				continue;

			auto title_ = ModernMPV::Properties::get_node_string(cached_metadata[key]);

			if(title_.empty())
				continue;

			title = title_;
		}

		std::stringstream stream;

		if(artist.empty() && title.empty())
			stream << cached_filename;
		else if(artist.empty())
			stream << title;
		else
			stream << artist << " - " << title;

		return stream.str();
	}
}
