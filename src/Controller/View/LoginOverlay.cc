#include <Controller/Core/AccountManager.h>
#include <Controller/Core/ViewManager.h>
#include <Controller/View/LoginOverlay.h>
#include <Version.h>

EVENTO_UI_START

LoginOverlay::LoginOverlay(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void LoginOverlay::onCreate() {
    auto& self = *this;

    self->on_link_login([this] { bridge.getAccountManager().requestLogin(); });
    self->set_version("v" VERSION_FULL);
}

void LoginOverlay::onShow() {
    auto& self = *this;
    static bool firstShow = true;
    if (firstShow) {
        firstShow = false;
        self.bridge.getAccountManager().tryLoginDirectly();
    }
}

void LoginOverlay::onLogin() {
    auto& self = *this;
    if (bridge.getViewManager().isVisible(ViewName::LoginOverlay)) {
        bridge.getViewManager().priorView();
    }
}

EVENTO_UI_END