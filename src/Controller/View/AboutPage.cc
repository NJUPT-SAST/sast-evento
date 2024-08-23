#include <Controller/View/AboutPage.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Utils/Result.h>
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
    // self->on_check_update([this] {
    //     auto result = evento::networkClient()->getLatestRelease();
    //     return result.unwarp().ta
    // });
    // self->on_get_log([this] {
    //     auto result = evento::networkClient()->getLatestRelease();
    //     return result.unwrap().body;
    // });
}

EVENTO_UI_END