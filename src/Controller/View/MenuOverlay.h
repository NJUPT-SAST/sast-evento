#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class MenuOverlay : public BasicView, private GlobalAgent<MenuOverlayBridge> {
public:
    MenuOverlay(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    MenuOverlay(MenuOverlay&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END