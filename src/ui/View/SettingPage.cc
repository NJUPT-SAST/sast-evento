#include <ui/View/SettingPage.h>

EVENTO_UI_START

SettingPage::SettingPage(slint::ComponentHandle<UiEntryName> uiEntry)
    : GlobalAgent(uiEntry) {}

void SettingPage::onCreate() {
    auto& self = *this;
}

EVENTO_UI_END