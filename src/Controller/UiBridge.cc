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