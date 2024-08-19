#pragma once

#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
#include <chrono>
#include <string>

EVENTO_UI_START

class MessageManager : private GlobalAgent<MessageManagerBridge> {
    friend class UiBridge;
    UiBridge& bridge;
    std::string logOrigin = "MessageManager";

public:
    MessageManager(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    MessageManager(MessageManager&) = delete;

    void showMessage(std::string content,
                     MessageType type = MessageType::Info,
                     std::chrono::milliseconds timeout = std::chrono::milliseconds(3000));
};

EVENTO_UI_END