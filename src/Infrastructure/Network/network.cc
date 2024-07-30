#include <Infrastructure/Utils/Logger.h>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <sast_link.h>
#include <thread>

namespace net = boost::asio;

void start_sast_link() {
    std::thread t{[]() {
        net::io_context ioc;
        net::co_spawn(
            ioc,
            []() -> net::awaitable<void> {
                auto result = co_await sast_link::login();
                if (result) {
                    spdlog::info("code: {}\n", result.value());
                } else {
                    spdlog::error("Login failed: {}\n", result.error());
                }
            },
            net::detached);
        net::signal_set signals{ioc, SIGINT, SIGTERM};
        signals.async_wait([&ioc](auto, auto) { ioc.stop(); });
        ioc.run();
    }};
    t.detach();
}
