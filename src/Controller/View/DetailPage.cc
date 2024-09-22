#include "app.h"
#include <Controller/AsyncExecutor.hh>
#include <Controller/Convert.h>
#include <Controller/UiBridge.h>
#include <Controller/View/DetailPage.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <string>

EVENTO_UI_START

DetailPage::DetailPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void DetailPage::onCreate() {
    auto& self = *this;
    self->on_load_feedback([&self = *this]() { self.loadFeedback(); });
    self->on_load_event([&self = *this]() { self.loadEvent(); });
    self->on_feedback([&self = *this](int rate, slint::SharedString content) {
        self.feedbackEvent(self->get_event_model().id, rate, content.data());
    });
    self->on_check_in([&self = *this](slint::SharedString checkInCode) {
        self.checkIn(self->get_event_model().id, checkInCode.data());
    });
    self->on_subscribe(
        [&self = *this](bool subscribe) { self.subscribe(self->get_event_model().id, subscribe); });
}

void DetailPage::onShow() {
    auto& self = *this;
    self->set_event_model(self.bridge.getViewManager().getData<EventStruct>());
    loadEvent();
    if (self->get_event_model().state == EventState::Completed) {
        loadFeedback();
    }
}

void DetailPage::loadEvent() {
    auto& self = *this;
    executor()->asyncExecute(networkClient()->getEventById(self->get_event_model().id),
                             [&self = *this](Result<EventQueryRes> result) {
                                 if (result.isErr()) {
                                     self.bridge.getMessageManager()
                                         .showMessage(result.unwrapErr().what(), MessageType::Error);
                                     return;
                                 }

                                 auto elements = result.unwrap().elements;
                                 if (elements.empty()) {
                                     self.bridge.getMessageManager().showMessage("活动信息错误",
                                                                                 MessageType::Error);
                                     return;
                                 }

                                 self->set_event_model(convert::from(elements.front()));
                             });
}

void DetailPage::loadFeedback() {
    auto& self = *this;
    self->set_state(PageState::Loading);
    executor()->asyncExecute(networkClient()->getUserFeedback(self->get_event_model().id),
                             [&self = *this](Result<std::optional<FeedbackEntity>> result) {
                                 if (result.isErr()) {
                                     self->set_state(PageState::Error);
                                     self.bridge.getMessageManager()
                                         .showMessage(result.unwrapErr().what(), MessageType::Error);
                                     return;
                                 }

                                 self->set_feedback_model(convert::from(result.unwrap()));
                                 self->set_state(PageState::Normal);
                             });
}

void DetailPage::checkIn(int eventId, std::string checkInCode) {
    auto& self = *this;
    executor()->asyncExecute(
        networkClient()->checkInEvent(eventId, checkInCode), [&self = *this](Result<bool> result) {
            if (result.isErr()) {
                self.bridge.getMessageManager().showMessage(result.unwrapErr().what(),
                                                            MessageType::Error);
                return;
            }

            auto success = result.unwrap();
            if (success) {
                auto model = self->get_event_model();
                model.is_checkIn = true;
                self->set_event_model(model);
                self.bridge.getMessageManager().showMessage("签到成功", MessageType::Success);
            } else {
                self.bridge.getMessageManager().showMessage("签到失败，可能是签到码不对哦",
                                                            MessageType::Error);
            }
        });
}

void DetailPage::subscribe(int eventId, bool subscribe) {
    auto& self = *this;
    executor()->asyncExecute(networkClient()->subscribeEvent(eventId, subscribe),
                             [&self = *this, subscribe](Result<bool> result) {
                                 if (result.isErr()) {
                                     self.bridge.getMessageManager()
                                         .showMessage(result.unwrapErr().what(), MessageType::Error);
                                     return;
                                 }

                                 auto success = result.unwrap();
                                 auto message = std::format("{}{}",
                                                            subscribe ? "报名" : "取消报名",
                                                            success ? "成功" : "失败，请重试");

                                 if (success) {
                                     auto model = self->get_event_model();
                                     model.is_subscribed = subscribe;
                                     self->set_event_model(model);
                                 }

                                 self.bridge.getMessageManager().showMessage(message,
                                                                             MessageType::Error);
                             });
}

void DetailPage::feedbackEvent(int eventId, int rate, std::string content) {
    auto& self = *this;
    executor()
        ->asyncExecute(networkClient()->addUserFeedback(eventId, rate, content),
                       [&self = *this, rate, content](Result<bool> result) {
                           if (result.isErr()) {
                               self.bridge.getMessageManager().showMessage(result.unwrapErr().what(),
                                                                           MessageType::Error);
                               return;
                           }

                           auto success = result.unwrap();
                           if (success) {
                               auto newModel = FeedbackStruct{
                                   .success = true,
                                   .has_feedbacked = true,
                                   .rate = rate,
                                   .content = slint::SharedString(content),
                               };
                               self->set_feedback_model(newModel);
                               self.bridge.getMessageManager().showMessage("反馈成功",
                                                                           MessageType::Success);
                           } else {
                               self.bridge.getMessageManager().showMessage("反馈失败",
                                                                           MessageType::Error);
                           }
                       });
}

EVENTO_UI_END