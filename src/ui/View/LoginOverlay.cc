#include <Infrastructure/Network/network.h>
#include <Version.h>
#include <ui/View/LoginOverlay.h>

EVENTO_UI_START

LoginOverlay::LoginOverlay(slint::ComponentHandle<UiEntryName> uiEntry)
    : GlobalAgent(uiEntry) {}

void LoginOverlay::onCreate() {
    auto& self = *this;

    self->on_link_login(start_sast_link);
    self->set_version("v" VERSION_FULL);
}

EVENTO_UI_END