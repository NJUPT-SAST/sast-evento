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
    loadHomeSlides();

    static bool firstShow = true;
    if (firstShow) {
        firstShow = false;
        slidesAutoRotation();
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
                                 self->set_active_events_state(PageState::Normal);
                                 self->set_active_events(convert::from(eventQueryRes.elements));
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
                                 self->set_latest_events_state(PageState::Normal);
                                 self->set_latest_events(convert::from(eventQueryRes.elements));
                             });
}

void DiscoveryPage::loadHomeSlides() {
    auto& self = *this;
    executor()->asyncExecute(
        networkClient()->getHomeSlide(10min), [&self = *this, this](Result<SlideEntityList> result) {
            if (result.isErr()) {
                return;
            }
            auto list = result.unwrap();
            auto total = std::min(static_cast<std::size_t>(3), list.size());
            for (int i = 0; i < total; ++i) {
                executor()->asyncExecute(networkClient()->getFile(list[i].url),
                                         [&self = *this, i](Result<std::filesystem::path> result) {
                                             if (result.isErr()) {
                                                 spdlog::warn("image load failed: {}",
                                                              result.unwrapErr().what());
                                                 return;
                                             }
                                             self->get_carousel_source()->set_row_data(
                                                 i,
                                                 slint::Image::load_from_path(
                                                     result.unwrap().string().c_str()));
                                         });
            }
        });
}

void DiscoveryPage::slidesAutoRotation() {
    timer.start(slint::TimerMode::Repeated, 5s, [&self = *this] {
        self->set_image_index((self->get_image_index() + 1)
                              % static_cast<int>(self->get_carousel_source()->row_count()));
    });
}

EVENTO_UI_END
