#include <ui/View/MyEventPage.h>

EVENTO_UI_START

MyEventPage::MyEventPage(slint::ComponentHandle<UiEntryName> uiEntry)
    : GlobalAgent(uiEntry) {}

void MyEventPage::onCreate() {
    auto& self = *this;
}

EVENTO_UI_END