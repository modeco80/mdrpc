#include <algorithm>

namespace Utils {
	
	template<typename T>
	struct Singleton {
		
		/**
		 * Gets the instance. Allocates instance if currently null
		 */
		template<class... Args>
		T& Get(Args... args) {
			if(!instance_)
				instance_ = new T(std::forward<Args>(args)...);

			if(!instance_)
				throw std::runtime_error("Allocation failure");
			
			return *instance_;
		}
		
		/**
		 * Destroy held instance
		 */
		void Destroy() {
		    if(instance_)
				delete instance_;
		}
	private:
		/**
		 * Internal instance.
		 */
		static T* instance_;
	};
	
	template<class T>
	T* Singleton<T>::instance_ = nullptr;

}
