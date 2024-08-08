#include <Controller/View/SearchPage.h>

EVENTO_UI_START

SearchPage::SearchPage(slint::ComponentHandle<UiEntryName> uiEntry)
    : GlobalAgent(uiEntry) {}

void SearchPage::onCreate() {
    auto& self = *this;
}

EVENTO_UI_END