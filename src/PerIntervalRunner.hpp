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
         * Start a function without any arguments
         */
        template<class F>
        void Start(std::uint16_t interval, F fun) {
            runner_thread.reset(new std::thread(std::bind(&PerIntervalRunner::Runner<F>, this, interval, fun)));
        }

        /**
         * Start a function with arguments
         */
        template<class F, class ...Args>
        void Start(std::uint16_t interval, F fun, Args... args) {
            runner_thread.reset(new std::thread(std::bind(&PerIntervalRunner::Runner<F, Args...>, this, interval, fun, std::forward<Args...>(args...))));
        }

        void Stop() {
            stop_mutex.lock();
            stop = true;
            stop_mutex.unlock();
        }
    private:

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

        std::shared_ptr<std::thread> runner_thread;
        bool started = false;

        std::mutex stop_mutex;
        bool stop = false;
    };


}