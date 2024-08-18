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
        static void viewActionTriggered(std::string origin,
                                        std::string actionName,
                                        std::string viewName = std::string("All-View"));
        static void viewVisibilityChanged(std::string origin,
                                          std::string actionName,
                                          std::string viewName);
        static void newMessageShowed(std::string origin, std::string content);
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