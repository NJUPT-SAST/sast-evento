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
    , _renewAccessTokenTimer(evento::executor()->getIoContext()) {
    auto& self = *this;
    self->on_request_login([this] { return requestLogin(); });
    self->on_request_logout([this] { return requestLogout(); });
    loadConfig();
}

AccountManager::~AccountManager() {
    saveConfig();
}

bool AccountManager::isLogin() const {
    return _loginState;
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

            self.setKeychainRefreshToken(data.refreshToken);
            self.scheduleRenewAccessToken();
            self._expiredTime = std::chrono::system_clock::now() + std::chrono::days(7);

            self.setNetworkAccessToken(data.accessToken);
            self.performGetUserInfo();
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
            self._userInfo = result.unwrap();
            spdlog::info("get user info success");
            self.setLoginState(true);
            self.saveConfig();
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
    self._userInfo = UserInfoEntity();

    setKeychainRefreshToken("");
    _renewAccessTokenTimer.cancel();

    setNetworkAccessToken("");

    setLoginState(false);
}

void AccountManager::tryLoginDirectly() {
    if (isLogin()) {
        return;
    }
    auto& self = *this;
    if (std::chrono::system_clock::now() + 15min < _expiredTime) {
        self.bridge.getMessageManager().showMessage("登录过期，请重新登录", MessageType::Info);
        return;
    }

    spdlog::info("Try login directly, expired time: {}", _expiredTime.time_since_epoch().count());
    self->set_loading(true);

    // If the token is not expired after 15min, we don't need to login again
    if (getKeychainRefreshToken()) {
        performRefreshToken();
    } else {
        self->set_loading(false);
        self.bridge.getMessageManager().showMessage("登录过期，请重新登录", MessageType::Info);
    }
}

UserInfoEntity& AccountManager::userInfo() {
    return _userInfo;
}

void AccountManager::loadConfig() {
    setLoginState(false);
    auto [year, month, day] = evento::account.expire.date;
    auto [hour, minute, second, _] = evento::account.expire.time;
    std::tm t = {year, month, day, hour, minute, second};

    _expiredTime = std::chrono::system_clock::from_time_t(std::mktime(&t));
    _userInfo.linkId = evento::account.userId;
}

void AccountManager::saveConfig() {
    auto expire = std::chrono::system_clock::to_time_t(_expiredTime);
    auto expireTm = *std::localtime(&expire);
    evento::account.expire
        = toml::date_time{toml::date{expireTm.tm_year + 1900, expireTm.tm_mon + 1, expireTm.tm_mday},
                          toml::time{expireTm.tm_hour, expireTm.tm_min, expireTm.tm_sec}};
    evento::account.userId = _userInfo.linkId;
}

void AccountManager::setKeychainRefreshToken(const std::string& refreshToken) const {
    keychain::Error err;
    keychain::setPassword(package, service, _userInfo.linkId, refreshToken, err);

    if (err.type != keychain::ErrorType::NoError) {
        spdlog::error("Failed to save refresh token: {}", err.message);
        return;
    }
    spdlog::info("Save refresh token success");
}

std::optional<std::string> AccountManager::getKeychainRefreshToken() const {
    keychain::Error err;
    auto refreshToken = keychain::getPassword(package, service, _userInfo.linkId, err);

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
    _renewAccessTokenTimer.expires_after(55min);
    _renewAccessTokenTimer.async_wait([&self = *this](const boost::system::error_code& ec) {
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

// #ifdef EVENTO_API_V1
std::optional<std::string> AccountManager::getKeychainAccessToken() const {
    if (!settings.autoLogin) {
        return std::nullopt;
    }

    keychain::Error err;
    auto accessToken = keychain::getPassword(package, service, _userInfo.linkId, err);

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
    if (!settings.autoLogin) {
        return;
    }
    keychain::Error err;
    keychain::setPassword(package, service, _userInfo.linkId, accessToken, err);

    if (err.type != keychain::ErrorType::NoError) {
        debug(), (int) err.type;
        spdlog::error("Failed to save access token: {}", err.message);
        return;
    }
    spdlog::info("Save access token success");
}
// #endif

void AccountManager::setLoginState(bool newState) {
    auto& self = *this;
    self->set_loading(false);
    if (_loginState != newState) {
        _loginState = newState;
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