#pragma once

#include <Controller/Core/BasicView.h>
#include <Controller/Core/GlobalAgent.hh>
#include <Controller/Core/UiBase.h>

EVENTO_UI_START

class HistoryPage : public BasicView, private GlobalAgent<HistoryPageBridge> {
public:
    HistoryPage(slint::ComponentHandle<UiEntryName> uiEntry, UiBridge& bridge);
    HistoryPage(HistoryPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END