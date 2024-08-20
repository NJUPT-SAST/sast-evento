#include <Controller/View/SettingPage.h>


EVENTO_UI_START

SettingPage::SettingPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void SettingPage::onCreate() {
    auto& self = *this;
}
int SettingPage::on_timestate(){
    int current_itemt=0;
    //TODO:
    return current_itemt;
}
int SettingPage::on_languagestate(){
    int current_iteml=0;
    //TODO:
    return current_iteml;
}
EVENTO_UI_END