#include <Controller/AsyncExecutor.hh>
#include <Controller/Core/AccountManager.h>
#include <Controller/Core/UiUtility.h>
#include <Controller/UiBridge.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <Infrastructure/Utils/Config.h>
#include <Infrastructure/Utils/Result.h>
#include <keychain/keychain.h>
#include <optional>
#include <sast_link.h>
#include <spdlog/spdlog.h>

EVENTO_UI_START

AccountManager::AccountManager(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : GlobalAgent(uiEntry)
    , bridge(bridge) {
    auto& self = *this;
    self->on_request_login([this] { return requestLogin(); });
    self->on_request_logout([this] { return requestLogout(); });
    // TODO

    loadConfig();
}

AccountManager::~AccountManager() {
    saveConfig();
}

bool AccountManager::isLogin() {
    return loginState;
}

void AccountManager::requestLogin() {
    if (isLogin()) {
        return;
    }
    evento::executor()->asyncExecute(
        []() -> evento::Task<Result<LoginResEntity>> {
            auto codeResult = co_await sast_link::login();
            if (!codeResult) {
                spdlog::error("Login failed: {}\n", codeResult.error());
                co_return Err(Error(Error::Unknown, "Login failed"));
            }

            auto loginResult = co_await evento::networkClient()->loginViaSastLink(
                codeResult.value());
            if (loginResult.isErr()) {
                spdlog::error("Login failed: {}\n", loginResult.unwrapErr().what());
                co_return Err(Error(Error::Unknown, "Login failed"));
            }

            co_return loginResult.unwrap();
        }(),
        [weak_this = weak_from_this(), &self = *this, this](Result<LoginResEntity> result) {
            if (auto alive = weak_this.lock()) {
                if (result.isErr()) {
                    // TODO: onLoginFail();
                    return;
                }
                auto data = result.unwrap();

                self.userInfo = data.userInfo;

                setKeychainRefreshToken(data.refreshToken);
                setNetworkAccessToken(data.accessToken);
                scheduleRenewAccessToken();
                scheduleRenewRefreshToken();

                slint::invoke_from_event_loop([&self]() { self.setLoginState(true); });
                spdlog::info("login success");
            }
        });
}

void AccountManager::requestLogout() {
    // TODO: net logout
}

void AccountManager::loadConfig() {
    setLoginState(false);
    // TODO: load last time
}

void AccountManager::saveConfig() {
    // TODO: save
}

void AccountManager::setKeychainRefreshToken(const std::string& refreshToken) {
    keychain::Error err;
    keychain::setPassword(package, service, userInfo.id, refreshToken, err);

    if (err.code != 0) {
        spdlog::error("Failed to save refresh token: {}\n", err.message);
    }
}

std::string AccountManager::getKeychainRefreshToken() {
    keychain::Error err;
    auto refreshToken = keychain::getPassword(package, service, userInfo.id, err);

    if (err.code != 0) {
        spdlog::error("Failed to save refresh token: {}\n", err.message);
    }

    return refreshToken;
}

void AccountManager::scheduleRenewAccessToken() {
    // TODO
}

void AccountManager::scheduleRenewRefreshToken() {
    // TODO
}

void AccountManager::setNetworkAccessToken(std::string accessToken) {
    evento::networkClient()->tokenBytes = accessToken;
}

void AccountManager::setLoginState(bool newState) {
    auto& self = *this;
    if (loginState != newState) {
        loginState = newState;
        self->set_is_login(newState);
    }
}

void AccountManager::onStateChanged() {
    if (isLogin()) {
        UiUtility::StylishLog::viewActionTriggered(logOrigin, "onLogin");
        bridge.call(bridge.actions.onLogin);
    } else {
        UiUtility::StylishLog::viewActionTriggered(logOrigin, "onLogout");
        bridge.call(bridge.actions.onLogout);
    }
}

EVENTO_UI_END