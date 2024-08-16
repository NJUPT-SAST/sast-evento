#include <Controller/AsyncExecutor.hh>
#include <Controller/Core/AccountManager.h>
#include <Controller/UiBridge.h>
#include <Controller/View/MenuOverlay.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <chrono>
#include <string_view>
#include <thread>

EVENTO_UI_START

MenuOverlay::MenuOverlay(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void MenuOverlay::onCreate() {
    auto& self = *this;
    self->on_open_link_web([this] { return openLinkWeb(); });
}

void MenuOverlay::onShow() {
    auto& self = *this;
    executor()->asyncExecute([]() -> Task<void> { co_return; },
                             [&]() {
                                 slint::invoke_from_event_loop([&] { self->set_is_show(true); });
                             },
                             std::chrono::milliseconds(200),
                             TimerFlag::Once);
}

void MenuOverlay::onLogin() {
    auto& self = *this;
    auto userInfo = bridge.getAccountManager().getUserInfo();
    self->set_user_name(std::string_view(userInfo.nickname));
    // ?? no signature
}

void MenuOverlay::openLinkWeb() {
    // TODO: call to system and open web here
    return;

    using namespace std::string_literals;
    std::string url = "";
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__)
    system(("start " + url).c_str());
#elif defined(__linux__)
    system(("xdg-open "s + url).c_str());
#elif defined(__APPLE__)
    system(("open "s + url).c_str());
#else
    spdlog::error("unable to open sast link website: unsupported os");
#endif
}

EVENTO_UI_END