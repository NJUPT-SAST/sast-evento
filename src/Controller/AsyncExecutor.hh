#pragma once

#include "slint.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/system/detail/error_code.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <thread>
#include <utility>
#include <vector>

using namespace boost::asio::experimental::awaitable_operators;

namespace evento {

namespace net = boost::asio; // from <boost/asio.hpp>

template<typename T>
using Task = net::awaitable<T>;

class AsyncExecutor {
public:
    AsyncExecutor(const AsyncExecutor&) = delete;
    AsyncExecutor& operator==(const AsyncExecutor&) = delete;

    /**
     * @param task:      return value of a coroutine,
     *                   an awaitable object wrapping a T type value
     *
     * @param callback:  callback function, called in the main thread when coroutine is done
     *                   parameter: any based on T except T&&   
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
     * @param task:      return value of a coroutine, 
     *                   an awaitable object wrapping a T type value
     *
     * @param callback:  callback function, called in the main thread when coroutine is done
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
     * @param func:      coroutine function pointer
     *
     * @param callback:  callback function, called in the main thread when coroutine is done periodically
     *                   parameters: (std::exception_ptr, T Type)
     *                   T Type: any based on T(awaitable object wrapped type) except T&&
     *
     * @param interval:  interval between each coroutine call
     */
    template<typename TaskFunc,
             BOOST_ASIO_COMPLETION_TOKEN_FOR(
                 typename net::detail::awaitable_signature<net::result_of_t<TaskFunc()>>::type)
                 CompletionCallback>
    void asyncExecute(TaskFunc&& func,
                      CompletionCallback&& callback,
                      std::chrono::steady_clock::duration interval) {
        asyncExecuteByTimer(std::forward<TaskFunc>(func),
                            std::forward<CompletionCallback>(callback),
                            interval);
    }

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
            _ioc.run();
        });
    }

    static AsyncExecutor* getInstance() {
        static AsyncExecutor s_instance;
        return &s_instance;
    }

    template<typename TaskFunc,
             BOOST_ASIO_COMPLETION_TOKEN_FOR(
                 typename net::detail::awaitable_signature<net::result_of_t<TaskFunc()>>::type)
                 CompletionCallback>
    void asyncExecuteByTimer(TaskFunc&& func,
                             CompletionCallback&& callback,
                             std::chrono::steady_clock::duration interval) {
        auto timer = std::make_shared<net::steady_timer>(_ioc, interval);
        asyncExecuteByTimerHelper(std::forward<TaskFunc>(func),
                                  std::forward<CompletionCallback>(callback),
                                  _taskId);
        ++_taskId;
        _timers.push_back(timer);
    }

    template<typename TaskFunc,
             BOOST_ASIO_COMPLETION_TOKEN_FOR(
                 typename net::detail::awaitable_signature<net::result_of_t<TaskFunc()>>::type)
                 CompletionCallback>
    void asyncExecuteByTimerHelper(TaskFunc&& func, CompletionCallback&& callback, int taskId) {
        _timers[taskId]->async_wait([=,
                                     func = std::forward<TaskFunc>(func),
                                     callback = std::forward<CompletionCallback>(callback),
                                     this](const boost::system::error_code& ec) {
            if (!ec) {
                net::co_spawn(_ioc, func, [callback]() { slint::invoke_from_event_loop(callback); });
                asyncExecuteByTimerHelper(std::move(func), std::move(callback), taskId);
            } else {
                spdlog::error(ec.what());
            }
        });
    }

private:
    net::io_context _ioc;
    std::thread _iocThread;
    std::vector<std::shared_ptr<net::steady_timer>> _timers;
    inline static int _taskId = 0;

    friend AsyncExecutor* executor();
};

inline AsyncExecutor* executor() {
    return AsyncExecutor::getInstance();
}

} // namespace evento