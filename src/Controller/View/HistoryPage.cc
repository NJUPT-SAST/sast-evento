#include <Controller/View/HistoryPage.h>

EVENTO_UI_START

HistoryPage::HistoryPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void HistoryPage::onCreate() {
    auto& self = *this;
}

EVENTO_UI_END