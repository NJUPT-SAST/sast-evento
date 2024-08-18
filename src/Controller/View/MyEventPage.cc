#include <Controller/View/MyEventPage.h>

EVENTO_UI_START

MyEventPage::MyEventPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void MyEventPage::onCreate() {
    auto& self = *this;
}

EVENTO_UI_END