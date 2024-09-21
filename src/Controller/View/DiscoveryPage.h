#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class DiscoveryPage : public BasicView, private GlobalAgent<DiscoveryPageBridge> {
public:
    DiscoveryPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);

private:
    void onCreate() override;
    void onShow() override;

    void loadActiveEvents();
    void loadLatestEvents();
    void loadHomeSlides();
    void slidesAutoRotation();
};

EVENTO_UI_END