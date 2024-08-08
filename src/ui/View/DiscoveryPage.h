#pragma once

#include <ui/Core/BasicView.h>
#include <ui/Core/GlobalAgent.hpp>
#include <ui/Core/UiBase.h>

EVENTO_UI_START

class DiscoveryPage : public BasicView, private GlobalAgent<DiscoveryPageBridge> {
public:
    DiscoveryPage(slint::ComponentHandle<UiEntryName> uiEntry);
    DiscoveryPage(DiscoveryPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END