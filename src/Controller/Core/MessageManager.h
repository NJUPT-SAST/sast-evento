#pragma once

#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
#include <chrono>
#include <functional>
#include <map>
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

    // @return message id
    int showMessage(std::string content,
                    MessageType type = MessageType::Info,
                    std::chrono::steady_clock::duration timeout = std::chrono::milliseconds(3000));

    // unnecessary to invoke if set correct timeout
    void hideMessage(int id);

private:
    // id, int
    int nextId = 0;

    // sync with slint, every line stand for a toast instance
    std::shared_ptr<slint::VectorModel<ToastData>> toastList;
    // every toast instance will read data from here, mapping id to message data
    std::map<int, MessageData> messageData;
    MessageData getMessage(int id);
    int getIndex(int id);

    /**
     * toast life cycle:
     * - called newToast()
     *   - new instance, invisible (elevation=0)
     * - called showToast()
     *   - visible
     * - called hideToast()
     *   - invisible (removed) (exist in list)
     * - called deleteToast()
     */
    void newToast(int id, MessageData data);
    void showToast(int id);
    void hideToast(int id);
    void deleteToast(int id);

    void operateToastData(int id, std::function<ToastData(ToastData)> operation);
    static inline auto increaseElevation = [](ToastData data) -> ToastData {
        data.elevation += 1;
        return data;
    };
    static inline auto decreaseElevation = [](ToastData data) -> ToastData {
        data.elevation -= 1;
        return data;
    };
    static inline auto markRemoved = [](ToastData data) -> ToastData {
        data.removed = true;
        return data;
    };

    const static inline auto animationLength = std::chrono::milliseconds(200);
    static auto inline doNothing = []() -> Task<void> { co_return; };
};

EVENTO_UI_END