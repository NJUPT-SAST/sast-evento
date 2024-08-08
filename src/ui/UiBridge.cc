#include <memory>
#include <ui/UiBridge.h>
#include <ui/View/AboutPage.h>
#include <ui/View/DetailPage.h>
#include <ui/View/DiscoveryPage.h>
#include <ui/View/HistoryPage.h>
#include <ui/View/LoginOverlay.h>
#include <ui/View/MenuOverlay.h>
#include <ui/View/MyEventPage.h>
#include <ui/View/SearchPage.h>
#include <ui/View/SettingPage.h>

EVENTO_UI_START

UiBridge::UiBridge(slint::ComponentHandle<UiEntryName>& uiEntry)
    : manager(uiEntry) {
    manager.attach(ViewName::DiscoveryPage, std::make_unique<DiscoveryPage>(uiEntry));
    manager.attach(ViewName::SearchPage, std::make_unique<SearchPage>(uiEntry));
    manager.attach(ViewName::HistoryPage, std::make_unique<HistoryPage>(uiEntry));
    manager.attach(ViewName::MyEventPage, std::make_unique<MyEventPage>(uiEntry));
    manager.attach(ViewName::DetailPage, std::make_unique<DetailPage>(uiEntry));
    manager.attach(ViewName::AboutPage, std::make_unique<AboutPage>(uiEntry));
    manager.attach(ViewName::SettingPage, std::make_unique<SettingPage>(uiEntry));
    manager.attach(ViewName::LoginOverlay, std::make_unique<LoginOverlay>(uiEntry));
    manager.attach(ViewName::MenuOverlay, std::make_unique<MenuOverlay>(uiEntry));

    manager.pushView(ViewName::DiscoveryPage);
    manager.pushView(ViewName::LoginOverlay);
}

ViewManager* UiBridge::operator->() {
    return &manager;
}

EVENTO_UI_END