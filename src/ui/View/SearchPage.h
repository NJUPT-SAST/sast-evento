#pragma once

#include <ui/Core/BasicView.h>
#include <ui/Core/GlobalAgent.hpp>
#include <ui/Core/UiBase.h>

EVENTO_UI_START

class SearchPage : public BasicView, private GlobalAgent<SearchPageBridge> {
public:
    SearchPage(slint::ComponentHandle<UiEntryName> uiEntry);
    SearchPage(SearchPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END