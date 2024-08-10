#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hpp>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class LoginOverlay : public BasicView, private GlobalAgent<LoginOverlayBridge> {
public:
    LoginOverlay(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    LoginOverlay(LoginOverlay&) = delete;

private:
    void onCreate() override;

    static void login();
};

EVENTO_UI_END