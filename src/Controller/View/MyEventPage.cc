#include <Controller/Convert.h>
#include <Controller/UiBridge.h>
#include <Controller/View/MyEventPage.h>
#include <Infrastructure/IPC/SocketClient.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <Infrastructure/Utils/Config.h>
#include <Infrastructure/Utils/Tools.h>
#include <spdlog/spdlog.h>

EVENTO_UI_START

MyEventPage::MyEventPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void MyEventPage::onCreate() {
    auto& self = *this;
    self->on_load_events([this]() { loadSubscribedEvents(); });
    self->on_navigate_to_detail([this](EventStruct eventStruct) {
        spdlog::debug("navigate to DetailPage, current event is {}", eventStruct.summary.data());
        bridge.getViewManager().navigateTo(ViewName::DetailPage, eventStruct);
    });
}

void MyEventPage::onLogin() {
    executor()->asyncExecute(
        []() -> net::awaitable<Result<EventQueryRes>> {
            auto res = co_await networkClient()->getSubscribedEvent();
            co_return res;
        },
        [this](Result<EventQueryRes> result) { refreshUiModel(std::move(result)); },
        5min);
}

void MyEventPage::onShow() {
    auto& self = *this;
    loadSubscribedEvents();
};

void MyEventPage::loadSubscribedEvents() {
    auto& self = *this;
    self->set_state(PageState::Loading);

    executor()->asyncExecute(networkClient()->getSubscribedEvent(),
                             [&self = *this](Result<EventQueryRes> result) {
                                 if (result.isErr()) {
                                     self->set_state(PageState::Error);
                                     self->set_error_message(
                                         slint::SharedString(result.unwrapErr().what()));
                                     return;
                                 }
                                 self.refreshUiModel(std::move(result));
                             });
}

void MyEventPage::refreshUiModel(Result<EventQueryRes> result) {
    auto& self = *this;
    if (result.isErr()) {
        self->set_state(PageState::Error);
        self.bridge.getMessageManager().showMessage(result.unwrapErr().what(), MessageType::Error);
        return;
    }

    auto res = result.unwrap();

    std::vector<EventEntity> models[4];
    for (int i = 0; i < 3; i++) {
        std::copy_if(res.elements.begin(),
                     res.elements.end(),
                     std::back_inserter(models[i]),
                     [i](const EventEntity& element) { return (int) element.state == i; });
    }

    if (evento::settings.noticeBegin)
        for (auto const& entity : models[(int) EventState::SigningUp]) {
            auto time = parseIso8601Utc(entity.start.c_str());
            ipc()->showOrUpdateMessage(entity.id,
                                       std::format("活动 {} 还有 15 分钟就要开始了，记得参加哦",
                                                   entity.summary),
                                       std::chrono::system_clock::from_time_t(time) - 15min);
        }

    if (evento::settings.noticeEnd)
        for (auto const& entity : models[(int) EventState::Active]) {
            auto time = parseIso8601Utc(entity.end.c_str());
            ipc()->showOrUpdateMessage(entity.id,
                                       std::format("活动 {} 结束了，记得反馈哦", entity.summary),
                                       std::chrono::system_clock::from_time_t(time));
        }

    for (auto const& entity : models[(int) EventState::Cancelled]) {
        auto time = parseIso8601Utc(entity.start.c_str());
        ipc()->cancelMessage(entity.id);
    }

    self->set_not_started_model(convert::from(models[(int) EventState::SigningUp]));
    self->set_active_model(convert::from(models[(int) EventState::Active]));
    self->set_completed_model(convert::from(models[(int) EventState::Completed]));
}

EVENTO_UI_END