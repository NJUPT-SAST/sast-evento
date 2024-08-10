#include <Controller/AsyncExecutor.hh>
#include <Controller/View/LoginOverlay.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Version.h>
#include <keychain/keychain.h>
#include <sast_link.h>

EVENTO_UI_START

LoginOverlay::LoginOverlay(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void LoginOverlay::onCreate() {
    auto& self = *this;

    self->on_link_login(LoginOverlay::login);
    self->set_version("v" VERSION_FULL);
}

void LoginOverlay::login() {
    evento::executor()->asyncExecute(
        []() -> evento::Task<void> {
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

EVENTO_UI_END