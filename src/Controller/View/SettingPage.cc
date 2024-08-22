#include <Controller/View/SettingPage.h>


EVENTO_UI_START

SettingPage::SettingPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void SettingPage::onCreate() {
    auto& self = *this;
}
int SettingPage::on_timestate(){
    int time=0;
    return time;
}
int SettingPage::on_languagestate(){
    int lang=0;
    return lang;
}
EVENTO_UI_END