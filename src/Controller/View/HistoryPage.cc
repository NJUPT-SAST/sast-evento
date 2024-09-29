#include <Controller/AsyncExecutor.hh>
#include <Controller/Convert.h>
#include <Controller/UiBridge.h>
#include <Controller/View/HistoryPage.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <slint.h>
#include <spdlog/spdlog.h>

EVENTO_UI_START

HistoryPage::HistoryPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void HistoryPage::onCreate() {
    auto& self = *this;

    self->on_load_events(
        [this](int pageIndex, int size) { loadHistoryEvents(pageIndex + 1, size); });
    self->on_comment([this](int eventId, int rating, slint::SharedString content) {
        feedbackEvent(eventId, rating, content.data());
    });
    self->on_navigate_to_detail([this](EventStruct eventStruct) {
        spdlog::debug("navigate to DetailPage, current event is {}", eventStruct.summary.data());
        bridge.getViewManager().navigateTo(ViewName::DetailPage, eventStruct);
    });
}

void HistoryPage::onShow() {
    auto& self = *this;

    loadHistoryEvents(self->get_current_page_index() + 1, self->get_page_size());
}

void HistoryPage::loadHistoryEvents(int page, int size) {
    auto& self = *this;
    self->set_state(PageState::Loading);

    executor()->asyncExecute(loadHistoryEventsTask(page, size),
                             [&self = *this, this](Result<std::vector<EventFeedbackStruct>> result) {
                                 if (result.isErr()) {
                                     return;
                                 }

                                 auto list = result.unwrap();
                                 self->set_total(static_cast<int>(list.size()));
                                 self->set_models(
                                     std::make_shared<slint::VectorModel<EventFeedbackStruct>>(
                                         list));
                                 self->set_state(PageState::Normal);
                             });
}

Task<Result<std::vector<EventFeedbackStruct>>> HistoryPage::loadHistoryEventsTask(int page,
                                                                                  int size) {
    auto& self = *this;
    auto historyEventsRes = co_await networkClient()->getHistoryEventList(page, size);
    if (historyEventsRes.isErr()) {
        self->set_state(PageState::Error);
        co_return historyEventsRes.unwrapErr();
    }

    auto historyEvents = historyEventsRes.unwrap();
    std::vector<EventFeedbackStruct> res;
    for (auto const& event : historyEvents.elements) {
        auto feedbackRes = co_await networkClient()->getUserFeedback(event.id);
        if (feedbackRes.isErr()) {
            spdlog::warn("feedback load failed: {}", feedbackRes.unwrapErr().what());
            self.bridge.getMessageManager().showMessage(feedbackRes.unwrapErr().what(),
                                                        MessageType::Error);
            res.emplace_back(convert::from(event), FeedbackStruct{});
        } else {
            res.emplace_back(convert::from(event), convert::from(feedbackRes.unwrap()));
        }
    }
    co_return res;
}

void HistoryPage::feedbackEvent(int eventId, int rating, std::string content) {
    auto& self = *this;
    auto trans = [](const auto& e) { return e.id; };
    executor()
        ->asyncExecute(networkClient()->addUserFeedback(eventId, rating, content),
                       [&self = *this, this](Result<bool> result) {
                           if (result.isErr()) {
                               self.bridge.getMessageManager().showMessage(result.unwrapErr().what(),
                                                                           MessageType::Error);
                               return;
                           }
                           auto success = result.unwrap();
                           if (success) {
                               self.bridge.getMessageManager().showMessage("反馈成功",
                                                                           MessageType::Success);
                           } else {
                               self.bridge.getMessageManager().showMessage("反馈失败",
                                                                           MessageType::Error);
                           }
                       });
}

EVENTO_UI_END