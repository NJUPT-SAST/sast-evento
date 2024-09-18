#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class SettingPage : public BasicView, private GlobalAgent<SettingPageBridge> {
public:
    SettingPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);

private:
    void onCreate() override;

    void onShow() override;
};

EVENTO_UI_END