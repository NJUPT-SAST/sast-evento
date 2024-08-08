#pragma once

#include <ui/Core/BasicView.h>
#include <ui/Core/GlobalAgent.hpp>
#include <ui/Core/UiBase.h>

EVENTO_UI_START

class SettingPage : public BasicView, private GlobalAgent<SettingPageBridge> {
public:
    SettingPage(slint::ComponentHandle<UiEntryName> uiEntry);
    SettingPage(SettingPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END