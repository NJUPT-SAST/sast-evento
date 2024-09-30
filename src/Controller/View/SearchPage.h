#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>

EVENTO_UI_START

class SearchPage : public BasicView, private GlobalAgent<SearchPageBridge> {
public:
    SearchPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);

private:
    void onCreate() override;

    void onShow() override;

    void loadDepartmentList();

    void loadDepartmentEvents(int page, int departmentIdx);

    void search(std::string const& keyword);

    std::vector<slint::SharedString> departments;
};

EVENTO_UI_END
