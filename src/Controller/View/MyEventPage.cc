#include "Controller/AsyncExecutor.hh"
#include "Infrastructure/Network/NetworkClient.h"
#include "Infrastructure/Utils/Result.h"
#include "slint.h"
#include "slint_string.h"
#include <Controller/View/MyEventPage.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <vector>

EVENTO_UI_START

enum InfoPart{
    sum = 1,
    description = 2,
    start = 3,
    end = 4,
    location = 5,
    tag = 6,
    larkMeetingRoomName = 7,
    larkDepartmentName = 8,
    state = 9,
    isSubscribed = 10,
    isCheckedIn = 11
};

evento::EventEntityList lists_sub = MyEventPage::get_events(slint::SharedString("subscriptions"));
evento::EventEntityList lists_end = MyEventPage::get_events(slint::SharedString("end"));
evento::EventQueryRes res_active = MyEventPage::get_events();

MyEventPage::MyEventPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void MyEventPage::onCreate() {
    auto& self = *this;
    
}
void MyEventPage::onShow() {
    auto& self = *this;
    // show event when the page is shown
    
    //if click button , check code
    
};

evento::EventEntityList MyEventPage::get_events(slint::SharedString part) {
    evento::EventEntityList eventList;
    if (part == slint::SharedString("subscriptions")) {
        evento::executor()->asyncExecute(networkClient()->getSubscribedEvent(), [&](Result<EventEntityList> list){
        if (list.isOk()){
            eventList = list.unwrap();
        }else {
            return ;
        }
    });
    }
    else if (part == slint::SharedString("end")) {
        evento::executor()->asyncExecute(networkClient()->getParticipatedEvent(), [&](Result<EventEntityList> list){
        if (list.isOk()){
            eventList = list.unwrap();
        }else {
            return ;
        }
    });
    }
    return eventList;
}

evento::EventQueryRes MyEventPage::get_events(){
    evento::EventQueryRes eventlists;
    evento::executor()->asyncExecute(networkClient()->getActiveEventList(), [&](Result<evento::EventQueryRes> list){
        if (list.isOk()){
            eventlists = list.unwrap();
        }else {
            return ;
        }
    });
    return eventlists;
}

std::vector<slint::SharedString> MyEventPage::process_events(std::string& part) {
    std::vector<slint::SharedString> result;
    auto size1 = lists_sub.size();
    auto size2 = lists_end.size();
    auto size3 = res_active.elements.size();
    if (part == "subscriptions") {

    }else if (part == "end") {
    
    }else if (part == "active") {
    
    }
    return result;
}

// ToDo implement this function about eventIde
bool MyEventPage::check_code(slint::SharedString code, int id) {
    bool is_check_in = false;
    std::string the_code = std::string(code);
    evento::executor()->asyncExecute(evento::networkClient()->checkInEvent(id, the_code), [&](Result<bool> check){
        if (check.isOk()) {
            is_check_in = check.unwrap();
        } else if (check.isErr()) {
            return ;
        }
    });
    return is_check_in;
}

EVENTO_UI_END