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

EVENTO_UI_END