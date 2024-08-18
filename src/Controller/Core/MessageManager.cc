#include "GlobalAgent.hh"
#include "UiUtility.h"
#include <Controller/Core/MessageManager.h>
#include <Controller/Core/UiBase.h>
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
        spdlog::debug("pre new message");
        showMessage(std::string(content), type);
    });
}

void MessageManager::showMessage(std::string content,
                                 MessageType type,
                                 std::chrono::milliseconds timeout) {
    auto& self = *this;
    UiUtility::StylishLog::newMessageShowed(logOrigin, content);
    spdlog::debug("new message");

    self->set_type(type);
    self->set_content(std::string_view(content));
    self->set_timeout(timeout.count());
    self->set_visible(true);
    // TODO: find a better way to delay
    std::thread t([&self] {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        slint::invoke_from_event_loop([&self] { self->set_visible(false); });
    });
    t.detach();
}

EVENTO_UI_END