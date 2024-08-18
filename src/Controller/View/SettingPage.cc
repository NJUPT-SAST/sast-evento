#include <Controller/View/SettingPage.h>

EVENTO_UI_START

SettingPage::SettingPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void SettingPage::onCreate() {
    auto& self = *this;
}

EVENTO_UI_END