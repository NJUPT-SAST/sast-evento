#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Utils/Logger.h>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl/context.hpp>
#include <keychain/keychain.h>
#include <openssl/ssl.h>
#include <sast_link.h>
#include <spdlog/spdlog.h>
#include <thread>

namespace net = boost::asio;
namespace ssl = net::ssl;

void start_sast_link() {
    std::thread t{[]() {
        net::io_context ioc;

        ssl::context ctx(ssl::context::sslv23);
        ctx.set_default_verify_paths();

        evento::NetworkClient client(ctx);

        net::co_spawn(
            ioc,
            [&client]() -> net::awaitable<void> {
                auto codeResult = co_await sast_link::login();
                if (!codeResult) {
                    spdlog::error("Login failed: {}\n", codeResult.error());
                    co_return;
                }
                auto code = codeResult.value();

                auto loginResult = co_await client.loginViaSastLink(code);
                if (loginResult.isErr()) {
                    spdlog::error("Login failed: {}\n", loginResult.unwrapErr().what());
                    co_return;
                }

                auto entity = loginResult.unwrap();
                client.token() = entity.accessToken;

                std::string const package = "org.sast.evento";
                std::string const service = "refresh-token";

                keychain::Error err;
                keychain::setPassword(package, service, entity.userInfo.id, entity.refreshToken, err);

                if (err.code != 0) {
                    spdlog::error("Failed to save refresh token: {}\n", err.message);
                }

                spdlog::info("Login successful\n");
            },
            [](std::exception_ptr e) {
                if (!e)
                    return;
                try {
                    std::rethrow_exception(e);
                } catch (std::exception& ex) {
                    spdlog::error(ex.what());
                }
            });
        net::signal_set signals{ioc, SIGINT, SIGTERM};
        signals.async_wait([&ioc](auto, auto) { ioc.stop(); });
        ioc.run();
    }};
    t.detach();
}
