#include <Controller/View/AboutPage.h>
#include <Version.h>

EVENTO_UI_START

AboutPage::AboutPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

callback checkUpdate()->bool;

void AboutPage::onCreate() {
    auto& self = *this;

    self->set_version("v" VERSION_FULL);

    self->on_checkUpdate([]() -> bool {
        // TODO
    });
}
    EVENTO_UI_END