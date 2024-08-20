#include <Controller/Core/UiUtility.h>
#include <set>
#include <spdlog/spdlog.h>

EVENTO_UI_START

std::string UiUtility::getViewName(ViewName target) {
    return viewNameMapper.find(target) == viewNameMapper.end() ? "[Unknown View]"
                                                               : viewNameMapper.at(target);
}

bool UiUtility::isTransparent(ViewName target) {
    static const std::set<ViewName> overlayList{
        ViewName::MenuOverlay,
        // no longer transparent (no guest login)
        // ViewName::LoginOverlay,
    };
    return overlayList.find(target) != overlayList.end();
}

void UiUtility::StylishLog::viewActionTriggered(std::string origin,
                                                std::string actionName,
                                                std::string viewName) {
    spdlog::debug("{}: {}: triggered {}", origin, viewName, actionName);
}

void UiUtility::StylishLog::viewVisibilityChanged(std::string origin,
                                                  std::string actionName,
                                                  std::string viewName) {
    spdlog::debug("{}: {} visibility changed: {}", origin, viewName, actionName);
}
void UiUtility::StylishLog::newMessageShowed(std::string origin, std::string content) {
    spdlog::debug("{}: new message: {}", origin, content);
}

EVENTO_UI_END