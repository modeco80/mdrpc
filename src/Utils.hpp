#include <iostream>
#include <vector>

namespace Utils {

		/**
		 * Function template that writes value into stream if it the given expression is false.
		 *
		 * \param[in] stream Stream to write to
		 * \param[in] value Value to write in
		 * \param[in] fun Function to call
		 * \tparam T Type
		 * \tparam TOStream Output stream type
		 * \tparam TFun Function (Must return bool)
		 */
		template<typename T, typename TOStream, class TFun>
		inline bool AddIf(TOStream& stream, const T& value, TFun expr) {
			if(!expr(value)) {
				if(!stream)
					// insertion will most likely fail
					return false;

				stream << value;
				return true;
			}
			return false;
		}

		/**
		 * Function template to convert a string to a C string (of the string's underlying value type)
		 * \tparam String String type to convert, must follow ReversibleContainer like std::basic_string<CharT>
		 */
		template<typename String>
		inline std::vector<typename String::value_type> StringToC(const String& str) {
			std::vector<char> conv;
			conv.resize(str.length() + 1);

			// filtration because mpv injects random NULs
			std::remove_copy_if(str.begin(), str.end(), conv.begin(), [](typename String::value_type c) {
				return c == '\0';
			});

			// put in a NUL at proper place
			conv.push_back('\0');
			return conv;
		}

}
