#include <Controller/AsyncExecutor.hh>
#include <Controller/Core/AccountManager.h>
#include <Controller/UiBridge.h>
#include <Controller/View/MenuOverlay.h>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <Infrastructure/Utils/Tools.h>
#include <chrono>

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
    self.refreshUserInfo(userInfo);
    executor()->asyncExecute(
        []() -> Task<Result<UserInfoEntity>> {
            auto res = co_await networkClient()->getUserInfo();
            co_return res;
        },
        [&self = *this](Result<UserInfoEntity> result) {
            if (result.isErr()) {
                spdlog::error("Failed to get user info: {}", result.unwrapErr().what());
                return;
            }
            self.refreshUserInfo(result.unwrap());
        },
        10min,
        AsyncExecutor::Periodic | AsyncExecutor::Delay);
}

void MenuOverlay::refreshUserInfo(UserInfoEntity const& userInfo) {
    auto& self = *this;
    self->set_user_name(slint::SharedString(userInfo.nickname));
    self->set_user_signature(
        slint::SharedString(userInfo.biography.value_or("这个人很神秘，什么也没留下 ")));
    if (userInfo.avatar.has_value())
        executor()->asyncExecute(networkClient()->getFile(*userInfo.avatar),
                                 [&self = *this](Result<std::filesystem::path> result) {
                                     if (result.isErr()) {
                                         spdlog::error("Failed to get user avatar: {}",
                                                       result.unwrapErr().what());
                                         return;
                                     }
                                     self->set_user_avatar(slint::Image::load_from_path(
                                         slint::SharedString(result.unwrap().string().c_str())));
                                 });
}

EVENTO_UI_END