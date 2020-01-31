#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>

// mpv types
#include <mpv/client.h>

namespace mdrpc {

	/**
	 * Safe handle concept for mpv handles
	 */
	struct SafeMpvHandle {
		SafeMpvHandle() 
			: h(nullptr) {

		}
	
		SafeMpvHandle(mpv_handle* handle) 
			: h(handle) {

		}

		/**
		 * Gets the underlying handle to mpv.
		 */ 
		mpv_handle* get() {
			if(h) 
				return h; 
			throw std::runtime_error("attempt to get handle when it's null");
		}

		/**
		 * Cast operator. Returns the result of SafeMpvhandle::get().
		 */
		operator mpv_handle*() { return get(); }

	private:
		mpv_handle* h;
	};


	namespace property {
		
		/**
		 * Get a bool/flag property.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		void get_bool(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			int flag_value;

			if(mpv_get_property(handle, property_name.c_str(), MPV_FORMAT_FLAG, &flag_value) < 0)
				return;

			callback(flag_value);
		}

		/**
		 * Get a int64 property.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		void get_int64(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			std::int64_t value;

			if(mpv_get_property(handle, property_name.c_str(), MPV_FORMAT_INT64, &value) < 0)
				return;

			callback(value);
		}

		/**
		 * Get a double property.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		void get_double(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			double value;

			if(mpv_get_property(handle, property_name.c_str(), MPV_FORMAT_DOUBLE, &value) < 0)
				return;

			callback(value);
		}

		/**
		 * Get a string property.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
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

		/**
		 * Get a string property with OSD formatting.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		void get_string_osd(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			char* value = mpv_get_property_osd_string(handle, property_name.c_str());

			if(!value)
				return;
	
			callback(value);
			mpv_free(value);
		}
		
		/**
		 * Get a node map property.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		void get_node_map(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			mpv_node node;
			if(mpv_get_property(handle, property_name.c_str(), MPV_FORMAT_NODE, &node) < 0)
				return;
			
			if(node.format != MPV_FORMAT_NODE_MAP) {
				mpv_free_node_contents(&node);
				return;
			}

			callback(node);
			mpv_free_node_contents(&node);
		}

		// print singular node
		// (debugging only)
		void print_node(mpv_node node) {
			switch(node.format) {
				default:
					std::cout << "ignoring type " << (int)node.format << '\n';
				break;

				case MPV_FORMAT_STRING:
					std::cout << node.u.string << '\n';
				break;
			}
		}

	}

}
