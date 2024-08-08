#pragma once

#include <ui/Core/BasicView.h>
#include <ui/Core/GlobalAgent.hpp>
#include <ui/Core/UiBase.h>

EVENTO_UI_START

class HistoryPage : public BasicView, private GlobalAgent<HistoryPageBridge> {
public:
    HistoryPage(slint::ComponentHandle<UiEntryName> uiEntry);
    HistoryPage(HistoryPage&) = delete;

private:
    void onCreate() override;
};

EVENTO_UI_END