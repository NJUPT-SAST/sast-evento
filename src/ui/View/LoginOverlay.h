#pragma once

#include <ui/Core/BasicView.h>
#include <ui/Core/GlobalAgent.hpp>
#include <ui/Core/UiBase.h>

EVENTO_UI_START

class LoginOverlay : public BasicView, private GlobalAgent<LoginOverlayBridge> {
public:
    LoginOverlay(slint::ComponentHandle<UiEntryName> uiEntry);
    LoginOverlay(LoginOverlay&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END