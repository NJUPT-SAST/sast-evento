#pragma once

#include "slint_string.h"
#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
#include <Infrastructure/Network/ResponseStruct.h>

EVENTO_UI_START

class MyEventPage : public BasicView, private GlobalAgent<MyEventPageBridge> {
public:
    MyEventPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    //get info
    evento::EventQueryRes get_events(slint::SharedString part);
private:
    void onCreate() override;
    void onShow() override;

    bool check_code(slint::SharedString code, int id);
    
};

EVENTO_UI_END