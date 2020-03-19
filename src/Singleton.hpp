#include <algorithm>

#ifdef DOXYGEN
namespace Utils {
#else
namespace Utils LOCAL_SYM {
#endif
	
	/**
	 * Class to allow only one single instance of
	 * T to be allocated.
	 *
	 * \tparam T Type that should be made a singleton.
	 */
	template<typename T>
	struct Singleton {

		/**
		 * Destructor for RAII-ness.
		 */
		inline ~Singleton() {
			Destroy();
		}		

		/**
		 * Gets the instance. Allocates instance if not allocated
		 */
		template<class... Args>
		inline T& Get(Args... args) {
			if(!instance_)
				instance_ = new T(std::forward<Args>(args)...);

			if(!instance_)
				throw std::runtime_error("Allocation failure");
			
			return *instance_;
		}
		
		/**
		 * Destroy held instance if one exists.
		 */
		inline void Destroy() {
		    if(instance_)
				delete instance_;
		}
	private:
		/**
		 * Internal single instance of T.
		 */
		static T* instance_;
	};
	
	template<typename T>
	T* Singleton<T>::instance_ = nullptr;

}
