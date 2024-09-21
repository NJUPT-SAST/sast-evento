#include <Controller/AsyncExecutor.hh>
#include <Controller/Convert.h>
#include <Controller/UiBridge.h>
#include <Controller/View/HistoryPage.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <ranges>
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

    executor()->asyncExecute(
        networkClient()->getHistoryEventList(page, size),
        [&self = *this, this](Result<EventQueryRes> result) {
            if (result.isErr()) {
                self->set_state(PageState::Error);
                self.bridge.getMessageManager().showMessage(result.unwrapErr().what(),
                                                            MessageType::Error);
                return;
            }

            auto res = result.unwrap();
            self->set_total(res.total);
            self->set_events(convert::from(res.elements));

            feedbacks.clear();
            auto feedbackSize = res.elements.size();

            auto trans = [](const auto& e) { return e.id; };
            for (const auto& eventId : res.elements | std::views::transform(trans)) {
                executor()->asyncExecute(
                    networkClient()->getUserFeedback(eventId, 0min),
                    [&self = *this, feedbackSize](Result<std::optional<FeedbackEntity>> result) {
                        if (result.isErr()) {
                            spdlog::warn("feedback load failed: {}", result.unwrapErr().what());
                            self.feedbacks.emplace_back(false, false, 0, "");
                        } else {
                            self.feedbacks.emplace_back(convert::from(result.unwrap()));
                        }

                        if (self.feedbacks.size() == feedbackSize) {
                            self->set_feedbacks(std::make_shared<slint::VectorModel<FeedbackStruct>>(
                                self.feedbacks));
                            self->set_state(PageState::Normal);
                        }
                    });
            }
        });
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