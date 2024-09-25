#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
#include <Infrastructure/Network/ResponseStruct.h>

EVENTO_UI_START

class MyEventPage : public BasicView, private GlobalAgent<MyEventPageBridge> {
public:
    MyEventPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);

private:
    void onCreate() override;
    void onLogin() override;
    void onShow() override;
    void loadSubscribedEvents();
};

EVENTO_UI_END