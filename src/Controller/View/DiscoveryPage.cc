#include <Controller/View/DiscoveryPage.h>

EVENTO_UI_START

DiscoveryPage::DiscoveryPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void DiscoveryPage::onCreate() {
    auto& self = *this;
}

EVENTO_UI_END