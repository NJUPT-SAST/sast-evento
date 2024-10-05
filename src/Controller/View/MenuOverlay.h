#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
#include <Infrastructure/Utils/Result.h>

EVENTO_UI_START

class MenuOverlay : public BasicView, private GlobalAgent<MenuOverlayBridge> {
public:
    MenuOverlay(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);

private:
    void onCreate() override;
    void onShow() override;
    void onLogin() override;

    static Task<Result<slint::Image>> loadUserInfoTask();
    void refreshUserInfo(UserInfoEntity const& userInfo);
};

EVENTO_UI_END