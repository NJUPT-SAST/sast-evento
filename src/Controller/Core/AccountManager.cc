#include <Controller/AsyncExecutor.hh>
#include <Controller/Core/AccountManager.h>
#include <Controller/Core/UiUtility.h>
#include <Controller/UiBridge.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <Infrastructure/Utils/Config.h>
#include <Infrastructure/Utils/Result.h>
#include <chrono>
#include <keychain/keychain.h>
#include <optional>
#include <sast_link.h>
#include <spdlog/spdlog.h>

EVENTO_UI_START

AccountManager::AccountManager(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : GlobalAgent(uiEntry)
    , bridge(bridge)
    , renewAccessTokenTimer(evento::executor()->getIoContext()) {
    auto& self = *this;
    self->on_request_login([this] { return requestLogin(); });
    self->on_request_logout([this] { return requestLogout(); });
    loadConfig();
}

AccountManager::~AccountManager() {
    saveConfig();
}

bool AccountManager::isLogin() const {
    return loginState;
}

void AccountManager::performLogin() {
    evento::executor()->asyncExecute(
        []() -> evento::Task<Result<LoginResEntity>> {
            spdlog::info("Start login");
            auto codeResult = co_await sast_link::login();
            if (!codeResult) {
                spdlog::error("SAST Link Auth failed: {}", codeResult.error());
                co_return Err(Error(Error::Unknown, "Auth failed"));
            }
            spdlog::info("SAST Link Auth success");
            auto loginResult = co_await evento::networkClient()->loginViaSastLink(
                codeResult.value());
            if (loginResult.isErr()) {
                spdlog::error("Login failed: {}", loginResult.unwrapErr().what());
                co_return Err(Error(Error::Unknown, "Login failed"));
            }
            co_return loginResult.unwrap();
        }(),
        [&self = *this](Result<LoginResEntity> result) {
            if (result.isErr()) {
                self.setLoginState(false);
                self.bridge.getMessageManager().showMessage(result.unwrapErr().what(),
                                                            MessageType::Error);
                return;
            }
            spdlog::info("Login success");

            auto data = result.unwrap();

            self.userInfo = data.userInfo;
#ifdef EVENTO_API_V1
            self.setKeychainAccessToken(data.accessToken);
            self.expiredTime = std::chrono::system_clock::now() + std::chrono::days(3);
#else
            self.setKeychainRefreshToken(data.refreshToken);
            self.scheduleRenewAccessToken();
            self.expiredTime = std::chrono::system_clock::now() + std::chrono::days(7);
#endif
            self.setNetworkAccessToken(data.accessToken);
            self.setLoginState(true);
            self.saveConfig();
        });
}

void AccountManager::performRefreshToken() {
    auto& self = *this;
    evento::executor()->asyncExecute(
        [](std::optional<std::string> refreshToken) -> evento::Task<Result<std::monostate>> {
            if (!refreshToken) {
                spdlog::error("No refresh token found");
                co_return Err(Error(Error::Unknown, "No refresh token found"));
            }
            auto result = co_await evento::networkClient()->refreshAccessToken(*refreshToken);
            if (result.isErr()) {
                spdlog::error("Failed to refresh token: {}", result.unwrapErr().what());
                co_return Err(Error(Error::Network, "Failed to refresh token"));
            }
            co_return Ok(std::monostate{});
        }(this->getKeychainRefreshToken()),
        [&self = *this](Result<std::monostate> result) {
            if (result.isErr()) {
                self.setLoginState(false);
                self.bridge.getMessageManager().showMessage("登录过期，请重新登录",
                                                            MessageType::Info);
                return;
            }
            self.performGetUserInfo();
            spdlog::info("refresh token success");
        });
}

void AccountManager::performGetUserInfo() {
    auto& self = *this;
    evento::executor()->asyncExecute(
        []() -> evento::Task<Result<UserInfoEntity>> {
            auto result = co_await evento::networkClient()->getUserInfo();
            if (result.isErr()) {
                spdlog::error("Failed to get user info: {}", result.unwrapErr().what());
                co_return result.unwrapErr();
            }
            co_return result.unwrap();
        }(),
        [&self = *this](Result<UserInfoEntity> result) {
            if (result.isErr()) {
                if (result.unwrapErr().kind == Error::Data) {
                    self.bridge.getMessageManager().showMessage("登录过期，请重新登录",
                                                                MessageType::Info);
                } else {
                    self.bridge.getMessageManager().showMessage(result.unwrapErr().what(),
                                                                MessageType::Error);
                }
                self.setLoginState(false);
                return;
            }
            self.userInfo = result.unwrap();
            spdlog::info("get user info success");
            self.setLoginState(true);
        });
}

void AccountManager::requestLogin() {
    if (isLogin()) {
        return;
    }
    performLogin();
}

void AccountManager::requestLogout() {
    if (!isLogin()) {
        return;
    }
    auto& self = *this;
    self.userInfo = UserInfoEntity();
#ifndef EVENTO_API_V1
    setKeychainRefreshToken("");
    renewAccessTokenTimer.cancel();
#endif
    setNetworkAccessToken("");

    setLoginState(false);
}

void AccountManager::tryLoginDirectly() {
    if (isLogin()) {
        return;
    }
    if (std::chrono::system_clock::now() + 15min < expiredTime) {
        auto& self = *this;
        self.bridge.getMessageManager().showMessage("登录过期，请重新登录", MessageType::Info);
        return;
    }

#ifdef EVENTO_API_V1
    if (auto token = getKeychainAccessToken()) {
        spdlog::info("Token is found. Login directly!");
        setNetworkAccessToken(*token);
        performGetUserInfo();
    }
#else
    spdlog::info("Try login directly, expired time: {}", expiredTime.time_since_epoch().count());

    // If the token is not expired after 15min, we don't need to login again
    if (getKeychainRefreshToken()) {
        performRefreshToken();
    }
#endif
}

UserInfoEntity AccountManager::getUserInfo() {
    return userInfo;
}

void AccountManager::loadConfig() {
    setLoginState(false);
    auto [year, month, day] = evento::account.expire.date;
    auto [hour, minute, second, _] = evento::account.expire.time;
    std::tm t = {year, month, day, hour, minute, second};

    expiredTime = std::chrono::system_clock::from_time_t(std::mktime(&t));
    userInfo.id = evento::account.userId;
}

void AccountManager::saveConfig() {
    auto expire = std::chrono::system_clock::to_time_t(expiredTime);
    auto expireTm = *std::localtime(&expire);
    evento::account.expire
        = toml::date_time{toml::date{expireTm.tm_year + 1900, expireTm.tm_mon + 1, expireTm.tm_mday},
                          toml::time{expireTm.tm_hour, expireTm.tm_min, expireTm.tm_sec}};
    evento::account.userId = userInfo.id;
}

void AccountManager::setKeychainRefreshToken(const std::string& refreshToken) const {
    keychain::Error err;
    keychain::setPassword(package, service, userInfo.id, refreshToken, err);

    if (err.type != keychain::ErrorType::NoError) {
        spdlog::error("Failed to save refresh token: {}", err.message);
        return;
    }
    spdlog::info("Save refresh token success");
}

std::optional<std::string> AccountManager::getKeychainRefreshToken() const {
    keychain::Error err;
    auto refreshToken = keychain::getPassword(package, service, userInfo.id, err);

    if (err.type != keychain::ErrorType::NoError) {
        spdlog::error("Failed to get refresh token: {}", err.message);
        return std::nullopt;
    }

    if (refreshToken.empty()) {
        return std::nullopt;
    }

    return refreshToken;
}

void AccountManager::scheduleRenewAccessToken() {
    auto& self = *this;
    renewAccessTokenTimer.expires_after(55min);
    renewAccessTokenTimer.async_wait([&self = *this](const boost::system::error_code& ec) {
        if (ec) {
            spdlog::error("Failed to renew access token: {}", ec.message());
            return;
        }
        self.performRefreshToken();
    });
}

void AccountManager::setNetworkAccessToken(std::string accessToken) {
    evento::networkClient()->tokenBytes = accessToken;
}

#ifdef EVENTO_API_V1
std::optional<std::string> AccountManager::getKeychainAccessToken() const {
    keychain::Error err;
    auto accessToken = keychain::getPassword(package, service, userInfo.id, err);

    if (err.type != keychain::ErrorType::NoError) {
        debug(), (int) err.type;
        spdlog::error("Failed to get access token: {}", err.message);
        return std::nullopt;
    }

    if (accessToken.empty()) {
        return std::nullopt;
    }

    return accessToken;
}

void AccountManager::setKeychainAccessToken(const std::string& accessToken) const {
    keychain::Error err;
    keychain::setPassword(package, service, userInfo.id, accessToken, err);

    if (err.type != keychain::ErrorType::NoError) {
        debug(), (int) err.type;
        spdlog::error("Failed to save access token: {}", err.message);
        return;
    }
    spdlog::info("Save access token success");
}
#endif

void AccountManager::setLoginState(bool newState) {
    auto& self = *this;
    if (loginState != newState) {
        loginState = newState;
        self->set_is_login(newState);
        onStateChanged();
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