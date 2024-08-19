#include <Controller/View/SettingPage.h>
#include <slint.h>
#include "appwindow.h"

EVENTO_UI_START

SettingPage::SettingPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void SettingPage::onCreate() {
    auto& self = *this;
}
// int SettingPage::getstage(){
//     int numstate;
//     self.
//     return numstate;
// }
// int SettingPage::language(){
    
// }

EVENTO_UI_END