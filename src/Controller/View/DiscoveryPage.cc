#include <Controller/AsyncExecutor.hh>
#include <Controller/Convert.h>
#include <Controller/Core/ViewManager.h>
#include <Controller/UiBridge.h>
#include <Controller/View/DiscoveryPage.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <spdlog/spdlog.h>

EVENTO_UI_START

DiscoveryPage::DiscoveryPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void DiscoveryPage::onCreate() {
    auto& self = *this;
    self->on_load_active_events([this] { loadActiveEvents(); });
    self->on_load_latest_events([this] { loadLatestEvents(); });
    self->on_image_manually_changed([this] { timer.restart(); });
    self->on_navigate_to_detail([this](EventStruct eventStruct) {
        spdlog::debug("navigate to DetailPage, current event is {}", eventStruct.summary.data());
        bridge.getViewManager().navigateTo(ViewName::DetailPage, eventStruct);
    });
}

void DiscoveryPage::onShow() {
    loadActiveEvents();
    loadLatestEvents();

    static bool firstShow = true;
    if (firstShow) {
        firstShow = false;
        loadHomeSlides();
    }
}

void DiscoveryPage::loadActiveEvents() {
    auto& self = *this;
    self->set_active_events_state(PageState::Loading);
    executor()->asyncExecute(networkClient()->getActiveEventList(),
                             [&self = *this, this](Result<EventQueryRes> result) {
                                 if (result.isErr()) {
                                     self->set_active_events_state(PageState::Error);
                                     self.bridge.getMessageManager()
                                         .showMessage(result.unwrapErr().what(), MessageType::Error);
                                     return;
                                 }
                                 auto eventQueryRes = result.unwrap();
                                 self->set_active_events(convert::from(eventQueryRes.elements));
                                 self->set_active_events_state(PageState::Normal);
                             });
}

void DiscoveryPage::loadLatestEvents() {
    auto& self = *this;
    self->set_latest_events_state(PageState::Loading);
    executor()->asyncExecute(networkClient()->getLatestEventList(),
                             [&self = *this, this](Result<EventQueryRes> result) {
                                 if (result.isErr()) {
                                     self->set_latest_events_state(PageState::Error);
                                     self.bridge.getMessageManager()
                                         .showMessage(result.unwrapErr().what(), MessageType::Error);
                                     return;
                                 }
                                 auto eventQueryRes = result.unwrap();
                                 self->set_latest_events(convert::from(eventQueryRes.elements));
                                 self->set_latest_events_state(PageState::Normal);
                             });
}

void DiscoveryPage::loadHomeSlides() {
    auto& self = *this;
    executor()->asyncExecute(loadHomeSlidesTask(), [this]() { slidesAutoRotation(); });
}

Task<void> DiscoveryPage::loadHomeSlidesTask() {
    auto result = co_await networkClient()->getHomeSlide();
    if (result.isErr()) {
        co_return;
    }
    auto list = result.unwrap();
    auto total = std::min(static_cast<std::size_t>(3), list.size());
    for (int i = 0; i < total; ++i) {
        auto fileResult = co_await networkClient()->getFile(list[i].url);
        if (fileResult.isErr()) {
            spdlog::warn("image load failed: {}", fileResult.unwrapErr().what());
            co_return;
        }
        auto imagePath = fileResult.unwrap();
        slint::blocking_invoke_from_event_loop([&, &self = *this]() {
            self->invoke_set_slide(i, slint::Image::load_from_path(imagePath.string().c_str()));
        });
    }
}

void DiscoveryPage::slidesAutoRotation() {
    timer.start(slint::TimerMode::Repeated, 5s, [&self = *this] {
        self->set_image_index((self->get_image_index() + 1)
                              % static_cast<int>(self->get_carousel_source()->row_count()));
    });
}

EVENTO_UI_END
