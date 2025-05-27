#pragma once

#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <chrono>

EVENTO_UI_START

class AccountManager : private GlobalAgent<AccountManagerBridge>,
                       std::enable_shared_from_this<AccountManager> {
    friend class UiBridge;
    UiBridge& bridge;
    std::string logOrigin = "AccountManager";

    bool _loginState = false;
    UserInfoEntity _userInfo;

    std::chrono::system_clock::time_point _expiredTime;
    net::steady_timer _renewAccessTokenTimer;

    static const inline std::string package = "org.sast.evento";
    static const inline std::string service =
#ifdef EVENTO_API_V1
        "access-token";
#else
        "refresh-token";
#endif

public:
    AccountManager(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    AccountManager(AccountManager&) = delete;
    ~AccountManager();

    bool isLogin() const;
    void requestLogin();
    void requestLogout();
    void tryLoginDirectly();

    UserInfoEntity& userInfo();

private:
    void loadConfig();
    void saveConfig();

    void setKeychainRefreshToken(const std::string& refreshToken) const;
    std::optional<std::string> getKeychainRefreshToken() const;

    void scheduleRenewAccessToken();

    static void setNetworkAccessToken(std::optional<std::string> accessToken);

#ifdef EVENTO_API_V1
    [[nodiscard]] std::optional<std::string> getKeychainAccessToken() const;
    void setKeychainAccessToken(const std::string& accessToken) const;
#endif

    void setLoginState(bool newState);
    void onStateChanged();

    void performLogin();
    void performRefreshToken();
    void performGetUserInfo();
};

EVENTO_UI_END