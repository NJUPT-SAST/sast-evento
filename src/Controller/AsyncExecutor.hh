#pragma once

#include <bitset>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/static_thread_pool.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/detail/error_code.hpp>
#include <chrono>
#include <memory>
#include <slint.h>
#include <spdlog/spdlog.h>
#include <thread>
#include <utility>

using namespace boost::asio::experimental::awaitable_operators;

namespace evento {

namespace net = boost::asio; // from <boost/asio.hpp>

template<typename T>
using Task = net::awaitable<T>;

class AsyncExecutor {
public:
    AsyncExecutor(const AsyncExecutor&) = delete;
    AsyncExecutor& operator=(const AsyncExecutor&) = delete;

    enum TimerFlag {
        Immediate = 1,
        Delay = 1 << 1,
        Once = 1 << 2,
        Periodic = 1 << 3,
    };

    /**
     * @brief            execute a coroutine and call the callback when it's done
     *
     * @param task       return value of a coroutine,
     *                   an awaitable object wrapping a T type value
     *
     * @param callback   callback function, called in the main thread when coroutine is done
     *                   parameter: any based on T(awaitable object wrapped type) except T&&   
    */
    template<typename T, BOOST_ASIO_COMPLETION_TOKEN_FOR(void(T&)) CompletionCallback>
    void asyncExecute(Task<T> task, CompletionCallback&& callback) {
        net::co_spawn(_ioc,
                      std::move(task),
                      [callback = std::forward<CompletionCallback>(callback)](std::exception_ptr e,
                                                                              T value) {
                          if (!e) {
                              slint::invoke_from_event_loop(
                                  [callback = std::move(callback), value = std::move(value)]() {
                                      callback(std::move(value));
                                  });
                              return;
                          }
                          try {
                              std::rethrow_exception(e);
                          } catch (std::exception& ex) {
                              spdlog::error(ex.what());
                          }
                      });
    }

    /**
     * @brief            execute a coroutine and call the callback when it's done
     *
     * @param task       return value of a coroutine, 
     *                   an awaitable object wrapping a T type value
     *
     * @param callback   callback function, called in the main thread when coroutine is done
     *                   parameter: void
     *
     * Specialization for "void"
    */
    template<BOOST_ASIO_COMPLETION_TOKEN_FOR(void()) CompletionCallback>
    void asyncExecute(Task<void> task, CompletionCallback&& callback) {
        net::co_spawn(_ioc,
                      std::move(task),
                      [callback = std::forward<CompletionCallback>(callback)](std::exception_ptr e) {
                          if (!e) {
                              slint::invoke_from_event_loop(callback);
                              return;
                          }
                          try {
                              std::rethrow_exception(e);
                          } catch (std::exception& ex) {
                              spdlog::error(ex.what());
                          }
                      });
    }

    /**
     * @brief            execute a coroutine and call the callback when it's done at specific intervals using a timer
     *
     * @param func       coroutine function pointer
     *
     * @param callback   callback function, called in the main thread when coroutine is done periodically
     *                   parameter: any based on T(awaitable object wrapped type) except T&&
     *
     * @param interval   interval between each coroutine call
     * 
     * @param flag       the strategy of the timer, MUST use `|` to combine two enum values below:
     *                   * Immediate: execute the coroutine immediately
     *                   * Delay: execute the coroutine after the interval
     *                   * Once: execute the coroutine once
     *                   * Periodic: execute the coroutine every interval periodically
     *
     *                   FORBIDDEN combinations:
     *                   * Immediate and Delay
     *                   * Periodic and Once
     */
    template<typename TaskFunc, typename CompletionCallback>
    void asyncExecute(TaskFunc&& func,
                      CompletionCallback&& callback,
                      std::chrono::steady_clock::duration interval,
                      int flag = TimerFlag::Periodic | TimerFlag::Immediate) {
        assert(std::bitset<32>(flag).count() == 2);
        assert(!(flag & TimerFlag::Immediate && flag & TimerFlag::Delay));
        assert(!(flag & TimerFlag::Periodic && flag & TimerFlag::Once));

        if (flag & TimerFlag::Immediate)
            asyncExecute(func(), callback);

        if (flag & TimerFlag::Periodic || flag & TimerFlag::Delay)
            asyncExecuteByTimer(std::forward<TaskFunc>(func),
                                std::forward<CompletionCallback>(callback),
                                interval,
                                flag);
    }

    net::io_context& getIoContext() { return _ioc; }

    ~AsyncExecutor() {
        _ioc.stop();
        if (_iocThread.joinable()) {
            _iocThread.join();
        }
    }

private:
    AsyncExecutor() {
        _iocThread = std::thread([this] {
            net::signal_set signals{_ioc, SIGINT, SIGTERM};
            signals.async_wait([&](auto, auto) { _ioc.stop(); });
            net::executor_work_guard<decltype(_ioc.get_executor())> work{_ioc.get_executor()};
            _ioc.run();
        });
    }

    static AsyncExecutor* getInstance() {
        static AsyncExecutor s_instance;
        return &s_instance;
    }

    template<typename TaskFunc, typename CompletionCallback>
        requires std::is_invocable_v<CompletionCallback>
    void asyncExecuteByTimer(TaskFunc&& func,
                             CompletionCallback&& callback,
                             std::chrono::steady_clock::duration interval,
                             int flag) {
        auto timer = std::make_shared<net::steady_timer>(_ioc, interval);
        timer->async_wait([=,
                           func = std::forward<TaskFunc>(func),
                           callback = std::forward<CompletionCallback>(callback),
                           this](const boost::system::error_code& ec) {
            if (!ec) {
                // ensure timer is captured
                timer.get();
                net::co_spawn(_ioc, func(), [callback](std::exception_ptr e) {
                    if (!e) {
                        slint::invoke_from_event_loop(callback);
                        return;
                    }
                    try {
                        std::rethrow_exception(e);
                    } catch (std::exception& ex) {
                        spdlog::error(ex.what());
                    }
                });
                if (flag & TimerFlag::Periodic) {
                    asyncExecuteByTimer(std::move(func), std::move(callback), interval, flag);
                }
            } else {
                spdlog::error(ec.what());
            }
        });
    }

    template<typename TaskFunc, typename CompletionCallback>
        requires(!std::is_same_v<net::awaitable<void>, std::invoke_result_t<TaskFunc>>)
    void asyncExecuteByTimer(TaskFunc&& func,
                             CompletionCallback&& callback,
                             std::chrono::steady_clock::duration interval,
                             int flag) {
        auto timer = std::make_shared<net::steady_timer>(_ioc, interval);
        timer->async_wait([=,
                           func = std::forward<TaskFunc>(func),
                           callback = std::forward<CompletionCallback>(callback),
                           this](const boost::system::error_code& ec) {
            if (!ec) {
                // ensure timer is captured
                timer.get();
                net::co_spawn(_ioc, func(), [callback](std::exception_ptr e, auto value) {
                    if (!e) {
                        slint::invoke_from_event_loop(
                            [callback = std::move(callback), value = std::move(value)]() {
                                callback(std::move(value));
                            });
                        return;
                    }
                    try {
                        std::rethrow_exception(e);
                    } catch (std::exception& ex) {
                        spdlog::error(ex.what());
                    }
                });
                if (flag & TimerFlag::Periodic) {
                    asyncExecuteByTimer(std::move(func), std::move(callback), interval, flag);
                }
            } else {
                spdlog::error(ec.what());
            }
        });
    }

private:
    net::io_context _ioc;
    std::thread _iocThread;

    friend AsyncExecutor* executor();
};

inline AsyncExecutor* executor() {
    return AsyncExecutor::getInstance();
}

} // namespace evento