#include "Core/UiUtility.h"
#include <Controller/Core/ViewManager.h>
#include <Controller/UiBridge.h>
#include <Controller/View/AboutPage.h>
#include <Controller/View/DetailPage.h>
#include <Controller/View/DiscoveryPage.h>
#include <Controller/View/HistoryPage.h>
#include <Controller/View/LoginOverlay.h>
#include <Controller/View/MenuOverlay.h>
#include <Controller/View/MyEventPage.h>
#include <Controller/View/SearchPage.h>
#include <Controller/View/SettingPage.h>
#include <memory>
#include <spdlog/spdlog.h>

EVENTO_UI_START

UiBridge::UiBridge(slint::ComponentHandle<UiEntryName> uiEntry)
    : uiEntry(uiEntry)
    , GlobalAgent(uiEntry)
    , viewManager(std::make_shared<ViewManager>(uiEntry, *this)) {
    attachAllViews();

    uiEntry->window().on_close_requested([this] {
        onExitEventLoop();
        return slint::CloseRequestResponse::HideWindow;
    });
    slint::invoke_from_event_loop([this] { return onEnterEventLoop(); });

    viewManager->initStack(ViewName::DiscoveryPage);
    // if (!accountManager.isLogin()) { // TODO: wait account manager
    viewManager->initStack(ViewName::LoginOverlay);
    // }
}

void UiBridge::attachView(ViewName name, std::shared_ptr<BasicView> object) {
    views.emplace(name, std::move(object));
}

ViewManager& UiBridge::getViewManager() {
    return *viewManager;
}

// AccountManager& UiBridge::getAccountManager() { // TODO: wait account manager
//     return accountManager;
// }

slint::ComponentHandle<UiEntryName> UiBridge::getUiEntry() {
    return uiEntry;
}

void UiBridge::show() {
    uiEntry->show();
}

void UiBridge::run() {
    UiUtility::StylishLog::viewActionTriggered(logOrigin, "onCreate");
    call(actions::onCreate);

    show();
    spdlog::debug("--- enter slint event loop ---");
    eventLoopRunning = true;
    slint::run_event_loop();
    eventLoopRunning = false;
    spdlog::debug("--- exit slint event loop ---");
    hide();

    UiUtility::StylishLog::viewActionTriggered(logOrigin, "onDestroy");
    call(actions::onDestroy);
}

void UiBridge::hide() {
    uiEntry->hide();
}

void UiBridge::exit() {
    if (eventLoopRunning) {
        UiUtility::StylishLog::viewActionTriggered(logOrigin, "onStop");
        slint::invoke_from_event_loop([&self = *this] { self.call(actions::onStop); });
        slint::quit_event_loop();
    }
}

bool UiBridge::inEventLoop() {
    return eventLoopRunning;
}

void UiBridge::call(Action& action) {
    std::for_each(views.begin(),
                  views.end(),
                  [&action](const std::pair<const ViewName, std::shared_ptr<BasicView>>& view) {
                      action(*view.second);
                  });
}

void UiBridge::call(Action& action, ViewName target) {
    action(*views.at(target));
}

void UiBridge::attachAllViews() {
    attachView(ViewName::DiscoveryPage, std::make_shared<DiscoveryPage>(uiEntry, *this));
    attachView(ViewName::SearchPage, std::make_shared<SearchPage>(uiEntry, *this));
    attachView(ViewName::HistoryPage, std::make_shared<HistoryPage>(uiEntry, *this));
    attachView(ViewName::MyEventPage, std::make_shared<MyEventPage>(uiEntry, *this));
    attachView(ViewName::DetailPage, std::make_shared<DetailPage>(uiEntry, *this));
    attachView(ViewName::AboutPage, std::make_shared<AboutPage>(uiEntry, *this));
    attachView(ViewName::SettingPage, std::make_shared<SettingPage>(uiEntry, *this));
    attachView(ViewName::LoginOverlay, std::make_shared<LoginOverlay>(uiEntry, *this));
    attachView(ViewName::MenuOverlay, std::make_shared<MenuOverlay>(uiEntry, *this));
}

void UiBridge::onEnterEventLoop() {
    auto& self = *this;

    UiUtility::StylishLog::viewActionTriggered(logOrigin, "onStart");
    self.call(actions::onStart);

    viewManager->onEnterEventLoop();
}

void UiBridge::onExitEventLoop() {
    auto& self = *this;

    viewManager->onExitEventLoop();

    UiUtility::StylishLog::viewActionTriggered(logOrigin, "onStop");
    call(actions::onStop);
}

EVENTO_UI_END