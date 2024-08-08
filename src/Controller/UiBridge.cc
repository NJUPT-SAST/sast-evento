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
    manager.attach(ViewName::DiscoveryPage, std::make_shared<DiscoveryPage>(uiEntry));
    manager.attach(ViewName::SearchPage, std::make_shared<SearchPage>(uiEntry));
    manager.attach(ViewName::HistoryPage, std::make_shared<HistoryPage>(uiEntry));
    manager.attach(ViewName::MyEventPage, std::make_shared<MyEventPage>(uiEntry));
    manager.attach(ViewName::DetailPage, std::make_shared<DetailPage>(uiEntry));
    manager.attach(ViewName::AboutPage, std::make_shared<AboutPage>(uiEntry));
    manager.attach(ViewName::SettingPage, std::make_shared<SettingPage>(uiEntry));
    manager.attach(ViewName::LoginOverlay, std::make_shared<LoginOverlay>(uiEntry));
    manager.attach(ViewName::MenuOverlay, std::make_shared<MenuOverlay>(uiEntry));

    manager.pushView(ViewName::DiscoveryPage);
    manager.pushView(ViewName::LoginOverlay);
}

ViewManager* UiBridge::operator->() {
    return &manager;
}

EVENTO_UI_END