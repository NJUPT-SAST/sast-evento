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
#include <variant> // 添加这个头文件以使用 std::monostate

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
    /*
    bool assumeLoginSuccess = true; // 可以根据需要设置条件
    if (assumeLoginSuccess) {
        spdlog::info("Assume login success for debugging purposes");
        this->setLoginState(true);
        this->userInfo = UserInfoEntity{
            .id = "dummy_id",
            .linkId = "dummy_link_id",
            .studentId = "dummy_student_id",
            .email = "dummy_email",
            .nickname = "dummy_nickname",
            .avatar = "dummy_avatar",
            .organization = "dummy_organization",
            .biography = "dummy_biography",
            .link = {"dummy_link1", "dummy_link2"},
        };
        this->setKeychainRefreshToken("dummy_refresh_token");
        this->setNetworkAccessToken("dummy_access_token");
        this->scheduleRenewAccessToken();
        spdlog::info("Assumed login success");
        return;
    }
    */
    
    evento::executor()->asyncExecute(
        []() -> evento::Task<Result<LoginResEntity>> {
            spdlog::info("Start login");
            auto codeResult = co_await sast_link::login();
            if (!codeResult) {
                spdlog::error("Login failed: {}\n", codeResult.error());
                co_return Err(Error(Error::Unknown, "Login failed"));
            }
            spdlog::info("Login success");
            auto loginResult = co_await evento::networkClient()->loginViaSastLink(
                codeResult.value());
            if (loginResult.isErr()) {
                spdlog::error("Login failed: {}\n", loginResult.unwrapErr().what());
                co_return Err(Error(Error::Unknown, "Login failed"));
            }
            spdlog::info("Login success");
            co_return loginResult.unwrap();
        }(),
        [&self = *this](Result<LoginResEntity> result) {
            if (result.isErr()) {
                self.setLoginState(false);
                return;
            }
            auto data = result.unwrap();

            self.userInfo = data.userInfo;

            self.setKeychainRefreshToken(data.refreshToken);
            self.setNetworkAccessToken(data.accessToken);
            self.scheduleRenewAccessToken();

            self.setLoginState(true);
            spdlog::info("login success");
        });
}

void AccountManager::performRefreshToken() {
    auto& self = *this;
    evento::executor()->asyncExecute(
        [refreshToken = getKeychainRefreshToken()]() -> evento::Task<Result<std::monostate>> {
            auto result = co_await evento::networkClient()->refreshAccessToken(
                refreshToken.value_or("NONE"));
            if (result.isErr()) {
                spdlog::error("Failed to refresh token: {}\n", result.unwrapErr().what());
                co_return Err(Error(Error::Network, "Failed to refresh token"));
            }
            co_return Ok(std::monostate{});
        }(),
        [weak_this = weak_from_this(), &self](Result<std::monostate> result) {
            if (auto alive = weak_this.lock()) {
                if (result.isErr()) {
                    self.setLoginState(false);
                    return;
                }
                spdlog::info("refresh token success");
            }
        });
}

void AccountManager::performGetUserInfo() {
    auto& self = *this;
    evento::executor()->asyncExecute(
        []() -> evento::Task<Result<UserInfoEntity>> {
            auto result = co_await evento::networkClient()->getUserInfo();
            if (result.isErr()) {
                spdlog::error("Failed to get user info: {}\n", result.unwrapErr().what());
                co_return Err(Error(Error::Unknown, "Failed to get user info"));
            }
            co_return result.unwrap();
        }(),
        [weak_this = weak_from_this(), &self](Result<UserInfoEntity> result) {
            if (auto alive = weak_this.lock()) {
                if (result.isErr()) {
                    self.setLoginState(false);
                    return;
                }
                self.userInfo = result.unwrap();
                spdlog::info("get user info success");
            }
        });
}

void AccountManager::requestLogin() {
    if (isLogin()) {
        return;
    }
    // If the token is expired, login again
    if (expiredTime - 15min >= std::chrono::system_clock::now()) {
        performRefreshToken();
        performGetUserInfo();
        scheduleRenewAccessToken();
        scheduleRenewRefreshToken();
        setLoginState(true);
        return;
    }
    performLogin();
}

void AccountManager::requestLogout() {
    if (!isLogin()) {
        return;
    }
    // TODO: net logout
    auto& self = *this;

    // TODO: 只清除本地的登录状态和相关的令牌
    // TODO: 通知后端登出
    self.userInfo = UserInfoEntity();
    setKeychainRefreshToken("");
    setNetworkAccessToken("");

    // 停止刷新令牌的定时器
    renewAccessTokenTimer.cancel();

    // if (logoutSuccess) {
    setLoginState(false);
    // }
}

UserInfoEntity AccountManager::getUserInfo() {
    return userInfo;
}

void AccountManager::loadConfig() {
    setLoginState(false);
    // TODO: load last time
    auto [year, month, day] = evento::account.expire.date;
    auto [hour, minute, second, _] = evento::account.expire.time;
    std::tm t = {year, month, day, hour, minute, second};

    expiredTime = std::chrono::system_clock::from_time_t(std::mktime(&t));
    userInfo.id = evento::account.userId;
}

void AccountManager::saveConfig() {
    // TODO: save
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

    if (err.code != 0) {
        spdlog::error("Failed to save refresh token: {}\n", err.message);
    }
}

std::optional<std::string> AccountManager::getKeychainRefreshToken() const {
    keychain::Error err;
    auto refreshToken = keychain::getPassword(package, service, userInfo.id, err);

    if (err.code != 0) {
        spdlog::error("Failed to save refresh token: {}\n", err.message);
    }

    if (refreshToken.empty()) {
        std::nullopt;
    }

    return refreshToken;
}

void AccountManager::scheduleRenewAccessToken() {
    auto& self = *this;
    renewAccessTokenTimer.expires_after(55min); // 设置定时器为30分钟
    renewAccessTokenTimer.async_wait(
        [weak_this = weak_from_this(), &self](const boost::system::error_code& ec) {
            if (ec) {
                spdlog::error("Failed to renew access token: {}\n", ec.message());
                return;
            }
            if (auto alive = weak_this.lock()) {
                self.performRefreshToken();
            }
        });
}

void AccountManager::scheduleRenewRefreshToken() {
    /*
    auto& self = *this;
    _renewRefreshTokenTimer.expires_after(std::chrono::hours(24)); // 设置定时器为24小时
    _renewRefreshTokenTimer.async_wait(
        [weak_this = weak_from_this(), &self](const boost::system::error_code& ec) {
            if (ec) {
                spdlog::error("Failed to renew refresh token: {}\n", ec.message());
                return;
            }
            if (auto alive = weak_this.lock()) {
                self.performRefreshToken();
            }
        });
    */
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
    onStateChanged();
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