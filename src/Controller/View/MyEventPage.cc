#include "Controller/AsyncExecutor.hh"
#include "Controller/Convert.h"
#include "Infrastructure/Network/NetworkClient.h"
#include "Infrastructure/Utils/Result.h"
#include "app.h"
#include "slint.h"
#include "slint_string.h"
#include <Controller/View/MyEventPage.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <memory>

EVENTO_UI_START

MyEventPage::MyEventPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void MyEventPage::onCreate() {
    auto& self = *this;
    
}
void MyEventPage::onShow() {
    auto& self = *this;
    // show event when the page is shown
    self->on_show_mess([this](slint::SharedString part){get_events(part);});
    //if click button , check code
    self->on_get_code([this](slint::SharedString code,int id){check_code(code, id);});
};

void MyEventPage::get_events(slint::SharedString part){
    auto& self = *this;
    evento::EventQueryRes eventlists;
    if (part == slint::SharedString("active")) {
        evento::executor()->asyncExecute(networkClient()->getActiveEventList(), [&](Result<evento::EventQueryRes> list){
        if (list.isOk()){
            eventlists = list.unwrap();
        }else {
            return ;
        }
    });
    }
    else if (part == slint::SharedString("subscriptions")) {
        evento::executor()->asyncExecute(networkClient()->getSubscribedEvent(), [&](Result<EventQueryRes> list){
        if (list.isOk()){
            eventlists = list.unwrap();
        }else {
            return ;
        }
    });
    }
    else if (part == slint::SharedString("end")) {
        evento::executor()->asyncExecute(networkClient()->getParticipatedEvent(), [&](Result<EventQueryRes> list){
        if (list.isOk()){
            eventlists = list.unwrap();
        }else {
            return ;
        }
    });
    }
    slint::VectorModel<int> id_model;
    for(auto & element : eventlists.elements) {
        id_model.push_back(element.id);
    }
    self->set_id(std::make_shared<slint::VectorModel<int>>(id_model));
    slint::VectorModel<EventStruct> event_model;
    for (auto & element : eventlists.elements) {
        event_model.push_back(convert::from(element));
    }
    self->set_event(std::make_shared<slint::VectorModel<EventStruct>>(event_model));
}

// ToDo implement this function about eventIde
bool MyEventPage::check_code(slint::SharedString code, int id) {
    auto& self = *this;
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