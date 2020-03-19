#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <map>
#include <vector>

// mpv types (and enumerations/data structures)
#include <mpv/client.h>

/**
 * ModernMPV main namespace
 */
namespace ModernMPV {

	/**
	 * Safe handle construct for handles to MPV.
	 * Will throw exception(s) if the handle is null.
	 */
	struct SafeHandle {

		SafeHandle() 
			: h(nullptr) {
		}
	
		SafeHandle(mpv_handle* handle) 
			: h(handle) {
		}

		/**
		 * Gets the underlying handle this object is holding
		 */ 
		inline mpv_handle* get() {
			if(h) 
				return h;

			throw std::runtime_error("SafeHandle::get() would return `nullptr` in this case");
		}

		/**
		 * Cast operator. Returns the result of SafeHandle::get().
		 */
		inline operator mpv_handle*() { 
			return get(); 
		}

	private:
		mpv_handle* h;
	};

	namespace Properties {

	/**
	 * Macro to generate parts of the function body
	 * that do not change for the following functions.
	 */
	#define MDN_GENERATE_GET_BODY(T, PropertyT) T value; \
			if(mpv_get_property(handle, property_name.c_str(), PropertyT, &value) < 0) \
			return;	\

		/**
		 * Get an bool/flag property.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		inline void get_bool(SafeHandle& handle, const std::string& property_name, Functor callback) {
			MDN_GENERATE_GET_BODY(int, MPV_FORMAT_FLAG);
			callback(value);
		}

		/**
		 * Get an int64 property.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		inline void get_int64(SafeHandle& handle, const std::string& property_name, Functor callback) {
			MDN_GENERATE_GET_BODY(std::int64_t, MPV_FORMAT_INT64);
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
		inline void get_double(SafeHandle& handle, const std::string& property_name, Functor callback) {
			MDN_GENERATE_GET_BODY(double, MPV_FORMAT_DOUBLE);
			callback(value);
		}

		/**
		 * Get an string property in it's raw form.
		 * Not reccomended. Use ModernMPV::Properties::get_string() instead.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		inline void get_string_raw(SafeHandle& handle, const std::string& property_name, Functor callback) {
			MDN_GENERATE_GET_BODY(char*, MPV_FORMAT_STRING);
			callback(value);

			// We get an allocation from the handle itself,
			// so we call mpv_free when our callback completes *on* the allocated memory
			// This means the pointer will be invalid at this point
			mpv_free(value);
		}

		/**
		 * Get an string property with OSD formatting in its raw form.
		 * Not reccomended. Use ModernMPV::Properties::get_osd_string() instead.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		inline void get_osd_string_raw(SafeHandle& handle, const std::string& property_name, Functor callback) {
			char* value = mpv_get_property_osd_string(handle, property_name.c_str());
			if(!value)
				return;
	
			callback(value);
			mpv_free(value);
		}
		
		/**
		 * Get a singular node property.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		inline void get_node(SafeHandle& handle, const std::string& property_name, Functor callback) {
			MDN_GENERATE_GET_BODY(mpv_node, MPV_FORMAT_NODE);
			
			if(value.format == MPV_FORMAT_NODE_ARRAY || value.format == MPV_FORMAT_NODE_MAP) {
				mpv_free_node_contents(&value);
				return;
			}

			callback(value);
			mpv_free_node_contents(&value);
		}

		/**
		 * Get an node map property in its raw form.
		 * Not reccomended. Use ModernMPV::Properties::get_node_map() instead.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		inline void get_node_map_raw(SafeHandle& handle, const std::string& property_name, Functor callback) {
			MDN_GENERATE_GET_BODY(mpv_node, MPV_FORMAT_NODE);
			
			if(value.format != MPV_FORMAT_NODE_MAP) {
				mpv_free_node_contents(&value);
				return;
			}

			callback(value);
			mpv_free_node_contents(&value);
		}

		/**
		 * Get an node array property in its raw form.
		 * Not reccomended. Use ModernMPV::Properties::get_node_array() instead.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 * \param[out] callback Callback function
		 */
		template<class Functor>
		void get_node_array_raw(SafeHandle& handle, const std::string& property_name, Functor callback) {
			MDN_GENERATE_GET_BODY(mpv_node, MPV_FORMAT_NODE);
			
			if(value.format != MPV_FORMAT_NODE_ARRAY) {
				mpv_free_node_contents(&value);
				return;
			}

			callback(value);
			mpv_free_node_contents(&value);
		}

		/**
		 * Get an string property converted to a std::string.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 */
		inline std::string get_string(SafeHandle& handle, const std::string& property_name) {
			std::string str;

			get_string_raw(handle, property_name, [&](char* returned) {
				auto len = strlen(returned);
				str.resize(len);

				for(int i = 0; i < len; ++i)
					if(returned[i] != '\0')
						str.push_back(returned[i]);
			});

			return str;
		}

		/**
		 * Get an OSD-string property converted to a std::string.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 */
		inline std::string get_osd_string(SafeHandle& handle, const std::string& property_name) {
			std::string str;

			get_osd_string_raw(handle, property_name, [&](char* returned) {
				auto len = strlen(returned);
				str.resize(len);
				for(int i = 0; i < len; ++i)
					if(returned[i] != '\0')
						str.push_back(returned[i]);
			});

			return str;
		}

		/**
		 * Convert a existing string node to a std::string.
		 * \param[in] node Node to convert
		 */
		inline std::string get_node_string(mpv_node node) {
			std::string str;

			if(node.format != MPV_FORMAT_STRING)
				return str; 

			auto size = strlen(node.u.string);
			str.resize(size);

			for(int i = 0; i < size; ++i)
				if(node.u.string[i] != '\0')
					str.push_back(node.u.string[i]);

			return str;
		}

		/**
		 * Get an node map property converted to a C++ map.
		 * 
		 * \param[in] handle Safe handle to use
		 * \param[in] property_name Name of property to fetch
		 */
		inline std::map<std::string, mpv_node> get_node_map(SafeHandle& handle, const std::string& property_name) {
			std::map<std::string, mpv_node> values;

			get_node_map_raw(handle, property_name, [&](mpv_node node) {
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
		inline std::vector<mpv_node> get_node_array(SafeHandle& handle, const std::string& property_name) {
			std::vector<mpv_node> values;

			get_node_array_raw(handle, property_name, [&](mpv_node node) {
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
#undef MDN_GENERATE_GET_BODY
}
