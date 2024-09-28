#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class MenuOverlay : public BasicView, private GlobalAgent<MenuOverlayBridge> {
public:
    MenuOverlay(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);

private:
    void onCreate() override;
    void onShow() override;
    void onLogin() override;
    void refreshUserInfo(UserInfoEntity const& userInfo);
};

EVENTO_UI_END