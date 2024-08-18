#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class DetailPage : public BasicView, private GlobalAgent<DetailPageBridge> {
public:
    DetailPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    DetailPage(DetailPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END