#include "Controller/AsyncExecutor.hh"
#include <Controller/Core/MessageManager.h>
#include <Controller/Core/UiBase.h>
#include <Controller/Core/UiUtility.h>
#include <chrono>
#include <slint.h>
#include <spdlog/spdlog.h>
#include <string_view>
#include <thread>

EVENTO_UI_START

MessageManager::MessageManager(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : GlobalAgent(uiEntry)
    , bridge(bridge) {
    auto& self = *this;

    self->on_show_message([this](slint::SharedString content, MessageType type) {
        showMessage(std::string(content), type);
    });
}

void MessageManager::showMessage(std::string content,
                                 MessageType type,
                                 std::chrono::steady_clock::duration timeout) {
    auto& self = *this;

    if (isMessageShow) {
        hideMessage();
        evento::executor()->asyncExecute(
            doNothing,
            [&, this] { showMessage(content, type, timeout); },
            animationLength,
            AsyncExecutor::Once | AsyncExecutor::Delay);
    }

    UiUtility::StylishLog::newMessageShowed(logOrigin, content);
    isMessageShow = true;
    self->set_type(type);
    self->set_content(std::string_view(content));
    self->set_visible(true);
    evento::executor()->asyncExecute(
        doNothing,
        [this] { hideMessage(); },
        timeout + animationLength,
        AsyncExecutor::Once | AsyncExecutor::Delay);
}

void MessageManager::hideMessage() {
    auto& self = *this;

    isMessageShow = false;
    self->set_visible(false);
}

EVENTO_UI_END