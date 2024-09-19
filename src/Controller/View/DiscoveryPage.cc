#include "Controller/AsyncExecutor.hh"
#include "Controller/Convert.hh"
#include "Controller/Core/ViewManager.h"
#include "Controller/UiBridge.h"
#include "Infrastructure/Network/NetworkClient.h"
#include "app.h"
#include <Controller/View/DiscoveryPage.h>
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
    self->on_navigate_to_detail([this](EventStruct eventStruct) {
        spdlog::debug("navigate to DetailPage, current event is {}", eventStruct.summary.data());
        bridge.getViewManager().navigateTo(ViewName::DetailPage, eventStruct);
    });
}

void DiscoveryPage::onShow() {
    loadActiveEvents();
    loadLatestEvents();
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

EVENTO_UI_END
