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

    bool loginState = false;
    UserInfoEntity userInfo;

    std::chrono::system_clock::time_point expiredTime;
    net::steady_timer renewAccessTokenTimer;

    static const inline std::string package = "org.sast.evento";
    static const inline std::string service = "refresh-token";

public:
    AccountManager(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    AccountManager(AccountManager&) = delete;
    ~AccountManager();

    bool isLogin() const;
    void requestLogin();
    void requestLogout();

    UserInfoEntity getUserInfo();

private:
    void loadConfig();
    void saveConfig();

    void setKeychainRefreshToken(const std::string& refreshToken) const;
    std::optional<std::string> getKeychainRefreshToken() const;

    void scheduleRenewAccessToken();

    static void setNetworkAccessToken(std::string accessToken);

    void setLoginState(bool newState);
    void onStateChanged();

    void performLogin();
    void performRefreshToken();
    void performGetUserInfo();
};

EVENTO_UI_END