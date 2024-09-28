#pragma once

#include <Controller/UiBridge.h>

EVENTO_UI_START

class LoginOverlay : public BasicView, private GlobalAgent<LoginOverlayBridge> {
public:
    LoginOverlay(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);

private:
    void onCreate() override;
    void onShow() override;
    void onLogin() override;
};

EVENTO_UI_END