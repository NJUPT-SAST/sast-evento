#include <Controller/View/AboutPage.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Version.h>
#include <string>
#include <type_traits>
#include <iostream>
#include <cstdlib>

EVENTO_UI_START

AboutPage::AboutPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}


void AboutPage::onCreate() {
    auto& self = *this;

    self->set_version("v" VERSION_FULL);
}

void AboutPage::onCheck() {
    auto& self = *this;

    // self->on_open_web_site([site] {
    //     // Macos
    //     std::string cmd = "open " + site;
    //     system(cmd.c_str());
    // });

}

    EVENTO_UI_END