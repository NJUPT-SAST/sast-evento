#pragma once

#include <ui/Core/BasicView.h>
#include <ui/Core/GlobalAgent.hpp>
#include <ui/Core/UiBase.h>

EVENTO_UI_START

class MenuOverlay : public BasicView, private GlobalAgent<MenuOverlayBridge> {
public:
    MenuOverlay(slint::ComponentHandle<UiEntryName> uiEntry);
    MenuOverlay(MenuOverlay&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END