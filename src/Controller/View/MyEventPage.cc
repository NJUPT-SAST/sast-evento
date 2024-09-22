#include "Controller/AsyncExecutor.hh"
<<<<<<< HEAD
#include "Controller/Convert.h"
=======
#include "Controller/Convert.hh"
>>>>>>> ce4a2495d1ca609be509b7dccab8eb2ffae07bd5
#include "Infrastructure/Network/NetworkClient.h"
#include "Infrastructure/Utils/Result.h"
#include "app.h"
#include "slint.h"
#include "slint_string.h"
#include <Controller/View/MyEventPage.h>
#include <Infrastructure/Network/ResponseStruct.h>
<<<<<<< HEAD
#include <memory>

=======
#include <string>
#include <vector>
>>>>>>> ce4a2495d1ca609be509b7dccab8eb2ffae07bd5
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
    self->on_get_code([this](slint::SharedString code,int id){return check_code(code, id);});
};

void MyEventPage::get_events(slint::SharedString part){
    auto& self = *this;
    evento::EventQueryRes eventlists;
    int num = 0;
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
    num = (int)eventlists.elements.size();
    if (part == "subscriptions") {
    self->set_num_sub(num);
    self->set_event_sub(convert::from(eventlists.elements));
    }else if (part == "end") {
    self->set_num_end(num);
    self->set_event_end(convert::from(eventlists.elements));
    }else if (part == "active") {
    self->set_num_active(num);
    self->set_event_active(convert::from(eventlists.elements));
    }
}

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
void MyEventPage::onShow() {
    auto& self = *this;
    //获取订阅活动的信息
    evento::executor()->asyncExecute(evento::networkClient()->getSubscribedEvent(),
                                     [&self](Result<EventEntityList> event) {
                                         if (event.isErr()) {
                                             return;
                                         } else {
                                             self->set_info_sub(
                                                 evento::convert::from(event.unwrap()));
                                         }
                                     });
    auto m_list_sub = self->get_info_sub();
    //获取进行中活动的信息
    evento::executor()->asyncExecute(evento::networkClient()->getActiveEventList(),
                                     [&self](Result<EventQueryRes> event) {
                                         if (event.isErr()) {
                                             return;
                                         } else {
                                             self->set_info_start(
                                                 evento::convert::from(event.unwrap().elements));
                                         }
                                     });
    auto m_list_active = self->get_info_start();
    //获取结束活动的信息
    evento::executor()->asyncExecute(evento::networkClient()->getParticipatedEvent(),
                                     [&self](Result<EventEntityList> event) {
                                         if (event.isErr()) {
                                             return;
                                         } else {
                                             self->set_info_end(
                                                 evento::convert::from(event.unwrap()));
                                         }
                                     });
    auto m_list_end = self->get_info_end();
    //返回数组的回调，获取当前订阅的活动个数
    self->set_sub_event_num(int(m_list_sub->row_count()));
    auto sub_num = self->get_sub_event_num();
    self->on_return_array_sub([sub_num] {
        if (sub_num <= 0) {
            return 0;
        } else {
            auto back = new int[sub_num];
            auto back_true = *back;
            delete[] back;
            return back_true;
        }
    });
    //返回数组的回调，获取当前进行的活动个数
    self->set_start_event_num(int(m_list_active->row_count()));
    auto active_num = self->get_start_event_num();
    self->on_return_array_start([active_num] {
        if (active_num <= 0) {
            return 0;
        } else {
            auto back = new int[active_num];
            auto back_true = *back;
            delete[] back;
            return back_true;
        }
    });
    int end_num = (int) self->get_info_end()->row_count();
    //获取id，根据id得到信息
    //sub的id
    std::vector<int> now_id_subscribe;
    for (auto n = 1; n <= sub_num; ++n) {
        now_id_subscribe.push_back(m_list_sub->row_data(n)->id);
    }
    //active(start)的id
    std::vector<int> now_id_active;
    for (auto n = 1; n <= active_num; ++n) {
        now_id_active.push_back(m_list_active->row_data(n)->id);
    }
    //end的id
    std::vector<int> now_id_end;
    for (auto n = 1; n <= end_num; ++n) {
        now_id_end.push_back(m_list_end->row_data(n)->id);
    }
    //得到id的索引
    int sub, active = -1;
    self->on_get_num([&sub, &active](slint::SharedString part) {
        if (part == "subscribe") {
            return sub + 1;
        } else if (part == "active") {
            return active + 1;
        }
    });
    //获取id
    self->on_get_event_id(
        [now_id_subscribe, now_id_active, now_id_end](int count, slint::SharedString state) {
            std::string State = std::string(state);
            if (State == "subscribe") {
                return now_id_subscribe[count];
            } else if (State == "active") {
                return now_id_active[count];
            } else if (State == "end") {
                return now_id_end[count];
            }
        });

    //展示信息
    self->on_show_mess([&self](int id, slint::SharedString state, slint::SharedString part) {
        std::string the_state = std::string(state);
        auto m_list = self->get_info_sub();
        if (the_state == "subscribe") {
            m_list = self->get_info_sub();
        } else if (the_state == "active") {
            m_list = self->get_info_start();
        } else if (the_state == "end") {
            m_list = self->get_info_end();
        }
        int cow = 0;
        int e_num = (int) m_list->row_count();
        while (cow < e_num) {
            if (id == m_list->row_data(cow)->id) {
                break;
            }
            ++cow;
        }
        std::string Part = std::string(part);
        auto m_list_get = m_list->row_data(cow)->tag;
        if (Part == "summary") {
            m_list_get = m_list->row_data(cow)->summary;
        } else if (Part == "time") {
            m_list_get = m_list->row_data(cow)->time;
        } else if (Part == "description") {
            m_list_get = m_list->row_data(cow)->description;
        } else if (Part == "tag") {
            m_list_get = m_list->row_data(cow)->tag;
        } else if (Part == "department") {
            m_list_get = m_list->row_data(cow)->larkDepartmentName;
        } else if (Part == "room") {
            m_list_get = m_list->row_data(cow)->larkMeetingRoomName;
        } else if (Part == "location") {
            m_list_get = m_list->row_data(cow)->location;
        }
        return m_list_get;
    });

    //根据code判断签到是否成功
    self->on_get_code([&self](slint::SharedString Code) { self->set_event_code(Code); });
    auto event_code = std::string(self->get_event_code());
    int the_id = 1;
    self->on_back_event_id([&the_id](int id) {
        the_id = id;
        return the_id;
    });
    evento::executor()->asyncExecute(evento::networkClient()->checkInEvent(the_id, event_code),
                                     [&self](Result<bool> or_check) {
                                         if (or_check.isErr()) {
                                             return;
                                         } else {
                                             if (or_check.unwrap()) {
                                                 self->on_is_check_in([] { return true; });
                                             } else {
                                                 self->on_is_check_in([] { return false; });
                                             }
                                         }
                                     });
}
EVENTO_UI_END