#include <Controller/AsyncExecutor.hh>
#include <Controller/Core/MessageManager.h>
#include <Controller/Core/UiBase.h>
#include <Controller/Core/UiUtility.h>
#include <chrono>
#include <memory>
#include <spdlog/spdlog.h>

EVENTO_UI_START

MessageManager::MessageManager(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge)
    : GlobalAgent(uiEntry)
    , bridge(bridge) {
    auto& self = *this;

    toastList = std::make_shared<slint::VectorModel<ToastData>>();

    self->on_show_message([this](slint::SharedString content, MessageType type) {
        return showMessage(std::string(content), type);
    });

    self->on_get_message([this](int id) -> MessageData { return getMessage(id); });

    self->on_hide_message([this](int id) { hideMessage(id); });

    self->set_toast_list(toastList);
}

int MessageManager::showMessage(std::string content,
                                MessageType type,
                                std::chrono::steady_clock::duration timeout) {
    auto& self = *this;
    auto id = nextId++;
    auto messageDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timeout
                                                                                 + animationLength);

    UiUtility::StylishLog::general(logOrigin,
                                   std::format("new message [{}] content = \"{}\"", id, content));
    newToast(id, {.content = slint::SharedString(content), .type = type});

    // set auto hide
    slint::Timer::single_shot(messageDuration, [this, id] { hideMessage(id); });

    return id;
}

void MessageManager::hideMessage(int id) {
    if (auto index = getIndex(id); index != -1 && !toastList->row_data(getIndex(id))->removed) {
        hideToast(id);
    } else {
        UiUtility::StylishLog::messageOperation(
            logOrigin, id, "scheduled hide cancelled: already hidden or deleted");
    }
}

int MessageManager::getIndex(int id) {
    for (int i = 0; i < toastList->row_count(); i++) {
        if (toastList->row_data(i)->id == id) {
            return i;
        }
    }
    return -1;
}

void MessageManager::newToast(int id, MessageData data) {
    // prepare data then instantiate a toast
    messageData.insert({id, data});
    toastList->push_back({id, 0});
    UiUtility::StylishLog::messageOperation(logOrigin, id, "instantiate toast, data added");

    // make animation available
    using namespace std::chrono_literals;
    slint::Timer::single_shot(1ms, [this, id] { showToast(id); });
}

void MessageManager::showToast(int id) {
    operateToastData(id, increaseElevation);
    UiUtility::StylishLog::messageOperation(logOrigin, id, "show");

    // correct existing toast elevation
    for (int i = 0; i < toastList->row_count(); i++) {
        if (i != getIndex(id)) {
            operateToastData(toastList->row_data(i)->id, increaseElevation);
        }
    }
}

void MessageManager::hideToast(int id) {
    operateToastData(id, markRemoved);
    UiUtility::StylishLog::messageOperation(logOrigin, id, "hide");

    // wait animation finished
    slint::Timer::single_shot(animationLength, [this, id] { deleteToast(id); });
}

void MessageManager::deleteToast(int id) {
    // correct existing toast elevation bigger than given id
    int removedElevation = toastList->row_data(getIndex(id))->elevation;
    for (int i = 0; i < toastList->row_count(); i++) {
        auto toastData = toastList->row_data(i);
        if (toastData->elevation > removedElevation) {
            operateToastData(toastData->id, decreaseElevation);
        }
    }

    // delete instance and data
    toastList->erase(getIndex(id));
    messageData.erase(id);
    UiUtility::StylishLog::general(logOrigin, std::format("delete message [{}]", id));
}

void MessageManager::operateToastData(int id, std::function<ToastData(ToastData)> operation) {
    auto index = getIndex(id);
    auto data = toastList->row_data(index).value();
    toastList->set_row_data(index, operation(data));
}

MessageData MessageManager::getMessage(int id) {
    auto& self = *this;

    return messageData.at(id);
}

EVENTO_UI_END