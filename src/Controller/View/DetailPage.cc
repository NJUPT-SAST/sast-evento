#include <Controller/View/DetailPage.h>

EVENTO_UI_START

DetailPage::DetailPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void DetailPage::onCreate() {
    auto& self = *this;
}

EVENTO_UI_END