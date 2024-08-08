#include <Controller/View/MenuOverlay.h>

EVENTO_UI_START

MenuOverlay::MenuOverlay(slint::ComponentHandle<UiEntryName> uiEntry)
    : GlobalAgent(uiEntry) {}

void MenuOverlay::onCreate() {
    auto& self = *this;
}

EVENTO_UI_END