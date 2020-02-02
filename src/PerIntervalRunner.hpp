#include <thread>
#include <mutex>
#include <functional>
#include <chrono>
#include <memory>

namespace mdrpc {

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
         * \param[in] interval Interval to call function,
         * \param[in] fun Function to call on an interval
         */
        template<class F>
        void Start(std::uint16_t interval, F fun) {
            if(runner_thread.get() != nullptr || started)
                Stop();

            runner_thread.reset(new std::thread(std::bind(&PerIntervalRunner::Runner<F>, this, interval, fun)));
        }

        /**
         * Start calling a function on an interval with arguments.
         * 
         * \param[in] interval Interval to call function,
         * \param[in] fun Function to call on an interval
         * \param[in] args All arguments to pass to the function
         */
        template<class F, class ...Args>
        void Start(std::uint16_t interval, F fun, Args... args) {
            if(runner_thread.get() != nullptr || started)
                Stop();
                
            runner_thread.reset(new std::thread(std::bind(&PerIntervalRunner::Runner<F, Args...>, this, interval, fun, std::forward<Args...>(args...))));
        }

        /**
         * Stop the current runner.
         */ 
        void Stop() {
            if(!started)
                return;
            
            stop_mutex.lock();
            stop = true;
            stop_mutex.unlock();
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
                std::lock_guard<std::mutex> lock(stop_mutex);
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
         * \param[in] interval Interval (in milliseconds) passed by Start<F, Args...>()
         * \param[in] fun Function to call
         */
        template<class F, class ...Args>
        void Runner(std::uint16_t interval, F fun, Args... args) {
            started = true;

            while(true) {
                std::lock_guard<std::mutex> lock(stop_mutex);
                if(stop)
                    break;

                fun(std::forward<Args...>(args...));
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
            }

            started = false;
        }

        /**
         * Runner thread.
         */ 
        std::shared_ptr<std::thread> runner_thread;


        /**
         * Whether or not the runner thread is active.
         */ 
        bool started = false;

        /**
         * Lock for the `stop` variable
         */
        std::mutex stop_mutex;

        /**
         * Indicates that the runner function should stop and kill the thread,
         * stopping execution of the user's specified function.
         */
        bool stop = false;
    };


}