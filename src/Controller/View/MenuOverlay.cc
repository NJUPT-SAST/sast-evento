#include <Controller/AsyncExecutor.hh>
#include <Controller/Core/AccountManager.h>
#include <Controller/UiBridge.h>
#include <Controller/View/MenuOverlay.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <Infrastructure/Utils/Tools.h>
#include <chrono>
#include <slint.h>

EVENTO_UI_START

MenuOverlay::MenuOverlay(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : BasicView(bridge)
    , GlobalAgent(uiEntry) {}

void MenuOverlay::onCreate() {
    auto& self = *this;
    self->on_open_link_web([] { openBrowser("https://link.sast.fun/home"); });
}

void MenuOverlay::onShow() {
    auto& self = *this;
    executor()->asyncExecute([]() -> Task<void> { co_return; },
                             [&]() { self->set_is_show(true); },
                             std::chrono::milliseconds(200),
                             AsyncExecutor::Once | AsyncExecutor::Delay);
}

void MenuOverlay::onLogin() {
    auto& self = *this;
    auto userInfo = bridge.getAccountManager().getUserInfo();
    self->set_user_name(slint::SharedString(userInfo.nickname));
    self->set_user_signature(
        slint::SharedString(userInfo.biography.value_or("这个人很神秘，什么也没留下")));
    if (userInfo.avatar.has_value())
        self->set_user_avatar(slint::Image::load_from_path(slint::SharedString(*userInfo.avatar)));
}

EVENTO_UI_END