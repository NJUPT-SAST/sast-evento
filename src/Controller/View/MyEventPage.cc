#include <Controller/UiBridge.h>
#include <Controller/View/MyEventPage.h>
#include <Infrastructure/Network/ResponseStruct.h>

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
    loadSubscribedEvents();
}

void MyEventPage::onShow() {
    auto& self = *this;
    loadSubscribedEvents();
};

void MyEventPage::loadSubscribedEvents() {
    // TODO: implement me!
}

EVENTO_UI_END