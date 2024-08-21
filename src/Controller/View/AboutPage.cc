#include <Controller/View/AboutPage.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Utils/Tools.h>
#include <Version.h>
#include <chrono>
#include <slint.h>
#include <string>
#include <type_traits>

EVENTO_UI_START

AboutPage::AboutPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void AboutPage::onCreate() {
    auto& self = *this;

    self->set_version("v" VERSION_FULL);
    self->on_open_web([this](slint::SharedString url) { openBrowser(std::string(url)); });
}

// void AboutPage::onClick() {
//     auto& self = *this;

//     // self->on_open_URL([this] { openBrowser("https://baidu.com"); });
// }

EVENTO_UI_END