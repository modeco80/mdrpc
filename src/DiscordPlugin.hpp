#pragma once

#include "Utils.hpp"
#include "IntervalRunner.hpp"
#include "ModernMPV.hpp"

#include <discord_rpc.h>

namespace mdrpc LOCAL_SYM {

	/**
	 * Player state enumeration
	 */
	enum PlayerState : std::uint8_t {
		Idle,
		Paused,
		Playing,
		Buffering,
		Count_
	};

	struct DiscordPlugin {

		DiscordPlugin(mpv_handle* handle);
		~DiscordPlugin();

		/**
		 * Processes events as they are recieved from MPV.
		 * 
		 * \param[in] ev Native MPV event.
		 */
		void ProcessEvent(mpv_event* ev);


		/**
		 * Handle to mpv.
		 */
		ModernMPV::SafeMpvHandle mpvHandle;

	private:

		/**
		 * \defgroup Discord functions
		 * @{
		 */

		/**
		 * Initalizes Discord RPC.
		 */
		void RpcThreadInit();

		/**
		 * Creates and sends state to Discord.
		 */
		void RpcThreadInterval();


		/**
		 * Callback for when Discord is ready.
		 */
		void DiscordReady(const DiscordUser* user);

		/**
		 * Callback for when Discord disconnects.
		 */
		void DiscordDisconnect(int error, const char* reason);

		/**
		 * uh oh system fuck
		 */
		void DiscordError(int error, const char* reason);
	
		/** @} */

		/**
		 * Updates the current player state.
		 */
		void StateThreadInterval();

		/**
		 * Returns the current state in a human readable fashion.
		 */
		std::string GetState();

		/**
		 * Returns the formatted song metadata (or filename if metadata does not exist).
		 */
		std::string GetSong();

		/**
		 * Cached file metadata for file that is currently playing.
		 */ 
		std::map<std::string, mpv_node> cached_metadata;

		/**
		 * Cached filename.
		 */
		std::string cached_filename;

		/**
		 * Interval runner for Discord.
		 */
		Utils::IntervalRunner discord_runner;

		/**
		 * Interval runner for updating states.
		 */
		Utils::IntervalRunner state_runner;

		/**
		 * The current player state.
		 */
		PlayerState current_state;
	};

}
