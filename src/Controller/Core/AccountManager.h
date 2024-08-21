#pragma once

#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <chrono>
#include <memory>

EVENTO_UI_START

class AccountManager : private GlobalAgent<AccountManagerBridge>,
                       std::enable_shared_from_this<AccountManager> {
    friend class UiBridge;
    UiBridge& bridge;
    std::string logOrigin = "AccountManager";

    bool loginState = false;
    UserInfoEntity userInfo;
    // refreshToken(expired in 7d) saved to keychain

    std::chrono::system_clock::time_point lastRefreshTokenRenewTime;

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

    static const inline std::string package = "org.sast.evento";
    static const inline std::string service = "refresh-token";
    void setKeychainRefreshToken(const std::string& refreshToken) const;
    std::string getKeychainRefreshToken() const;

    void scheduleRenewAccessToken();
    void scheduleRenewRefreshToken();

    static void setNetworkAccessToken(std::string accessToken);

    void setLoginState(bool newState);
    void onStateChanged();
};

EVENTO_UI_END