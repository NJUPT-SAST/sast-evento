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

    bool isMessageShow = false;
    const static inline auto animationLength = std::chrono::milliseconds(200);

public:
    MessageManager(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    MessageManager(MessageManager&) = delete;

    void showMessage(std::string content,
                     MessageType type = MessageType::Info,
                     std::chrono::steady_clock::duration timeout = std::chrono::milliseconds(3000));

    // unnecessary to invoke if set correct timeout
    void hideMessage();

private:
    static auto inline doNothing = []() -> Task<void> { co_return; };
};

EVENTO_UI_END