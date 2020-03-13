#pragma once

#include "Utils.hpp"
#include "IntervalRunner.hpp"
#include "IMpvPlugin.hpp"
#include <discord_rpc.h>

namespace mdrpc LOCAL_SYM {

	/**
	 * Discord state enumeration
	 */
	enum DiscordState : std::uint8_t {
		Idle,
		Paused,
		Playing,
		Buffering,
		Count_
	};

	struct DiscordPlugin : public IMpvPlugin {

		DiscordPlugin(mpv_handle* handle);
		~DiscordPlugin();

		/**
		 * Processes events as they are recieved from MPV.
		 * 
		 * \param[in] ev Native MPV event.
		 */
		void ProcessEvent(mpv_event* ev);

	private:

		void DiscordInit();
		void DiscordUpdate();

		void DiscordReady(const DiscordUser* user);
		void DiscordDisconnect(int error, const char* reason);

		/**
		 * uh oh system fuck
		 */
		void DiscordError(int error, const char* reason);

		/**
		 * Updates the current state.
		 */
		void StateUpdate();

		/**
		 * Returns formatted state.
		 */
		std::string GetState();

		/**
		 * Returns formatted song metadata or filename.
		 */
		std::string GetSong();

		/**
		 * Cached file metadata for file that is currently playing
		 */ 
		std::map<std::string, mpv_node> cached_metadata;

		std::string filename;
		Utils::IntervalRunner discord_runner;
		Utils::IntervalRunner state_runner;
		DiscordState current_state;
	};

}
