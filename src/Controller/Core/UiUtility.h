#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class UiUtility {
public:
    static std::string getViewName(ViewName target);
    static bool isTransparent(ViewName target);

    class StylishLog {
    public:
        static void general(std::string origin, std::string content) {
            spdlog::debug("{}: {}", origin, content);
        }
        static void viewActionTriggered(std::string origin,
                                        std::string actionName,
                                        std::string viewName = std::string("All-View")) {
            spdlog::debug("{}: {}: triggered {}", origin, viewName, actionName);
        }
        static void viewVisibilityChanged(std::string origin,
                                          std::string actionName,
                                          std::string viewName) {
            spdlog::debug("{}: {} visibility changed: {}", origin, viewName, actionName);
        }
        static void messageOperation(std::string origin, int id, std::string content) {
            spdlog::debug("{}: message [{}]: {}", origin, id, content);
        }
    };

private:
    static inline const std::unordered_map<ViewName, std::string> viewNameMapper{
        {ViewName::DiscoveryPage, "DiscoveryPage"},
        {ViewName::SearchPage, "SearchPage"},
        {ViewName::HistoryPage, "HistoryPage"},
        {ViewName::MyEventPage, "MyEventPage"},
        {ViewName::DetailPage, "DetailPage"},
        {ViewName::AboutPage, "AboutPage"},
        {ViewName::SettingPage, "SettingPage"},
        {ViewName::LoginOverlay, "LoginOverlay"},
        {ViewName::MenuOverlay, "MenuOverlay"},
    };
};

EVENTO_UI_END