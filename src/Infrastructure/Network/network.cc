#include <Controller/AsyncExecutor.hh>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Utils/Logger.h>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ssl/context.hpp>
#include <keychain/keychain.h>
#include <openssl/ssl.h>
#include <sast_link.h>
#include <spdlog/spdlog.h>

namespace net = boost::asio;

void start_sast_link() {
    evento::executor()->asyncExecute(
        []() -> net::awaitable<void> {
            auto codeResult = co_await sast_link::login();
            if (!codeResult) {
                spdlog::error("Login failed: {}\n", codeResult.error());
                co_return;
            }
            auto code = codeResult.value();

            auto loginResult = co_await evento::networkClient()->loginViaSastLink(code);
            if (loginResult.isErr()) {
                spdlog::error("Login failed: {}\n", loginResult.unwrapErr().what());
                co_return;
            }

            auto entity = loginResult.unwrap();
            evento::networkClient()->tokenBytes = entity.accessToken;

            std::string const package = "org.sast.evento";
            std::string const service = "refresh-token";

            keychain::Error err;
            keychain::setPassword(package, service, entity.userInfo.id, entity.refreshToken, err);

            if (err.code != 0) {
                spdlog::error("Failed to save refresh token: {}\n", err.message);
            }

            spdlog::info("Login successful\n");
        }(),
        []() { spdlog::debug("operation \"start_sast_link\" done."); });
}
