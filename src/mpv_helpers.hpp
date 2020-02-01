// === Helper utility functions for better/modern C++ interaction with MPV ===

#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <map>
#include <vector>

// mpv types
#include <mpv/client.h>

namespace mdrpc {

	/**
	 * Safe handle concept for handles to MPV.
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
		 * Cast operator. Returns the result of SafeMpvHandle::get().
		 */
		operator mpv_handle*() { return get(); }

	private:
		mpv_handle* h;
	};


	namespace property {
		
		/**
		 * Get an bool/flag property.
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
		 * Get an int64 property.
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
		 * Get an double property.
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
		 * Get an string property.
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
			// (we've passed go and collected 200$), we still check
			if(!value)
				return;
	
			callback(value);

			// We get an allocation from the handle itself,
			// so we call mpv_free when our callback completes on the allocated memory
			mpv_free(value);
		}

		/**
		 * Get an string property with OSD formatting.
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
		 * Get an singular node property.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		void get_node(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			mpv_node node;
			if(mpv_get_property(handle, property_name.c_str(), MPV_FORMAT_NODE, &node) < 0)
				return;
			
			// Already handled by the below get_node_map and get_node_array functions
			// (as well as conversion functions)
			if(node.format == MPV_FORMAT_NODE_ARRAY || node.format == MPV_FORMAT_NODE_MAP) {
				mpv_free_node_contents(&node);
				return;
			}

			callback(node);
			mpv_free_node_contents(&node);
		}

		/**
		 * Get an node map property.
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

		/**
		 * Get an node array property.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		void get_node_array(SafeMpvHandle& handle, const std::string& property_name, Functor callback) {
			mpv_node node;
			if(mpv_get_property(handle, property_name.c_str(), MPV_FORMAT_NODE, &node) < 0)
				return;
			
			if(node.format != MPV_FORMAT_NODE_ARRAY) {
				mpv_free_node_contents(&node);
				return;
			}

			callback(node);
			mpv_free_node_contents(&node);
		}

		/**
		 * Get an node map property converted to a C++ map.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 */
		std::map<std::string, mpv_node> get_node_map_converted(SafeMpvHandle& handle, const std::string& property_name) {
			std::map<std::string, mpv_node> values;

			get_node_map(handle, property_name.c_str(), [&](mpv_node node) {
					if(node.u.list->num == 0)
						return;

					// convert map
					for(int i = 0; i < node.u.list->num; ++i) {
						mpv_node n;
						memcpy(&n, &node.u.list->values[i], sizeof(mpv_node));
						values[node.u.list->keys[i]] = n;
					}
			});

			return values;
		}

		/**
		 * Get an node array property converted to a C++ vector.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 */
		std::vector<mpv_node> get_node_array_converted(SafeMpvHandle& handle, const std::string& property_name) {
			std::vector<mpv_node> values;

			get_node_array(handle, property_name.c_str(), [&](mpv_node node) {
					if(node.u.list->num == 0)
						return;

					for(int i = 0; i < node.u.list->num; ++i) {
						mpv_node n;
						memcpy(&n, &node.u.list->values[i], sizeof(mpv_node));
						values.push_back(n);
					}
			});

			return values;
		}

	}

}
