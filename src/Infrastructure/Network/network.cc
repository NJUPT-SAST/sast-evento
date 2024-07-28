#include <Infrastructure/Utils/Logger.h>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
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
                    LOG_INFO("code: {}\n", result.value());
                } else {
                    LOG_ERROR("Login failed: {}\n", result.error());
                }
            },
            net::detached);
        ioc.run();
    }};
    t.detach();
}
