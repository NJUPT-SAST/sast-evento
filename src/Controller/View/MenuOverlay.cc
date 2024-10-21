#include <Controller/AsyncExecutor.hh>
#include <Controller/Core/AccountManager.h>
#include <Controller/UiBridge.h>
#include <Controller/View/MenuOverlay.h>
#include <Infrastructure/Network/NetworkClient.h>
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
    executor()->asyncExecute(loadUserInfoTask(), [&self = *this](Result<slint::Image> result) {
        if (result.isErr()) {
            spdlog::error("Failed to load user info: {}", result.unwrapErr().what());
            if (result.unwrapErr().kind != Error::Data) {
                self.bridge.getMessageManager().showMessage(result.unwrapErr().what(),
                                                            MessageType::Error);
            }
            return;
        }
    });
}

void MenuOverlay::onLogin() {
    refreshUserInfo(bridge.getAccountManager().userInfo());
}

Task<Result<slint::Image>> MenuOverlay::loadUserInfoTask() {
    auto result = co_await networkClient()->getUserInfo();
    if (result.isErr()) {
        co_return Err(result.unwrapErr());
    }
    auto userInfo = result.unwrap();
    if (userInfo.avatar.has_value()) {
        auto avatar = co_await networkClient()->getFile(*userInfo.avatar);
        if (avatar.isErr()) {
            co_return Err(avatar.unwrapErr());
        }
        co_return Ok(slint::Image::load_from_path(slint::SharedString(avatar.unwrap().u8string())));
    }
    slint::blocking_invoke_from_event_loop([&] { refreshUserInfo(userInfo); });
    bridge.getAccountManager().userInfo() = std::move(userInfo);
    co_return Err(Error(Error::Data, "User avatar not found"));
}

void MenuOverlay::refreshUserInfo(UserInfoEntity const& userInfo) {
    auto& self = *this;
    self->set_user_name(slint::SharedString(userInfo.nickname));
    self->set_user_signature(
        slint::SharedString(userInfo.bio.value_or("这个人很神秘，什么也没留下 ")));
    if (userInfo.avatar.has_value())
        executor()->asyncExecute(networkClient()->getFile(*userInfo.avatar),
                                 [&self = *this](Result<std::filesystem::path> result) {
                                     if (result.isErr()) {
                                         spdlog::error("Failed to get user avatar: {}",
                                                       result.unwrapErr().what());
                                         return;
                                     }
                                     self->set_user_avatar(slint::Image::load_from_path(
                                         slint::SharedString(result.unwrap().u8string())));
                                 });
}

EVENTO_UI_END