#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>
//#include <Controller/Core/ViewManager.h>
#include <string.h>
#include <slint.h>

EVENTO_UI_START

class SearchPage : public BasicView, private GlobalAgent<SearchPageBridge> {
public:
    SearchPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    SearchPage(SearchPage&) = delete;

private:
    void onCreate() override;
    void cd_detail_page();
    void Searchtext(std::string_view& v);
    void clickFilterDivision(bool T);
};

EVENTO_UI_END
