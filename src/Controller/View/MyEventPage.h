#pragma once

#include "Infrastructure/Network/NetworkClient.h"
#include "slint_string.h"
#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <vector>

EVENTO_UI_START

class MyEventPage : public BasicView, private GlobalAgent<MyEventPageBridge> {
public:
    MyEventPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    //get info
    static evento::EventEntityList get_events(slint::SharedString part);
    static evento::EventQueryRes get_events();
private:
    void onCreate() override;
    void onShow() override;

    static bool check_code(slint::SharedString code, int id);
    //process info
    static std::vector<slint::SharedString> process_events(std::string& part);
    static std::vector<slint::SharedString> process_events(const evento::EventEntityList& events, std::string& part);
};

EVENTO_UI_END