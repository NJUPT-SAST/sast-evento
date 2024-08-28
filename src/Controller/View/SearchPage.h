#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class SearchPage : public BasicView, private GlobalAgent<SearchPageBridge> {
public:
    SearchPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    SearchPage(SearchPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END
