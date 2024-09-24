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
    self->on_for_refresh_mess([this](slint::SharedString part) { get_events(part); });
    //if click button , check code
    self->on_check_code([this](slint::SharedString code, int id) { return check_code(code, id); });
};

void MyEventPage::get_events(slint::SharedString part) {
    auto& self = *this;
    evento::EventQueryRes eventlists;
    int num = 0;
    if (part == slint::SharedString("active")) {
        evento::executor()->asyncExecute(networkClient()->getActiveEventList(),
                                         [&](Result<evento::EventQueryRes> list) {
                                             if (list.isOk()) {
                                                 eventlists = list.unwrap();
                                             } else {
                                                 return;
                                             }
                                         });
    } else if (part == slint::SharedString("subscriptions")) {
        evento::executor()->asyncExecute(networkClient()->getSubscribedEvent(),
                                         [&](Result<EventQueryRes> list) {
                                             if (list.isOk()) {
                                                 eventlists = list.unwrap();
                                             } else {
                                                 return;
                                             }
                                         });
    } else if (part == slint::SharedString("end")) {
        evento::executor()->asyncExecute(networkClient()->getParticipatedEvent(),
                                         [&](Result<EventQueryRes> list) {
                                             if (list.isOk()) {
                                                 eventlists = list.unwrap();
                                             } else {
                                                 return;
                                             }
                                         });
    }
    num = (int) eventlists.elements.size();
    if (part == "subscriptions") {
        self->set_num_sub(num);
        self->set_event_sub(convert::from(eventlists.elements));
    } else if (part == "end") {
        self->set_num_end(num);
        self->set_event_end(convert::from(eventlists.elements));
    } else if (part == "active") {
        self->set_num_active(num);
        self->set_event_active(convert::from(eventlists.elements));
    }
}

bool MyEventPage::check_code(slint::SharedString code, int id) {
    auto& self = *this;
    bool is_check_in = false;
    std::string the_code = std::string(code);
    evento::executor()->asyncExecute(evento::networkClient()->checkInEvent(id, the_code),
                                     [&](Result<bool> check) {
                                         if (check.isOk()) {
                                             is_check_in = check.unwrap();
                                         } else if (check.isErr()) {
                                             return;
                                         }
                                     });
    return is_check_in;
}

EVENTO_UI_END