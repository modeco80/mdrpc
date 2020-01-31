#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>

// mpv types
#include <mpv/client.h>

namespace mdrpc {

	// safer way of passing handles
	struct SafeMpvHandle {
		SafeMpvHandle() 
			: h(nullptr) {

		}
	
		SafeMpvHandle(mpv_handle* handle) 
			: h(handle) {

		}

		mpv_handle* get() {
			if(h) 
				return h; 
			throw std::runtime_error("attempt to get handle when it's null");
		}

		operator mpv_handle*() { return get(); }

	private:
		mpv_handle* h;
	};

	// Helpers for getting property values
	namespace property {

		template<class Functor>
		void get_bool(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			int flag_value;

			if(mpv_get_property(handle, property_name.c_str(), MPV_FORMAT_FLAG, &flag_value) < 0)
				return;

			callback(flag_value);
		}

		template<class Functor>
		void get_int64(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			std::int64_t value;

			if(mpv_get_property(handle, property_name.c_str(), MPV_FORMAT_INT64, &value) < 0)
				return;

			callback(value);
		}

		template<class Functor>
		void get_double(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			double value;

			if(mpv_get_property(handle, property_name.c_str(), MPV_FORMAT_DOUBLE, &value) < 0)
				return;

			callback(value);
		}


		template<class Functor>
		void get_string(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			char* value = nullptr;

			if(mpv_get_property(handle, property_name.c_str(), MPV_FORMAT_STRING, &value) < 0)
				return;

			// while it's *safe* to assume at this point we have a valid value
			// (we've passed property and collected 200$), we still check
			if(!value)
				return;
	
			callback(value);

			// We get an allocation from the handle itself,
			// so we call mpv_free when our callback completes on the allocated memory
			mpv_free(value);
		}

		// Similar to above, but with OSD formatting instead.
		template<class Functor>
		void get_string_osd(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			char* value = mpv_get_property_osd_string(handle, property_name.c_str());

			if(!value)
				return;
	
			callback(value);
			mpv_free(value);
		}
		
	}

}
