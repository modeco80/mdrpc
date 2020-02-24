#include <thread>
#include <mutex>
#include <functional>
#include <chrono>
#include <memory>

//
// define PIR_NOT_USED_BY_MDRPC to gain access to a Start() overload that can handle calling 
// the interval function with arguments
//

namespace Utils {

    /**
     * Runs code at an specified interval in a new thread.
     */
    struct PerIntervalRunner {

        ~PerIntervalRunner() {
            if(runner_thread.get() != nullptr) {
                if(runner_thread->joinable())
                    runner_thread->join();
                else
                    runner_thread.reset();
            }
        }

        /**
         * Start calling a function on an interval without arguments.
         * 
         * \param[in] interval Interval to call function
         * \param[in] fun Function to call on an interval
         */
        template<class F>
        void Start(std::uint16_t interval, F fun) {
            if(runner_thread.get() != nullptr || started)
                Stop();

            runner_thread.reset(new std::thread(std::bind(&PerIntervalRunner::Runner<F>, this, interval, fun)));
        }

        /**
         * Start calling a function on an interval without arguments.
         * Also calls an init function
         * 
         * \param[in] interval Interval to call function
         * \param[in] fun Function to call on an interval
         */
        template<class F, class FInit>
        void Start(std::uint16_t interval, F fun, FInit initFun) {
            if(runner_thread.get() != nullptr || started)
                Stop();

            runner_thread.reset(new std::thread(std::bind(&PerIntervalRunner::Runner<F, FInit>, this, interval, fun, initFun)));
        }

#ifdef PIR_NOT_USED_BY_MDRPC
        /**
         * Start calling a function on an interval with arguments.
         * 
         * \param[in] interval Interval to call function
         * \param[in] fun Function to call on an interval
         * \param[in] args All arguments to pass to the function
         */
        template<class F, class ...Args>
        void Start(std::uint16_t interval, F fun, Args... args) {
            if(runner_thread.get() != nullptr || started)
                Stop();
                
            runner_thread.reset(new std::thread(std::bind(&PerIntervalRunner::Runner<F, Args...>, this, interval, fun, std::forward<Args...>(args...))));
        }
#endif

        /**
         * Stop the current runner.
         */ 
        void Stop() {
            if(!started)
                return;
            
            stop = true;
        }

		/**
		 * Returns true if this runner is currently started.
		 */
		const bool Running() const {
			return started; 
		}

    private:

        /**
         * Generic function for the the thread to run.
         * 
         * \param[in] interval Interval (in milliseconds) passed by Start<F>()
         * \param[in] fun Function to call
         */
        template<class F>
        void Runner(std::uint16_t interval, F fun) {
            started = true;

            while(true) {
                if(stop)
                    break;

                fun();
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }

            started = false;
        }

       /**
         * Generic function for the the thread to run.
         * 
         * \param[in] interval Interval (in milliseconds) passed by Start<F>()
         * \param[in] fun Function to call
         * \param[in] initFun run on thread init
         */
        template<class F, class FInit>
        void Runner(std::uint16_t interval, F fun, FInit initFun) {
            started = true;

            initFun();

            while(true) {
                if(stop)
                    break;

                fun();
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }

            started = false;
        }

#ifdef PIR_NOT_USED_BY_MDRPC
        /**
         * Generic function for the the thread to run.
         * 
         * \param[in] interval Interval (in milliseconds) passed by Start<F, Args...>()
         * \param[in] fun Function to call
         */
        template<class F, class ...Args>
        void Runner(std::uint16_t interval, F fun, Args... args) {
            started = true;

            while(true) {
                if(stop)
                    break;

                fun(std::forward<Args...>(args...));
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }

            started = false;
        }
#endif

        /**
         * Runner thread.
         */ 
        std::shared_ptr<std::thread> runner_thread;


        /**
         * Whether or not the runner thread is active.
         */ 
        bool started = false;

        /**
         * Indicates that the runner function should stop and kill the thread,
         * stopping execution of the user's specified function.
         */
        bool stop = false;
    };


}
