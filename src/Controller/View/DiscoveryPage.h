#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hpp>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class DiscoveryPage : public BasicView, private GlobalAgent<DiscoveryPageBridge> {
public:
    DiscoveryPage(slint::ComponentHandle<UiEntryName> uiEntry);
    DiscoveryPage(DiscoveryPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END